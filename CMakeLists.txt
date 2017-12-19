cmake_minimum_required(VERSION 3.9)

# it me
project(erc)

include(sources.cmake)

if(DEFINED ENV{STATIC_ANALYSIS})
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --analyze")
endif()

if(CMAKE_C_FLAGS MATCHES "--analyze")
	set(CMAKE_C_LINK_EXECUTABLE "echo 'Will not execute binary when running static analysis'")
endif()

if(APPLE)
	set(sdl_library /Library/Frameworks/SDL2.framework)
	set(sdl_headers /Library/Frameworks/SDL2.framework/Headers)
endif()

if(NOT sdl_library)
	message(FATAL_ERROR "This CMake file is not yet educated as to where SDL2 resides on your platform. Sorry!")
endif()

foreach(src ${erc_sources})
	string(CONCAT relsrc src/ ${src})
	list(APPEND sources ${relsrc})
endforeach(src)

# our header files
include_directories(include /usr/local/include ${sdl_headers})

link_directories(/usr/local/lib)

# our bullshit
add_executable(erc ${sources} src/main.c)

# Graphics
target_link_libraries(erc ${sdl_library})
