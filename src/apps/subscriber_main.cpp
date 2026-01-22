/**
 * @file subscriber_main.cpp
 * @brief ZeroMQ subscriber that receives and displays sensor data messages
 *
 * This application demonstrates:
 * - Subscribing to ZeroMQ messages
 * - Deserializing Protocol Buffer messages
 * - Using the Logger utility for formatted output
 */

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <atomic>
#include <iomanip>
#include <sstream>

#include <zmq.hpp>

#include "utils/Logger.hpp"
#include "sensor_data.pb.h"

namespace
{
   std::atomic<bool> g_running{true};

   void signalHandler(int signal)
   {
      utils::Logger::info("Received signal {}, shutting down...", signal);
      g_running = false;
   }

   /**
    * @brief Format a timestamp for display
    */
   std::string formatTimestamp(int64_t timestampMs)
   {
      auto time = std::chrono::system_clock::time_point(
         std::chrono::milliseconds(timestampMs)
      );
      auto timeT = std::chrono::system_clock::to_time_t(time);

      std::stringstream ss;
      ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");

      // Add milliseconds
      auto ms = timestampMs % 1000;
      ss << "." << std::setfill('0') << std::setw(3) << ms;

      return ss.str();
   }

   /**
    * @brief Get string representation of sensor status
    */
   std::string sensorStatusToString(messages::SensorStatus status)
   {
      switch (status)
      {
         case messages::SENSOR_STATUS_ONLINE:      return "ONLINE";
         case messages::SENSOR_STATUS_OFFLINE:     return "OFFLINE";
         case messages::SENSOR_STATUS_ERROR:       return "ERROR";
         case messages::SENSOR_STATUS_CALIBRATING: return "CALIBRATING";
         case messages::SENSOR_STATUS_MAINTENANCE: return "MAINTENANCE";
         default:                                  return "UNKNOWN";
      }
   }
}

int main(int argc, char* argv[])
{
   // Initialize logger
   utils::Logger::init("Subscriber", utils::LogLevel::Debug);

   utils::Logger::info("===========================================");
   utils::Logger::info("StarterCpp ZeroMQ Subscriber");
   utils::Logger::info("===========================================");

   // Setup signal handling for graceful shutdown
   std::signal(SIGINT, signalHandler);
   std::signal(SIGTERM, signalHandler);

   try
   {
      // Create ZeroMQ context and socket
      zmq::context_t context(1);
      zmq::socket_t socket(context, zmq::socket_type::sub);

      const std::string endpoint = "tcp://localhost:5555";
      socket.connect(endpoint);

      // Subscribe to all messages (empty filter)
      socket.set(zmq::sockopt::subscribe, "");

      // Set receive timeout so we can check for shutdown
      socket.set(zmq::sockopt::rcvtimeo, 500);

      utils::Logger::info("Subscriber connected to {}", endpoint);
      utils::Logger::info("Waiting for messages...");
      utils::Logger::info("Press Ctrl+C to stop");
      utils::Logger::info("-------------------------------------------");

      int messageCount = 0;

      while (g_running)
      {
         zmq::message_t message;
         auto result = socket.recv(message, zmq::recv_flags::none);

         if (!result.has_value())
         {
            // Timeout - no message received, loop again
            continue;
         }

         // Deserialize the protobuf message
         messages::SensorReading reading;
         if (!reading.ParseFromArray(message.data(), static_cast<int>(message.size())))
         {
            utils::Logger::warn("Failed to parse received message");
            continue;
         }

         ++messageCount;

         // Log the received data using spdlog
         utils::Logger::info("-------------------------------------------");
         utils::Logger::info("Message #{} received", messageCount);
         utils::Logger::info("  Sensor ID:   {}", reading.sensor_id());
         utils::Logger::info("  Sensor Name: {}", reading.sensor_name());
         utils::Logger::info("  Value:       {:.2f} {}", reading.value(), reading.unit());
         utils::Logger::info("  Quality:     {}%", reading.quality());
         utils::Logger::info("  Status:      {}", sensorStatusToString(reading.status()));
         utils::Logger::info("  Timestamp:   {}", formatTimestamp(reading.timestamp_ms()));

         // Log location if present
         if (reading.has_location())
         {
            const auto& loc = reading.location();
            utils::Logger::info("  Location:    ({:.4f}, {:.4f}) alt={:.1f}m",
               loc.latitude(), loc.longitude(), loc.altitude());
         }

         // Log metadata
         if (reading.metadata_size() > 0)
         {
            utils::Logger::debug("  Metadata:");
            for (const auto& [key, value] : reading.metadata())
            {
               utils::Logger::debug("    {}: {}", key, value);
            }
         }
      }

      // Cleanup
      socket.close();
      context.close();

      utils::Logger::info("-------------------------------------------");
      utils::Logger::info("Total messages received: {}", messageCount);
      utils::Logger::info("Subscriber shutdown complete");
   }
   catch (const zmq::error_t& e)
   {
      utils::Logger::error("ZeroMQ error: {}", e.what());
      return EXIT_FAILURE;
   }
   catch (const std::exception& e)
   {
      utils::Logger::error("Error: {}", e.what());
      return EXIT_FAILURE;
   }

   utils::Logger::shutdown();
   return EXIT_SUCCESS;
}
