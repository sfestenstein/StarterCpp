# =============================================================================
# StarterCpp - Root CMakeLists.txt
# =============================================================================
cmake_minimum_required(VERSION 3.25)

# Project definition
project(StarterCpp
   VERSION 1.0.0
   DESCRIPTION "A robust C++ starter project template"
   LANGUAGES CXX
)

# =============================================================================
# Global Settings
# =============================================================================

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for clang-tidy and IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add cmake/ to module path for Coverage.cmake etc.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
   set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
   set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
      "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# =============================================================================
# Options
# =============================================================================

option(BUILD_TESTS "Build unit tests" ON)
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
option(ENABLE_SANITIZERS "Enable ASan and UBSan" OFF)
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)

# =============================================================================
# Compiler Warnings
# =============================================================================

# Define warning flags for different compilers
set(GCC_WARNINGS
   -Wall
   -Wextra
   -Wshadow
   -Wnon-virtual-dtor
   -Wold-style-cast
   -Wcast-align
   -Wunused
   -Woverloaded-virtual
   -Wconversion
   -Wsign-conversion
   -Wnull-dereference
   -Wdouble-promotion
   -Wformat=2
   -Wimplicit-fallthrough
)

set(MSVC_WARNINGS
   /W4
   /permissive-
   /w14640
   /w14242
   /w14254
   /w14263
   /w14265
   /w14287
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
   add_compile_options(${GCC_WARNINGS})
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
   add_compile_options(${MSVC_WARNINGS})
endif()



# =============================================================================
# Sanitizers
# =============================================================================

if(ENABLE_SANITIZERS)
   if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      message(STATUS "Enabling Address Sanitizer (ASan) and Undefined Behavior Sanitizer (UBSan)")
      add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
      add_link_options(-fsanitize=address,undefined)
   else()
      message(WARNING "Sanitizers are not supported with this compiler")
   endif()
endif()

# =============================================================================
# Code Coverage
# =============================================================================

if(ENABLE_COVERAGE)
   if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      message(STATUS "Enabling code coverage with gcov")
      add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
      add_link_options(--coverage)
   elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      message(STATUS "Enabling code coverage with llvm-cov")
      add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
      add_link_options(-fprofile-instr-generate -fcoverage-mapping)
   else()
      message(WARNING "Code coverage is not supported with this compiler")
   endif()
endif()

# =============================================================================
# Clang-Tidy
# =============================================================================

if(ENABLE_CLANG_TIDY)
   find_program(CLANG_TIDY_PROGRAM clang-tidy)
   if(CLANG_TIDY_PROGRAM)
      message(STATUS "Found clang-tidy: ${CLANG_TIDY_PROGRAM}")
      set(CMAKE_CXX_CLANG_TIDY
         "${CLANG_TIDY_PROGRAM}"
         "--header-filter=${CMAKE_SOURCE_DIR}/src/.*"
      )
   else()
      message(WARNING "clang-tidy requested but not found")
   endif()
endif()

# =============================================================================
# Dependencies (Conan)
# =============================================================================

# Find Conan-generated package files
find_package(spdlog REQUIRED)
find_package(protobuf REQUIRED)
find_package(cppzmq REQUIRED)
find_package(CycloneDDS REQUIRED)
find_package(CycloneDDS-CXX REQUIRED)
find_package(Crow REQUIRED)

if(BUILD_TESTS)
   find_package(GTest REQUIRED)
endif()

# =============================================================================
# Output Directories
# =============================================================================

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# =============================================================================
# Subdirectories
# =============================================================================

# Libraries
add_subdirectory(src/libs/CommonUtils)
add_subdirectory(src/libs/proto)
add_subdirectory(src/libs/PubSub)
add_subdirectory(src/libs/CycloneDDS)
add_subdirectory(src/libs/Vita49_2)

# Applications
add_subdirectory(src/apps)

# Tests
if(BUILD_TESTS)
   enable_testing()
   add_subdirectory(tests/CommonUtilsTests)
   add_subdirectory(tests/ProtoTests)
   add_subdirectory(tests/PubSubTests)
   add_subdirectory(tests/DDSTests)
   add_subdirectory(tests/Vita49_2Tests)

   # Unified coverage target — runs all tests, generates a combined report
   include(Coverage)
   add_unified_coverage_target(
      NAME    coverage
      TITLE   "StarterCpp Coverage Report"
   )
endif()

# =============================================================================
# Installation (Optional)
# =============================================================================

include(GNUInstallDirs)

install(TARGETS CommonUtils ProtoLib PubSubLib Vita49_2
   EXPORT StarterCppTargets
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
   INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# =============================================================================
# Summary
# =============================================================================

message(STATUS "")
message(STATUS "=== StarterCpp Configuration ===")
message(STATUS "  Version:          ${PROJECT_VERSION}")
message(STATUS "  Build type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard:     ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler:         ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build tests:      ${BUILD_TESTS}")
message(STATUS "  Coverage:         ${ENABLE_COVERAGE}")
message(STATUS "  Sanitizers:       ${ENABLE_SANITIZERS}")
message(STATUS "  Clang-tidy:       ${ENABLE_CLANG_TIDY}")
message(STATUS "================================")
message(STATUS "")
