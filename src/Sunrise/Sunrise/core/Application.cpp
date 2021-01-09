#include "srpch.h"
#include "Application.h"
#include "../scene/Scene.h"

#include "../configuration/ConfigSystem.h"
#include "Window.h"
#include "../graphics/vulkan/renderer/Renderer.h"

namespace sunrise {

    using namespace gfx;

	Application::Application(Scene* initialScene)
		: loadedScenes({initialScene})
	{

	}

	Application::~Application()
	{
	}

	void Application::startup()
	{
		PROFILE_FUNCTION;

        {// configure main threaqd priority
            SR_CORE_TRACE("Initializing Proccess Priority");

            auto nativeHandle = GetCurrentThread();
            //SetThreadPriority(nativeHandle, THREAD_PRIORITY_TIME_CRITICAL);
            auto nativeProessHandle = GetCurrentProcess();
            SetPriorityClass(nativeProessHandle, HIGH_PRIORITY_CLASS);
            auto r = GetPriorityClass(nativeProessHandle);
            //printf("d"); 
            //SetPriorityClass()
        }


        {// Configure Marl
            SR_CORE_TRACE("Initializing Marl");

            PROFILE_SCOPE("Configutre Marl");
            auto confic = marl::Scheduler::Config();

            confic.setWorkerThreadCount(std::thread::hardware_concurrency() - 1);

            scheduler = new marl::Scheduler(confic);

            scheduler->bind();
        }

        SR_CORE_TRACE("Initializing Configuration system");
        configSystem.readFromDisk();
        {
            SR_CORE_TRACE("Initializing GLFW");

            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        }
        SR_CORE_TRACE("Initializing VK Instance");
        createInstance();


        auto cfgWindows = configSystem.global().windows;

        SR_CORE_INFO("Creating {} windows",cfgWindows.size());

        // create windows
        for (size_t i = 0; i < cfgWindows.size(); i++)
        {
            SR_CORE_TRACE("Creating Window {}",i);
            PROFILE_SCOPE("create window");

            auto window = new Window(this, i);

            window->createWindowAndSurface();

            window->camera.fov = 60;
            window->camera.zNear = 0.1;
            window->camera.zFar = 100'000;//1'000'000;

            windows.push_back(window);


            auto deviceIndex = createDevice(windows.size() - 1);

            createRenderer(deviceIndex);

            window->device = devices[deviceIndex];
            window->renderer = renderers[deviceIndex];
            window->finishInit();

            if (deviceIndex == devices.size() - 1 && deviceIndex != 0) {
                window->indexInRenderer = renderers[deviceIndex]->windows.size();
                renderers[deviceIndex]->windows.push_back(window);

                renderers[deviceIndex]->camFrustroms.emplace_back(windows[i]->camera.view());
            }


            if (window->swapChainImages.size() > maxSwapChainImages) {
                maxSwapChainImages = window->swapChainImages.size();
            }
        }

        for (size_t i = 0; i < renderers.size(); i++) {
            SR_CORE_TRACE("Creating Renderer Resources for renderer {}", i);
            renderers[i]->createAllResources();
        }



        SR_CORE_TRACE("Initializing Scene");
        loadedScenes[0]->load();

        SR_CORE_INFO("Initialization Complete!");
	}

	void Application::run()
	{
		PROFILE_FUNCTION;

        SR_CORE_INFO("Running");
        runLoop();
	}

	void Application::shutdown()
	{
		PROFILE_FUNCTION;

	}

	void Application::loadScene(Scene* scene, void* animationProperties)
	{

	}


    void Application::runLoop()
    {

        //worldScene->updateScene();

        while (shouldLoop()) {
            runLoopIteration();
        }

        windows[0]->device.waitIdle();
    }

    bool Application::shouldLoop() {
        PROFILE_FUNCTION
            return !glfwWindowShouldClose(windows[0]->window);
    }

    void Application::runLoopIteration()
    {
        PROFILE_FUNCTION_LEVEL2;
        {
#if SR_PROFILING
            std::string s = "loooooooooooooong string";
#endif
            PROFILE_SCOPE("glfwPollEvents");
            glfwPollEvents();
        }
        //return;
        //drawView();

        // update scene
        loadedScenes[0]->update();

        renderers[0]->beforeRenderScene();

        // draw view - this should be able to be done all in parallel
        for (size_t i = 0; i < windows.size(); i++)
        {
            PROFILE_SCOPE("Loop of Window render loop");
            if (windows[i]->getDrawable() == true) continue;
            renderers[0]->renderFrame(*windows[i]);
            windows[i]->presentDrawable();
        }

        currentFrameID++;
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }






    void Application::createInstance()
    {
        PROFILE_FUNCTION
            auto appInfo = vk::ApplicationInfo(
                "Flight Sim Terrain System",
                VK_MAKE_VERSION(1, 0, 0),
                "Sunrise",
                VK_MAKE_VERSION(1, 0, 0),
                VK_API_VERSION_1_2
            );



        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;


        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        const char* description;
        int code = glfwGetError(&description);

        if (description)
            SR_CORE_ERROR(description);

        vk::InstanceCreateInfo info = vk::InstanceCreateInfo();

#if RDX_ENABLE_VK_VALIDATION_LAYERS
        SR_CORE_INFO("Vulkan Validation Enabled");
#else
        SR_CORE_WARN("Vulkan Validation Disabled");
#endif

        info.pApplicationInfo = &appInfo;
        info.enabledLayerCount = validationLayers.size();
        info.ppEnabledLayerNames = validationLayers.data();
        info.enabledExtensionCount = glfwExtensionCount;
        info.ppEnabledExtensionNames = glfwExtensions;


        instance = vk::createInstance(info);
    }


