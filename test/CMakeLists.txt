cmake_minimum_required (VERSION 2.8.1)

project ("test")

include_directories(
	../jlib)

link_directories(
	../lib)

set(LIBRARY_FILES
	jlib
	)

set(SOURCE_FILES
	"test_tcp_client.hpp"
	"test_tcp_server.hpp"
	"main.cpp")

SET(CMAKE_CXX_COMPILER "g++")
add_definitions("-std=c++11 -O0 -g3 -gdwarf-2 -fmessage-length=0 -MMD -MP")

MESSAGE(STATUS "################" ${PROJECT_SOURCE_DIR})
MESSAGE(STATUS "################" ${PROJECT_BINARY_DIR})
MESSAGE(STATUS "################" ${CMAKE_SOURCE_DIR})
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

add_executable(test ${SOURCE_FILES})
add_dependencies(test jlib)
target_link_libraries(test ${LIBRARY_FILES})
