cmake_minimum_required(VERSION 3.9)

project(emp-test)

include_directories(../include /usr/local/include)

link_directories(/usr/local/lib)

include(../sources.cmake)

foreach(src ${emp_sources})
	string(CONCAT relsrc ../src/ ${src})
	list(APPEND sources ${relsrc})
endforeach(src)

add_executable(emp-test ${sources} ./main.c)

target_link_libraries(emp-test criterion)
