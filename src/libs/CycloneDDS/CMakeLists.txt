# =============================================================================
# DDS Library - CMakeLists.txt
#
# Uses Eclipse Cyclone DDS (C++ binding) for topic-based pub/sub.
# IDL files in the idl/ directory are automatically compiled into
# C++ types by the Cyclone DDS idlc compiler via IDLCXX_GENERATE().
# =============================================================================

cmake_minimum_required(VERSION 3.25)

project(CycloneDDSLib CXX)

find_package(CycloneDDS REQUIRED)
find_package(CycloneDDS-CXX REQUIRED)

# ---------------------------------------------------------------------------
# Fix IMPORTED_LOCATION for CycloneDDS::idlc in non-Release configs.
# Conan's build module sets IMPORTED_LOCATION but CMake's generation step
# sometimes fails when the build type differs from the Conan profile.
# ---------------------------------------------------------------------------
if(TARGET CycloneDDS::idlc)
   get_target_property(_idlc_loc CycloneDDS::idlc IMPORTED_LOCATION)
   if(_idlc_loc)
      set_target_properties(CycloneDDS::idlc PROPERTIES
         IMPORTED_LOCATION_DEBUG "${_idlc_loc}"
         IMPORTED_LOCATION_RELEASE "${_idlc_loc}"
      )
   endif()
endif()

# ---------------------------------------------------------------------------
# IDL code generation via Cyclone DDS build modules
# ---------------------------------------------------------------------------

file(GLOB IDL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/idl/*.idl")

idlcxx_generate(TARGET DDSMessages FILES ${IDL_FILES})

# ---------------------------------------------------------------------------
# CycloneDDSLib target (compiled library — .cpp sources + header-only templates)
# ---------------------------------------------------------------------------

file(GLOB CYCLONE_DDS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(${PROJECT_NAME} STATIC ${CYCLONE_DDS_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(${PROJECT_NAME} PUBLIC
   DDSMessages
   CycloneDDS-CXX::ddscxx
   CommonUtils
)
