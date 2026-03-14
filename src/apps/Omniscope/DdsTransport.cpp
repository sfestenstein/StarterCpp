#include "DdsTransport.h"

#include "CycloneDDS/CycloneDDSConfig.h"
#include "CycloneDDS/DDSPublisher.h"
#include "CycloneDDS/DDSSubscriber.h"
#include "CommonUtils/GeneralLogger.h"

#include "SensorData.hpp"
#include "TrackData.hpp"

#include <crow/json.h>

#include <mutex>
#include <unordered_set>

namespace Omniscope
{

// ============================================================================
//  JSON serialisation helpers — DDS IDL types → JSON string
// ============================================================================

static crow::json::wvalue toJson(const dds_messages::SensorReading &m)
{
   crow::json::wvalue j;
   j["sensor_id"]    = m.sensor_id();
   j["sensor_name"]  = m.sensor_name();
   j["value"]        = m.value();
   j["unit"]         = m.unit();
   j["timestamp_ms"] = m.timestamp_ms();
   j["quality"]      = m.quality();
   j["status"]       = static_cast<int>(m.status());
   j["latitude"]     = m.latitude();
   j["longitude"]    = m.longitude();
   j["altitude"]     = m.altitude();
   return j;
}

static crow::json::wvalue toJson(const dds_messages::TrackUpdate &m)
{
   crow::json::wvalue j;
   j["track_id"]       = m.track_id();
   j["track_name"]     = m.track_name();
   j["latitude"]       = m.latitude();
   j["longitude"]      = m.longitude();
   j["altitude"]       = m.altitude();
   j["heading"]        = m.heading();
   j["speed"]          = m.speed();
   j["classification"] = static_cast<int>(m.classification());
   j["timestamp_ms"]   = m.timestamp_ms();
   j["update_number"]  = m.update_number();
   j["confidence"]     = m.confidence();
   return j;
}

// ============================================================================
//  JSON deserialisation helpers — JSON string → DDS IDL types (for playback)
// ============================================================================

static dds_messages::SensorReading sensorFromJson(const crow::json::rvalue &d)
{
   dds_messages::SensorReading m;
   m.sensor_id(d["sensor_id"].s());
   m.sensor_name(d["sensor_name"].s());
   m.value(d["value"].d());
   m.unit(d["unit"].s());
   m.timestamp_ms(d["timestamp_ms"].i());
   m.quality(static_cast<int32_t>(d["quality"].i()));
   m.status(static_cast<dds_messages::SensorStatus>(d["status"].i()));
   m.latitude(d["latitude"].d());
   m.longitude(d["longitude"].d());
   m.altitude(d["altitude"].d());
   return m;
}

static dds_messages::TrackUpdate trackFromJson(const crow::json::rvalue &d)
{
   dds_messages::TrackUpdate m;
   m.track_id(d["track_id"].s());
   m.track_name(d["track_name"].s());
   m.latitude(d["latitude"].d());
   m.longitude(d["longitude"].d());
   m.altitude(d["altitude"].d());
   m.heading(d["heading"].d());
   m.speed(d["speed"].d());
   m.classification(
      static_cast<dds_messages::TrackClassification>(d["classification"].i()));
   m.timestamp_ms(d["timestamp_ms"].i());
   m.update_number(static_cast<int32_t>(d["update_number"].i()));
   m.confidence(d["confidence"].d());
   return m;
}

// ============================================================================
//  DdsTransport::Impl
// ============================================================================

struct DdsTransport::Impl
{
   explicit Impl(uint32_t domainId)
      : domainId(domainId)
   {
      ddsConfig.defaultInitialize();
      const auto &config = ddsConfig.config();

      sensorPub = std::make_unique<
         CycloneDDS::DDSPublisher<dds_messages::SensorReading>>(
         domainId, config, "MonitorSensorPub");
      trackPub = std::make_unique<
         CycloneDDS::DDSPublisher<dds_messages::TrackUpdate>>(
         domainId, config, "MonitorTrackPub");

      sensorSub = std::make_unique<
         CycloneDDS::DDSSubscriber<dds_messages::SensorReading>>(
         domainId, config, "MonitorSensor");
      trackSub = std::make_unique<
         CycloneDDS::DDSSubscriber<dds_messages::TrackUpdate>>(
         domainId, config, "MonitorTrack");
   }

