#include "srpch.h"
#include "Window.h"
#include "Application.h"

#include "../graphics/vulkan/renderer/Renderer.h"
#include "../graphics/vulkan/generalAbstractions/GPUSelector.h"
#include "../graphics/vulkan/renderPipelines/concrete/TerrainPipeline.h"
#include "../graphics/vulkan/renderPipelines/concrete/DeferredPassPipeline.h"


namespace sunrise {

    using namespace gfx;

    Window::Window(Application* app, size_t globalIndex)
        : app(*app), globalIndex(globalIndex)
    {

    }

    void Window::createWindowAndSurface() {
        PROFILE_FUNCTION;

        createWindow();


        createSurface();
    }

    void Window::finishInit()
    {
        createSwapchain();

        createSwapchainImageViews();

        createFrameBufferImages();

        // make graphics pipeline 

        renderPassManager = new RenderPassManager(device, albedoFormat, normalFormat, aoFormat, swapchainImageFormat, depthBufferFormat);
        pipelineCreator = new TerrainPipeline(device, swapchainExtent, *renderPassManager);

        pipelineCreator->createPipeline();

        //TODO: find better way to abstract use of multiple windows with different shaders 
        // create deferred pipeline

        deferredPass = new DeferredPass(device, { swapchainExtent }, *renderPassManager);

        deferredPass->createPipeline();

        createFramebuffers();

        createSemaphores();
    }

    void Window::recreateSwapchain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }


        device.waitIdle();

        cleanupSwapchain();

        createSwapchain();
        createSwapchainImageViews();

        renderPassManager = new RenderPassManager(device, albedoFormat, normalFormat, aoFormat, swapchainImageFormat, depthBufferFormat);
        pipelineCreator = new TerrainPipeline(device, swapchainExtent, *renderPassManager);

        pipelineCreator->createPipeline();

        deferredPass = new DeferredPass(device, swapchainExtent, *renderPassManager);

        deferredPass->createPipeline();

        createFrameBufferImages();
        createFramebuffers();


        //TODO: call renderer.updateDescriptors() somehow

        renderer->windowSizeChanged(indexInRenderer);

        //TODO buffers might need to be recreated
        //createCommandBuffers();

    }


    void Window::createSwapchain()
    {
        PROFILE_FUNCTION;
        SwapChainSupportDetails swapChainSupport = GPUSelector::querySwapChainSupport(renderer->physicalDevice, surface);

        VkSurfaceFormatKHR surfaceFormat = GPUSelector::chooseSwapSurfaceFormat(swapChainSupport.formats);
        vk::PresentModeKHR presentMode = GPUSelector::chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = GPUSelector::chooseSwapExtent(swapChainSupport.capabilities, window);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }


        assert(renderer->queueFamilyIndices.graphicsFamily == renderer->queueFamilyIndices.presentFamily);

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



            depthOptions.format = vk::Format(depthBufferFormat);


            depthImage = new Image(device, renderer->allocator, { swapchainExtent.width,swapchainExtent.height,1 }, depthOptions, vk::ImageAspectFlagBits::eDepth);
        }



    }

    void Window::createFramebuffers()
    {
        PROFILE_FUNCTION
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

        auto monitor = glfwGetPrimaryMonitor();

        makeWindwWithMode(windowConfig, monitor);


        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    void Window::makeWindwWithMode(ConfigSystem::Config::Window& winConfig, GLFWmonitor* monitor)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        if (winConfig.size.x == 0 || winConfig.mode != ConfigSystem::Config::Window::WindowMode::windowed) {
            winConfig.size.x = mode->width;
            winConfig.size.y = mode->height;
        }

        switch (winConfig.mode)
        {
        case ConfigSystem::Config::Window::WindowMode::windowed:
            window = glfwCreateWindow(winConfig.size.x, winConfig.size.y, "GPUObjectsV6", nullptr, nullptr);
            break;
        case ConfigSystem::Config::Window::WindowMode::FullscreenBorderless:

            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);


            window = glfwCreateWindow(mode->width, mode->height, "GPUObjectsV6", monitor, NULL);

        case ConfigSystem::Config::Window::WindowMode::Fullscreen:
            window = glfwCreateWindow(winConfig.size.x, winConfig.size.y, "GPUObjectsV6", monitor, nullptr);
            break;
        default:
            break;
        }


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

        app.instance.destroySurfaceKHR(surface);
        app.instance.destroy();

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
        PROFILE_FUNCTION
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
        PROFILE_FUNCTION
            glfwDestroyWindow(window);

        glfwTerminate();
    }

}