cmake_minimum_required(VERSION 4.2)
project(http-server C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)

add_executable(server src/server/server.c src/utilities/socketutil.c)
add_executable(client src/client/client.c src/utilities/socketutil.c)

target_include_directories(server PRIVATE src/utilities)
target_include_directories(client PRIVATE src/utilities)