   uint32_t domainId;
   CycloneDDS::CycloneDDSConfig ddsConfig;

   std::unique_ptr<CycloneDDS::DDSPublisher<dds_messages::SensorReading>> sensorPub;
   std::unique_ptr<CycloneDDS::DDSPublisher<dds_messages::TrackUpdate>> trackPub;
   std::unique_ptr<CycloneDDS::DDSSubscriber<dds_messages::SensorReading>> sensorSub;
   std::unique_ptr<CycloneDDS::DDSSubscriber<dds_messages::TrackUpdate>> trackSub;

   std::unordered_set<std::string> activeTopics;
   mutable std::mutex mutex;
};

// ============================================================================
//  DdsTransport public API
// ============================================================================

DdsTransport::DdsTransport(uint32_t domainId)
   : _impl(std::make_unique<Impl>(domainId))
{
   GPINFO("DdsTransport: created on domain {}", domainId);
}

DdsTransport::~DdsTransport()
{
   // Stop all active subscriptions
   for (const auto &topic : _impl->activeTopics)
   {
      if (topic == CycloneDDS::SENSOR_TOPIC) _impl->sensorSub->stop();
      else if (topic == CycloneDDS::TRACK_TOPIC) _impl->trackSub->stop();
   }
}

std::string DdsTransport::name() const
{
   return "DDS";
}

std::vector<std::string> DdsTransport::topicNames() const
{
   return {std::string(CycloneDDS::SENSOR_TOPIC),
           std::string(CycloneDDS::TRACK_TOPIC)};
}

void DdsTransport::subscribe(const std::string &topic, MessageCallback callback)
{
   std::lock_guard lock(_impl->mutex);
   if (_impl->activeTopics.contains(topic)) return;

   if (topic == CycloneDDS::SENSOR_TOPIC && _impl->sensorSub)
   {
      _impl->sensorSub->subscribe(std::string(CycloneDDS::SENSOR_TOPIC),
         [callback](const dds_messages::SensorReading &msg) {
            callback(std::string(CycloneDDS::SENSOR_TOPIC), toJson(msg).dump());
         });
      _impl->sensorSub->start();
      _impl->activeTopics.insert(topic);
   }
   else if (topic == CycloneDDS::TRACK_TOPIC && _impl->trackSub)
   {
      _impl->trackSub->subscribe(std::string(CycloneDDS::TRACK_TOPIC),
         [callback](const dds_messages::TrackUpdate &msg) {
            callback(std::string(CycloneDDS::TRACK_TOPIC), toJson(msg).dump());
         });
      _impl->trackSub->start();
      _impl->activeTopics.insert(topic);
   }

   GPINFO("DdsTransport: subscribed to {}", topic);
}

void DdsTransport::unsubscribe(const std::string &topic)
{
   std::lock_guard lock(_impl->mutex);
   if (!_impl->activeTopics.contains(topic)) return;

   if (topic == CycloneDDS::SENSOR_TOPIC && _impl->sensorSub)
   {
      _impl->sensorSub->stop();
      _impl->activeTopics.erase(topic);
   }
   else if (topic == CycloneDDS::TRACK_TOPIC && _impl->trackSub)
   {
      _impl->trackSub->stop();
      _impl->activeTopics.erase(topic);
   }

   GPINFO("DdsTransport: unsubscribed from {}", topic);
}

bool DdsTransport::isSubscribed(const std::string &topic) const
{
   std::lock_guard lock(_impl->mutex);
   return _impl->activeTopics.contains(topic);
}

void DdsTransport::publishFromJson(const std::string &topic,
                                   const std::string &jsonData)
{
   auto data = crow::json::load(jsonData);
   if (!data) return;

   if (topic == CycloneDDS::SENSOR_TOPIC && _impl->sensorPub)
   {
      _impl->sensorPub->publish(std::string(CycloneDDS::SENSOR_TOPIC),
                                sensorFromJson(data));
   }
   else if (topic == CycloneDDS::TRACK_TOPIC && _impl->trackPub)
   {
      _impl->trackPub->publish(std::string(CycloneDDS::TRACK_TOPIC),
                               trackFromJson(data));
   }
}

} // namespace Omniscope
