#include "srpch.h"
#include "Window.h"
#include "Application.h"

#include "../graphics/vulkan/renderer/Renderer.h"
#include "../graphics/vulkan/generalAbstractions/GPUSelector.h"
#include "../graphics/vulkan/renderPipelines/concrete/TerrainPipeline.h"
#include "../graphics/vulkan/renderPipelines/concrete/DeferredPassPipeline.h"
#include "../graphics/vulkan/renderPipelines/concrete/gpuDriven/GPUGenCommandsPipeline.h"
#include "../graphics/vulkan/SingalPassRenderPassManager.h"
#include "../world/WorldScene.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

namespace sunrise {

    using namespace gfx;

    Window::Window(Application* app, size_t globalIndex, bool isPrimary,bool isVirtual)
        : app(*app), globalIndex(globalIndex), _virtual(isVirtual), _primary(isPrimary)
    {

    }

    void Window::createWindowAndSurface() {
        PROFILE_FUNCTION;

        if (!_virtual) {
            createWindow();


            createSurface();
        }
    }

    void Window::destroyWindowAndSurface()
    {
        destroySurface();
        destroyWindow();
    }

    void Window::finishInit()
    {
        if (_virtual) {
            for (auto window : subWindows) {

                window->createSwapchain();
                window->createSwapchainImageViews();
            }

            swapchainExtent = subWindows[0]->swapchainExtent;
            swapchainImageFormat = subWindows[0]->swapchainImageFormat;

        }
        else {
            createSwapchain();
            createSwapchainImageViews();

        }

        // temporary for splititng between legacy world system and neo CRP (composable rener pass) system
        //WorldScene* world = dynamic_cast<WorldScene*>(app.loadedScenes[0]);


        // make graphics pipeline


     /*   if (world != nullptr)
        {
            createFrameBufferImages();
            renderPassManager = new RenderPassManager(device, albedoFormat, normalFormat, aoFormat, swapchainImageFormat, depthBufferFormat);
            if (_virtual) {
                renderPassManager->multiViewport = true;
                renderPassManager->multiViewCount = subWindows.size();
            }

            renderPassManager->createMainRenderPass();
        }
        */
        

//        if (world != nullptr) {
//            pipelineCreator = new TerrainPipeline(device, swapchainExtent, *renderPassManager);
//
//            pipelineCreator->createPipeline();
//
//            //TODO: find better way to abstract use of multiple windows with different shaders 
//        
//            // create deferred pipeline
//
//            deferredPass = new DeferredPass(device, { swapchainExtent }, *renderPassManager);
//
//            deferredPass->createPipeline();
//
//#if RenderMode == RenderModeGPU
//            gpuGenPipe = new GPUGenCommandsPipeline(app, device, *pipelineCreator);
//#endif
//            worldLoadedPipes.push_back(pipelineCreator);
//            worldLoadedPipes.push_back(deferredPass);
//
//            createFramebuffers();
//        }

        createSemaphores();

        //if (!_virtual && world != nullptr)
        //    SetupImgui();
        //else if (world != nullptr) {
        //    // give pointers of grphics pipelines and renderpass to subwindows
        //    for (size_t i = 0; i < subWindows.size(); i++)
        //    {
        //        subWindows[i]->pipelineCreator = pipelineCreator;
        //        subWindows[i]->renderPassManager = renderPassManager;
        //        subWindows[i]->deferredPass = deferredPass;
        //        subWindows[i]->gpuGenPipe = gpuGenPipe;
        //    }
        //}

    }

