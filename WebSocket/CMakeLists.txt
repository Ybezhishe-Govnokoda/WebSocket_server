cmake_minimum_required(VERSION 3.20)
project(WebSocket LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# DLL
add_library(WebSocket SHARED
    src/ws_client.cpp
    src/ws_server.cpp
    src/ws_api.cpp
    gui/mainwindow.ui
)

target_include_directories(WebSocket
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/boost
)

target_link_libraries(WebSocket
    PRIVATE
        Qt6::Widgets
)

# GUI CLIENT
add_executable(ws_gui_client
    gui/main.cpp
    gui/main.hpp
    gui/mainwindow.ui
    gui/ws_client.cpp
    gui/ws_client.hpp
)

target_link_libraries(ws_gui_client
    PRIVATE
        WebSocket
        Qt6::Widgets
)

# TEST SERVER
add_executable(test_server
    tests/test_server.cpp
)

target_link_libraries(test_server
    PRIVATE
        WebSocket
)

# ===== Windows specific =====
if (WIN32)
    target_link_libraries(WebSocket PRIVATE ws2_32)
    target_link_libraries(ws_gui_client PRIVATE ws2_32)
    target_link_libraries(test_server PRIVATE ws2_32)
endif()
