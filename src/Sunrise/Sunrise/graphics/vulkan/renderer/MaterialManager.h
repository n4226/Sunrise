#pragma once

#include "srpch.h"
#include "Renderer.h"
#include "../generalAbstractions/VkAbstractions.h"
#include "../resources/ResourceTransferTask.h"

namespace sunrise {

	struct MaterialImages
	{
		std::vector<gfx::Image*> albedoImages;
		std::vector<gfx::Image*> normalImages;
		std::vector<gfx::Image*> metallicImages;
		std::vector<gfx::Image*> roughnessImages;
		std::vector<gfx::Image*> aoImages;
	};

	class MaterialManager
	{
	public:
		MaterialManager(gfx::Renderer& renderer);

		void loadStatic();

		void loadMat(std::string& matRootPath, const char* matFolder);


	private:


		std::vector<gfx::Image*>  images;
		std::vector<gfx::Buffer*> buffers;

		std::vector<gfx::Sampler*> samplers
			;
		std::vector<gfx::ResourceTransferer::Task> pendingTasks = {};
		std::vector<gfx::ResourceTransferer::Task> pendingGFXTasks = {};

		std::tuple<gfx::Buffer*, gfx::Image*> loadTex(const char* path);

		glm::uint32 FinishLoadingTexture(std::tuple<gfx::Buffer*, gfx::Image*> texture);

		void addTexToGlobal(gfx::Image* image, glm::uint32 imageIndex);

		void addCopyToTasks(gfx::Buffer* buffer, gfx::Image* image);
		void addMipMapToTasks(gfx::Image* image);


		gfx::Renderer& renderer;
	};


}