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

# Set properties
set_target_properties(ZyrePublisher ZyreSubscriber HighBandwidthSubscriber HighBandwidthPublisher PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
