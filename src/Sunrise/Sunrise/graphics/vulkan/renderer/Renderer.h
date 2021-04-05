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

			Renderer(Application& app, vk::Device device, vk::PhysicalDevice physicalDevice, VmaAllocator allocator, std::vector<Window*> windows, GPUQueues& deviceQueues, QueueFamilyIndices& queueFamilyIndices);
			void createAllResources();
			~Renderer();

			void beforeRenderScene();
			void renderFrame(Window& window);

			// systems
			TerrainSystem* terrainSystem = nullptr;

			MaterialManager* materialManager = nullptr;

			// handles
			vk::Device device;
			vk::PhysicalDevice physicalDevice;
			VmaAllocator allocator;
			GPUQueues& deviceQueues;
			QueueFamilyIndices& queueFamilyIndices;

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




			std::vector<std::vector<Buffer*>> uniformBuffers;

			// dindless vars

			VaribleIndexAllocator* gloablVertAllocator = nullptr;
			VaribleIndexAllocator* gloablIndAllocator  = nullptr;

			BindlessMeshBuffer* globalMeshBuffer;

			// thye main thread staging buffer
			BindlessMeshBuffer* globalMeshStagingBuffer;

			libguarded::shared_guarded < std::vector<size_t>> freeThreadLocalGlobalMeshandModelStagingBufferIndicies;
			libguarded::shared_guarded < std::unordered_map<std::thread::id, size_t>> ThreadLocalGlobalMeshandModelStagingBufferThreadIndicies;

			std::vector<BindlessMeshBuffer*> threadLocalGlobalMeshStagingBuffers;


			IndexAllocator* globalModelBufferAllocator;
			std::array<Buffer*, 2> globalModelBuffers;

			size_t gpuUnactiveGlobalModelBuffer = 1;
			size_t gpuActiveGlobalModelBuffer = 0;

			Buffer* globalModelBufferStaging;

			std::vector<Buffer*> threadLocalGlobalModelStagingBuffers;


			IndexAllocator* matUniformAllocator;
			Buffer* globalMaterialUniformBufferStaging;
			Buffer* globalMaterialUniformBuffer;


			void windowSizeChanged(size_t allWindowIndex);

			ResourceTransferer* resouceTransferer;

			// deivce spacific features

			bool supportsMultiViewport = false;

		private:

			void createRenderResources();

			void makeGlobalMeshBuffers(const VkDeviceSize& vCount, const VkDeviceSize& indexCount);

			void createDescriptorPoolAndSets();

			void allocateDescriptors();

			void resetDescriptorPools();

			void createUniformsAndDescriptors();

			void updateLoadTimeDescriptors(Window& window);
			void updateRunTimeDescriptors(Window& window);

			void createDynamicRenderCommands();

			void submitFrameQueue(Window& window, vk::CommandBuffer* buffers, uint32_t bufferCount);



			void encodeDeferredPass(Window& window);

			void encodeGBufferPass(Window& window);

			void updateCameraUniformBuffer(Window& window);

			// render resources


			std::vector<std::vector<vk::CommandPool  >> dynamicCommandPools;
			std::vector<std::vector<vk::CommandBuffer>> dynamicCommandBuffers;



			vk::DescriptorPool descriptorPool;
			// first dimension is windows second is surface index
			std::vector<std::vector<VkDescriptorSet>> descriptorSets;


			// deferred pass

			Buffer* deferredPassVertBuff;
			size_t deferredPassBuffIndexOffset;


			vk::DescriptorPool deferredDescriptorPool;
			std::vector<std::vector<VkDescriptorSet>> deferredDescriptorSets;

			friend TerrainSystem;
			friend MaterialManager;
			friend Application;


			std::vector<math::Frustum> camFrustroms;

			// handles

			Application& app;

		};
	}
}