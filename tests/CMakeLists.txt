# =============================================================================
# Tests - CMakeLists.txt
# =============================================================================

include(GoogleTest)

# Enable CTest
enable_testing()

# Test executable
add_executable(unit_tests
   test_main.cpp
   test_logger.cpp
   test_timer.cpp
   test_async_queue.cpp
)

target_link_libraries(unit_tests
   PRIVATE
      StarterCpp::utils
      GTest::gtest
      GTest::gtest_main
      GTest::gmock
)

# Include directories
target_include_directories(unit_tests
   PRIVATE
      ${CMAKE_SOURCE_DIR}/src/utils/include
)

# Discover tests for CTest
gtest_discover_tests(unit_tests
   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
   PROPERTIES
      LABELS "unit"
)

# Set output directory
set_target_properties(unit_tests PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# =============================================================================
# Coverage Target
# =============================================================================

if(ENABLE_COVERAGE)
   # Find coverage tools
   find_program(LCOV_EXECUTABLE lcov)
   find_program(GENHTML_EXECUTABLE genhtml)
   find_program(GCOV_EXECUTABLE gcov)

   if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
      # Create coverage target
      add_custom_target(coverage
         COMMAND ${LCOV_EXECUTABLE} --zerocounters --directory ${CMAKE_BINARY_DIR}
         COMMAND ${CMAKE_COMMAND} -E echo "Running tests..."
         COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
         COMMAND ${LCOV_EXECUTABLE}
            --capture
            --directory ${CMAKE_BINARY_DIR}
            --output-file ${CMAKE_BINARY_DIR}/coverage.info
            --gcov-tool ${GCOV_EXECUTABLE}
         COMMAND ${LCOV_EXECUTABLE}
            --remove ${CMAKE_BINARY_DIR}/coverage.info
            '/usr/*'
            '${CMAKE_BINARY_DIR}/*'
            '*/test/*'
            '*/tests/*'
            --output-file ${CMAKE_BINARY_DIR}/coverage.info
         COMMAND ${GENHTML_EXECUTABLE}
            ${CMAKE_BINARY_DIR}/coverage.info
            --output-directory ${CMAKE_BINARY_DIR}/coverage_report
            --title "StarterCpp Coverage Report"
            --legend
            --demangle-cpp
         COMMAND ${CMAKE_COMMAND} -E echo "Coverage report: ${CMAKE_BINARY_DIR}/coverage_report/index.html"
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
         DEPENDS unit_tests
         COMMENT "Generating code coverage report"
         VERBATIM
      )
   else()
      message(WARNING "lcov/genhtml not found. Coverage report generation disabled.")

      # Simplified coverage target without HTML generation
      add_custom_target(coverage
         COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
         DEPENDS unit_tests
         COMMENT "Running tests (coverage HTML disabled - lcov/genhtml not found)"
      )
   endif()
endif()
