cmake_minimum_required(VERSION 3.0)
project(zmq-exersices C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package(ZMQ 4.0 REQUIRED)
find_package(GSL 2.3 REQUIRED)

include_directories(${ZMQ_INCLUDE_DIRS} ${GSL_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src)
link_libraries(${ZMQ_LIBRARIES})

add_executable(zmq-helloserver src/helloserver.c)
add_executable(zmq-helloclient src/helloclient.c)
add_executable(zmq-version src/version.c)

add_executable(zmq-wuserver src/wuserver.c)
target_link_libraries(zmq-wuserver ${GSL_LIBRARIES})
add_executable(zmq-wuclient src/wuclient.c)

add_executable(zmq-taskvent src/taskvent.c)
target_link_libraries(zmq-taskvent ${GSL_LIBRARIES})
add_executable(zmq-taskwork src/taskwork.c)
add_executable(zmq-tasksink src/tasksink.c)
