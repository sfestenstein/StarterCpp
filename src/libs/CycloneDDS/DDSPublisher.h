#ifndef DDSPUBLISHER_H_
#define DDSPUBLISHER_H_

// Project headers
#include "CommonUtils/GeneralLogger.h"
#include "CycloneDDS/DDSTopicConfig.h"

// Cyclone DDS C++ headers
#include <dds/dds.hpp>

// System headers
#include <memory>
#include <string>
#include <unordered_map>

namespace CycloneDDS
{

/**
 * @brief Generic DDS topic-based publisher using Cyclone DDS.
 *
 * Constructed with a DDSTopicConfig that defines every allowed topic and
 * its QoS policies.  When publish() is called, the writer QoS is looked
 * up from the config automatically, ensuring the publisher and any
 * matching subscriber share the same QoS.
 *
 * @tparam T The IDL-generated DDS data type to publish.
 *
 * Usage:
 * @code
 *    CycloneDDS::DDSTopicConfig config({
 *       {"SensorTopic", writerQos, readerQos},
 *    });
 *    CycloneDDS::DDSPublisher<dds_messages::SensorReading> pub(0, config);
 *    pub.publish("SensorTopic", msg);
 * @endcode
 */
template <typename T>
class DDSPublisher
{
public:
   /**
    * @brief Construct a DDS publisher with a topic configuration.
    *
    * @param domainId DDS domain ID (participants on the same domain discover each other)
    * @param config   Topic configuration that defines allowed topics and their QoS
    * @param participantName Human-readable name (logged, not used by DDS)
    */
   DDSPublisher(uint32_t domainId,
                const DDSTopicConfig &config,
                const std::string &participantName = "")
      : _participant(domainId)
      , _publisher(_participant)
      , _config(config)
   {
      GPINFO("DDSPublisher created: domain={}, name={}", domainId, participantName);
   }

   ~DDSPublisher() = default;

   // Non-copyable, movable
   DDSPublisher(const DDSPublisher &) = delete;
   DDSPublisher &operator=(const DDSPublisher &) = delete;
   DDSPublisher(DDSPublisher &&) = delete;
   DDSPublisher &operator=(DDSPublisher &&) = delete;

   /**
    * @brief Publish a message on the given topic.
    *
    * The writer QoS is looked up from the DDSTopicConfig.
    * The topic and writer are lazily created on first use.
    *
    * @param topicName The DDS topic name (must be registered in the config)
    * @param message The data sample to publish
    * @throws std::out_of_range if the topic is not in the config
    */
   void publish(const std::string &topicName, const T &message)
   {
      auto &writer = getOrCreateWriter(topicName);
      writer.write(message);
   }

   /**
    * @brief Get the underlying DDS participant.
    */
   [[nodiscard]] dds::domain::DomainParticipant &participant()
   {
      return _participant;
   }

   /**
    * @brief Get the topic configuration.
    */
   [[nodiscard]] const DDSTopicConfig &config() const
   {
      return _config;
   }

private:
   using WriterType = dds::pub::DataWriter<T>;
   using TopicType = dds::topic::Topic<T>;

   /**
    * @brief Lazily create or retrieve a DataWriter for the given topic.
    *        Writer QoS is taken from the DDSTopicConfig.
    */
   WriterType &getOrCreateWriter(const std::string &topicName)
   {
      auto it = _writers.find(topicName);
      if (it == _writers.end())
      {
         const auto &qos = _config.writerQos(topicName);
         auto topic = TopicType(_participant, topicName);
         auto writer = WriterType(_publisher, topic, qos);
         auto [inserted, _] = _writers.emplace(topicName, std::move(writer));
         GPINFO("DDSPublisher: created writer for topic '{}'", topicName);
         return inserted->second;
      }
      return it->second;
   }

   dds::domain::DomainParticipant _participant;
   dds::pub::Publisher _publisher;
   const DDSTopicConfig &_config;
   std::unordered_map<std::string, WriterType> _writers;
};

} // namespace CycloneDDS

#endif // DDSPUBLISHER_H_
