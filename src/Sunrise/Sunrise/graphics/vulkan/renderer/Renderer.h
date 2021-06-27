#pragma once

#include "srpch.h"
#include "../generalAbstractions/VkAbstractions.h"
#include "Sunrise/Sunrise/Math.h"
#include "Sunrise/Sunrise/memory.h"
#include "Sunrise/Sunrise/graphics/vulkan/resources/MeshBuffers.h"

namespace sunrise {

	class MaterialManager;
	class TerrainSystem;
	class WorldScene;
	class Window;

	namespace gfx {

		class ResourceTransferer;

		class SUNRISE_API Renderer
		{
		public:

#pragma region Main API

			Renderer(Application& app, vk::Device device, vk::PhysicalDevice physicalDevice,
				VmaAllocator allocator, std::vector<Window*> windows, GPUQueues& deviceQueues, QueueFamilyIndices& queueFamilyIndices, VkDebug debugObject);
			void createAllResources();
			~Renderer();

			void beforeRenderScene();
			void renderFrame(Window& window);

			void windowSizeChanged(size_t allWindowIndex);

#pragma endregion

#pragma region handles

			vk::Device device;
			vk::PhysicalDevice physicalDevice;
			VmaAllocator allocator;
			GPUQueues& deviceQueues;
			QueueFamilyIndices& queueFamilyIndices;

			/// <summary>
			/// ?? look into exact specs of this array
			/// </summary>
			std::vector<Window*> windows;
			/// <summary>
			/// includes _owned windows
			/// Order is not garnteed to be the same as windows
			/// </summary>
			std::vector<Window*> allWindows;

			/// <summary>
			/// just windows that are !_virtual
			/// Order is not garnteed to be the same as windows
			/// </summary>
			std::vector<Window*> physicalWindows;

			MaterialManager* materialManager = nullptr;

			VkDebug debugObject;

			ResourceTransferer* resouceTransferer;

#pragma endregion

#pragma region Resources

			// one per surface
			std::vector<std::vector<Buffer*>> uniformBuffers;

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


		private:
			friend TerrainSystem;
			friend MaterialManager;
			friend Application;

#pragma region methods

#pragma region Bindless

			void makeGlobalMeshBuffers(const VkDeviceSize& vCount, const VkDeviceSize& indexCount);


#pragma endregion

#pragma region Frame Command Proccesing

			void submitFrameQueue(Window& window, vk::CommandBuffer* buffers, uint32_t bufferCount);

#pragma endregion


#pragma region Descriptors


			void createDescriptorPoolAndSets();

			void allocateDescriptors();

			void resetDescriptorPools();

			void updateLoadTimeDescriptors(Window& window);
			void updateRunTimeDescriptors(Window& window);

#pragma endregion


#pragma endregion


			void createRenderResources();


			void createUniformsAndDescriptors();


			void createDynamicRenderCommands();


			void updateCameraUniformBuffer(Window& window);


			// render resources


			std::vector<std::vector<vk::CommandPool  >> dynamicCommandPools;
			std::vector<std::vector<vk::CommandBuffer>> dynamicCommandBuffers;



			//vk::DescriptorPool descriptorPool;
			//// first dimension is windows second is surface index
			//std::vector<std::vector<VkDescriptorSet>> descriptorSets;




			//vk::DescriptorPool deferredDescriptorPool;
			//std::vector<std::vector<VkDescriptorSet>> deferredDescriptorSets;



			std::vector<math::Frustum> camFrustroms;


			// handles

			Application& app;

		};
	}
}