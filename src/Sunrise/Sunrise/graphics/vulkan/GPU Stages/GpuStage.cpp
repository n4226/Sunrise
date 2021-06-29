#include "srpch.h"
#include "GpuStage.h"

#include "Sunrise/Sunrise/graphics/vulkan/renderer/SceneRenderCoordinator.h"

namespace sunrise::gfx {

	GPUStage::GPUStage(SceneRenderCoordinator* coord, std::string&& name)
		: app(coord->app), coord(coord)
#if SR_LOGGING
		,name(name)
#endif
	{

	}

	GPUStage::~GPUStage()
	{

	}

}