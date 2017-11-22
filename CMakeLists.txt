cmake_minimum_required(VERSION 3.9)

# it me
project(emp)

include(sources.cmake)

foreach(src ${emp_sources})
	string(CONCAT relsrc src/ ${src})
	list(APPEND sources ${relsrc})
endforeach(src)

# our header files
include_directories(include /usr/local/include)

link_directories(/usr/local/lib)

# our bullshit
add_executable(emp ${sources} src/main.c)
