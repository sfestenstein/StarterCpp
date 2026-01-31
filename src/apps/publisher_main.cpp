/**
 * @file publisher_main.cpp
 * @brief ZeroMQ publisher that sends sensor data messages via protobuf
 *
 * This application demonstrates:
 * - Using the Timer utility class for periodic events
 * - Serializing data with Protocol Buffers
 * - Publishing messages over ZeroMQ
 * - Using the Logger utility for output
 */

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <atomic>
#include <exception>
#include <random>
#include <string>
#include <thread>

#include <zmq.hpp>

#include "GeneralLogger.h"
#include "Timer.h"
#include "sensor_data.pb.h"

namespace
{
   std::atomic<bool> running{true};

   void signalHandler(int /*signal*/)
   {
      GPINFO("Received shutdown signal");
      running = false;
   }

/**
 * @brief Generate a random sensor reading for demonstration
 */
messages::SensorReading generateSensorReading()
{
   static std::random_device rd;
   static std::mt19937 gen(rd());
   static std::uniform_real_distribution<> tempDist(18.1, 29.1);
   static std::uniform_int_distribution<> qualityDist(85, 100);
   static int messageCount = 0;

   messages::SensorReading reading;
   reading.set_sensor_id("sensor-001");
   reading.set_sensor_name("Temperature Sensor");
   reading.set_value(tempDist(gen));
   reading.set_unit("celsius");
   reading.set_timestamp_ms(
      std::chrono::duration_cast<std::chrono::milliseconds>(
         std::chrono::system_clock::now().time_since_epoch()
      ).count()
   );
   reading.set_quality(qualityDist(gen));
   reading.set_status(messages::SENSOR_STATUS_ONLINE);

   // Add some metadata
   (*reading.mutable_metadata())["message_number"] = std::to_string(++messageCount);
   (*reading.mutable_metadata())["publisher"] = "StarterCpp Demo!";

   // Add location
   auto* location = reading.mutable_location();
   location->set_latitude(37.7749);
   location->set_longitude(-122.4194);
   location->set_altitude(10.0);

   return reading;
}
}

int main(int /*argc*/, char* /*argv*/[])
{
   // Initialize logger
   CommonUtils::GeneralLogger logger;
   logger.init("PublisherLog");

   GPINFO("===========================================");
   GPINFO("StarterCpp ZeroMQ Publisher");
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
      zmq::socket_t socket(context, zmq::socket_type::pub);

      const std::string endpoint = "tcp://localhost:5555";
      socket.bind(endpoint);
      GPINFO("Publisher bound to {}", endpoint);

      // Create a timer that fires every second
      CommonUtils::Timer timer;

      timer.startPeriodic([&socket]()
      {
         if (!running)
         {
            return;
         }

         // Generate sensor data
         const messages::SensorReading reading = generateSensorReading();

         // Serialize to string
         std::string serialized;
         if (!reading.SerializeToString(&serialized))
         {
            GPERROR("Failed to serialize message");
            return;
         }

         // Send the message
         zmq::message_t message(serialized.data(), serialized.size());
         auto result = socket.send(message, zmq::send_flags::none);

         if (result.has_value())
         {
            GPINFO(
               "Published: sensor={}, value={:.2f} {}, quality={}",
               reading.sensor_name(),
               reading.value(),
               reading.unit(),
               reading.quality()
            );
         }
         else
         {
            GPWARN("Failed to send message");
         }
      }, 1000);

      GPINFO("Starting publisher timer (1 second interval)...");
      GPINFO("Press Ctrl+C to stop");
      GPINFO("-------------------------------------------");

      // Main loop - wait for shutdown signal
      while (running)
      {
         std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      // Cleanup
      timer.stop();
      socket.close();
      context.close();

      GPINFO("Publisher shutdown complete");
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
