set(target test_easynet) 
project (${target})

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_BUILD_TYPE "Debug")#Release

set(PROJECT_DIR .)
set(EXECUTABLE_OUTPUT_PATH ..)

add_definitions(-Wall -Werror -D_GNU_SOURCE -DLINUX -DX64 -fstack-protector-all -D_LARGEFILE64_SOURCE -DDEBUG -std=c++11)

include_directories(${EASYLOG_DIR}/include)
include_directories(${EASYNET_DIR}/include)

aux_source_directory(. src)

link_directories(/usr/lib64) 

add_executable(${target} ${src})

target_link_libraries(${target} rt dl -pthread)
target_link_libraries(${target} ${EASYLOG_DIR}/lib/libeasylog.a)
target_link_libraries(${target} ${EASYNET_DIR}/lib/libeasynet.a)
target_link_libraries(${target} ${TCMALLOC_DIR}/lib/libtcmalloc_minimal.a)

