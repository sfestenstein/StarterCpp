# =============================================================================
# Omniscope — web-based IPC traffic inspector
# =============================================================================

# ---------------------------------------------------------------------------
# Embed the monitor HTML as a C++ raw-string include
# ---------------------------------------------------------------------------
set(MONITOR_HTML_SRC ${CMAKE_CURRENT_SOURCE_DIR}/web/monitor.html)
set(MONITOR_HTML_INC ${CMAKE_CURRENT_BINARY_DIR}/web/monitor.html.inc)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/web)

add_custom_command(
   OUTPUT  ${MONITOR_HTML_INC}
   COMMAND ${CMAKE_COMMAND}
      -DHTML_SRC=${MONITOR_HTML_SRC}
      -DHTML_INC=${MONITOR_HTML_INC}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/../EmbedHtml.cmake
   DEPENDS ${MONITOR_HTML_SRC}
           ${CMAKE_CURRENT_SOURCE_DIR}/../EmbedHtml.cmake
   COMMENT "Embedding monitor.html into C++ include"
)
add_custom_target(OmniscopeHtml DEPENDS ${MONITOR_HTML_INC})

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------
file(GLOB OMNISCOPE_SOURCES "*.cpp")

add_executable(Omniscope ${OMNISCOPE_SOURCES})
add_dependencies(Omniscope OmniscopeHtml)

# Crow headers trigger clang-tidy false positives — disable for this target.
set_target_properties(Omniscope PROPERTIES CXX_CLANG_TIDY "")

target_include_directories(Omniscope PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(Omniscope
   PRIVATE CycloneDDSLib CommonUtils PubSubLib ProtoLib Crow::Crow
)

set_target_properties(Omniscope PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
