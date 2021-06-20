#include "srpch.h"
#include "Application.h"
#include "../scene/Scene.h"

#include "../configuration/ConfigSystem.h"
#include "../fileSystem/FileManager.h"
#include "../fileSystem/FileSystem.h"
#include "Window.h"
#include "../graphics/vulkan/renderer/Renderer.h"
#include "../graphics/vulkan/renderer/SceneRenderCoordinator.h"

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

            PROFILE_SCOPE("Configure Marl");
            auto confic = marl::Scheduler::Config();

            confic.setWorkerThreadCount(std::thread::hardware_concurrency() - 1);

            scheduler = new marl::Scheduler(confic);

            scheduler->bind();
        }

        {
            SR_CORE_TRACE("Initializing File System");

            FileSystem::initilize();

        }

        if (!wantsWindows()) return;

        {
            SR_CORE_TRACE("Initializing GLFW");

            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        }

   
        SR_CORE_TRACE("Initializing VK Instance");
        createInstance();
    


        SR_CORE_TRACE("Initializing Configuration system");
        configSystem.readFromDisk();
        configSystem.writeHelpDoc();

        // setup debug callback registration for vulkan

#if SR_VALIDATION

        VkDebugReportCallbackCreateInfoEXT debugInfo;

        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debugInfo.flags =
            VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
            VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
            VK_DEBUG_REPORT_ERROR_BIT_EXT; // | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
        debugInfo.pfnCallback = debugCallbackFunc;
        debugInfo.pNext = nullptr;
        debugInfo.pUserData = nullptr;

        vk::DynamicLoader dl;

        // This dispatch class will fetch function pointers for the passed device if possible, else for the passed instance

        auto dldid = vk::DispatchLoaderDynamic(instance, dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));


        auto loc_vkCreateDebugReportCallbackEXT =(PFN_vkCreateDebugReportCallbackEXT)dldid.vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
        
        loc_vkCreateDebugReportCallbackEXT(instance, &debugInfo,nullptr, &debugObject);
