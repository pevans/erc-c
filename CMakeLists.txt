cmake_minimum_required(VERSION 3.9)

# it me
project(erc)

include(sources.cmake)

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
target_link_libraries(erc glfw)
