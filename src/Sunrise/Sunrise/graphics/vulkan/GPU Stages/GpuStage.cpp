#include "srpch.h"
#include "GpuStage.h"

namespace sunrise::gfx {

	GPUStage::GPUStage(Application& app,std::string&& name)
		: app(app)
#if SR_LOGGING
		,name(name)
#endif
	{

	}

	GPUStage::~GPUStage()
	{

	}

}