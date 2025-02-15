#include "srpch.h"

#include "Sunrise/Sunrise/debuging/VkDebugMarker.h"

#define SR_PROFILING 1
#define SR_PROFILING_LITE 0
#define SR_VALIDATION 1

#define SR_MULTI_THREADED_PROFILING 1

#define SR_RenderDocCompatible 0

//#if SR_MULTI_THREADED_PROFILING
//dddd
//#endif

#if SR_PROFILING
#define PROFILE_SCOPE(name) InstrumentationTimer timer##__line__(name);
#define PROFILE_FUNCTION PROFILE_SCOPE(__FUNCSIG__)
#define PROFILE_SCOPE_LEVEL2(name) InstrumentationTimer timer##__line__(name);
#define PROFILE_FUNCTION_LEVEL2 PROFILE_SCOPE_LEVEL2(__FUNCSIG__)
#elif SR_PROFILING_LITE
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION
#define PROFILE_SCOPE_LEVEL2(name) InstrumentationTimer timer##__line__(name);
#define PROFILE_FUNCTION_LEVEL2 PROFILE_SCOPE_LEVEL2(__FUNCSIG__)
#else
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION 
#define PROFILE_SCOPE_LEVEL2(name)
#define PROFILE_FUNCTION_LEVEL2
#endif


#if SR_DEBUG || SR_RELEASE 
#define SR_LOGGING 1
#else
#define SR_LOGGING 0
#endif

#if SR_DEBUG && SR_VALIDATION
#define SR_ENABLE_VK_VALIDATION_LAYERS 1
#define SR_ENABLE_PRECONDITION_CHECKS 1
#define SR_FILEOPEN_CHECKS 1
#else
#define SR_ENABLE_VK_VALIDATION_LAYERS 0
#define SR_ENABLE_PRECONDITION_CHECKS 0
#endif


// testing environment

//enum RenderingModes
//{
//	modeCPU1 = 0,
//	modeCPU2 = 1,
//	modeGPU = 0,
//};

#define RenderModeCPU1 0
#define RenderModeCPU2 1
#define RenderModeGPU  2 

#define RenderMode RenderModeCPU2



#if RenderMode == RenderModeCPU1
#define VK_SUBPASS_INDEX_GBUFFER 0
#define VK_SUBPASS_INDEX_LIGHTING 1
#elif RenderMode == RenderModeCPU2
#define VK_SUBPASS_INDEX_GBUFFER 0
#define VK_SUBPASS_INDEX_LIGHTING 1
#else
#define VK_SUBPASS_INDEX_COMPUTE_PREPASS 0
#define VK_SUBPASS_INDEX_GBUFFER 1
#define VK_SUBPASS_INDEX_LIGHTING 2
#endif

// constants 

/// 100,000
#define maxModelUniformDescriptorArrayCount 100'000

/// 100,000
#define maxMaterialTextureDescriptorArrayCount 100'000
/// normally 10,000
#define FLOATING_ORIGIN_SNAP_DISTANCE 100'000

#define SR_ASSERT(value) assert(value)

// used to be in post build stage

//XCOPY "$(SolutionDir)MeshGenerator\terrain" "$(TargetDir)\terrain\" /S /Y
//XCOPY "$(SolutionDir)MeshGenerator\terrain" "$(ProjectDir)\terrain\" /S /Y

// vulkan debugging
//TODO: ddd
//#if SR_ENABLE_VK_VALIDATION_LAYERS
//#define vkMarker
//#else
//#endif