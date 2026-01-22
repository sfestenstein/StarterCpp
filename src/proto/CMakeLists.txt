# =============================================================================
# Protocol Buffer Library - CMakeLists.txt
# =============================================================================

# Find protobuf compiler from Conan
find_package(protobuf REQUIRED)
find_program(PROTOC_EXECUTABLE
   NAMES protoc
   HINTS ${protobuf_PACKAGE_FOLDER_DEBUG}/bin ${protobuf_PACKAGE_FOLDER_RELEASE}/bin
   REQUIRED
)

# Proto source files
set(PROTO_FILES
   ${CMAKE_SOURCE_DIR}/proto/sensor_data.proto
   ${CMAKE_SOURCE_DIR}/proto/commands.proto
   ${CMAKE_SOURCE_DIR}/proto/configuration.proto
)

# Output directories for generated files
set(PROTO_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_GENERATED_DIR})

# Generate C++ files from proto files
set(PROTO_SRCS)
set(PROTO_HDRS)

foreach(PROTO_FILE ${PROTO_FILES})
   get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

   set(PROTO_SRC ${PROTO_GENERATED_DIR}/${PROTO_NAME}.pb.cc)
   set(PROTO_HDR ${PROTO_GENERATED_DIR}/${PROTO_NAME}.pb.h)

   list(APPEND PROTO_SRCS ${PROTO_SRC})
   list(APPEND PROTO_HDRS ${PROTO_HDR})

   add_custom_command(
      OUTPUT ${PROTO_SRC} ${PROTO_HDR}
      COMMAND ${PROTOC_EXECUTABLE}
         --proto_path=${CMAKE_SOURCE_DIR}/proto
         --cpp_out=${PROTO_GENERATED_DIR}
         ${PROTO_FILE}
      DEPENDS ${PROTO_FILE}
      COMMENT "Generating C++ files from ${PROTO_NAME}.proto"
      VERBATIM
   )
endforeach()

# Create the proto library
add_library(proto_lib STATIC
   ${PROTO_SRCS}
   ${PROTO_HDRS}
)

add_library(StarterCpp::proto ALIAS proto_lib)

# Include directories
target_include_directories(proto_lib
   PUBLIC
      $<BUILD_INTERFACE:${PROTO_GENERATED_DIR}>
      $<INSTALL_INTERFACE:include>
)

# Link protobuf
target_link_libraries(proto_lib
   PUBLIC
      protobuf::libprotobuf
)

# Suppress warnings in generated code
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
   target_compile_options(proto_lib PRIVATE -w)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
   target_compile_options(proto_lib PRIVATE /w)
endif()

# Set properties
set_target_properties(proto_lib PROPERTIES
   OUTPUT_NAME "proto"
   POSITION_INDEPENDENT_CODE ON
)
