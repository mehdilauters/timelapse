cmake_minimum_required(VERSION 2.8.3)
project( bread )
set(OpenCV_DIR "/home/mehdi/Mehdi/programmation/opencv/git/build/")

option(USE_KODI "Use kodi client" OFF)

#set(OpenCV_DIR "/home/pi/Mehdi/opencv/build/")
find_package( OpenCV REQUIRED )

find_package(Boost REQUIRED filesystem system)

add_definitions(-std=c++0x -g)

include_directories( 	${OpenCV_INCLUDE_DIRS}
						./inc
						${CMAKE_CURRENT_BINARY_DIR}
						${Boost_INCLUDE_DIRS}
						)


if(USE_KODI)
    add_definitions(-DUSE_KODI)
    
    add_custom_target(common_stubs
	DEPENDS ${CMAKE_BINARY_DIR}/gen/abstractstubserver.h ${CMAKE_BINARY_DIR}/gen/stubclient.h
    )
                
    add_custom_target(
        gen
	COMMAND jsonrpcstub ${CMAKE_CURRENT_SOURCE_DIR}/kodi.json --cpp-client=XbmcRemoteClient --cpp-client-file=${CMAKE_BINARY_DIR}/xbmcremote.h
	COMMENT "Generating Client XBMC Stubfile"
    )
    
    set( kodi_libs jsoncpp jsonrpccpp-common jsonrpccpp-client)
    
endif()
						
FILE( GLOB source_files ./*.cpp  ./src/*.cpp )

add_executable(cam ${source_files})
target_link_libraries( cam ${OpenCV_LIBS} ${Boost_LIBRARIES} ${kodi_libs})

if(USE_KODI)
    add_dependencies(cam gen)
endif()
