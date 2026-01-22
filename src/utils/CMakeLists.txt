# =============================================================================
# Utility Library - CMakeLists.txt
# =============================================================================

add_library(utils STATIC
   Logger.cpp
   Timer.cpp
   AsyncQueue.cpp
)

add_library(StarterCpp::utils ALIAS utils)

target_include_directories(utils
   PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(utils
   PUBLIC
      spdlog::spdlog
)

# Set target properties
set_target_properties(utils PROPERTIES
   OUTPUT_NAME "utils"
   VERSION ${PROJECT_VERSION}
   SOVERSION ${PROJECT_VERSION_MAJOR}
)
