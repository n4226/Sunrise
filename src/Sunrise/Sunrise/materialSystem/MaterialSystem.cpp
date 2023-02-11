#include "srpch.h"
#include "MaterialSystem.h"
#include "Sunrise/core/Application.h"
#include "../graphics/vulkan/renderer/Renderer.h"
#include "../graphics/vulkan/renderer/MaterialManager.h"
#include "../world/gfxPipelines/StandardPBRPipeline.h"
#include "../world/materials/StaticMaterialTable.h"

#include <spirv_reflect.h>


namespace sunrise {

	sunrise::MaterialSystem::Ticket::Ticket(glm::uint64 val)
		: val(val)
	{

	}

	MaterialSystem::System::System(glm::uint32 val) 
		: val(val)
	{

	}

	

	MaterialSystem::MaterialSystem(Application& app)
		: app(app)
	{

	}

	void MaterialSystem::startup()
	{
		//register all static mats at launch
		registerStaticMaterials();
	}

	//thread safe
	sunrise::MaterialSystem::System MaterialSystem::createSystem(const SystemCreateInfo& options)
	{
		System sys(systemCount.fetch_add(1));
		
		{
			auto handle = systems.lock();
			handle->emplace(std::make_pair(sys.val,options));
		}

		return sys;
	}

	sunrise::MaterialSystem::MaterialID MaterialSystem::registerMaterial(const Material& material)
	{
		auto count = dynamicMaterialCount.fetch_add(1);

		MaterialID matID(count + lowerDynamicMaterialIDBound);
		
		//add to material list
		{
			auto handle = materials.lock();
			handle->emplace(std::make_pair(matID, material));
		}

		return matID;
	}

	void MaterialSystem::unregisterMaterial(MaterialID matID)
	{
		//right now does nothing
	}

	sunrise::MaterialSystem::Ticket MaterialSystem::loadMaterial(System system, MaterialID mat, const GPUBitField& renderers, const std::vector<ReplacementMaterial>& backupMaterials)
	{
		/*
			* clean up pipeline - fix descriptors with refraction and others ...
			* create or use cashed grpahics pipeline for this material
			* make sure all required gpus have enough space - if not go into contingency - complicated
			* load the required images on all needed renderers
			* update descriptors - done by material managers
			* 
			
			* mark the allocation as belonging to the system ---- i dont know what this means
		*/
		SR_CORE_ASSERT(!renderers.none());
		

		//TODO: here goes material checking and possible inserting engine descritpor set layout



		

		//
		

		Ticket ticket(handedOutTicketNumbers.fetch_add(1));
		Material material;
		SystemCreateInfo sys;
		
		{
			auto handle = materials.lock();
			material = handle->at(mat);
		}

		{
			auto handle = systems.lock();
			sys = handle->at(system.val);
		}

		//modify and verify material

		{
			auto handle = materials.lock();
			handle->emplace(std::pair(mat, material));
		}


		for (int i = 0; i < app.renderers.size() ; i++)
		{
			if (renderers.test(i))
			{
				auto renderer = app.renderers[i];
				auto canLoad = renderer->materialManager->loadMaterial(ticket, material, sys.priority);
			}
		}
		

		return ticket;
	}

	sunrise::MaterialSystem::GPUBitField MaterialSystem::ticketLoaded(Ticket ticket)
	{

		GPUBitField result;
		
		for (size_t i = 0; i < app.renderers.size() ; i++)
		{
			result[i] = app.renderers[i]->materialManager->ticketLoaded(ticket);
		}
		
		return result;
	}

	void MaterialSystem::registerStaticMat(const Material& material, MaterialID matID)
	{
		//add to material list
		{
			auto handle = materials.lock();
			handle->emplace(std::make_pair(matID, material));
		}

	}

	void MaterialSystem::registerStaticMaterials()
	{
		if (staticMatsRegistered) {
			SR_CORE_WARN("already registered static materials");
			return;
		}

		SR_CORE_INFO("Attempting to load Required materials", StaticMaterialTable::entries.size());

		auto matRootPath = FileManager::engineMaterialDir() + "staticAlloc/";



		for (auto mat : StaticMaterialTable::reverseEntries)
		{
			if (FileManager::exists(matRootPath + mat.second)) {

				auto imageSetDescriptor = MaterialImageSetDescriptor::stdPBRSet(matRootPath + mat.second);

				Material material{};
				material.imageSets = { imageSetDescriptor };
				material.type = Material::Type::Graphics;
				material.pipleineOptions = StandardPBRDef();
				
				registerStaticMat(material,{mat.first});
			}
			else SR_CORE_WARN("Skiping {} world material becuase its directory was not found", mat.second);

		}
	}

}
