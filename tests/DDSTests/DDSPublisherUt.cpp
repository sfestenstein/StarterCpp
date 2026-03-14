/**
 * @file DDSPublisherUt.cpp
 * @brief Unit tests for DDSPublisher.
 *
 * Tests basic construction and publish operations using a local-only
 * DDS domain (high domain ID to avoid conflicts).
 */

#include <gtest/gtest.h>
#include "CycloneDDS/DDSPublisher.h"
#include "CycloneDDS/DDSTopicConfig.h"

#include "SensorData.hpp"
#include "Command.hpp"
#include "TrackData.hpp"

// Use a high domain ID to isolate test traffic
static constexpr uint32_t TEST_DOMAIN_ID = 99;

class DDSPublisherTest : public ::testing::Test
{
protected:
   void SetUp() override {}
   void TearDown() override {}

   /**
    * @brief Build a DDSTopicConfig that covers all topics used in tests.
    */
   static CycloneDDS::DDSTopicConfig makeConfig(
      std::initializer_list<std::string> topics)
   {
      std::vector<CycloneDDS::TopicEntry> entries;
      for (const auto &t : topics)
      {
         entries.push_back(
            {t, dds::pub::qos::DataWriterQos{}, dds::sub::qos::DataReaderQos{}});
      }
      return CycloneDDS::DDSTopicConfig(entries);
   }
};

TEST_F(DDSPublisherTest, Construction_ValidDomain_Succeeds)
{
   auto cfg = makeConfig({"AnyTopic"});
   EXPECT_NO_THROW({
      CycloneDDS::DDSPublisher<dds_messages::SensorReading> pub(TEST_DOMAIN_ID, cfg, "TestPub");
   });
}

TEST_F(DDSPublisherTest, Publish_SensorReading_NoThrow)
{
   auto cfg = makeConfig({"TestSensorTopic"});
   CycloneDDS::DDSPublisher<dds_messages::SensorReading> pub(TEST_DOMAIN_ID, cfg, "SensorPub");

   dds_messages::SensorReading msg;
   msg.sensor_id("test-sensor");
   msg.sensor_name("Test Temperature");
   msg.value(22.5);
   msg.unit("celsius");
   msg.timestamp_ms(1000);
   msg.quality(90);
   msg.status(dds_messages::SensorStatus::SENSOR_ONLINE);
   msg.latitude(37.0);
   msg.longitude(-122.0);
   msg.altitude(100.0);

   EXPECT_NO_THROW(pub.publish("TestSensorTopic", msg));
}

TEST_F(DDSPublisherTest, Publish_TrackUpdate_NoThrow)
{
   auto cfg = makeConfig({"TestTrackTopic"});
   CycloneDDS::DDSPublisher<dds_messages::TrackUpdate> pub(TEST_DOMAIN_ID, cfg, "TrackPub");

   dds_messages::TrackUpdate msg;
   msg.track_id("track-001");
   msg.track_name("Test Track");
   msg.latitude(37.5);
   msg.longitude(-122.5);
   msg.altitude(5000.0);
   msg.heading(90.0);
   msg.speed(250.0);
   msg.classification(dds_messages::TrackClassification::TRACK_FRIENDLY);
   msg.timestamp_ms(2000);
   msg.update_number(1);
   msg.confidence(0.85);

   EXPECT_NO_THROW(pub.publish("TestTrackTopic", msg));
}

TEST_F(DDSPublisherTest, Publish_Command_NoThrow)
{
   auto cfg = makeConfig({"TestCommandTopic"});
   CycloneDDS::DDSPublisher<dds_messages::Command> pub(TEST_DOMAIN_ID, cfg, "CmdPub");

   dds_messages::Command msg;
   msg.command_id("cmd-001");
   msg.source_id("controller");
   msg.target_id("sensor-001");
   msg.type(dds_messages::CommandType::CMD_START);
   msg.priority(dds_messages::CommandPriority::PRIORITY_HIGH);
   msg.payload("start acquisition");
   msg.timestamp_ms(3000);
   msg.sequence_number(1);

   EXPECT_NO_THROW(pub.publish("TestCommandTopic", msg));
}

TEST_F(DDSPublisherTest, Publish_MultipleSamples_SameTopic)
{
   auto cfg = makeConfig({"TestBatchTopic"});
   CycloneDDS::DDSPublisher<dds_messages::SensorReading> pub(TEST_DOMAIN_ID, cfg, "MultiPub");

   for (int i = 0; i < 10; ++i)
   {
      dds_messages::SensorReading msg;
      msg.sensor_id("sensor-" + std::to_string(i));
      msg.value(static_cast<double>(i) * 1.5);
      msg.unit("celsius");
      msg.timestamp_ms(static_cast<int64_t>(i) * 100);

      EXPECT_NO_THROW(pub.publish("TestBatchTopic", msg));
   }
}

TEST_F(DDSPublisherTest, Publish_MultipleTopics_CreatesWriters)
{
   auto cfg = makeConfig({"TopicA", "TopicB", "TopicC"});
   CycloneDDS::DDSPublisher<dds_messages::SensorReading> pub(TEST_DOMAIN_ID, cfg, "MultiTopicPub");

   dds_messages::SensorReading msg;
   msg.sensor_id("s1");
   msg.value(10.0);

   EXPECT_NO_THROW(pub.publish("TopicA", msg));
   EXPECT_NO_THROW(pub.publish("TopicB", msg));
   EXPECT_NO_THROW(pub.publish("TopicC", msg));

   // Publishing again to the same topics should reuse writers
   EXPECT_NO_THROW(pub.publish("TopicA", msg));
   EXPECT_NO_THROW(pub.publish("TopicB", msg));
}

TEST_F(DDSPublisherTest, Publish_UnregisteredTopic_Throws)
{
   auto cfg = makeConfig({"RegisteredTopic"});
   CycloneDDS::DDSPublisher<dds_messages::SensorReading> pub(TEST_DOMAIN_ID, cfg, "ThrowPub");

   dds_messages::SensorReading msg;
   msg.sensor_id("s1");

   EXPECT_THROW(pub.publish("UnregisteredTopic", msg), std::out_of_range);
}