    void Window::SetupImgui()
    {

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();
        


        // Create imgui Descriptor Pool
        {
            VkDescriptorPoolSize pool_sizes[] =
            {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
            };
            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
            pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
            pool_info.pPoolSizes = pool_sizes;
            auto err = vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiDescriptorQueue);
            //(err);
        }

        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = app.instance;
        init_info.PhysicalDevice = renderer->physicalDevice;
        init_info.Device = renderer->device;
        init_info.QueueFamily = renderer->queueFamilyIndices.graphicsFamily.value();
        init_info.Queue = renderer->deviceQueues.graphics;
        init_info.PipelineCache = nullptr;
        init_info.DescriptorPool = imguiDescriptorQueue;
        init_info.Allocator = nullptr;
        init_info.Subpass = 1;
        //TODO set this to an actual value
        init_info.MinImageCount = 2;
        init_info.ImageCount = swapChainImages.size();
        init_info.CheckVkResultFn = nullptr;
        // for now going to try to use the standard world renderpass
        ImGui_ImplVulkan_Init(&init_info, renderPassManager->renderPass);

        io.Fonts->AddFontDefault();
        // not sure if this is supposed to be called explicitily but error if not
        io.Fonts->Build();

    }

    void Window::recreateSwapchain()
    {
        throw std::runtime_error(
            "Resizing windows hasbeen disabled to re enable, will need to make it work with multi window / MVR / multi gpu and CRP and scene coordinator");
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        device.waitIdle();

        cleanupSwapchain();

        if (_virtual)
            SR_ASSERT(0 && "resize of virtual windows not implimented yet");
        createSwapchain();
        createSwapchainImageViews();

        //WorldScene* world = dynamic_cast<WorldScene*>(app.loadedScenes[0]);
       /* if (world != nullptr) {
            renderPassManager = new RenderPassManager(device, albedoFormat, normalFormat, aoFormat, swapchainImageFormat, depthBufferFormat);

            if (_virtual) {
                renderPassManager->multiViewport = true;
                renderPassManager->multiViewCount = subWindows.size();
            }

            renderPassManager->createMainRenderPass();

        }*/
        
  /*    
        if (world != nullptr) {
            pipelineCreator = new TerrainPipeline(device, swapchainExtent, *renderPassManager);

            pipelineCreator->createPipeline();

            deferredPass = new DeferredPass(device, swapchainExtent, *renderPassManager);

            deferredPass->createPipeline();
            gpuGenPipe = new GPUGenCommandsPipeline(app, device, *pipelineCreator);
        }*/

        createFrameBufferImages();
        createFramebuffers();


        //TODO: call renderer.updateDescriptors() somehow

        renderer->windowSizeChanged(indexInRenderer);

        //TODO buffers might need to be recreated
        //createCommandBuffers();

        //update IMGUI
        ImGui_ImplVulkan_SetMinImageCount(2);

    }


    void Window::createSwapchain()
    {
        PROFILE_FUNCTION;
        SwapChainSupportDetails swapChainSupport = GPUSelector::querySwapChainSupport(renderer->physicalDevice, surface);

        VkSurfaceFormatKHR surfaceFormat = GPUSelector::chooseSwapSurfaceFormat(swapChainSupport.formats);
        vk::PresentModeKHR presentMode = GPUSelector::chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = GPUSelector::chooseSwapExtent(swapChainSupport.capabilities, window);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        SR_CORE_TRACE("createing window swapchain with {} images.", imageCount);

        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }


        SR_ASSERT(renderer->queueFamilyIndices.graphicsFamily == renderer->queueFamilyIndices.presentFamily);

        /* vk::SwapchainCreateInfoKHR swapInfo = vk::SwapchainCreateInfoKHR(
             vk::SwapchainCreateFlagsKHR{}, surface, imageCount,
             vk::Format(surfaceFormat.format), vk::ColorSpaceKHR(surfaceFormat.colorSpace),
             vk::Extent2D(extent), 1, vk::ImageUsageFlags(vk::ImageUsageFlagBits::eColorAttachment),
             vk::SharingMode::eExclusive, {}, vk::SurfaceTransformFlagBitsKHR(swapChainSupport.capabilities.currentTransform), vk::CompositeAlphaFlagBitsKHR::eOpaque,
             presentMode, vk::Bool32(VK_TRUE),VK_NULL_HANDLE
             );*/

             //device.createSwapchainKHR()

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;

        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        // this is what needs to change if allowing suport for different presentation and drawing queues
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = VkPresentModeKHR(presentMode);
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;


        SR_CORE_INFO("creating swapchain");
        swapChain = device.createSwapchainKHR(vk::SwapchainCreateInfoKHR(createInfo), nullptr);

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapchainImageFormat = surfaceFormat.format;
        swapchainExtent = extent;

    }

    void Window::createSwapchainImageViews()
    {
        PROFILE_FUNCTION
            swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];

            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapchainImageFormat;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            if (_virtual)
                createInfo.subresourceRange.layerCount = subWindows.size();
            else
                createInfo.subresourceRange.layerCount = 1;

            swapChainImageViews[i] = device.createImageView({ createInfo });
        }

    }

    void Window::createFrameBufferImages()
    {
        //GBUffer Images

        albedoFormat = VkFormat(
            //vk::Format::eR8G8B8A8Unorm);
            vk::Format::eB8G8R8A8Unorm);
        //TODO: roughness field will have half the precioins if it uses range [0,1] so consider changing this
        normalFormat = VkFormat(vk::Format::eB8G8R8A8Unorm);
        aoFormat = VkFormat(vk::Format::eR8Unorm);

        ImageCreationOptions createOptions;

        createOptions.sharingMode = vk::SharingMode::eExclusive;
        createOptions.storage = ResourceStorageType::gpu;
        createOptions.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment;

        createOptions.type = vk::ImageType::e2D;
        createOptions.layout = vk::ImageLayout::eUndefined;
        createOptions.tilling = vk::ImageTiling::eOptimal;

        createOptions.layers = 1;
        if (_virtual)
           createOptions.layers = subWindows.size();

        createOptions.format = vk::Format(albedoFormat);

        gbuffer_albedo_metallic = new Image(device, renderer->allocator, { swapchainExtent.width,swapchainExtent.height,1 }, createOptions, vk::ImageAspectFlagBits::eColor);

        createOptions.format = vk::Format(normalFormat);

        gbuffer_normal_roughness = new Image(device, renderer->allocator, { swapchainExtent.width,swapchainExtent.height,1 }, createOptions, vk::ImageAspectFlagBits::eColor);

        createOptions.format = vk::Format(aoFormat);

        gbuffer_ao = new Image(device, renderer->allocator, { swapchainExtent.width,swapchainExtent.height,1 }, createOptions, vk::ImageAspectFlagBits::eColor);



        { // Depth
            depthBufferFormat = VkFormat(GPUSelector::findSupportedFormat(renderer->physicalDevice, { vk::Format::eD32Sfloat }, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment));


            ImageCreationOptions depthOptions;

            depthOptions.sharingMode = vk::SharingMode::eExclusive;
            depthOptions.storage = ResourceStorageType::gpu;
            depthOptions.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eInputAttachment;

            depthOptions.type = vk::ImageType::e2D;
            depthOptions.layout = vk::ImageLayout::eUndefined;
            depthOptions.tilling = vk::ImageTiling::eOptimal;

            depthOptions.layers = createOptions.layers;

            depthOptions.format = vk::Format(depthBufferFormat);


            depthImage = new Image(device, renderer->allocator, { swapchainExtent.width,swapchainExtent.height,1 }, depthOptions, vk::ImageAspectFlagBits::eDepth);
        }



    }

    //TODO remove
    void Window::createFramebuffers()
    {
        return;

        PROFILE_FUNCTION;
        swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            // see renderpass.cpp for info on order of attachments
            std::vector<vk::ImageView> attachments = {
                //Gbuffer
                gbuffer_albedo_metallic->view,
                gbuffer_normal_roughness->view,
                gbuffer_ao->view,
                depthImage->view,
                //Deferred
                swapChainImageViews[i],
            };
             
            vk::FramebufferCreateInfo framebufferInfo{};
            framebufferInfo.renderPass = renderPassManager->renderPass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapchainExtent.width;
            framebufferInfo.height = swapchainExtent.height;
            framebufferInfo.layers = 1;

            swapChainFramebuffers[i] = device.createFramebuffer(framebufferInfo);
        }
    }

    void Window::createSemaphores()
    {
        PROFILE_FUNCTION

            imageAvailableSemaphores.resize(app.MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(app.MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(app.MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

        vk::SemaphoreCreateInfo semaphoreInfo{};
        vk::FenceCreateInfo fenceInfo{};
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
        for (size_t i = 0; i < app.MAX_FRAMES_IN_FLIGHT; i++) {
            imageAvailableSemaphores[i] = device.createSemaphore(semaphoreInfo);
            renderFinishedSemaphores[i] = device.createSemaphore(semaphoreInfo);
            inFlightFences[i] = device.createFence(fenceInfo);
        }
    }

    GLFWmonitor* Window::getMonitorFromNativeName(std::string& const name)
    {
        int monitorsCount = 0;
        auto monitors = glfwGetMonitors(&monitorsCount);
        SR_CORE_INFO("Picking monitor for window {}",globalIndex);
        for (size_t i = 0; i < monitorsCount; i++)
        {
            SR_CORE_TRACE("comparing monitor {} to requested {}", glfwGetWin32Monitor(monitors[i]), name.c_str());
            if (strcmp(glfwGetWin32Monitor(monitors[i]), name.c_str()) == 0) {
                return monitors[i];
            }
        }
        SR_CORE_WARN("Could not find requested monitor ({}) for window {} so chosing primary monitor", name, globalIndex);

#if SR_DEBUG
        //TODO return secondary monitor monitor when debugging to prevent screen lockups on crash or breakpoint;
#endif

        return glfwGetPrimaryMonitor();
    }

    void Window::createSurface()
    {
        PROFILE_FUNCTION
            VkSurfaceKHR _surface;
        auto result = glfwCreateWindowSurface(app.instance, window, nullptr, &_surface);

        assert(result == VK_SUCCESS);

        surface = vk::SurfaceKHR(_surface);
    }


    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void Window::createWindow()
    {
        PROFILE_FUNCTION;

        auto windowConfig = configSystem.global().windows[globalIndex];

        auto monitor = getMonitorFromNativeName(windowConfig.monitor);
        SR_CORE_INFO("creating window {} on monitor {}", globalIndex, glfwGetWin32Monitor(monitor));



        makeWindwWithMode(windowConfig, monitor);


        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    void Window::makeWindwWithMode(ConfigSystem::Config::Window& winConfig, GLFWmonitor* monitor)
    {
        auto windowName = app.getName();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // see this for why this is turned off https://github.com/glfw/glfw/issues/447
        // this is dissabeld for to stop full screen (posibly boarderless) windows in a multimonitor enviroment from minimising when the mouse is clicked on another monitor
        glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);


        if (winConfig.size.x == 0 || winConfig.mode != ConfigSystem::Config::Window::WindowMode::windowed) {
            winConfig.size.x = mode->width;
            winConfig.size.y = mode->height;
        }

        switch (winConfig.mode)
        {
        case ConfigSystem::Config::Window::WindowMode::windowed:
            window = glfwCreateWindow(winConfig.size.x, winConfig.size.y, windowName, nullptr, nullptr);

            // position the window on the correct monitor

            int monitor_xpos, monitor_ypos;
            glfwGetMonitorPos(monitor, &monitor_xpos, &monitor_ypos);

            glfwSetWindowPos(window, 
                monitor_xpos + mode->width * winConfig.monitorLocalPostion.x - winConfig.size.x / 2,
                monitor_ypos + mode->height * winConfig.monitorLocalPostion.y - winConfig.size.y / 2);

            break;
        case ConfigSystem::Config::Window::WindowMode::FullscreenBorderless:

            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);


            window = glfwCreateWindow(mode->width, mode->height, windowName, monitor, NULL);

            break;
        case ConfigSystem::Config::Window::WindowMode::Fullscreen:
            window = glfwCreateWindow(winConfig.size.x, winConfig.size.y, windowName, monitor, nullptr);
            break;
        default:
            break;
        }


    }



    bool Window::isVirtual()
    {
        return _virtual;
    }

    bool Window::isPrimary()
    {
        return _primary;
    }

    bool Window::isOwned()
    {
        return _owned;
    }

    void Window::addSubWindow(Window* subWindow)
    {
        SR_ASSERT(_virtual);

        subWindows.push_back(subWindow);
    }

    bool Window::shouldClose()
    {
        if (!_virtual) {
            if (!_owned && glfwWindowShouldClose(window))
                return true;
        }
        else {
            return false;
        }
        return false;
    }

    void Window::cleanupSwapchain()
    {

        delete gbuffer_albedo_metallic;
        delete gbuffer_normal_roughness;
        delete gbuffer_ao;
        delete depthImage;

        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }


        delete pipelineCreator;
        delete deferredPass;
        delete renderPassManager;
            

        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }


        device.destroySwapchainKHR(swapChain);
    }

    Window::~Window()
    {
        PROFILE_FUNCTION

            device.waitIdle();

        delete deferredPass;



        for (size_t i = 0; i < app.MAX_FRAMES_IN_FLIGHT; i++) {
            device.destroySemaphore(imageAvailableSemaphores[i]);
            device.destroySemaphore(renderFinishedSemaphores[i]);
            device.destroyFence(inFlightFences[i]);
        }

        cleanupSwapchain();

        vmaDestroyAllocator(renderer->allocator);

        device.destroy();

        destroySurface();
        destroyWindow();
    }




    void Window::runWindowLoop()
    {
        //TODO - posibly place run loop here on seperate threads
        PROFILE_FUNCTION
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                //drawView();
            }
    }


    bool Window::getDrawable()
    {
        PROFILE_FUNCTION

        vkWaitForFences(device, 1, &inFlightFences[app.currentFrame], VK_TRUE, UINT64_MAX);

        auto index = device.acquireNextImageKHR(swapChain, UINT64_MAX, imageAvailableSemaphores[app.currentFrame], nullptr);
        //cout << "current index = " << index.value << endl;
        currentSurfaceIndex = index.value;


        if (index.result == vk::Result::eErrorOutOfDateKHR)
        {
            recreateSwapchain();
            return 1;
        }
        else if (static_cast<int>(index.result) < 0) {//(index.result != vk::Result::eSuccess && index.result != vk::Result::eSuboptimalKHR) {
           // throw std::runtime_error("failed to acquire swap chain image");
        }

        // Check if a previous frame is using this image (i.e. there is its fence to wait on)
        // wait when the current image is being used by a frame that is still inflight
        if (imagesInFlight[currentSurfaceIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(device, 1, &imagesInFlight[currentSurfaceIndex], VK_TRUE, UINT64_MAX);
        }

        // Mark the image as now being in use by this frame
        imagesInFlight[currentSurfaceIndex] = inFlightFences[app.currentFrame];

        return 0;
    }

    void Window::presentDrawable()
    {
        PROFILE_FUNCTION;
        // present frame on screen

        vk::PresentInfoKHR presentInfo{};

        std::vector<vk::Semaphore> signalSemaphores = { renderFinishedSemaphores[app.currentFrame] };

        presentInfo.setWaitSemaphores(signalSemaphores);

        vk::SwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &currentSurfaceIndex;

        presentInfo.pResults = nullptr; // Optional

        auto result = renderer->deviceQueues.presentation.presentKHR(&presentInfo);

        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized)
        {
            framebufferResized = false;
            //TODO - not working yet need to manually intercept window size changes
            recreateSwapchain();
        }
        else if (result != vk::Result::eSuccess) {
            // throw std::runtime_error("failed to present swap chain image");
        }
    }


    void Window::destroyWindow()
    {
        PROFILE_FUNCTION;

        glfwDestroyWindow(window);
    }


    void Window::destroySurface()
    {
        PROFILE_FUNCTION;

        app.instance.destroySurfaceKHR(surface);
    }


}