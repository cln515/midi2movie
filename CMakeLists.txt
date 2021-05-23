cmake_minimum_required (VERSION 3.10)

project(midi2movie)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")


add_executable(${PROJECT_NAME} 
	src/main.cpp
	src/scoredrawer.cpp
	src/mapping.cpp
	
	midifile/src/MidiFile.cpp
	midifile/src/MidiEvent.cpp
	midifile/src/MidiEventList.cpp
	midifile/src/MidiMessage.cpp
	midifile/src/Options.cpp
	midifile/src/Binasc.cpp


)

find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
  include_directories(
      ${OpenCV_INCLUDE_DIRS}
    )
  target_link_libraries(${PROJECT_NAME} 
    ${OpenCV_LIBS}
    )
endif(OpenCV_FOUND)

find_package(Eigen3 REQUIRED)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}  ${OPENGL_LIBRARIES})

find_package(Eigen3 REQUIRED)
include_directories(
	include
	midifile/include
	ffmpeg
	${EIGEN3_INCLUDE_DIR}
)

find_package(nlohmann_json 3.2.0 REQUIRED)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)

