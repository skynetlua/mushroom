#CMake最低版本号要求
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

set(APP_NAME tutorial)

PROJECT(${APP_NAME})

MESSAGE(STATUS "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
MESSAGE(STATUS "PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")

if(WIN32)
else()
#SET(CMAKE_BUILE_TYPE DEBUG)
#SET(CMAKE_C_FLAGS_DEBUG "-g -Wall")
SET(CMAKE_C_FLAGS "-g -Wall")
endif()

file(COPY "${PROJECT_SOURCE_DIR}/mrtool/mr_buffer.h" DESTINATION  "${PROJECT_SOURCE_DIR}/include")
file(COPY "${PROJECT_SOURCE_DIR}/mrtool/mr_code.h" DESTINATION  "${PROJECT_SOURCE_DIR}/include")

file(COPY "${PROJECT_SOURCE_DIR}/mrsocket/mr_mem.h" DESTINATION  "${PROJECT_SOURCE_DIR}/include")
file(COPY "${PROJECT_SOURCE_DIR}/mrsocket/mr_socket.h" DESTINATION  "${PROJECT_SOURCE_DIR}/include")
file(COPY "${PROJECT_SOURCE_DIR}/mrsocket/socket_info.h" DESTINATION  "${PROJECT_SOURCE_DIR}/include")
file(COPY "${PROJECT_SOURCE_DIR}/mrsocket/mrsocket.h" DESTINATION  "${PROJECT_SOURCE_DIR}/include")

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib CACHE PATH "Single directory for all static libraries.")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib CACHE PATH "Single directory for all dynamic libraries.")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin CACHE PATH "Single directory for all executable.")

SET(LIBMRSOCKET mrsocket)
SET(LIBMRTOOL mrtool)

include_directories(${PROJECT_SOURCE_DIR}/${LIBMRSOCKET})
add_subdirectory(${PROJECT_SOURCE_DIR}/${LIBMRSOCKET})

include_directories(${PROJECT_SOURCE_DIR}/${LIBMRTOOL})
add_subdirectory(${PROJECT_SOURCE_DIR}/${LIBMRTOOL})

if(WIN32)
link_libraries(ws2_32)
else()
link_libraries(pthread)
endif()


#demo
add_executable(demo demo.c)
target_link_libraries(demo ${LIBMRSOCKET} ${LIBMRTOOL})


#examples文件夹的测试例子
#demo例子
#客户端
#add_executable(demo_client examples/demo/demo_client.c)
#target_link_libraries(demo_client ${LIBMRSOCKET} ${LIBMRTOOL})
#服务端
#add_executable(demo_server examples/demo/demo_server.c)
#target_link_libraries(demo_server ${LIBMRSOCKET} ${LIBMRTOOL})

#tcp例子
#tcp客户端
add_executable(tcp_client examples/tcp/tcp_client.c)
target_link_libraries(tcp_client ${LIBMRSOCKET} ${LIBMRTOOL})
#tcp服务端
add_executable(tcp_server examples/tcp/tcp_server.c)
target_link_libraries(tcp_server ${LIBMRSOCKET} ${LIBMRTOOL})

#udp例子
#udp客户端
add_executable(udp_client examples/udp/udp_client.c)
target_link_libraries(udp_client ${LIBMRSOCKET} ${LIBMRTOOL})
#udp服务端
add_executable(udp_server examples/udp/udp_server.c)
target_link_libraries(udp_server ${LIBMRSOCKET} ${LIBMRTOOL})


#codebuffer
#add_executable(codebuffer examples/codebuffer.c)
#target_link_libraries(codebuffer ${LIBMRSOCKET} ${LIBMRTOOL})

