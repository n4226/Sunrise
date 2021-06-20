#pragma once

#include "srpch.h"
#include "generalAbstractions/GPUSelector.h"

namespace sunrise {
	class Window;

	namespace gfx {

		struct DeviceInfo {
			bool supportsMultiViewport;
		};

		class Renderer;

		class SUNRISE_API RenderContext
		{
		public:

			vk::Instance instance;


			std::vector<Window*> windows;

			// - one per each device - shared indicies
			std::vector<vk::Device         >          devices;
			std::vector<DeviceInfo*        >          deviceInfos;
			std::vector<vk::PhysicalDevice >          physicalDevices;
			std::vector<Renderer*          >          renderers;
			std::vector<VmaAllocator       >          allocators;
			std::vector<GPUQueues          >          deviceQueues;
			std::vector<QueueFamilyIndices >          queueFamilyIndices;
			//

			size_t currentFrameID = 0;

			size_t currentFrame = 0;
			size_t maxSwapChainImages = 0;

			const size_t MAX_FRAMES_IN_FLIGHT = 3;

		protected:


			std::vector<const char*> validationLayers = {
				#if SR_ENABLE_VK_VALIDATION_LAYERS
				"VK_LAYER_KHRONOS_validation"
				#endif
			};

			std::vector<const char*> extraInstanceLayers = {
#if SR_ENABLE_VK_VALIDATION_LAYERS
				VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#endif
			};

			virtual void createInstance() = 0;
			 
			virtual bool createRenderer(size_t deviceIndex) = 0;
			virtual size_t createDevice(size_t window) = 0;
			virtual void createAllocator(size_t deviceIndex) = 0;
			 
			virtual void runLoop() = 0;
			virtual bool shouldLoop() = 0;
			virtual void runLoopIteration() = 0;


		};

	}
}