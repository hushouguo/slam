cmake_minimum_required (VERSION 2.8)

set(target luaT) 
project (${target})

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_BUILD_TYPE "Debug")#Release

set(ROOT_DIR ${PROJECT_SOURCE_DIR})
set(DEPS_DIR ${PROJECT_SOURCE_DIR}/../..)
set(RAPIDXML_DIR ${DEPS_DIR}/rapidxml-1.13)
set(RAPIDJSON_DIR ${DEPS_DIR}/rapidjson-1.1.0)
set(LUA_DIR ${DEPS_DIR}/luajit-2.0.5)

set(PROJECT_DIR .)
set(LIBRARY_OUTPUT_PATH ../lib)

add_definitions(-Wall -Werror -D_GNU_SOURCE -DLINUX -DX64 -fstack-protector-all -D_LARGEFILE64_SOURCE -DDEBUG -std=c++11)

include_directories(.)
include_directories(${RAPIDXML_DIR})
include_directories(${RAPIDJSON_DIR})
include_directories(${LUA_DIR}/include)

aux_source_directory(xml src)
aux_source_directory(json src)
aux_source_directory(. src)

add_library(${target} ${src})
