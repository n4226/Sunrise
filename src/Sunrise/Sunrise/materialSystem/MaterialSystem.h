#pragma once

#include "srpch.h"
#include "material.h"
#include "../graphics/vulkan/renderer/Renderer.h"


namespace sunrise {

	class MaterialManager;

	class MaterialSystem
	{
	public:
		MaterialSystem(Application& app);

		struct SystemCreateInfo {
			std::string name;
			//100 is default priority
			glm::int8 priority = 100;
		};

		struct System {
		private:
			glm::uint32 val;

			System(glm::uint32 val);

			friend MaterialSystem;
		};

		using GPUBitField = std::bitset<MAX_NUM_GPUS>;

		/// <summary>
		/// recommended to do this on startup and not during run cycle as this function requires internal locking for thread safety
		/// </summary>
		/// <param name="options"></param>
		/// <returns></returns>
		System createSystem(const SystemCreateInfo& options);


		using MaterialID = uint64_t;

		/// <summary>
		/// used to represent an open order for material(s) that are currently in use by a system
		/// 
		/// create one by loading a material 
		/// tell the material system a ticket is still being used each frame or it will be asumed unused and may be preempted (i.e deallocated)
		/// 
		/// should you be able to formally close a ticket?
		/// </summary>
		struct Ticket {
		private:
			glm::uint64 val;


			friend MaterialSystem;
			friend MaterialManager;

			Ticket(glm::uint64 val);
		};

		struct ReplacementMaterial
		{
			/// <summary>
			/// false = interchangeable with original material
			/// true = only if original is not available - then will be replaced in order provided
			/// </summary>
			bool equialPriority = false;
			MaterialID material;
		};

		//TODO: maybe make into a shared pointer so unregistering is automatic
		//TODO: IMPORTANT: how to deal with modifing material config / and or shader - can't change shader but should be able to modify paramteters / images
		MaterialID registerMaterial(const Material& material);
		void unregisterMaterial(MaterialID matID);

		//TODO: allow baqckup materials and opting in or out of allowign this system to hot swap materials on fly when memory usage requires
		//TODO: allow user configs to whole material system like limiting texture sizes 

		/// <summary>
		/// loads a material to certain renderers
		/// creates ticket that will be used to reference the material(s) (right now jsut a single material
		/// </summary>
		/// <param name="mat"></param>
		/// <param name="renderer"></param>
		/// <returns></returns>
		Ticket loadMaterial(System system, MaterialID mat,const GPUBitField& renderers, const std::vector<ReplacementMaterial>& backupMaterials);		

		/// <summary>
		/// returns bit field of renderers that have the material loaded
		/// each material manager has a method to see if it has the ticket loaded
		/// this is a read only funciton adn does not effect the arc of a material so do not use this as only method or ticket will become invalid and deallocated
		/// </summary>
		/// <returns></returns>
		GPUBitField ticketLoaded(Ticket ticket);
		

	private:

		void registerStaticMat(const Material& material, MaterialID matID);
		void registerStaticMaterials();
		bool staticMatsRegistered = false;

		Application& app;

		//Storage Mechanisms:

		std::atomic_uint32_t systemCount = 0;
		libguarded::shared_guarded<std::unordered_map<glm::uint32, SystemCreateInfo>> systems;
		

		//Materials
		
		/// <summary>
		/// todo - deal with deallocations
		/// </summary>
		std::atomic_uint64_t handedOutTicketNumbers = 0;

		/// <summary>
		/// all material ids below this are reserved for statically allocated engine materials - wright now this is the first 32,768 materials
		/// </summary>
		const glm::uint64 lowerDynamicMaterialIDBound = 0x8000;
		std::atomic_uint64_t dynamicMaterialCount = 0;
		libguarded::shared_guarded<std::unordered_map<MaterialID, Material>> materials;

		friend MaterialManager;
	};

}


