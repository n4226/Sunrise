#include "srpch.h"

#include "Sunrise/Sunrise/debuging/VkDebugMarker.h"

#define SR_PROFILING 1
#define SR_PROFILING_LITE 0
#define SR_VALIDATION 1

#define SR_MULTI_THREADED_PROFILING 1

#define SR_RenderDocCompatible 1 && SR_DEBUG
#define SR_SingleQueueForRenderDoc 0 && SR_DEBUG

// will not build without modifications
#define SR_SingleQueueForRenderDoc_onlyCreateOne 0

//#if SR_MULTI_THREADED_PROFILING
//dddd
//#endif

#if defined(__FUNCSIG__)
     #define SR_FUNC_SIG __FUNCSIG__
 #elif defined(__PRETTY_FUNCTION__)
     #define SR_FUNC_SIG __PRETTY_FUNCTION__
 #else
     #define SR_FUNC_SIG __FUNCTION__
 #endif

#if SR_PROFILING && SR_DEBUG
#define PROFILE_SCOPE(name) InstrumentationTimer timer##__line__(name);
#define PROFILE_FUNCTION PROFILE_SCOPE(SR_FUNC_SIG)
#define PROFILE_SCOPE_LEVEL2(name) InstrumentationTimer timer##__line__(name);
#define PROFILE_FUNCTION_LEVEL2 PROFILE_SCOPE_LEVEL2(SR_FUNC_SIG)
#elif SR_PROFILING_LITE
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION
#define PROFILE_SCOPE_LEVEL2(name) InstrumentationTimer timer##__line__(name);
#define PROFILE_FUNCTION_LEVEL2 PROFILE_SCOPE_LEVEL2(SR_FUNC_SIG)
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
#define SR_ENABLE_VK_DEBUG 1
#define SR_ENABLE_PRECONDITION_CHECKS 1
#define SR_FILEOPEN_CHECKS 1
#define SR_VK_OBJECT_NAMES 1
#else
#define SR_ENABLE_VK_VALIDATION_LAYERS 0
#define SR_ENABLE_VK_DEBUG 0
#define SR_ENABLE_PRECONDITION_CHECKS 0
#define SR_FILEOPEN_CHECKS 0
#define SR_VK_OBJECT_NAMES 0
#endif

// for windows rutime stuff e.g static or dynamiclyh link to c++ runtime:
// good reasource for that: https://stackoverflow.com/questions/3007312/resolving-lnk4098-defaultlib-msvcrt-conflicts-with

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
#define VK_GPUDriven 1
#endif

// constants 

/// 100,000
#define maxModelUniformDescriptorArrayCount 100'000

/// 100,000
#define maxMaterialTextureDescriptorArrayCount 100'000
/// normally 10,000
#define FLOATING_ORIGIN_SNAP_DISTANCE 100'000

#define SR_ASSERT(value) assert(value)
#define SR_CORE_ASSERT(value) assert(value)

// used to be in post build stage

//XCOPY "$(SolutionDir)MeshGenerator\terrain" "$(TargetDir)\terrain\" /S /Y
//XCOPY "$(SolutionDir)MeshGenerator\terrain" "$(ProjectDir)\terrain\" /S /Y

// vulkan debugging
//TODO: ddd
//#if SR_ENABLE_VK_VALIDATION_LAYERS
//#define vkMarker
//#else
//#endif

/*

Good git restore explanation: (this is seperate I will find a place to put this shortly)
https://stackoverflow.com/questions/58003030/what-is-the-git-restore-command-and-what-is-the-difference-between-git-restor
*/

//vulkan constants

#if SR_PLATFORM_WINDOWS
#define SR_USE_AFTERMATH
#endif
