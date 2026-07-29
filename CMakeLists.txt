cmake_minimum_required(VERSION 3.10)
project(dictionary_generation_lib CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(OpenMP REQUIRED)

add_library(common src/common.cpp)
target_include_directories(common PUBLIC src)

add_executable(mindistances src/mindistances.cpp)
target_link_libraries(mindistances common)

add_executable(greedy2022 src/greedy2022.cpp)
target_link_libraries(greedy2022 common)

add_executable(proposed_optimal src/proposed_optimal.cpp)
target_link_libraries(proposed_optimal common)

add_executable(aruco2014 src/aruco2014.cpp)
target_link_libraries(aruco2014 common OpenMP::OpenMP_CXX)

add_executable(apriltag2011 src/apriltag2011.cpp)
target_link_libraries(apriltag2011 common OpenMP::OpenMP_CXX gmpxx gmp)

set(ORTOOLS_ROOT ${PROJECT_SOURCE_DIR}/third_party/ortools)
if(EXISTS ${ORTOOLS_ROOT}/include/ortools/sat/cp_model.h)
    add_executable(milp2016 src/milp2016.cpp)
    target_include_directories(milp2016 PRIVATE ${ORTOOLS_ROOT}/include)
    target_link_libraries(milp2016 common ${ORTOOLS_ROOT}/lib/libortools.so ${ORTOOLS_ROOT}/lib/libprotobuf.a)
    set_target_properties(milp2016 PROPERTIES BUILD_RPATH ${ORTOOLS_ROOT}/lib INSTALL_RPATH ${ORTOOLS_ROOT}/lib)

    add_executable(proposed_suboptimal src/proposed_suboptimal.cpp)
    target_include_directories(proposed_suboptimal PRIVATE ${ORTOOLS_ROOT}/include)
    target_link_libraries(proposed_suboptimal common ${ORTOOLS_ROOT}/lib/libortools.so ${ORTOOLS_ROOT}/lib/libprotobuf.a)
    set_target_properties(proposed_suboptimal PROPERTIES BUILD_RPATH ${ORTOOLS_ROOT}/lib INSTALL_RPATH ${ORTOOLS_ROOT}/lib)
else()
    message(STATUS "OR-Tools not found in third_party/ortools -> skipping some methods")
endif()
