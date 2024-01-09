cmake_minimum_required(VERSION 3.5)

project(glucose4)

option(BUILD_SHARED_LIBS OFF "True for building shared object")
option(BUILD_SYRUP OFF "True for building glucose4-syrup and the parallel library")

set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Dependencies {{{
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
# }}}

set(main_simp "simp/Main.cc")
set(main_parallel "parallel/Main.cc")

# Basic Library
file(GLOB lib_srcs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} mtl/*.cc core/*.cc simp/*.cc utils/*.cc cglucose4.cc)
list(REMOVE_ITEM lib_srcs ${main_simp} ${main_parallel})

message(${lib_srcs})

# Parallel Library
file(GLOB lib_parallel_srcs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} parallel/*.cc)
list(REMOVE_ITEM lib_parallel_srcs ${main_simp} ${main_parallel})


if(BUILD_SHARED_LIBS)
    set(lib_type "SHARED")
else()
    set(lib_type "STATIC")
endif()


include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${ZLIB_INCLUDE_DIR})

link_libraries(${ZLIB_LIBRARY})

# base library
add_library(glucose4 ${lib_type} ${lib_srcs})

add_executable(glucose4-simp ${main_simp})
target_link_libraries(glucose4-simp glucose4)

if(BUILD_SYRUP)
    # PARALLEL STUFF:
    add_library(glucosep4 ${lib_type} ${lib_srcs} ${lib_parallel_srcs})
    add_executable(glucose4-syrup ${main_parallel})
    target_link_libraries(glucose4-syrup glucosep4 ${CMAKE_THREAD_LIBS_INIT})
endif()

install(TARGETS glucose4 glucose4-simp)

if(BUILD_SYRUP)
    install(TARGETS glucosep4 glucose4-syrup)
endif()
