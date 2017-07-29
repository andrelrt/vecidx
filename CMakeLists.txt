project(vecidx)
cmake_minimum_required(VERSION 2.8)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")

find_package(Boost 1.63.0 REQUIRED COMPONENTS timer chrono system)
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

find_package(BoostSimd)

# -Rpass-missed=.*
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -march=native -mtune=native -Wall -fno-strict-aliasing")

include_directories(${Boost_INCLUDE_DIRS} ${BOOST_SIMD_INCLUDE_DIR})

add_subdirectory(test)
