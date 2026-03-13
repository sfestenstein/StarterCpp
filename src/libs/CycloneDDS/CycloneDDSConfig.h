#ifndef CYCLONEDDSCONFIG_H_
#define CYCLONEDDSCONFIG_H_

// Project headers
#include "CycloneDDS/DDSTopicConfig.h"

namespace CycloneDDS
{

/**
 * @brief Provides a pre-built DDSTopicConfig with sensible default QoS
 *        for every project topic.
 *
 * Centralises topic names and QoS policies so that publishers and
 * subscribers always agree.  Call defaultInitialize() once and share
 * the returned config across all participants.
 *
 * Usage:
 * @code
 *    CycloneDDS::CycloneDDSConfig defaults;
 *    defaults.defaultInitialize();
 *    const auto &config = defaults.config();
 *
 *    CycloneDDS::DDSPublisher<dds_messages::SensorReading>  pub(0, config);
 *    CycloneDDS::DDSSubscriber<dds_messages::SensorReading> sub(0, config);
 * @endcode
 */
class CycloneDDSConfig
{
public:
   /**
    * @brief Populate the internal DDSTopicConfig with default topics/QoS.
    *
    * Current defaults:
    *   - SensorTopic : Reliable (1 s timeout), KeepLast(10)
    *   - TrackTopic  : BestEffort, KeepLast(1)
    */
   void defaultInitialize();

   /**
    * @brief Access the initialised DDSTopicConfig.
    */
   [[nodiscard]] const DDSTopicConfig &config() const;

private:
   DDSTopicConfig _config{{}};
};

} // namespace CycloneDDS

#endif // CYCLONEDDSCONFIG_H_
