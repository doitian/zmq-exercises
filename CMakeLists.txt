cmake_minimum_required(VERSION 3.0)
project(zmq-exersices C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package(ZMQ 4.0 REQUIRED)
find_package(GLIB 2.50 REQUIRED)

include_directories(${ZMQ_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src)
link_libraries(${ZMQ_LIBRARIES})

add_library(zmqex STATIC EXCLUDE_FROM_ALL src/zmqex.c)

add_executable(zmq-helloserver src/helloserver.c)
add_executable(zmq-helloclient src/helloclient.c)
add_executable(zmq-version src/version.c)

add_executable(zmq-wuserver src/wuserver.c)
target_link_libraries(zmq-wuserver ${GLIB_LIBRARIES})
add_executable(zmq-wuclient src/wuclient.c)

add_executable(zmq-taskvent src/taskvent.c)
target_link_libraries(zmq-taskvent ${GLIB_LIBRARIES} zmqex)
add_executable(zmq-taskwork src/taskwork.c)
target_link_libraries(zmq-taskwork zmqex)
add_executable(zmq-tasksink src/tasksink.c)

add_executable(zmq-router src/router.c)
target_link_libraries(zmq-router zmqex)
