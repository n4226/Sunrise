#pragma once

#include "srpch.h"
#include <GLFW/glfw3.h>

#include "../graphics/vulkan/renderPipelines/GraphicsPipeline.h"
#include "../scene/Camera.h"
#include "../configuration/ConfigSystem.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderPipelines/concrete/DeferredPassPipeline.h"


#include "../graphics/vulkan/generalAbstractions/Image.h"

//#ifdef SR_PLATFORM_WINDOWS
//#define SR_NATIVE_MONITOR GLFWmonitor*
//#elif defined (SR_PLATFORM_MACOS)
//#define SR_NATIVE_MONITOR CGDirectDisplayID
//#endif

#ifdef SR_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(SR_PLATFORM_MACOS)
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3native.h>

namespace sunrise {

	class WorldScene;
	namespace gfx {
		class Renderer;
		class GPUGenCommandsPipeline;
	}
	class Application;


	/// <summary>
	/// This class represents a window
	/// 
	/// can either be a physical window or a virtual window
	/// a physical window represents an acuall window presented on a monitor for the user to see
	/// a virtual window reprents all physical windows in the same group on the same gpu
	/// 
	/// all windows in a virtual window must be on the same gpu
	/// 
	/// the virtual window allows for Multi Viewport Rendering so if a graphics card does not support it
	/// than their can not be andy virtual windows for winodws on that gpu
	/// 
	/// windows in a virtual window may be on different monitors
	/// 
	/// virtual windows do still have a global index into application arrays that phyisical winodws do but content is just duplicated from its alaised windiws addreses
	/// 
	/// it can be ditermaned weather a window is virtual or not by querring its isVirtual() method
	/// 
	/// for virtual windows:
	/// 
	///		every sub window has a: window, surface, swapchain, swapchain image views, swapchain images
	/// 
	/// each sub window will have its own swpachain and assicited images and the final pass images which will be multi layer will have to be coppied into each window swapchain at the end of the render pass
	/// 
	///		pipelines and renderpassmanager are created for virtual window but pointers are given to subwindows
	/// 
	/// for extreme performance make windows components for an entity component system
	/// 
	/// 
	/// MVR put on hold here is current progress:
	///		
	///		virtual windows are being created and subwindows are being put in currently at the part where renderer descriptors have to be fixed to deal with input attatchments for gbuffer and defered stage to work with multiple layers
	/// 
	/// </summary>
	class SUNRISE_API Window
	{
	public:

		Window(Application* app, size_t globalIndex,bool isPrimary,bool isVirtual);
		~Window();

		void createWindowAndSurface();
		void destroyWindowAndSurface();
		void finishInit();

		/// <summary>
		/// the index of the monitor in application unique for all windows
		/// </summary>
		size_t globalIndex;

		//// application spacific - will move to other class after refactor - not sure about render pass manager yet


			// manager objects

		gfx::RenderPassManager* renderPassManager;
		gfx::GraphicsPipeline* pipelineCreator;

		//// window spacific - will keep here after refactor


		/// <summary>
		/// 
		/// </summary>
		/// <returns>weather to abort the frame</returns>
		bool getDrawable();
		void presentDrawable();

		Camera camera;

		GLFWwindow* window = nullptr;

		// Swap Chain

		vk::SwapchainKHR swapChain = nullptr;
		std::vector<VkImage> swapChainImages;
		VkFormat swapchainImageFormat;
		VkExtent2D swapchainExtent;
		std::vector<vk::ImageView> swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFramebuffers;

		/* Manualy created Frame Buffer Images */

		VkFormat depthBufferFormat;

		VkFormat albedoFormat;
		VkFormat normalFormat;
		VkFormat aoFormat;

		//GBuffer
		gfx::Image* gbuffer_albedo_metallic;
		gfx::Image* gbuffer_normal_roughness;
		gfx::Image* gbuffer_ao;
		gfx::Image* depthImage;

		//Deferred 
		//Image* deferred_colorAttachment; - right now befoe adding post processing passes the deferred writes directly to swap chain

		gfx::DeferredPass* deferredPass;
		gfx::GPUGenCommandsPipeline* gpuGenPipe;

		std::unordered_map<const gfx::VirtualGraphicsPipeline*,gfx::GraphicsPipeline*> loadedPipes = {};
		std::vector <gfx::GraphicsPipeline*> worldLoadedPipes = {};

		vk::SurfaceKHR surface;

		uint32_t currentSurfaceIndex;



		// Synchronization objects

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		/// <summary>
		/// indicies are frames i.e. app.currentFrame
		/// </summary>
		std::vector<VkFence> inFlightFences;
		/// <summary>
		/// indicies are window surface inndexes i.e. window.currentSurfaceIndex
		/// </summary>
		std::vector<VkFence> imagesInFlight;

		bool framebufferResized = false;

		void recreateSwapchain();

		// refrences =

		vk::Device device;
		gfx::Renderer* renderer;
		size_t indexInRenderer;
		size_t allIndexInRenderer;
		size_t physicalIndexInRenderer;

		Application& app;

		bool isVirtual();
		bool isPrimary();
		bool isOwned();

		bool _owned = false;

		void addSubWindow(Window* subWindow);

		bool shouldClose();

        

        static std::string getNativeMonitorName(GLFWmonitor* monitor) {
        #ifdef SR_PLATFORM_WINDOWS
                return glfwGetWin32Monitor(monitor)->name
        #elif defined(SR_PLATFORM_MACOS)
                return std::to_string(glfwGetCocoaMonitor(monitor));
        #else
        #error Platform not supported
        #endif
        }

        
	private:

		bool _virtual;
		bool _primary = false;

		/// <summary>
		/// only valid for virtual windows
		/// </summary>
		std::vector<Window*> subWindows;

		void cleanupSwapchain();

		void runWindowLoop();


		void createWindow();

		void makeWindwWithMode(ConfigSystem::Config::Window& winConfig, GLFWmonitor* monitor);


		void createSurface();


		void createSwapchain();

		void createSwapchainImageViews();
		void createFrameBufferImages();

		void createFramebuffers();

		void createSemaphores();

        void SetupImgui();

		VkDescriptorPool imguiDescriptorQueue;

        GLFWmonitor* getMonitorFromNativeName(const std::string& name);
        
		void destroyWindow();
		void destroySurface();

	};

}
