cmake_minimum_required (VERSION 2.8)

set(target easydb) 
project (${target})

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_BUILD_TYPE "Debug")#Release

set(ROOT_DIR ${PROJECT_SOURCE_DIR})
set(DEPS_DIR ${PROJECT_SOURCE_DIR}/../..)
set(MYSQL_DIR ${DEPS_DIR}/mysql-5.6.37)

set(PROJECT_DIR .)
set(LIBRARY_OUTPUT_PATH ../lib)

add_definitions(-Wall -Werror -D_GNU_SOURCE -DLINUX -DX64 -fstack-protector-all -D_LARGEFILE64_SOURCE -DDEBUG -std=c++11)

include_directories(.)
include_directories(${MYSQL_DIR}/include)

aux_source_directory(. src)
aux_source_directory(mysql src)

add_library(${target} ${src})
