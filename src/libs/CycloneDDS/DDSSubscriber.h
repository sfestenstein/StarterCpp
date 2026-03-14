#ifndef DDSSUBSCRIBER_H_
#define DDSSUBSCRIBER_H_

// Project headers
#include "CommonUtils/GeneralLogger.h"
#include "CycloneDDS/DDSTopicConfig.h"

// Cyclone DDS C++ headers
#include <dds/dds.hpp>

// System headers
#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include <vector>

namespace CycloneDDS
{

/**
 * @brief Generic DDS topic-based subscriber using Cyclone DDS.
 *
 * Constructed with a DDSTopicConfig that defines every allowed topic and
 * its QoS policies.  When subscribe() is called, the reader QoS is
 * looked up from the config automatically, ensuring the subscriber and
 * any matching publisher share the same QoS.
 *
 * @tparam T The IDL-generated DDS data type to subscribe to.
 *
 * Usage:
 * @code
 *    CycloneDDS::DDSTopicConfig config({
 *       {"SensorTopic", writerQos, readerQos},
 *    });
 *    CycloneDDS::DDSSubscriber<dds_messages::SensorReading> sub(0, config);
 *    sub.subscribe("SensorTopic", [](const dds_messages::SensorReading &msg) {
 *       std::cout << "Received: " << msg.sensor_id() << std::endl;
 *    });
 *    sub.start();
 *    // ... run until done ...
 *    sub.stop();
 * @endcode
 */
template <typename T>
class DDSSubscriber
{
public:
   /**
    * @brief Callback type for received messages.
    */
   using MessageHandler = std::function<void(const T &)>;

   /**
    * @brief Construct a DDS subscriber with a topic configuration.
    *
    * @param domainId DDS domain ID (must match the publisher's domain)
    * @param config   Topic configuration that defines allowed topics and their QoS
    * @param participantName Human-readable name (logged, not used by DDS)
    */
   DDSSubscriber(uint32_t domainId,
                 const DDSTopicConfig &config,
                 const std::string &participantName = "")
      : _participant(domainId)
      , _subscriber(_participant)
      , _config(config)
      , _running(false)
   {
      GPINFO("DDSSubscriber created: domain={}, name={}", domainId, participantName);
   }

   ~DDSSubscriber()
   {
      stop();
   }

   // Non-copyable, non-movable (owns thread)
   DDSSubscriber(const DDSSubscriber &) = delete;
   DDSSubscriber &operator=(const DDSSubscriber &) = delete;
   DDSSubscriber(DDSSubscriber &&) = delete;
   DDSSubscriber &operator=(DDSSubscriber &&) = delete;

   /**
    * @brief Subscribe to a topic with a callback handler.
    *
    * The reader QoS is looked up from the DDSTopicConfig.
    * Must be called before start(). Multiple topics can be subscribed to.
    *
    * @param topicName The DDS topic name (must be registered in the config)
    * @param handler Callback invoked for each received sample
    * @throws std::out_of_range if the topic is not in the config
    */
   void subscribe(const std::string &topicName, MessageHandler handler)
   {
      const auto &qos = _config.readerQos(topicName);
      auto topic = dds::topic::Topic<T>(_participant, topicName);
      auto reader = dds::sub::DataReader<T>(_subscriber, topic, qos);

      _subscriptions.emplace_back(
         Subscription{topicName, std::move(reader), std::move(handler)});

      GPINFO("DDSSubscriber: subscribed to topic '{}'", topicName);
   }

   /**
    * @brief Start the polling thread for receiving messages.
    *
    * Launches a background thread that polls all subscribed readers.
    */
   void start()
   {
      if (_running.exchange(true))
      {
         return; // Already running
      }

      _pollThread = std::thread([this]() { pollLoop(); });
      GPINFO("DDSSubscriber: polling started");
   }

   /**
    * @brief Stop the polling thread.
    *
    * Blocks until the polling thread has exited.
    */
   void stop()
   {
      if (!_running.exchange(false))
      {
         return; // Already stopped
      }

      if (_pollThread.joinable())
      {
         _pollThread.join();
      }
      GPINFO("DDSSubscriber: polling stopped");
   }

   /**
    * @brief Check if the subscriber is currently running.
    */
   [[nodiscard]] bool isRunning() const
   {
      return _running.load();
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
   struct Subscription
   {
      std::string topicName;
      dds::sub::DataReader<T> reader;
      MessageHandler handler;
   };

   /**
    * @brief Background loop that polls all readers for new data.
    */
   void pollLoop()
   {
      while (_running.load())
      {
         for (auto &sub : _subscriptions)
         {
            auto samples = sub.reader.take();
            for (const auto &sample : samples)
            {
               if (sample.info().valid())
               {
                  sub.handler(sample.data());
               }
            }
         }
         std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
   }

   dds::domain::DomainParticipant _participant;
   dds::sub::Subscriber _subscriber;
   const DDSTopicConfig &_config;
   std::vector<Subscription> _subscriptions;
   std::atomic<bool> _running;
   std::thread _pollThread;
};

} // namespace CycloneDDS

#endif // DDSSUBSCRIBER_H_
