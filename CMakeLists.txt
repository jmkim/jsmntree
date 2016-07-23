cmake_minimum_required(VERSION 2.4)

project(jsmntree C CXX)

# We need C++ 11
if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.1)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED on)
else(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.1)
    set(CMAKE_CXX_FLAGS "-std=c++11")
endif(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.1)

# Make an environment for build
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/lib)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(jsmn STATIC ${PROJECT_SOURCE_DIR}/include/jsmn/jsmn.c)
add_library(adt STATIC ${PROJECT_SOURCE_DIR}/include/algorithm/adt/list.c)
add_library(jsmntree STATIC ${PROJECT_SOURCE_DIR}/lib/jsmntree.c)

add_executable(json_minimizer ${PROJECT_SOURCE_DIR}/example/json_minimizer.c)

target_link_libraries(jsmntree LINK_PUBLIC adt)             # adt
target_link_libraries(json_minimizer LINK_PUBLIC jsmn)      # jsmn
target_link_libraries(json_minimizer LINK_PUBLIC jsmntree)  # jsmnlist
