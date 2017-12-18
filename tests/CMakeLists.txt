cmake_minimum_required(VERSION 3.9)

project(erc-test)

set(CMAKE_BUILD_TYPE Debug)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --analyze")

if(CMAKE_C_FLAGS MATCHES "--analyze")
	set(CMAKE_C_LINK_EXECUTABLE "echo 'Will not execute binary when running static analysis'")
endif()

if(APPLE)
	set(opengl_library /System/Library/Frameworks/OpenGL.framework)
endif()

if(NOT opengl_library)
	message(FATAL_ERROR "This CMake file is not yet educated as to where OpenGL resides on your platform. Sorry!")
endif()

include_directories(../include /usr/local/include)

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
target_link_libraries(erc-test)
