cmake_minimum_required(VERSION 3.13)
project(freerfb C)

set(CMAKE_C_STANDARD 99)

#add_subdirectory(log)
#add_subdirectory(connection)

add_executable(demo-server
        demo-server.c
        ../log/log.c
        ../connection/tcp_connection.c
        )

add_executable(simple-server
        simple-server.c
        ../log/log.c
        ../connection/tcp_connection.c)

add_executable(simple-client
        simple-client.c
        ../log/log.c
        ../connection/tcp_connection.c)