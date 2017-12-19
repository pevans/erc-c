cmake_minimum_required(VERSION 3.9)

project(erc-test)

set(CMAKE_BUILD_TYPE Debug)

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

include_directories(../include /usr/local/include ${sdl_headers})

link_directories(/usr/local/lib)

include(../sources.cmake)

foreach(src ${erc_sources})
	string(CONCAT relsrc ../src/ ${src})
	list(APPEND sources ${relsrc})
endforeach(src)

# test_sources should also include the main source file, so we don't need to
# make any particular effort to include it in add_executable().
file(GLOB test_sources "*.c")

add_executable(erc-test ${sources} ${test_sources})

# Our unit-testing library
target_link_libraries(erc-test criterion)

# Graphics
target_link_libraries(erc-test ${sdl_library})
