cmake_minimum_required (VERSION 2.6)
project (buffalo)

file(GLOB SOURCES "src/*.c")
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-g -O2 -Wall -Werror -Wextra -pthread -DNDEBUG")
add_executable(buffalo ${SOURCES})


