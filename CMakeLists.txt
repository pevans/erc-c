cmake_minimum_required(VERSION 3.9)

# it me
project(erc)

include(sources.cmake)

if(APPLE)
	set(opengl_library /System/Library/Frameworks/OpenGL.framework)
endif()

if(NOT opengl_library)
	message(FATAL_ERROR "This CMake file is not yet educated as to where OpenGL resides on your platform. Sorry!")
endif()

foreach(src ${erc_sources})
	string(CONCAT relsrc src/ ${src})
	list(APPEND sources ${relsrc})
endforeach(src)

# our header files
include_directories(include /usr/local/include)

link_directories(/usr/local/lib)

# our bullshit
add_executable(erc ${sources} src/main.c)

# Graphics
target_link_libraries(erc ${opengl_library} glfw GLEW)
