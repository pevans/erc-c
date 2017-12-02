cmake_minimum_required(VERSION 3.9)

project(emp-test)

set(CMAKE_BUILD_TYPE Debug)

include_directories(../include /usr/local/include)

link_directories(/usr/local/lib)

include(../sources.cmake)

foreach(src ${emp_sources})
	string(CONCAT relsrc ../src/ ${src})
	list(APPEND sources ${relsrc})
endforeach(src)

# test_sources should also include the main source file, so we don't need to
# make any particular effort to include it in add_executable().
file(GLOB test_sources "*.c")

add_executable(emp-test ${sources} ${test_sources})

# Our unit-testing library
target_link_libraries(emp-test criterion)
