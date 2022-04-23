#pragma once

#include "srpch.h"
#include "Renderer.h"
#include "../generalAbstractions/VkAbstractions.h"
#include "../resources/ResourceTransferTask.h"
#include "Sunrise/fileFormats/binary/Bimage.h"

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
	class SUNRISE_API MaterialManager
	{
	public:
		MaterialManager(gfx::Renderer& renderer);
        ~MaterialManager();
        
		void loadStaticEarth();

		void loadMat(std::string& matRootPath, const char* matFolder);

		const std::vector<gfx::Image*>& allImages();


	private:
		friend gfx::SceneRenderCoordinator;

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


		std::vector<gfx::Image*>  images;
		std::vector<gfx::Buffer*> buffers;

		std::vector<gfx::Sampler*> samplers
			;
		std::vector<gfx::ResourceTransferer::Task> pendingTasks = {};
		std::vector<gfx::ResourceTransferer::Task> pendingGFXTasks = {};
        
		std::tuple<gfx::Buffer*, gfx::Image*> loadTex(const char* path, const char* name);
		/// <summary>
		/// path is to perspacetive cash image
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


		gfx::Renderer& renderer;


		std::vector<std::pair<gfx::SceneRenderCoordinator*, std::unordered_map<const Window*, std::vector<gfx::DescriptorSet*>>*>> registeredDescriptors{};

		bool staticEarthLoaded = false;
	};


}
