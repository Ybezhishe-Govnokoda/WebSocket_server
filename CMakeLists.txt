cmake_minimum_required(VERSION 3.20)
project(WebSocket LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTORCC ON)

#find_package(Qt6 REQUIRED COMPONENTS Widgets)

# OpenSSL (MinGW)
set(OPENSSL_ROOT_DIR ${CMAKE_SOURCE_DIR}/external/openssl-mingw/usr/local)

set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
set(OPENSSL_LIBRARIES
    ${OPENSSL_ROOT_DIR}/lib64/libssl.a
    ${OPENSSL_ROOT_DIR}/lib64/libcrypto.a
)

# WebSocket DLL
set(WEBSOCKET_HEADERS
)

set(WEBSOCKET_SOURCES
    src/wss_api.cpp
    src/wss_client.cpp
    src/wss_server.cpp
)

add_library(WebSocket SHARED
    ${WEBSOCKET_HEADERS}
    ${WEBSOCKET_SOURCES}
)

target_compile_definitions(WebSocket PRIVATE WS_DLL_EXPORTS)

target_include_directories(WebSocket
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/boost_1_90_0
        ${OPENSSL_INCLUDE_DIR}
)

# GUI Client
#add_executable(ws_gui_client
#    main.cpp
#    mainwindow.cpp
#    mainwindow.hc
#    mainwindow.ui
#    src/qt_ws_client.cpp
#)

# Server
add_executable(server
    src/server.cpp
)

# Tests WSS
add_executable(test_server_wss
    tests/test_server_wss.cpp
)

add_executable(test_client_wss
    tests/test_client_wss.cpp
)

# ========================================
# TESTS WS
# ========================================
add_library(TEST SHARED
    src/ws_api.cpp
    src/ws_client.cpp
    src/ws_server.cpp
    src/wss_api.cpp
    src/wss_client.cpp
    src/wss_server.cpp
)

target_compile_definitions(TEST PRIVATE WS_DLL_EXPORTS)

target_include_directories(TEST
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/boost_1_90_0
        ${OPENSSL_INCLUDE_DIR}
)

# Server
add_executable(TEST_SERVER_WS
    tests/test_server_ws.cpp
)

target_link_libraries(TEST_SERVER_WS
    PRIVATE
        TEST
)

add_executable(TEST_CLIENT_WS
    tests/test_client_ws.cpp
)

target_link_libraries(TEST_CLIENT_WS
    PRIVATE
        TEST
)

# Windows
if (WIN32)
    target_link_libraries(TEST PRIVATE ws2_32 mswsock)
    target_link_libraries(TEST_CLIENT_WS PRIVATE ws2_32 mswsock)
    target_link_libraries(TEST_SERVER_WS PRIVATE ws2_32 mswsock)
endif()
# ========================================
# ========================================

# Linking libraries for all targets
if (WIN32)
    set(WINDOWS_LIBS
        ws2_32
        mswsock
        crypt32
        advapi32
        ucrt
        kernel32
        user32
        gdi32
        shell32
        ole32
        uuid
    )

    target_link_libraries(WebSocket PRIVATE
        #Qt6::Widgets
        ${OPENSSL_LIBRARIES}
        ${WINDOWS_LIBS}
    )

    target_link_libraries(TEST PRIVATE
        ${OPENSSL_LIBRARIES}
        ${WINDOWS_LIBS}
    )

    #target_link_libraries(ws_gui_client PRIVATE
    #    WebSocket
    #    Qt6::Widgets
    #    ${WINDOWS_LIBS}
    #)

    target_link_libraries(server PRIVATE
        WebSocket
        ${WINDOWS_LIBS}
    )

    target_link_libraries(test_server_wss PRIVATE
        WebSocket
        ${WINDOWS_LIBS}
    )

    target_link_libraries(test_client_wss PRIVATE
        WebSocket
        ${WINDOWS_LIBS}
    )
else()
    target_link_libraries(WebSocket PRIVATE
        Qt6::Widgets
        ${OPENSSL_LIBRARIES}
    )
    #target_link_libraries(ws_gui_client PRIVATE
    #    WebSocket
    #    Qt6::Widgets
    #)

    target_link_libraries(server PRIVATE
        WebSocket
    )

    target_link_libraries(test_server_wss PRIVATE
        WebSocket
    )

    target_link_libraries(test_client_wss PRIVATE
        WebSocket
    )
endif()

