cmake_minimum_required(VERSION 3.20)

project(PubSubLib CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CZMQ REQUIRED libczmq)
pkg_check_modules(ZYRE REQUIRED libzyre)
pkg_check_modules(ZMQ REQUIRED libzmq)

file(GLOB SOURCES     "*.cpp")
file(GLOB HEADERS     "*.h")

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    ${CZMQ_INCLUDE_DIRS}
    ${ZYRE_INCLUDE_DIRS}
    ${ZMQ_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    ${CZMQ_LIBRARIES}
    ${ZYRE_LIBRARIES}
    ${ZMQ_LIBRARIES}
    CommonUtils
    protobuf::libprotobuf
)

# Also propagate link directories if pkg-config provided them (less ideal than
# proper imported targets, but keeps behavior similar to previous global link dirs).
if(CZMQ_LIBRARY_DIRS)
    target_link_directories(${PROJECT_NAME} PUBLIC ${CZMQ_LIBRARY_DIRS})
endif()
if(ZYRE_LIBRARY_DIRS)
    target_link_directories(${PROJECT_NAME} PUBLIC ${ZYRE_LIBRARY_DIRS})
endif()
if(ZMQ_LIBRARY_DIRS)
    target_link_directories(${PROJECT_NAME} PUBLIC ${ZMQ_LIBRARY_DIRS})
endif()