#endif

        createWindows();

        if (loadedScenes.size() > 0)
        for (size_t i = 0; i < renderers.size(); i++) {
            SR_CORE_TRACE("Creating Renderer Resources for renderer {}", i);
            renderers[i]->createAllResources();
        }



        SR_CORE_TRACE("Initializing Scene");

        Scene* firstScene = loadedScenes[0];
        loadScene(firstScene,nullptr);

        SR_CORE_INFO("Initialization Complete!");
	}

    bool Application::wantsWindows()
    {
        return true;
    }

    void Application::createWindows()
    {
        auto cfgWindows = configSystem.global().windows;


        SR_CORE_INFO("Creating {} windows", cfgWindows.size());

        


        // create all windows
        for (size_t i = 0; i < cfgWindows.size(); i++)
        {
            PROFILE_SCOPE("create window and if requred device+renderer");
            // create window
            SR_CORE_TRACE("Creating Window {}", i);

            auto window = new Window(this, i,i==0,false);

            window->createWindowAndSurface();

            window->camera.fov = 60;
            window->camera.zNear = 0.1;
            window->camera.zFar = 100'000;//1'000'000;

            windows.push_back(window);

            //

            // create device + renderer if required
            auto deviceIndex = createDevice(windows.size() - 1);

            auto existed = createRenderer(deviceIndex);

            window->device = devices[deviceIndex];
            window->renderer = renderers[deviceIndex];

            // if could be in window group hold of on full initilization for now
            if (!window->renderer->supportsMultiViewport)
                window->finishInit();

            //
            
            // add window to existing render 
            //TODOL this will need to be updated for multi gpu
            if (existed) {
                //window->indexInRenderer = renderers[deviceIndex]->windows.size();
                renderers[deviceIndex]->windows.push_back(window);

                //renderers[deviceIndex]->camFrustroms.emplace_back(windows[i]->camera.view());
            }
        }

        // each indeax is gpuIndex,groupIndex
        std::vector<std::pair<size_t,size_t>> virtualGroupIndicies;
        std::vector<std::vector<size_t>> virtualGroups{};
        // determ if there will be any virtual windows

        // put wondows into groups
        for (size_t i = 0; i < renderers.size(); i++)
        {
            if (renderers[i]->supportsMultiViewport && renderers[i]->windows.size() > 1) {
                for (size_t j = 0; j < renderers[i]->windows.size(); j++)
                {
                    auto globalIndex = renderers[i]->windows[j]->globalIndex;
                    auto group = cfgWindows[globalIndex].group;

                    auto groupIndex = std::find(virtualGroupIndicies.begin(), virtualGroupIndicies.end(), std::pair{ i,group });

                    // if group exists
                    if (groupIndex != virtualGroupIndicies.end()) {
                        int index = groupIndex - virtualGroupIndicies.begin();
                        virtualGroups[index].push_back(globalIndex);
                    }
                    else {
                        virtualGroupIndicies.push_back({ i, group });
                        virtualGroups.push_back({ globalIndex });
                    }

                }
            }
        }

        // go through each group and make virtual windows

        for (size_t group = 0; group < virtualGroups.size(); group++)
        {
            // check if the size of all windows is the same
            auto size = cfgWindows[0].size;
            for (auto win : virtualGroups[group]) {
                if (cfgWindows[win].size != size) {
                    SR_CORE_WARN("Group {} on device {} is invalid because not all windows are the same size so \
                        falling back to individual windows instead");
                    goto dispandGroup;
                }
            }

            if (virtualGroups[group].size() > 1) {

                auto virtualWindow = new Window(this, windows.size(), windows.size() == 0, true);

                // attatch physical windows
                for (size_t i = 0; i < virtualGroups[group].size(); i++)
                {
                    auto winIndex = virtualGroups[group][i];
                    windows[winIndex]->_owned = true;
                    virtualWindow->addSubWindow(windows[winIndex]);
                    
                    auto& rendererWins = windows[winIndex]->renderer->windows;
                    auto& rendererAllWins = windows[winIndex]->renderer->allWindows;
                    auto& rendererPhysicalWins = windows[winIndex]->renderer->physicalWindows;

                    auto inrendererIndex = std::find(rendererWins.begin(),rendererWins.end(),windows[winIndex]);

                    if (inrendererIndex != rendererWins.end()) {
                        windows[winIndex]->indexInRenderer = 0;
                        windows[winIndex]->allIndexInRenderer = rendererAllWins.size();
                        windows[winIndex]->physicalIndexInRenderer = rendererPhysicalWins.size();
                        rendererWins.erase(inrendererIndex);
                        rendererAllWins.push_back(windows[winIndex]);
                        rendererPhysicalWins.push_back(windows[winIndex]);
                    }

                }

                virtualWindow->createWindowAndSurface();

                auto firstSubWindow = windows[virtualGroups[group][0]];
                virtualWindow->device = firstSubWindow->device;
                virtualWindow->renderer = firstSubWindow->renderer;


                virtualWindow->finishInit();

                windows.push_back(virtualWindow);

                virtualWindow->renderer->windows.push_back(virtualWindow);
                virtualWindow->renderer->allWindows.push_back(virtualWindow);

                continue; 
            }
        dispandGroup: ;

        }

        for (size_t i = 0; i < windows.size(); i++)
        {
            if (!windows[i]->isVirtual() && !windows[i]->_owned) {
                windows[i]->finishInit();
                windows[i]->renderer->allWindows.push_back(windows[i]);
                windows[i]->renderer->physicalWindows.push_back(windows[i]);
            }


            if (windows[i]->swapChainImages.size() > maxSwapChainImages) {
                maxSwapChainImages = windows[i]->swapChainImages.size();
            }
        }

    }

	void Application::run()
	{
		PROFILE_FUNCTION;

        SR_CORE_INFO("Running");
        
        if (!wantsWindows()) return;

        runLoop();
	}

	void Application::shutdown()
	{
		PROFILE_FUNCTION;

        SR_CORE_INFO("Shutdown");

        if (!wantsWindows()) return;

        //TODO: fix this. see line below
        SR_CORE_ERROR("deallocation of application objects (windows, renderers, devices, etc) not performed");

        vk::DynamicLoader dl;


        auto dldid = vk::DispatchLoaderDynamic(instance, dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));


        auto loc_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)dldid.vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");

        loc_vkDestroyDebugReportCallbackEXT(instance, debugObject, nullptr);

        instance.destroy();

        glfwTerminate();
	}

	void Application::loadScene(Scene* scene, void* animationProperties)
	{
        SR_CORE_INFO("Loading scene at addr: {}", reinterpret_cast<void*>(scene));
        scene->load();
        scene->coordinator->createPasses();
        // load all coordinator registered pipeliens if not already
        scene->coordinator->loadOrGetRegisteredPipesInAllWindows();
	}


    void Application::runLoop()
    {

        //worldScene->updateScene();

        while (shouldLoop()) {
            runLoopIteration();
        }

        SR_CORE_INFO("Shutdown Requested");

        windows[0]->device.waitIdle();
    }

    bool Application::shouldLoop() {
        PROFILE_FUNCTION;
        for (size_t i = 0; i < windows.size(); i++)
        {
            // abstract this to a window spacific function
            if (windows[i]->shouldClose())
                return false;
        }
        return true;
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

        if (!wantsWindows()) return;

        // update scene
        loadedScenes[0]->update();

        renderers[0]->beforeRenderScene();

        // draw view - this should be able to be done all in parallel
        for (size_t i = 0; i < windows.size(); i++)
        {
            PROFILE_SCOPE("Loop of Window render loop");
            if (windows[i]->_owned) continue;
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
                getName(),//"Flight Sim Terrain System",
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

#if SR_ENABLE_VK_VALIDATION_LAYERS
        SR_CORE_INFO("Vulkan Validation Enabled");
#else
        SR_CORE_WARN("Vulkan Validation Disabled");
#endif

        auto fullExtensions = extraInstanceLayers;
        std::vector<const char*> glfwExtensionsv = { glfwExtensions, glfwExtensions + glfwExtensionCount };
        fullExtensions.insert(fullExtensions.end(),glfwExtensionsv.begin(),glfwExtensionsv.end());

        info.pApplicationInfo = &appInfo;
        info.enabledLayerCount = validationLayers.size();
        info.ppEnabledLayerNames = validationLayers.data();
        info.enabledExtensionCount = fullExtensions.size();
        info.ppEnabledExtensionNames = fullExtensions.data();


        instance = vk::createInstance(info);
    }


    void Application::createAllocator(size_t deviceIndex)
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

    bool Application::createRenderer(size_t deviceIndex)
    {
        if (deviceIndex < renderers.size()) {
            SR_CORE_TRACE("not creating Renderer for device {} becuase it already eists", deviceIndex);
            return true;
        }
        SR_CORE_TRACE("Creating Renderer for device {}", deviceIndex);
        auto index = devices.size() - 1;

        // TODO: fix this because all windows are being added to all devices
        auto renderer =
            new Renderer(*this, devices[index], physicalDevices[index], allocators[index], windows, deviceQueues[index], queueFamilyIndices[index]);

        renderer->supportsMultiViewport = deviceInfos[index]->supportsMultiViewport;

        renderers.push_back(renderer);

        return false;
    }

    size_t Application::createDevice(size_t window)
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
        VkPhysicalDeviceFeatures requestedDeviceFeatures{};
        requestedDeviceFeatures.samplerAnisotropy = VK_TRUE;


        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

        bool multiViewportAvailable = false;

        if (supportedFeatures.multiViewport == VK_TRUE) {
            requestedDeviceFeatures.multiViewport = VK_TRUE;
            multiViewportAvailable = true;
        }


        VkPhysicalDeviceDescriptorIndexingFeatures desIndexingFeatures{};
        desIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        desIndexingFeatures.pNext = nullptr;

        desIndexingFeatures.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
        // not suported by gtx 1080 ti
        //desIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
        desIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
        desIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;

        desIndexingFeatures.runtimeDescriptorArray = VK_TRUE;

        bool debugExtAvailable = false;

        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());
        for (auto& ext : availableExtensions)
        {
            if (!strcmp(ext.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
            {
                debugExtAvailable = true;
            }
        }

        // devie extensions
        std::vector<const char*> extensionNames = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            //todo fix this
#if RenderMode == RenderModeGPU
            VK_NV_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME
#endif
        };

        if (debugExtAvailable) {
            extensionNames.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        }

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

        createInfo.pEnabledFeatures = &requestedDeviceFeatures;

        createInfo.enabledLayerCount = validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayers.data();

        createInfo.enabledExtensionCount = extensionNames.size();
        createInfo.ppEnabledExtensionNames = extensionNames.data();

        // features
        createInfo.pNext = &desIndexingFeatures;

        SR_CORE_TRACE("goint to create logical device");
        // error is on this line below
        
        auto device = physicalDevice.createDevice(vk::DeviceCreateInfo(createInfo), nullptr);

        VkDebug::init(device,this);
        VkDebug::active = debugExtAvailable;

        GPUQueues loc_deviceQueues;

        device.getQueue(loc_queueFamilyIndices.graphicsFamily.value(), 0, &loc_deviceQueues.graphics);
        device.getQueue(loc_queueFamilyIndices.presentFamily.value(), 0, &loc_deviceQueues.presentation);
        device.getQueue(loc_queueFamilyIndices.resourceTransferFamily.value(), 0, &loc_deviceQueues.resourceTransfer);

        devices.push_back(device);
        deviceInfos.push_back(new DeviceInfo{ multiViewportAvailable });
        physicalDevices.push_back(physicalDevice);
        deviceQueues.push_back(loc_deviceQueues);
        queueFamilyIndices.push_back(loc_queueFamilyIndices);

        createAllocator(devices.size() - 1);
        

        //TODO put this in a better place - fix for multi gpu - put this as a member of renderer so that each gpu has its own resource transferer

        SR_CORE_TRACE("retuning after succesfully creasting a logical device for window {}", window);
        return devices.size() - 1;
    }





    VkBool32 Application::debugCallbackFunc(
        VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
        int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
    {


        SR_CORE_CRITICAL("vulkan debug called for reason {}", flags);

        if (flags == 8) {
            SR_ASSERT(1);
        }

        SR_CORE_ERROR("{}",pMessage);

        return false;
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
        return "NO APPLICATION";
    }

}