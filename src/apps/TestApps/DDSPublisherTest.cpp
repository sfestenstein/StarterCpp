/**
 * @file DDSPublisherTest.cpp
 * @brief Test application for the DDS publisher.
 *
 * Publishes SensorReading and TrackUpdate samples on DDS topics.
 */

#include "CycloneDDS/CycloneDDSConfig.h"
#include "CycloneDDS/DDSPublisher.h"
#include "CommonUtils/GeneralLogger.h"

#include "SensorData.hpp"
#include "TrackData.hpp"
#include "Command.hpp"

#include <chrono>
#include <csignal>
#include <iostream>
#include <random>
#include <thread>

static std::atomic<bool> s_running{true};

void signalHandler(int)
{
   s_running.store(false);
}

int main(int argc, char *argv[])
{
   std::signal(SIGINT, signalHandler);
   std::signal(SIGTERM, signalHandler);

   CommonUtils::GeneralLogger logger;
   logger.init("DDSPublisherTest");

   uint32_t domainId = 0;
   if (argc > 1)
   {
      domainId = static_cast<uint32_t>(std::stoul(argv[1]));
   }
   GPINFO("Using DDS domain ID: {}", domainId);

   CycloneDDS::CycloneDDSConfig defaults;
   defaults.defaultInitialize();
   const auto &config = defaults.config();

   // Create typed publishers using the shared config
   CycloneDDS::DDSPublisher<dds_messages::SensorReading> sensorPub(domainId, config, "SensorPublisher");
   CycloneDDS::DDSPublisher<dds_messages::TrackUpdate> trackPub(domainId, config, "TrackPublisher");

   std::random_device rd;
   std::mt19937 gen(rd());
   std::uniform_real_distribution<> tempDist(20.0, 35.0);
   std::uniform_real_distribution<> latDist(37.0, 38.0);
   std::uniform_real_distribution<> lonDist(-123.0, -122.0);
   std::uniform_real_distribution<> headingDist(0.0, 360.0);
   std::uniform_real_distribution<> speedDist(0.0, 100.0);

   int sequence = 0;

   while (s_running.load())
   {
      auto now = std::chrono::system_clock::now();
      auto epochMs = std::chrono::duration_cast<std::chrono::milliseconds>(
         now.time_since_epoch()).count();

      // Publish a sensor reading
      dds_messages::SensorReading sensor;
      sensor.sensor_id("sensor-001");
      sensor.sensor_name("Temperature Sensor");
      sensor.value(tempDist(gen));
      sensor.unit("celsius");
      sensor.timestamp_ms(epochMs);
      sensor.quality(95);
      sensor.status(dds_messages::SensorStatus::SENSOR_ONLINE);
      sensor.latitude(37.7749);
      sensor.longitude(-122.4194);
      sensor.altitude(10.0);

      sensorPub.publish("SensorTopic", sensor);

      // Publish a track update
      dds_messages::TrackUpdate track;
      track.track_id("track-alpha");
      track.track_name("Aircraft Alpha");
      track.latitude(latDist(gen));
      track.longitude(lonDist(gen));
      track.altitude(10000.0);
      track.heading(headingDist(gen));
      track.speed(speedDist(gen));
      track.classification(dds_messages::TrackClassification::TRACK_FRIENDLY);
      track.timestamp_ms(epochMs);
      track.update_number(sequence);
      track.confidence(0.95);

      trackPub.publish("TrackTopic", track);

      ++sequence;
      if (sequence % 100 == 0)
      {
         GPINFO("Published {} sensor + track samples", sequence);
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
   }

   GPINFO("DDSPublisherTest shutting down after {} messages", sequence);
   return 0;
}
