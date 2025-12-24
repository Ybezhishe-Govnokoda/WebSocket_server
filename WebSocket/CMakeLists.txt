cmake_minimum_required(VERSION 3.20)
project(WebSocket LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =======================
# DLL
# =======================
add_library(WebSocket SHARED
    src/ws_client.cpp
    src/ws_server.cpp
    src/ws_api.cpp
)

target_include_directories(WebSocket
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/boost
)

# =======================
# TEST CLIENT (EXE)
# =======================
add_executable(test_client
    tests/test_client.cpp
)

target_link_libraries(test_client
    PRIVATE WebSocket
)

target_include_directories(test_client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# =======================
# TEST SERVER (EXE)
# =======================
add_executable(test_server
    tests/test_server.cpp
)

target_link_libraries(test_server
    PRIVATE WebSocket
)

target_include_directories(test_server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
