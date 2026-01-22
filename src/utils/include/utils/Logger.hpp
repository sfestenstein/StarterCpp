#pragma once
/**
 * @file Logger.hpp
 * @brief Convenient wrapper around spdlog for simplified logging
 *
 * This class provides a simplified interface for application logging,
 * wrapping spdlog functionality with easy-to-use static methods and
 * configurable log levels.
 */

#include <memory>
#include <string>
#include <string_view>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace utils
{

/**
 * @brief Log levels matching spdlog levels
 */
enum class LogLevel
{
   Trace,
   Debug,
   Info,
   Warn,
   Error,
   Critical,
   Off
};

/**
 * @brief Singleton logger wrapper around spdlog
 *
 * Provides a simplified interface for logging with automatic initialization,
 * configurable log levels, and support for both console and file output.
 *
 * @example
 * @code
 * Logger::init("MyApp", LogLevel::Debug);
 * Logger::info("Application started");
 * Logger::error("Error code: {}", errorCode);
 * @endcode
 */
class Logger
{
public:
   /**
    * @brief Initialize the logger with the given name and level
    * @param name Logger name (appears in log output)
    * @param level Minimum log level to output
    * @param logToFile If true, also log to a file
    * @param filePath Path to log file (if logToFile is true)
    */
   static void init(
      std::string_view name = "App",
      LogLevel level = LogLevel::Info,
      bool logToFile = false,
      std::string_view filePath = "app.log"
   );

   /**
    * @brief Shutdown the logger and flush all pending messages
    */
   static void shutdown();

   /**
    * @brief Set the minimum log level
    * @param level New minimum log level
    */
   static void setLevel(LogLevel level);

   /**
    * @brief Get the current log level
    * @return Current minimum log level
    */
   static LogLevel getLevel();

   /**
    * @brief Log a trace message
    * @tparam Args Format argument types
    * @param fmt Format string
    * @param args Format arguments
    */
   template<typename... Args>
   static void trace(spdlog::format_string_t<Args...> fmt, Args&&... args)
   {
      getLogger()->trace(fmt, std::forward<Args>(args)...);
   }

   /**
    * @brief Log a debug message
    */
   template<typename... Args>
   static void debug(spdlog::format_string_t<Args...> fmt, Args&&... args)
   {
      getLogger()->debug(fmt, std::forward<Args>(args)...);
   }

   /**
    * @brief Log an info message
    */
   template<typename... Args>
   static void info(spdlog::format_string_t<Args...> fmt, Args&&... args)
   {
      getLogger()->info(fmt, std::forward<Args>(args)...);
   }

   /**
    * @brief Log a warning message
    */
   template<typename... Args>
   static void warn(spdlog::format_string_t<Args...> fmt, Args&&... args)
   {
      getLogger()->warn(fmt, std::forward<Args>(args)...);
   }

   /**
    * @brief Log an error message
    */
   template<typename... Args>
   static void error(spdlog::format_string_t<Args...> fmt, Args&&... args)
   {
      getLogger()->error(fmt, std::forward<Args>(args)...);
   }

   /**
    * @brief Log a critical message
    */
   template<typename... Args>
   static void critical(spdlog::format_string_t<Args...> fmt, Args&&... args)
   {
      getLogger()->critical(fmt, std::forward<Args>(args)...);
   }

   /**
    * @brief Flush all pending log messages
    */
   static void flush();

private:
   Logger() = default;

   static std::shared_ptr<spdlog::logger>& getLogger();
   static spdlog::level::level_enum toSpdlogLevel(LogLevel level);
   static LogLevel fromSpdlogLevel(spdlog::level::level_enum level);

   static std::shared_ptr<spdlog::logger> s_logger;
   static bool s_initialized;
};

} // namespace utils
