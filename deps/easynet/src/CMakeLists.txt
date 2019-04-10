cmake_minimum_required (VERSION 2.8)

set(target easynet) 
project (${target})

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_BUILD_TYPE "Debug")#Release

set(PROJECT_DIR .)
set(LIBRARY_OUTPUT_PATH ../lib)

add_definitions(-Wall -Werror -D_GNU_SOURCE -DLINUX -DX64 -fstack-protector-all -D_LARGEFILE64_SOURCE -DDEBUG -std=c++11)

aux_source_directory(. src)

add_library(${target} ${src})
