#pragma once

#include "srpch.h"
#include "../graphics/vulkan/renderPipelines/GraphicsPipeline.h"
#include "../graphics/vulkan/renderPipelines/ComputePipeline.h"
#include "../graphics/vulkan/generalAbstractions/Image.h"

namespace sunrise {
	

	
	struct ExternalReferenceMaterialImageDescriptor {
		gfx::Image* image;

		//TODO: decide if external reference images should be allowed to be dealocated by the material system or are just seperate
		//TODO: have to probably keep these seperate but Material system must have a way for other systems to reserve some on board gpu space
		/*Memorry Managment*/
		//bool deallocatable = false;

	};
	
	struct InternalMaterialImageDescriptor {
		
		using PossibleNames = std::vector <std::string>;
		
		/// <summary>
		/// relative to the material directory given in MaterialSetDescriptor
		/// leave empty to use the same dir as parrent image set descriptor
		/// </summary>
		std::string path;

		/// <summary>
		/// possible keywords in texture name to look for within path directory. e.x "albedo" or "ao"
		/// first is used as debug name
		/// </summary>
		PossibleNames possibleNames;

		/// <summary>
		/// standard is - set true for albedo and false for all other input
		/// </summary>
		bool srgbColorSPace = false;

		//built in names:

		static PossibleNames AlbedoNames();
		static PossibleNames NormalNames();
		static PossibleNames RoughnessNames();
		static PossibleNames MetalicNames();
		static PossibleNames AONames();
	};

	using MaterialImageDescriptor = std::variant<InternalMaterialImageDescriptor, ExternalReferenceMaterialImageDescriptor>;


	//Material Image Descriptor
	struct MaterialImageSetDescriptor {
		/// <summary>
		/// do the images in this set need to be loaded contiguously in the global material descriptor array - commanly needed so only the first image index needs to be passed to the shader via push constants
		/// </summary>
		bool requireCcontiguous;
		std::vector<MaterialImageDescriptor> images;
		
		// path options
		std::string basePath;

		static MaterialImageSetDescriptor stdPBRSet(const std::string& basePath);
	};


	
	//the material is the definition which is then registered with the material system and given a material id

	/// <summary>
	/// a Material is the combinations of the shader and required inputs/config, A material can also represent a compute shader and associated inputs and outputs
	/// 
	/// shader is considered to be the combination of all shader files e.x vertex and fragment
	/// 
	/// for example many materials can use the standard pbr shader
	/// 
	/// this is the highest level abstraction for shaders in the engine - descritpor layouts must not be defined as they are generated from the shader
	/// the next lower level for graphics pipelines are virtual graphics pipelines
	/// then the lowest are grphics or compute pipeliens which create the relevent vulkan objects
	/// 
	/// </summary>
	class Material {

	public:
		
		//Material(const Material& other) = default;

		enum class Type {
			Graphics,
			Compute,
		};

		//Pipeline:

		Type type = Type::Graphics;
		
		//only one has to be valid - pointer memory must be managed externally 
		//the descriptor set layouts will be modified by material system
		//do nut put in descriptor layout for the engine's built in global descriptor set - if you want it enable flag below and it will be added automatically
		

		std::variant<gfx::GraphicsPipelineOptions, gfx::ComputePipelineOptions> pipleineOptions;
		//gfx::VirtualGraphicsPipeline* graphicsPipeline;
		//gfx::ComputePipeline* computePiplien;
		

		//Inputs

		/// <summary>
		/// needs to be true to load images
		/// </summary>
		bool usesGloabalDescriptorSet;

		std::vector<MaterialImageSetDescriptor> imageSets;

		//TODO: add constant input (up to 120 bytes)
	};

	

}

