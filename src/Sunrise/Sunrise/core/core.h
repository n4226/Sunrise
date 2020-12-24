#pragma once

#include "srpch.h"

#ifdef SR_PLATFORM_WINDOWS
	#ifdef SR_BUILD_DLL
		#define SUNRISE_API __declspec(dllexport)
	#else
		#define SUNRISE_API __declspec(dllimport)
	#endif
#else
#error Windows is currently the only supported platform
#endif 

#ifdef SR_DEBUG
#if defined(SR_PLATFORM_WINDOWS)
#define SR_DEBUGBREAK() __debugbreak()
#elif defined(SR_PLATFORM_LINUX)
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

namespace Sunrise {

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

}

#include "Engine.h"
#include "Log.h"
//#include "Application.h"