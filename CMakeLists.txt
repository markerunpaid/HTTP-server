cmake_minimum_required(VERSION 3.16)
project(http_server CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(server
    main.cpp
    TCPsocket.cpp
    EPOLL.cpp
    HTTP_parser.cpp
)

target_compile_options(server PRIVATE -Wall -Wextra)