#pragma once

#include "srpch.h"
#include "Renderer.h"
#include "../generalAbstractions/VkAbstractions.h"
#include "../resources/ResourceTransferTask.h"
#include "Sunrise/fileFormats/binary/Bimage.h"
#include "Sunrise/materialSystem/MaterialSystem.h"

namespace sunrise {

	struct MaterialImages
	{
		std::vector<gfx::Image*> albedoImages;
		std::vector<gfx::Image*> normalImages;
		std::vector<gfx::Image*> metallicImages;
		std::vector<gfx::Image*> roughnessImages;
		std::vector<gfx::Image*> aoImages;
	};

	namespace gfx {
		class SceneRenderCoordinator;
	}


	//TODO: eventual major material system needs to span accros gpus maybe - maybe it hsould be individual - has to deal with different sized memories of different gpus
	class SUNRISE_API MaterialManager: private gfx::RenderResourceTracker
	{
	public:
		MaterialManager(gfx::Renderer& renderer);
        ~MaterialManager();
        
		//NOTICE: some public api uses internal locking

		/// <summary>
		/// returns whether valid to use ticket
		/// </summary>
		/// <param name="ticket"></param>
		/// <returns></returns>
		bool useTicket(MaterialSystem::Ticket ticket, vk::CommandBuffer buffer, gfx::Renderer* renderer);

		/// <summary>
		/// binds a buffer to a widnow surface
		/// second part of using a ticket - by default scene renderer coordinator will call this for all stages
		/// can only be called once per frame per buffer
		/// </summary>
		/// <param name="buffer"></param>
		/// <param name="window"></param>
		/// <param name="surface"></param>
		void bindBuffer(vk::CommandBuffer buffer, Window* window, uint32_t surface);

		/// <summary>
		/// call to clear internal structures for a cmd buff so when deleting or re-encoding a cmd buff
		/// </summary>
		/// <param name="buffer"></param>
		void resetBuffer(vk::CommandBuffer buffer);

		bool ticketLoaded(MaterialSystem::Ticket ticket);


		void drawDebugView();

		//OLD API:

		void loadStaticEarth();

		void loadMat(uint32_t matID, std::string& matRootPath, const char* matFolder);

		const std::vector<gfx::Image*>& allImages();


		size_t getLoadedMatIndex(uint32_t matID);
		std::optional<size_t> tryGetLoadedMatIndex(uint32_t matID);



	private:
		friend gfx::SceneRenderCoordinator;
		friend MaterialSystem;		
		friend gfx::Renderer;


		/// <summary>
		/// called by renderer
		/// </summary>
		/// <param name="window"></param>
		/// <param name="surface"></param>
		void drawableReleased(Window* window, size_t surface) override;
		
		
		//called by material system
		//returns if material could be loaded
		bool loadMaterial(MaterialSystem::Ticket ticket, const Material& material, glm::int8 priority);

		void unloadMaterial(MaterialSystem::Ticket ticket);


		std::tuple<gfx::Buffer*, gfx::Image*> loadTex(const char* path, const char* name, bool albedo = false);

		/// <summary>
		/// path is to perspective cash image
		/// returns null if not found or could not load
		/// </summary>
		/// <param name="path"></param>
		/// <param name="name"></param>
		/// <returns></returns>
		Bimage* loadTex_fromCash(const char* path, const char* name);

		glm::uint32 FinishLoadingTexture(std::tuple<gfx::Buffer*, gfx::Image*> texture);

		void addTexToGlobal(gfx::Image* image, glm::uint32 imageIndex);

		void addCopyToTasks(gfx::Buffer* buffer, gfx::Image* image);
		void addMipMapToTasks(gfx::Image* image);

		/// <summary>
		/// key is material id - for static listed in static mat table
		/// value - index of material in material array - this is the value to pass in push constants for draw calls
		/// </summary>
		std::unordered_map<uint32_t, size_t> loadedMaterialIndicies;

		std::vector<gfx::Image*>  images;
		std::vector<gfx::Buffer*> buffers;

		std::vector<gfx::Sampler*> samplers;

		
		std::vector<gfx::ResourceTransferer::Task> pendingTasks = {};
		std::vector<gfx::ResourceTransferer::Task> pendingGFXTasks = {};
		

        
		gfx::Renderer& renderer;

		
		/// <summary>
		/// size of all self allocated device memory
		/// just an estimate
		/// </summary>
		VkDeviceSize allocatedSize = 0;
		//TODO: make proper way of setting this
		//now just flat 4 gigs
		VkDeviceSize allocationBudget = (4ul * 1024 * 1024 * 1024);
		
		//buffer of 512MB - only high priority items can be created when memory gets to this threshold
		VkDeviceSize allocationBuffer = 512 * 1024 * 1024;


		//MARK: material arc
		
	/*	/// <summary>
		/// first key is window
		/// second key is surface index
		/// value is reference count
		/// </summary>
		libguarded::shared_guarded < std::unordered_map<Window*, std::unordered_map<uint32_t, size_t>> surfaceArc;*/



		//key = cmd buff, value - tickets in use by it
		libguarded::shared_guarded<std::unordered_map<VkCommandBuffer, std::vector<glm::uint64>>> ticketLinks;

		/// <summary>
		/// first key is window
		/// second key is surface index
		/// value is vector of registered cmd buffers
		/// </summary>
		libguarded::shared_guarded<std::unordered_map<Window*, std::unordered_map<uint32_t,std::vector<VkCommandBuffer>>>> registeredBuffers;



		// data structures above are used to modify this one - this is the important one
		// when values hit zero they are deallocated
		//key = ticket, value = referenced count (number of cmd buffers referencing the ticket
		libguarded::shared_guarded<std::unordered_map<glm::uint64, size_t>> ticketArc;



		//OLD API:

		struct MaterialFiles {
			std::string albedoPath{};
			std::string normal{};
			std::string metalic{};
			std::string roughness{};
			std::string ambientOclusion{};
		};

		/// <summary>
		/// 
		/// </summary>
		/// <param name="matRootPath"></param>
		/// <param name="matFolder"></param>
		/// <param name="suportedFileExtensions"> if left blank all are considered supported</param>
		/// <returns></returns>
		MaterialFiles getFilesForMaterial(std::string& matRootPath, const char* matFolder, const std::unordered_set<std::string>& suportedFileExtensions = {});

		std::vector<std::pair<gfx::SceneRenderCoordinator*, std::unordered_map<const Window*, std::vector<gfx::DescriptorSet*>>*>> registeredDescriptors{};

		bool staticEarthLoaded = false;

	};


}