    void Application::createAllocator(int deviceIndex)
    {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
        allocatorInfo.physicalDevice = physicalDevices[deviceIndex];
        allocatorInfo.device = devices[deviceIndex];
        allocatorInfo.instance = instance;

        VmaAllocator allocator;
        vmaCreateAllocator(&allocatorInfo, &allocator);

        allocators.push_back(allocator);
    }

    void Application::createRenderer(int deviceIndex)
    {
        SR_CORE_TRACE("Creating Renderer for device {}", deviceIndex);
        if (deviceIndex < renderers.size()) return;
        auto index = devices.size() - 1;

        // TODO: fix this because all windows are being added to all devices
        auto renderer =
            new Renderer(*this, devices[index], physicalDevices[index], allocators[index], windows, deviceQueues[index], queueFamilyIndices[index]);


        renderers.push_back(renderer);

    }

    int Application::createDevice(int window)
    {
        SR_CORE_TRACE("Creating device for window {}",window);

        PROFILE_FUNCTION;

        auto physicalDevice = GPUSelector::primaryGPU(instance, windows[window]->surface);

        auto index = std::find(physicalDevices.begin(), physicalDevices.end(), physicalDevice);

        if (index != physicalDevices.end()) {
            return index - physicalDevices.begin();
        }

        SR_CORE_TRACE("goint to get queue family indicies");
        auto loc_queueFamilyIndices = GPUSelector::gpuQueueFamilies(physicalDevice, windows[window]->surface);

        const float queuePriority1 = 1.0f;
        vk::DeviceQueueCreateInfo gfxQueueCreateInfo({}, loc_queueFamilyIndices.graphicsFamily.value(), 1);
        gfxQueueCreateInfo.pQueuePriorities = &queuePriority1;
        vk::DeviceQueueCreateInfo transferQueueCreateInfo({}, loc_queueFamilyIndices.resourceTransferFamily.value(), 1);
        transferQueueCreateInfo.pQueuePriorities = &queuePriority1;


        std::array<VkDeviceQueueCreateInfo, 2> queueCreateInfos = { VkDeviceQueueCreateInfo(gfxQueueCreateInfo), VkDeviceQueueCreateInfo(transferQueueCreateInfo) };

        //TODO: add proper way of requesting and falling back on device features
        //vk::PhysicalDeviceFeatures deviceFeatures();
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkPhysicalDeviceDescriptorIndexingFeatures desIndexingFeatures{};
        desIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        desIndexingFeatures.pNext = nullptr;

        desIndexingFeatures.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
        // not suported by gtx 1080 ti
        //desIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
        desIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
        desIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;

        desIndexingFeatures.runtimeDescriptorArray = VK_TRUE;

        // devie extensions
        const std::vector<const char*> extensionNames = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if !SR_RenderDocCompatible
            VK_NV_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME
#endif
        };

        SR_CORE_INFO("creating logical device with the following extensions: ");
        
        for (auto& c : extensionNames) {
            SR_CORE_INFO("{}", c);
        }

        vk::DeviceCreateFlags flags();

        //vk::DeviceCreateInfo info(flags, 1, &queueCreateInfo, validationLayers.size(), validationLayers.data(), 1 ,extensionNames, &deviceFeatures);

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = queueCreateInfos.size();
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledLayerCount = validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayers.data();

        createInfo.enabledExtensionCount = extensionNames.size();
        createInfo.ppEnabledExtensionNames = extensionNames.data();

        // features
        createInfo.pNext = &desIndexingFeatures;

        SR_CORE_TRACE("goint to create logical device");
        // error is on this line below
        
        auto device = physicalDevice.createDevice(vk::DeviceCreateInfo(createInfo), nullptr);
       

        GPUQueues loc_deviceQueues;

        device.getQueue(loc_queueFamilyIndices.graphicsFamily.value(), 0, &loc_deviceQueues.graphics);
        device.getQueue(loc_queueFamilyIndices.presentFamily.value(), 0, &loc_deviceQueues.presentation);
        device.getQueue(loc_queueFamilyIndices.resourceTransferFamily.value(), 0, &loc_deviceQueues.resourceTransfer);

        devices.push_back(device);
        physicalDevices.push_back(physicalDevice);
        deviceQueues.push_back(loc_deviceQueues);
        queueFamilyIndices.push_back(loc_queueFamilyIndices);

        createAllocator(devices.size() - 1);
        

        //TODO put this in a better place - fix for multi gpu - put this as a member of renderer so that each gpu has its own resource transferer

        SR_CORE_TRACE("retuning after succesfully creasting a logical device for window {}", window);
        return devices.size() - 1;
    }











    //  NO____APPLICTION


    NO_APPLICATION::NO_APPLICATION()
        : Application(nullptr)
    {

    }

    NO_APPLICATION::~NO_APPLICATION()
    {
    }

    void NO_APPLICATION::startup()
    {

    }

    void NO_APPLICATION::shutdown()
    {
    }

    const char* NO_APPLICATION::getName()
    {
        return nullptr;
    }

}