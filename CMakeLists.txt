cmake_minimum_required(VERSION 3.13)
project(freerfb C)

set(CMAKE_C_STANDARD 99)

add_definitions(-DLOG_USE_COLOR)

#add_subdirectory(log)
#add_subdirectory(connection)
add_subdirectory(tests)

add_executable(freerfb main.c
        log/log.c
        connection/tcp_connection.c
        utils/utils.c
        utils/d3des.c
        common/inbuf.c
        rfb/protover.c
        rfb/auth.c rfb/init.c)