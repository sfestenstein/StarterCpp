# =============================================================================
# Applications - CMakeLists.txt
# =============================================================================

# Zyre Publisher executable
add_executable(ZyrePublisher  ZyrePublisherTest.cpp)

target_link_libraries(ZyrePublisher
   PRIVATE PubSubLib ProtoLib
)

# Zyre Subscriber executable
add_executable(ZyreSubscriber ZyreSubscriberTest.cpp)

target_link_libraries(ZyreSubscriber
   PRIVATE PubSubLib ProtoLib )

# High-Bandwidth Subscriber executable
add_executable(HighBandwidthSubscriber HighBandwidthSubscriberTester.cpp)

target_link_libraries(HighBandwidthSubscriber
   PRIVATE PubSubLib ProtoLib )

# High-Bandwidth Publisher executable
add_executable(HighBandwidthPublisher HighBandwidthPublisherTester.cpp)

target_link_libraries(HighBandwidthPublisher
   PRIVATE PubSubLib ProtoLib )

# VITA 49.2 Round-Trip Test
add_executable(Vita49RoundTripTest Vita49RoundTripTest.cpp)

target_link_libraries(Vita49RoundTripTest
   PRIVATE Vita49_2 CommonUtils )

# VITA 49.2 Performance Benchmark
add_executable(Vita49PerfBenchmark Vita49PerfBenchmark.cpp)

target_link_libraries(Vita49PerfBenchmark
   PRIVATE Vita49_2 CommonUtils )

# VITA 49.2 File Codec (generate / inspect / roundtrip)
add_executable(Vita49FileCodec Vita49FileCodec.cpp)

target_link_libraries(Vita49FileCodec
   PRIVATE Vita49_2 CommonUtils )

# Set properties
set_target_properties(
   ZyrePublisher ZyreSubscriber HighBandwidthSubscriber HighBandwidthPublisher
   Vita49RoundTripTest Vita49PerfBenchmark Vita49FileCodec
   PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
