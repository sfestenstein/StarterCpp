# =============================================================================
# Applications - CMakeLists.txt
# =============================================================================

# Publisher executable
add_executable(publisher
   publisher_main.cpp
)

target_link_libraries(publisher
   PRIVATE
      StarterCpp::utils
      StarterCpp::proto
      cppzmq
)

# Subscriber executable
add_executable(subscriber
   subscriber_main.cpp
)

target_link_libraries(subscriber
   PRIVATE
      StarterCpp::utils
      StarterCpp::proto
      cppzmq
)

# Set properties
set_target_properties(publisher subscriber PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
