cmake_minimum_required (VERSION 3.10)

project(cmake_template)
#set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")


add_executable(${PROJECT_NAME} 
	src/main.cpp
)

include_directories(
	include
)