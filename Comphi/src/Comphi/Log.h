#pragma once
#include "Comphi/Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_color_sinks.h"

//CORE
#define COMPHI_LOG_CORE_FATAL(...) Comphi::Log::GetCoreLogger()->critical(__VA_ARGS__)
#define COMPHI_LOG_CORE_ERROR(...) Comphi::Log::GetCoreLogger()->error(__VA_ARGS__)
#define COMPHI_LOG_CORE_WARN(...) Comphi::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define COMPHI_LOG_CORE_INFO(...) Comphi::Log::GetCoreLogger()->info(__VA_ARGS__)
#define COMPHI_LOG_CORE_TRACE(...) Comphi::Log::GetCoreLogger()->trace(__VA_ARGS__)
//CLIENT
#define COMPHI_FATAL(...) Comphi::Log::GetClientLogger()->critical(__VA_ARGS__)
#define COMPHI_ERROR(...) Comphi::Log::GetClientLogger()->error(__VA_ARGS__)
#define COMPHI_WARN(...) Comphi::Log::GetClientLogger()->warn(__VA_ARGS__)
#define COMPHI_INFO(...) Comphi::Log::GetClientLogger()->info(__VA_ARGS__)
#define COMPHI_TRACE(...) Comphi::Log::GetClientLogger()->trace(__VA_ARGS__)

namespace Comphi {
	class CPHI_API Log
	{
	public:
		static void Init();

		static inline std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; };
		static inline std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; };
		static inline void AssertN(bool core, bool b, std::string errmsg) { };

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;	
		
	};

}



