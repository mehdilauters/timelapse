cmake_minimum_required(VERSION 2.8.3)
project( bread )
#set(OpenCV_DIR "/home/mehdi/Mehdi/programmation/opencv/git/build/")
set(OpenCV_DIR "/home/pi/Mehdi/opencv/build/")
find_package( OpenCV REQUIRED )
add_definitions(-std=c++0x)

include_directories( 	${OpenCV_INCLUDE_DIRS}
						./inc
						${CMAKE_CURRENT_BINARY_DIR}
						)


FILE( GLOB source_files ./*.cpp  ./src/*.cpp )

add_executable(cam ${source_files})
target_link_libraries( cam ${OpenCV_LIBS})