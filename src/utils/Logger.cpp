/**
 * @file Logger.cpp
 * @brief Implementation of the Logger wrapper class
 */

#include "utils/Logger.hpp"

#include <vector>

namespace utils
{

std::shared_ptr<spdlog::logger> Logger::s_logger = nullptr;
bool Logger::s_initialized = false;

void Logger::init(
   std::string_view name,
   LogLevel level,
   bool logToFile,
   std::string_view filePath
)
{
   if (s_initialized)
   {
      return;
   }

   std::vector<spdlog::sink_ptr> sinks;

   // Console sink with colors
   auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
   consoleSink->set_level(spdlog::level::trace);
   consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
   sinks.push_back(consoleSink);

   // File sink (optional)
   if (logToFile)
   {
      auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
         std::string(filePath), true);
      fileSink->set_level(spdlog::level::trace);
      fileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v");
      sinks.push_back(fileSink);
   }

   // Create logger with multiple sinks
   s_logger = std::make_shared<spdlog::logger>(
      std::string(name), sinks.begin(), sinks.end());

   s_logger->set_level(toSpdlogLevel(level));
   s_logger->flush_on(spdlog::level::warn);

   // Register as default logger
   spdlog::set_default_logger(s_logger);

   s_initialized = true;
}

void Logger::shutdown()
{
   if (s_logger)
   {
      s_logger->flush();
   }
   spdlog::shutdown();
   s_logger.reset();
   s_initialized = false;
}

void Logger::setLevel(LogLevel level)
{
   getLogger()->set_level(toSpdlogLevel(level));
}

LogLevel Logger::getLevel()
{
   return fromSpdlogLevel(getLogger()->level());
}

void Logger::flush()
{
   getLogger()->flush();
}

std::shared_ptr<spdlog::logger>& Logger::getLogger()
{
   if (!s_initialized)
   {
      init();
   }
   return s_logger;
}

spdlog::level::level_enum Logger::toSpdlogLevel(LogLevel level)
{
   switch (level)
   {
      case LogLevel::Trace:    return spdlog::level::trace;
      case LogLevel::Debug:    return spdlog::level::debug;
      case LogLevel::Info:     return spdlog::level::info;
      case LogLevel::Warn:     return spdlog::level::warn;
      case LogLevel::Error:    return spdlog::level::err;
      case LogLevel::Critical: return spdlog::level::critical;
      case LogLevel::Off:      return spdlog::level::off;
      default:                 return spdlog::level::info;
   }
}

LogLevel Logger::fromSpdlogLevel(spdlog::level::level_enum level)
{
   switch (level)
   {
      case spdlog::level::trace:    return LogLevel::Trace;
      case spdlog::level::debug:    return LogLevel::Debug;
      case spdlog::level::info:     return LogLevel::Info;
      case spdlog::level::warn:     return LogLevel::Warn;
      case spdlog::level::err:      return LogLevel::Error;
      case spdlog::level::critical: return LogLevel::Critical;
      case spdlog::level::off:      return LogLevel::Off;
      default:                      return LogLevel::Info;
   }
}

} // namespace utils
