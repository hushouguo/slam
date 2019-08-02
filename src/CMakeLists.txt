cmake_minimum_required (VERSION 2.8)

set(target slam) 
project (${target})

set(CMAKE_CXX_COMPILER "gcc")
set(CMAKE_BUILD_TYPE "Debug")#Release

set(DEPS_DIR ${PROJECT_SOURCE_DIR}/../deps)
set(LUA_DIR ${DEPS_DIR}/luajit-2.0.5)
set(PROTOBUF_DIR ${DEPS_DIR}/protobuf-3.6.1)
#set(TCMALLOC_DIR ${DEPS_DIR}/gperftools-2.7)

set(PROJECT_DIR .)
set(EXECUTABLE_OUTPUT_PATH .)

add_definitions(-Wall -Werror -D_GNU_SOURCE -DLINUX -DX64 -fstack-protector-all -D_LARGEFILE64_SOURCE -DDEBUG)
add_definitions(-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free)

include_directories(${LUA_DIR}/include)
#include_directories(${TCMALLOC_DIR}/include)

aux_source_directory(. src)

link_directories(${PROJECT_SOURCE_DIR})

add_executable(${target} ${src})

target_link_libraries(${target} tcmalloc rt dl m protobuf)
#target_link_libraries(${target} ${LIBEVENT_DIR}/lib/libevent.a)
target_link_libraries(${target} ${LUA_DIR}/lib/libluajit-5.1.a)
#target_link_libraries(${target} ${PROTOBUF_DIR}/lib/libprotobuf.a)
#target_link_libraries(${target} ${MYSQL_DIR}/lib/libmysqlclient.a)
#target_link_libraries(${target} ${CURL_DIR}/lib/libcurl.a)
#target_link_libraries(${target} ${TCMALLOC_DIR}/lib/libtcmalloc_minimal.a) 

