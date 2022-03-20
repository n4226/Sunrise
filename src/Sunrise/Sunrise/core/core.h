#pragma once

#include "srpch.h"

//https://lists.apple.com/archives/xcode-users/2005/Jul/msg00565.html
// __attribute__((visibility("default")))

#ifdef SR_PLATFORM_WINDOWS
	// SR_DYNAMIC_BUILD does not excist but could be added in future to export required symbols again
	// TODO: fix but now this is requred
	#if 1
		#ifdef SR_BUILD_DLL
			#define SUNRISE_API __declspec(dllexport)
		#else
			//#define SUNRISE_API __declspec(dllimport)
			#define SUNRISE_API
		#endif
	#else
		#define SUNRISE_API
	#endif
#elif defined(SR_PLATFORM_MACOS)
    #ifdef SR_BUILD_DLL
        #define SUNRISE_API __attribute__((visibility("default")))
    #else
        #define SUNRISE_API
    #endif
#else
#error platform not supported
#endif 

#ifdef SR_DEBUG
    #if defined(SR_PLATFORM_WINDOWS)
        #define SR_DEBUGBREAK() __debugbreak()
    #elif defined(SR_PLATFORM_LINUX)
        #include <signal.h>
        #define SR_DEBUGBREAK() raise(SIGTRAP)
    #elif defined(SR_PLATFORM_MACOS)
        #include <signal.h>
        #define SR_DEBUGBREAK() raise(SIGTRAP)
    #else
        #error "Platform doesn't support debugbreak yet!"
    #endif

    #define SR_ENABLE_ASSERTS
#else
#define SR_DEBUGBREAK()
#endif

#define SR_EXPAND_MACRO(x) x
#define SR_STRINGIFY_MACRO(x) #x

namespace sunrise {

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	using Entity = entt::entity;
}

#include "Engine.h"
#include "Log.h"
#include "../debuging/Instrumentor.h"
//#include "Application.h"
