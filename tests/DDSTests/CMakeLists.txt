project(DDSTests)

include(GoogleTest)

enable_testing()

file(GLOB UNIT_TEST_SOURCE ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${UNIT_TEST_SOURCE})

target_link_libraries(${PROJECT_NAME}
   PRIVATE
      CycloneDDSLib
      CommonUtils
      GTest::gtest
      GTest::gmock
)

# Discover tests for CTest
gtest_discover_tests(${PROJECT_NAME}
   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
   PROPERTIES
      LABELS "unit"
)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# =============================================================================
# Coverage Target
# =============================================================================

if(ENABLE_COVERAGE)
   find_program(LCOV_EXECUTABLE lcov)
   find_program(GENHTML_EXECUTABLE genhtml)
   find_program(GCOV_EXECUTABLE gcov)

   set(COVERAGE_TARGET "DDSCoverage")

   if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
      set(LCOV_IGNORE_ERRORS --ignore-errors inconsistent,mismatch,unused,negative)

      add_custom_target(${COVERAGE_TARGET}
         COMMAND ${LCOV_EXECUTABLE} --zerocounters --directory ${CMAKE_BINARY_DIR} ${LCOV_IGNORE_ERRORS}
         COMMAND ${CMAKE_COMMAND} -E echo "Running DDS tests..."
         COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L unit
         COMMAND ${LCOV_EXECUTABLE}
            --capture
            --directory ${CMAKE_BINARY_DIR}
            --output-file ${CMAKE_BINARY_DIR}/${COVERAGE_TARGET}.info
            --gcov-tool ${GCOV_EXECUTABLE}
            ${LCOV_IGNORE_ERRORS}
         COMMAND ${LCOV_EXECUTABLE}
            --remove ${CMAKE_BINARY_DIR}/${COVERAGE_TARGET}.info
            "*/tests/*" "*/build/*" "/usr/*" "*/conan2/*"
            --output-file ${CMAKE_BINARY_DIR}/${COVERAGE_TARGET}_filtered.info
            ${LCOV_IGNORE_ERRORS}
         COMMAND ${GENHTML_EXECUTABLE}
            ${CMAKE_BINARY_DIR}/${COVERAGE_TARGET}_filtered.info
            --output-directory ${CMAKE_BINARY_DIR}/coverage/DDS
            --title "DDS Library Coverage"
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
         COMMENT "Generating DDS coverage report..."
         VERBATIM
      )
   endif()
endif()
