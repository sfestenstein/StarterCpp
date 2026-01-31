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
#include <ctime>
#include <atomic>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstdint>
#include <exception>

#include <zmq.hpp>

#include "GeneralLogger.h"
#include "sensor_data.pb.h"

namespace
{
   std::atomic<bool> running{true};

   void signalHandler(int /*signal*/)
   {
      GPINFO("SubscriberLog");
      running = false;
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
      std::tm tmBuf{};
      ss << std::put_time(localtime_r(&timeT, &tmBuf), "%Y-%m-%d %H:%M:%S");

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

int main(int /*argc*/, char* /*argv*/[])
{
   // Initialize logger
   CommonUtils::GeneralLogger logger;
   logger.init("SubscriberLog");

   GPINFO("===========================================");
   GPINFO("StarterCpp ZeroMQ Subscriber");
   GPINFO("===========================================");

   // Setup signal handling for graceful shutdown
   if (std::signal(SIGINT, signalHandler) == SIG_ERR)
   {
      GPERROR("Failed to set SIGINT handler");
      return EXIT_FAILURE;
   }
   if (std::signal(SIGTERM, signalHandler) == SIG_ERR)
   {
      GPERROR("Failed to set SIGTERM handler");
      return EXIT_FAILURE;
   }

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

      GPINFO("Subscriber connected to {}", endpoint);
      GPINFO("Waiting for messages...");
      GPINFO("Press Ctrl+C to stop");
      GPINFO("-------------------------------------------");

      int messageCount = 0;

      while (running)
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
            GPWARN("Failed to parse received message");
            continue;
         }

         ++messageCount;

         // Log the received data using spdlog
         GPINFO("-------------------------------------------");
         GPINFO("Message #{} received", messageCount);
         GPINFO("  Sensor ID:   {}", reading.sensor_id());
         GPINFO("  Sensor Name: {}", reading.sensor_name());
         GPINFO("  Value:       {:.2f} {}", reading.value(), reading.unit());
         GPINFO("  Quality:     {}%", reading.quality());
         GPINFO("  Status:      {}", sensorStatusToString(reading.status()));
         GPINFO("  Timestamp:   {}", formatTimestamp(reading.timestamp_ms()));

         // Log location if present
         if (reading.has_location())
         {
            const auto& loc = reading.location();
            GPINFO("  Location:    ({:.4f}, {:.4f}) alt={:.1f}m",
               loc.latitude(), loc.longitude(), loc.altitude());
         }

         // Log metadata
         if (reading.metadata_size() > 0)
         {
            GPDEBUG("  Metadata:");
            for (const auto& [key, value] : reading.metadata())
            {
               GPDEBUG("    {}: {}", key, value);
            }
         }
      }

      // Cleanup
      socket.close();
      context.close();

      GPINFO("-------------------------------------------");
      GPINFO("Total messages received: {}", messageCount);
      GPINFO("Subscriber shutdown complete");
   }
   catch (const zmq::error_t& e)
   {
      GPERROR("ZeroMQ error: {}", e.what());
      return EXIT_FAILURE;
   }
   catch (const std::exception& e)
   {
      GPERROR("Error: {}", e.what());
      return EXIT_FAILURE;
   }

   return EXIT_SUCCESS;
}
