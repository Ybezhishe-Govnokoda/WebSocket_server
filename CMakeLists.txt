cmake_minimum_required(VERSION 3.20)
project(WebSocket LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# WebSocket DLL
set(WEBSOCKET_HEADERS
    include/qt_ws_client.h
)

set(WEBSOCKET_SOURCES
    src/ws_api.cpp
    src/ws_client.cpp
    src/ws_server.cpp
    src/qt_ws_client.cpp
)

add_library(WebSocket SHARED
    ${WEBSOCKET_HEADERS}
    ${WEBSOCKET_SOURCES}
)

target_compile_definitions(WebSocket PRIVATE WS_DLL_EXPORTS)

target_include_directories(WebSocket
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/boost
)

target_link_libraries(WebSocket
    PRIVATE
        Qt6::Widgets
)

# GUI Client
add_executable(ws_gui_client
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(ws_gui_client
    PRIVATE
        WebSocket
        Qt6::Widgets
)

# Server
add_executable(server
    src/server.cpp
)

target_link_libraries(server
    PRIVATE
        WebSocket
)

# Windows
if (WIN32)
    target_link_libraries(WebSocket PRIVATE ws2_32 mswsock)
    target_link_libraries(ws_gui_client PRIVATE ws2_32 mswsock)
    target_link_libraries(server PRIVATE ws2_32 mswsock)
endif()
