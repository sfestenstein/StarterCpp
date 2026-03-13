# =============================================================================
# IPCMonitor — web-based IPC traffic inspector
# =============================================================================

# ---------------------------------------------------------------------------
# Embed the monitor HTML as a C++ raw-string include
# ---------------------------------------------------------------------------
set(MONITOR_HTML_SRC ${CMAKE_CURRENT_SOURCE_DIR}/web/monitor.html)
set(MONITOR_HTML_INC ${CMAKE_CURRENT_SOURCE_DIR}/web/monitor.html.inc)

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
add_custom_target(IPCMonitorHtml DEPENDS ${MONITOR_HTML_INC})

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------
file(GLOB IPC_MONITOR_SOURCES "*.cpp")

add_executable(IPCMonitor ${IPC_MONITOR_SOURCES})
add_dependencies(IPCMonitor IPCMonitorHtml)

# Crow headers trigger clang-tidy false positives — disable for this target.
set_target_properties(IPCMonitor PROPERTIES CXX_CLANG_TIDY "")

target_include_directories(IPCMonitor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(IPCMonitor
   PRIVATE CycloneDDSLib CommonUtils Crow::Crow
)

set_target_properties(IPCMonitor PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
