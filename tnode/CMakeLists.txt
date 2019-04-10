cmake_minimum_required (VERSION 2.8)

set(target tnode) 
project (${target})

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_BUILD_TYPE "Debug")#Release

set(PROJECT_DIR .)
set(EXECUTABLE_OUTPUT_PATH .)

set(ROOT_DIR ${PROJECT_SOURCE_DIR}) 
set(DEPS_DIR ${PROJECT_SOURCE_DIR}/../deps)

set(EASYLOG_DIR ${DEPS_DIR}/easylog)
set(EASYNET_DIR ${DEPS_DIR}/easynet)
set(PROTOBUF_DIR ${DEPS_DIR}/protobuf-3.6.1)
set(RAPIDXML_DIR ${DEPS_DIR}/rapidxml-1.13)
set(RAPIDJSON_DIR ${DEPS_DIR}/rapidjson-1.1.0)
set(LUA_DIR ${DEPS_DIR}/luajit-2.0.5)
set(MYSQL_DIR ${DEPS_DIR}/mysql-5.6.37)
set(LIBEVENT_DIR ${DEPS_DIR}/libevent-2.1.8)
set(CURL_DIR ${DEPS_DIR}/curl-7.58.0)
set(TCMALLOC_DIR ${DEPS_DIR}/gperftools-2.7)

add_definitions(-Wall -Werror -D_GNU_SOURCE -DLINUX -DX64 -fstack-protector-all -D_LARGEFILE64_SOURCE -DDEBUG -std=c++11)

include_directories(src)
include_directories(${EASYLOG_DIR}/include)
include_directories(${EASYNET_DIR}/include)
include_directories(${PROTOBUF_DIR}/include)
include_directories(${RAPIDXML_DIR})
include_directories(${RAPIDJSON_DIR})
include_directories(${LUA_DIR}/include)
include_directories(${MYSQL_DIR}/include)
include_directories(${LIBEVENT_DIR}/include)
include_directories(${CURL_DIR}/include)
include_directories(${TCMALLOC_DIR}/include)

aux_source_directory(src/config src)
aux_source_directory(src/json src)
aux_source_directory(src/xml src)
aux_source_directory(src/lua src)
aux_source_directory(src/message src)
aux_source_directory(src/service src)
aux_source_directory(src/time src)
aux_source_directory(src/tools src)

link_directories(/usr/lib64)
add_executable(${target} ${src})

target_link_libraries(${target} rt dl pthread unwind crypto ssl)
target_link_libraries(${target} ${EASYLOG_DIR}/lib/libeasylog.a)
target_link_libraries(${target} ${EASYNET_DIR}/lib/libeasynet.a)
target_link_libraries(${target} ${PROTOBUF_DIR}/lib/libprotobuf.a)
target_link_libraries(${target} ${LUA_DIR}/lib/libluajit-5.1.a)
target_link_libraries(${target} ${MYSQL_DIR}/lib/libmysqlclient.a)
target_link_libraries(${target} ${LIBEVENT_DIR}/lib/libevent.a)
target_link_libraries(${target} ${CURL_DIR}/lib/libcurl.a)
target_link_libraries(${target} ${TCMALLOC_DIR}/lib/libtcmalloc_minimal.a) 

