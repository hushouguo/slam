set(target test_luaT) 
project (${target})

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_BUILD_TYPE "Debug")#Release

set(PROJECT_DIR .)
set(EXECUTABLE_OUTPUT_PATH ..)

add_definitions(-Wall -Werror -D_GNU_SOURCE -DLINUX -DX64 -fstack-protector-all -D_LARGEFILE64_SOURCE -DDEBUG -std=c++11)

include_directories(${EASYLOG_DIR}/include)
include_directories(${LUA_DIR}/include)
include_directories(${LUAT_DIR}/include)

aux_source_directory(. src)

link_directories(/usr/lib64) 
link_directories(/home/hushouguo/packages/protobuf-3.6.1/local/lib) 

add_executable(${target} ${src})

target_link_libraries(${target} ${EASYLOG_DIR}/lib/libeasylog.a)
target_link_libraries(${target} ${LUAT_DIR}/lib/libluaT.a)
target_link_libraries(${target} ${LUA_DIR}/lib/libluajit-5.1.a)
#target_link_libraries(${target} ${PROTOBUF_DIR}/lib/libprotobuf.a)
target_link_libraries(${target} dl pthread protobuf)
target_link_libraries(${target} ${TCMALLOC_DIR}/lib/libtcmalloc_minimal.a)

