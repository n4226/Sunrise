#include "srpch.h"
#include "Material.h"


namespace sunrise {
	
	

	sunrise::InternalMaterialImageDescriptor::PossibleNames InternalMaterialImageDescriptor::AlbedoNames()
	{
		return {
			"albedo",
			"defuse",
			"color",
			"diffuse",
		};
	}

	sunrise::InternalMaterialImageDescriptor::PossibleNames InternalMaterialImageDescriptor::NormalNames()
	{
		return { "normal" };
	}

	sunrise::InternalMaterialImageDescriptor::PossibleNames InternalMaterialImageDescriptor::RoughnessNames()
	{
		return { "rough" };
	}

	sunrise::InternalMaterialImageDescriptor::PossibleNames InternalMaterialImageDescriptor::MetalicNames()
	{
		return { "metal", "metalic" };
	}

	sunrise::InternalMaterialImageDescriptor::PossibleNames InternalMaterialImageDescriptor::AONames()
	{
		return { "ao", "ambientocclusion",  "ambient_occlusion", "ambientOcclusion" };
	}



	
	sunrise::MaterialImageSetDescriptor MaterialImageSetDescriptor::stdPBRSet(const std::string& basePath)
	{
		MaterialImageSetDescriptor set{};

		set.basePath = basePath;
		set.requireCcontiguous = true;

		set.images.reserve(5);
		
		InternalMaterialImageDescriptor d1 = { "", InternalMaterialImageDescriptor::AlbedoNames(), true };
		set.images.push_back(d1);
		
		InternalMaterialImageDescriptor d2 = { "", InternalMaterialImageDescriptor::NormalNames(), false };
		set.images.push_back(d2);
		
		InternalMaterialImageDescriptor d3 = { "", InternalMaterialImageDescriptor::MetalicNames(), false };
		set.images.push_back(d3);
		
		InternalMaterialImageDescriptor d4 = { "", InternalMaterialImageDescriptor::AONames(), false };
		set.images.push_back(d4);

		InternalMaterialImageDescriptor d5 = { "", InternalMaterialImageDescriptor::RoughnessNames(), false };
		set.images.push_back(d5);

		return set;
	}

}