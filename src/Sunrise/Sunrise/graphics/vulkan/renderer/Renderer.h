#pragma once

#include "srpch.h"
#include "../generalAbstractions/VkAbstractions.h"
#include "Sunrise/Sunrise/Math.h"
#include "Sunrise/Sunrise/memory.h"
#include "Sunrise/Sunrise/graphics/vulkan/resources/MeshBuffers.h"
#include "DebugDrawer.h"

namespace sunrise {

	class MaterialManager;
	class TerrainSystem;
	class WorldScene;
	class Window;

	namespace gfx {

		class ResourceTransferer;

		class SUNRISE_API RenderResourceTracker {
		public:
			/// <summary>
			/// called in main render loop so any non trivial actions should be completed on a worker thread
			/// </summary>
			/// <param name="window"></param>
			/// <param name="surface"></param>
			virtual void drawableReleased(Window* window, size_t surface) {}

			virtual void frameReleased(size_t appFrame) {}

		};

		class SUNRISE_API Renderer: private RenderResourceTracker
		{
		public:

#pragma region Main API

			Renderer(Application& app, vk::Device device, vk::PhysicalDevice physicalDevice,
				VmaAllocator allocator, std::vector<Window*> windows, GPUQueues& deviceQueues, QueueFamilyIndices& queueFamilyIndices, VkDebug debugObject);
			void createAllResources();
			~Renderer();

			void printVMAAllocattedStats();

			void beforeRenderScene();
			void afterRenderScene();
			void renderFrame(Window& window);

			void windowSizeChanged(size_t allWindowIndex);


			/// <summary>
			/// sets the correct sharing and queue options on the object
			///	MAKE SURE TO SET THE CORRECT STORAGE AND USAGE FLAGS
			/// </summary>
			/// <returns></returns>
			BufferCreationOptions newBufferOptions();

#pragma endregion

#pragma region handles

			vk::Device device;
			vk::PhysicalDevice physicalDevice;
			VmaAllocator allocator;
			//when these were stored as references caused problems with multi gpu
			GPUQueues deviceQueues;
			//when these were stored as references caused problems with multi gpu
			QueueFamilyIndices	 queueFamilyIndices;

			/// <summary>
			/// ?? look into exact specs of this array - I bleive it is top level windows - i.e physical if not in gorup and a virtual for each group - basically all unowned windows
			/// </summary>
			std::vector<Window*> windows;
			/// <summary>
			/// includes _owned windows
			/// Order is not guaranteed to be the same as windows
			/// </summary>
			std::vector<Window*> allWindows;

			/// <summary>
			/// just windows that are !_virtual
			/// Order is not guaranteed to be the same as windows
			/// </summary>
			std::vector<Window*> physicalWindows;

			MaterialManager* materialManager = nullptr;

			VkDebug debugObject;

			ResourceTransferer* resouceTransferer;

#pragma endregion

#pragma region Resources

			//TODO: important for multi gpu (SUN-51) staging buffer dont have to be duplicated accros multiple gpus so we will have to see
			//how this turns out and the best way to do this
#pragma region Bindless resources

			VaribleIndexAllocator* gloablVertAllocator = nullptr;
			VaribleIndexAllocator* gloablIndAllocator = nullptr;

			// the actuall buff the gpu reads from for draw calls
			BindlessMeshBuffer* globalMeshBuffer;

			// thye main thread staging buffer
			BindlessMeshBuffer* globalMeshStagingBuffer;

			// so worker threads can directly allocate meshes to global bufferes 
			libguarded::shared_guarded < std::vector<size_t>> freeThreadLocalGlobalMeshandModelStagingBufferIndicies;
			libguarded::shared_guarded < std::unordered_map<std::thread::id, size_t>> ThreadLocalGlobalMeshandModelStagingBufferThreadIndicies;

			std::vector<BindlessMeshBuffer*> threadLocalGlobalMeshStagingBuffers;

			IndexAllocator* globalModelBufferAllocator;
			// 2 gpu buffs are required for snapping origin to update model transforms of current models without disrupting drawing
			std::array<Buffer*, 2> globalModelBuffers;

			// active buffer is the one the gpu is using for current encodes
			size_t gpuUnactiveGlobalModelBuffer = 1;
			size_t gpuActiveGlobalModelBuffer = 0;

			Buffer* globalModelBufferStaging;
			std::vector<Buffer*> threadLocalGlobalModelStagingBuffers;

			IndexAllocator* matUniformAllocator;
			Buffer* globalMaterialUniformBufferStaging;
			Buffer* globalMaterialUniformBuffer;

#pragma endregion

#pragma endregion


			// deivce spacific features

			bool supportsMultiViewport = false;

			std::vector<math::Frustum> camFrustroms;

			DebugDrawer* debugDraw;

			// handles
			Application& app;

		private:
			friend TerrainSystem;
			friend MaterialManager;
			friend Application;
			friend Window;

#pragma region methods

#pragma region Bindless

			void makeGlobalMeshBuffers(const VkDeviceSize& vCount, const VkDeviceSize& indexCount);


#pragma endregion

#pragma region Frame Command Proccesing

			void submitFrameQueue(Window& window, vk::CommandBuffer* buffers, uint32_t bufferCount);

#pragma endregion


#pragma region Descriptors

			void resetDescriptorPools();

#pragma endregion


#pragma endregion


			void createRenderResources();

			void createDynamicRenderCommands();

			/// <summary>
			/// callled in main rander loop so any non trivial actions should be cojmpleted on a worker thread
			/// </summary>
			/// <param name="window"></param>
			/// <param name="surface"></param>
			void drawableReleased(Window* window, size_t surface) override;


			// render resources
			std::vector<std::vector<vk::CommandPool  >> dynamicCommandPools;
			std::vector<std::vector<vk::CommandBuffer>> dynamicCommandBuffers;


		};
	}
}