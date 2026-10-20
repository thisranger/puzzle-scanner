cmake_minimum_required(VERSION 4.3)
project(puzzle_scanner)

find_package(OpenCV REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter)

include_directories( ${OpenCV_INCLUDE_DIRS} )

set(CMAKE_CXX_STANDARD 20)

add_executable(puzzle_scanner puzzle.cpp)
target_link_libraries(puzzle_scanner ${OpenCV_LIBS})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

