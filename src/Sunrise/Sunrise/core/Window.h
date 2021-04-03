#pragma once

#include "srpch.h"
#include <GLFW/glfw3.h>

#include "../graphics/vulkan/renderPipelines/GraphicsPipeline.h"
#include "../scene/Camera.h"
#include "../configuration/ConfigSystem.h"
#include "Sunrise/Sunrise/graphics/vulkan/renderPipelines/concrete/DeferredPassPipeline.h"


#include "../graphics/vulkan/generalAbstractions/Image.h"

namespace sunrise {

	class WorldScene;
	namespace gfx {
		class Renderer;
		class GPUGenCommandsPipeline;
	}
	class Application;

	class SUNRISE_API Window
	{
	public:

		Window(Application* app, size_t globalIndex);
		~Window();

		void createWindowAndSurface();
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


		vk::SurfaceKHR surface;

		uint32_t currentSurfaceIndex;



		// Synchronization objects

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;

		bool framebufferResized = false;

		void recreateSwapchain();

		// refrences =

		vk::Device device;
		gfx::Renderer* renderer;
		size_t indexInRenderer;

		Application& app;


	private:

		void cleanupSwapchain();

		void runWindowLoop();

		void destroyWindow();

		void createWindow();

		void makeWindwWithMode(ConfigSystem::Config::Window& winConfig, GLFWmonitor* monitor);


		void createSurface();


		void createSwapchain();

		void createSwapchainImageViews();
		void createFrameBufferImages();

		void createFramebuffers();

		void createSemaphores();

		void Window::SetupImgui();

		VkDescriptorPool imguiDescriptorQueue;

		GLFWmonitor* getMonitorFromNativeName(std::string& const name);
	};

}