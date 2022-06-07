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

				//window->prepareImagesInFlightArray();
				window->createSyncObjects();
            }

            swapchainExtent = subWindows[0]->swapchainExtent;
            swapchainImageFormat = subWindows[0]->swapchainImageFormat;

            createSwapchainImageViews();
        }
        else {
            createSwapchain();
            createSwapchainImageViews();

        }
        
		createSyncObjects();

    }
	
    void Window::recreateSwapchain()
    {
        throw std::runtime_error(
            "Resizing windows has been disabled to re enable, will need to make it work with multi window / MVR / multi gpu and CRP and scene coordinator");
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

        VkSurfaceFormatKHR surfaceFormat = static_cast<VkSurfaceFormatKHR>(GPUSelector::chooseSwapSurfaceFormat(swapChainSupport.formats));
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

        if (_owned)
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

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
        PROFILE_FUNCTION;

        if (_virtual) {
            //also need to create images

            virtualSwapImages.resize(numSwapImages());

            for (int i = 0; i < numSwapImages() ; i++)
            {
				ImageCreationOptions createOptions;

				createOptions.sharingMode = vk::SharingMode::eExclusive;
				createOptions.storage = ResourceStorageType::gpu;
				createOptions.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;

				createOptions.type = vk::ImageType::e2D;
				createOptions.layout = vk::ImageLayout::eUndefined;
				createOptions.tilling = vk::ImageTiling::eOptimal;
                createOptions.layers = numSubWindows();

                createOptions.format = vk::Format(swapchainImageFormat);

                auto image = new gfx::Image(renderer->device, renderer->allocator, { swapchainExtent.width,swapchainExtent.height,1 }, createOptions, vk::ImageAspectFlagBits::eColor);
                virtualSwapImages[i] = image;
            }
        }

        swapChainImageViews.resize(numSwapImages());


        for (size_t i = 0; i < numSwapImages(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            if (_virtual)
                createInfo.image = virtualSwapImages[i]->vkItem;
            else
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

    void Window::createSyncObjects()
    {
        PROFILE_FUNCTION;

        imageAvailableSemaphores.resize(app.MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(app.MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(app.MAX_FRAMES_IN_FLIGHT);
        prepareImagesInFlightArray();


        vk::SemaphoreCreateInfo semaphoreInfo{};
        vk::FenceCreateInfo fenceInfo{};
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
        for (size_t i = 0; i < app.MAX_FRAMES_IN_FLIGHT; i++) {
            if (!_virtual) {
                imageAvailableSemaphores[i] = device.createSemaphore(semaphoreInfo);
            }
            if (!_owned)
                renderFinishedSemaphores[i] = device.createSemaphore(semaphoreInfo);
                inFlightFences[i] = device.createFence(fenceInfo);
        }
    }

    GLFWmonitor* Window::getMonitorFromNativeName(const std::string& name)
    {
        int monitorsCount = 0;
        auto monitors = glfwGetMonitors(&monitorsCount);
        SR_CORE_INFO("Picking monitor for window {}",globalIndex);
        for (size_t i = 0; i < monitorsCount; i++)
        {
            SR_CORE_TRACE("comparing monitor {} to requested {}", getNativeMonitorName(monitors[i]), name.c_str());
            
            if (strcmp(getNativeMonitorName(monitors[i]).c_str(), name.c_str()) == 0) {
                return monitors[i];
            }
        }
        SR_CORE_WARN("Could not find requested monitor ({}) for window {} so chosing primary monitor", name, globalIndex);

#if SR_DEBUG
        //TODO return secondary monitor monitor when debugging to prevent screen lockups on crash or breakpoint;
        
        SR_CORE_WARN("Changing windowing mode to windowed during dev for safety");
        configSystem.global().windows[globalIndex].mode = ConfigSystem::Config::Window::WindowMode::windowed;
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

        auto& windowConfig = configSystem.global().windows[globalIndex];

        monitor = getMonitorFromNativeName(windowConfig.monitor);
        SR_CORE_INFO("creating window {} on monitor {}", globalIndex, getNativeMonitorName(monitor));


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

        subWindow->owner = this;
        subWindows.push_back(subWindow);
    }

	size_t Window::numSubWindows()
	{
        if (!_virtual)
            return 0;
        else
            return subWindows.size();
	}

	bool Window::shouldClose()
    {
        if (!_virtual) {
            if (!_owned && glfwWindowShouldClose(window))
                return true;
        }
        else {
            for each (auto child in subWindows)
            {
                if (child->shouldClose())
                    return true;
            }
        }
        return false;
    }

    void Window::cleanupSwapchain()
    {

        /* dont think therse are still used
        delete gbuffer_albedo_metallic;
        delete gbuffer_normal_roughness;
        delete gbuffer_ao;
        delete depthImage;
        */

        //CRPHolder destroys them
       /* for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }*/


        //delete pipelineCreator;
        
        //scene render coordinator owns this object this is just a reference
        //delete renderPassManager;
            

        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }


        device.destroySwapchainKHR(swapChain);
    }

    Window::~Window()
    {
        PROFILE_FUNCTION

        device.waitIdle();

        for (auto [vPipe, cPipe] : loadedPipes)
            delete cPipe;

        for (size_t i = 0; i < app.MAX_FRAMES_IN_FLIGHT; i++) {
            device.destroySemaphore(imageAvailableSemaphores[i]);
            device.destroySemaphore(renderFinishedSemaphores[i]);
			device.destroyFence(inFlightFences[i]);
        }
        /*for (int i = swapChainImages.size() - 1; i >= 0; i--)
        {
			device.destroyFence(imagesInFlight[i]);
        }*/

        cleanupSwapchain();


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


    bool Window::getDrawable(bool* outReleaseDrawable)
    {
        PROFILE_FUNCTION;

        if (!_owned) {
			// this is to wait for inflight frame to finish
		    // if cpu gets to far ahead, will wait here until an inflight frame finishes
			vkWaitForFences(device, 1, &inFlightFences[app.currentFrame], VK_TRUE, UINT64_MAX);

			renderer->frameReleased(app.currentFrame);
        }

        //if virtual - need to acquire swap image for each monitor in group
        //this will populate each child window with it's current swap image so that coordinator can copy into them
        if (_virtual) {
            bool fail = false;
            bool releaseDrawable = false;

            for (int i = 0; i < numSubWindows() ; i++)
            {
                bool release = false;
                if (subWindows[i]->getDrawable(&release))
                    fail = true;
                if (i == 0)
                    releaseDrawable = release;
            }

            //give a "fake" current surface id to this virtual window so that systems that allocate resources per surface can do so
            //it shouldnt mater that this surface id does not mean anything in relaiton to it's subwindows
            //currentSurfaceIndex = (currentSurfaceIndex + 1) % numSwapImages();
            //need to fake by having same index as first sub window
            currentSurfaceIndex = subWindows[0]->currentSurfaceIndex;

            if (releaseDrawable)
                renderer->drawableReleased(this, currentSurfaceIndex);

            return fail;
        }


        SR_CORE_ASSERT(!_virtual);

        VkSemaphore semaphore; 
        VkFence fence; 

        //not needed
        //if (_owned) {
        //    semaphore = owner->imageAvailableSemaphores[app.currentFrame];
        //    fence = owner->inFlightFences[app.currentFrame];
        //}
        //else {
            semaphore = imageAvailableSemaphores[app.currentFrame];
            fence = inFlightFences[app.currentFrame];
        //}

        auto index = device.acquireNextImageKHR(swapChain, UINT64_MAX, semaphore, nullptr);
        //cout << "current index = " << index.value << endl;
        currentSurfaceIndex = index.value;


        if (index.result == vk::Result::eErrorOutOfDateKHR)
        {
            recreateSwapchain();
            return true;
        }
        else if (static_cast<int>(index.result) < 0) {//(index.result != vk::Result::eSuccess && index.result != vk::Result::eSuboptimalKHR) {
           // throw std::runtime_error("failed to acquire swap chain image");
        }

        // Check if a previous frame is using this image (i.e. there is its fence to wait on)
        // wait when the current image is being used by a frame that is still inflight
        if (imagesInFlight[currentSurfaceIndex] != VK_NULL_HANDLE) {
            //todo consider using get fence and doing other work on this thread instead of waiting
            vkWaitForFences(device, 1, &imagesInFlight[currentSurfaceIndex], VK_TRUE, UINT64_MAX);

            if (!_owned)
                renderer->drawableReleased(this, currentSurfaceIndex);
            else if (outReleaseDrawable)
                *outReleaseDrawable = true;
        }

        // Mark the image as now being in use by this frame
        imagesInFlight[currentSurfaceIndex] = fence;

        return false;
    }

    void Window::presentDrawable()
    {
        PROFILE_FUNCTION;

    /*    if (_virtual) {

            for each(auto win in subWindows)
                win->presentDrawable();

            return;
        }*/

        // present frame on screen
        //SR_CORE_ASSERT(!_virtual);

        vk::PresentInfoKHR presentInfo{};

        std::vector<vk::Semaphore> waitSemaphors = { renderFinishedSemaphores[app.currentFrame] };

        //if owned use the virtual window's render finished semaphore since that is what was used as result of submiting render commands to queue
       /* if (_owned) {
            waitSemaphors[0] = owner->renderFinishedSemaphores[app.currentFrame];
        }*/
      /*  if (_virtual) {

        }*/


        presentInfo.setWaitSemaphores(waitSemaphors);

        std::vector<vk::SwapchainKHR> swapChains = {}; 
        std::vector<uint32_t> surfaceIndicies = {};
        swapChains.reserve(4);
        surfaceIndicies.reserve(4);

        if (_virtual) {
            for each (auto child in subWindows)
            {
                swapChains.push_back(child->swapChain);
                surfaceIndicies.push_back(child->currentSurfaceIndex);
            }
        }
        else {
            swapChains.push_back(swapChain);
            surfaceIndicies.push_back(currentSurfaceIndex);
        }

        presentInfo.swapchainCount = swapChains.size();
        presentInfo.pSwapchains = swapChains.data();
        presentInfo.pImageIndices = surfaceIndicies.data();

        presentInfo.pResults = nullptr; // Optional

#if SR_PROFILE_WITH_OPTICK
        //the vulkan impl for optick doesnt use this pointer at all
        //OPTICK_GPU_FLIP(nullptr);
#endif
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


	size_t Window::numSwapImages()
	{
        //TODO: assuming group has windows all with same swap image count - should asert this more promenently in future
        if (_virtual) {
            SR_CORE_ASSERT(subWindows.size() > 0);
            return subWindows[0]->swapChainImages.size();
        }
        else {
            return swapChainImages.size();
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
