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
#include <random>

#include <zmq.hpp>

#include "utils/Logger.hpp"
#include "utils/Timer.hpp"
#include "sensor_data.pb.h"

namespace
{
   std::atomic<bool> g_running{true};

   void signalHandler(int signal)
   {
      utils::Logger::info("Received signal {}, shutting down...", signal);
      g_running = false;
   }
}

/**
 * @brief Generate a random sensor reading for demonstration
 */
messages::SensorReading generateSensorReading()
{
   static std::random_device rd;
   static std::mt19937 gen(rd());
   static std::uniform_real_distribution<> tempDist(18.0, 28.0);
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
   (*reading.mutable_metadata())["publisher"] = "StarterCpp Demo";

   // Add location
   auto* location = reading.mutable_location();
   location->set_latitude(37.7749);
   location->set_longitude(-122.4194);
   location->set_altitude(10.0);

   return reading;
}

int main(int argc, char* argv[])
{
   // Initialize logger
   utils::Logger::init("Publisher", utils::LogLevel::Debug);

   utils::Logger::info("===========================================");
   utils::Logger::info("StarterCpp ZeroMQ Publisher");
   utils::Logger::info("===========================================");

   // Setup signal handling for graceful shutdown
   std::signal(SIGINT, signalHandler);
   std::signal(SIGTERM, signalHandler);

   try
   {
      // Create ZeroMQ context and socket
      zmq::context_t context(1);
      zmq::socket_t socket(context, zmq::socket_type::pub);

      const std::string endpoint = "tcp://localhost:5555";
      socket.bind(endpoint);
      utils::Logger::info("Publisher bound to {}", endpoint);

      // Create a timer that fires every second
      utils::Timer timer;
      timer.setInterval(std::chrono::seconds(1));

      timer.setCallback([&socket]()
      {
         if (!g_running)
         {
            return;
         }

         // Generate sensor data
         messages::SensorReading reading = generateSensorReading();

         // Serialize to string
         std::string serialized;
         if (!reading.SerializeToString(&serialized))
         {
            utils::Logger::error("Failed to serialize message");
            return;
         }

         // Send the message
         zmq::message_t message(serialized.data(), serialized.size());
         auto result = socket.send(message, zmq::send_flags::none);

         if (result.has_value())
         {
            utils::Logger::info(
               "Published: sensor={}, value={:.2f} {}, quality={}",
               reading.sensor_name(),
               reading.value(),
               reading.unit(),
               reading.quality()
            );
         }
         else
         {
            utils::Logger::warn("Failed to send message");
         }
      });

      utils::Logger::info("Starting publisher timer (1 second interval)...");
      utils::Logger::info("Press Ctrl+C to stop");
      utils::Logger::info("-------------------------------------------");

      timer.start();

      // Main loop - wait for shutdown signal
      while (g_running)
      {
         std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      // Cleanup
      timer.stop();
      socket.close();
      context.close();

      utils::Logger::info("Publisher shutdown complete");
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
