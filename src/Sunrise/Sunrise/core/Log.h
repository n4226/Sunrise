#pragma once

#include "srpch.h"

#include "core.h"

// This ignore	s all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

/* Some spdlog errrors I have come accross

	   _Check_C_return(_Mtx_lock(_Mymtx())); -- acces violation --- see this issue: https://github.com/gabime/spdlog/issues/633 
		tried uncommenting out disable #define SPDLOG_NO_THREAD_ID which is close to what was suggested becuase exact define did not exist in file anymore
*/

namespace sunrise {

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

#if defined SR_DEBUG || defined SR_RELEASE
// Core log macros
#define SR_CORE_TRACE(...)    ::sunrise::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define SR_CORE_INFO(...)     ::sunrise::Log::GetCoreLogger()->info(__VA_ARGS__)
#define SR_CORE_WARN(...)     ::sunrise::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define SR_CORE_ERROR(...)    ::sunrise::Log::GetCoreLogger()->error(__VA_ARGS__)
#define SR_CORE_CRITICAL(...) ::sunrise::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define SR_TRACE(...)         ::sunrise::Log::GetClientLogger()->trace(__VA_ARGS__)
#define SR_INFO(...)          ::sunrise::Log::GetClientLogger()->info(__VA_ARGS__)
#define SR_WARN(...)          ::sunrise::Log::GetClientLogger()->warn(__VA_ARGS__)
#define SR_ERROR(...)         ::sunrise::Log::GetClientLogger()->error(__VA_ARGS__)
#define SR_CRITICAL(...)      ::sunrise::Log::GetClientLogger()->critical(__VA_ARGS__)

#else

// Core log macros
#define SR_CORE_TRACE(...)   
#define SR_CORE_INFO(...)    
#define SR_CORE_WARN(...)    
#define SR_CORE_ERROR(...)   
#define SR_CORE_CRITICAL(...)

// Client log macros
#define SR_TRACE(...)        
#define SR_INFO(...)         
#define SR_WARN(...)         
#define SR_ERROR(...)        
#define SR_CRITICAL(...)     


#endif