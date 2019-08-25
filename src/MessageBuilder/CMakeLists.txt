cmake_minimum_required (VERSION 2.8)

set(target messagebuilder) 
project (${target})

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_BUILD_TYPE "Release")

set(PROJECT_DIR .)
set(LIBRARY_OUTPUT_PATH .)

set(DEPS_DIR ${PROJECT_SOURCE_DIR}/../lib)
set(LUA_DIR ${DEPS_DIR}/luajit-2.0.5)
set(PROTOBUF_DIR ${DEPS_DIR}/protobuf-3.3.0)

add_definitions(-Wall -Werror -D_GNU_SOURCE -DLINUX -DX64 -fstack-protector-all -D_LARGEFILE64_SOURCE -DDEBUG -std=c++11 -rdynamic)

include_directories(${LUA_DIR}/include)
include_directories(${PROTOBUF_DIR}/include)

aux_source_directory(. src)

add_library(${target} ${src})
