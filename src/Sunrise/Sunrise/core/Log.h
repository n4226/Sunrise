#pragma once

#include "srpch.h"

#include "core.h"

// This ignore	s all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Sunrise {

	class SUNRISE_API Log
	{
	public:
		static void Init();

		Log();
		~Log();

		static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:

		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
	};

}

#if defined SR_DEBUG  || defined SR_RELEASE
// Core log macros
#define SR_CORE_TRACE(...)    ::Sunrise::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define SR_CORE_INFO(...)     ::Sunrise::Log::GetCoreLogger()->info(__VA_ARGS__)
#define SR_CORE_WARN(...)     ::Sunrise::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define SR_CORE_ERROR(...)    ::Sunrise::Log::GetCoreLogger()->error(__VA_ARGS__)
#define SR_CORE_CRITICAL(...) ::Sunrise::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define SR_TRACE(...)         ::Sunrise::Log::GetClientLogger()->trace(__VA_ARGS__)
#define SR_INFO(...)          ::Sunrise::Log::GetClientLogger()->info(__VA_ARGS__)
#define SR_WARN(...)          ::Sunrise::Log::GetClientLogger()->warn(__VA_ARGS__)
#define SR_ERROR(...)         ::Sunrise::Log::GetClientLogger()->error(__VA_ARGS__)
#define SR_CRITICAL(...)      ::Sunrise::Log::GetClientLogger()->critical(__VA_ARGS__)

#else

// Core log macros
#define SR_CORE_TRACE(...)   
#define SR_CORE_INFO(...)    
#define SR_CORE_WARN(...)    
#define SR_CORE_ERROR(...)   
#define SR_CORE_CRITICAL(...)

// Client log macros
#define HZ_TRACE(...)        
#define HZ_INFO(...)         
#define HZ_WARN(...)         
#define HZ_ERROR(...)        
#define HZ_CRITICAL(...)     


#endif