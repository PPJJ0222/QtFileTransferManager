cmake_minimum_required(VERSION 3.16)
project(QtFileTransferManager)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/6.10.2/msvc2022_64")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Network Concurrent WebEngineWidgets WebChannel)

# Web 前端构建
set(WEB_SOURCE_DIR ${CMAKE_SOURCE_DIR}/web)
set(WEB_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/resources/web)

add_custom_target(build_web
    COMMAND npm run build
    WORKING_DIRECTORY ${WEB_SOURCE_DIR}
    COMMENT "构建 Web 前端..."
)

# 资源文件
set(RESOURCES
    resources/resources.qrc
)

add_executable(QtFileTransferManager
    src/main.cpp
    # WebEngine UI
    src/web/WebWindow.cpp
    src/web/Bridge.cpp
    src/web/FtpWorker.cpp
    # Core 层
    src/core/FileTransfer.cpp
    src/core/FtpClient.cpp
    src/core/LocalFileOps.cpp
    ${RESOURCES}
)

target_include_directories(QtFileTransferManager PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(QtFileTransferManager
    Qt6::Core
    Qt6::Network
    Qt6::Concurrent
    Qt6::WebEngineWidgets
    Qt6::WebChannel
)

# 确保构建主程序前先构建 web
add_dependencies(QtFileTransferManager build_web)

# 构建后复制配置文件到可执行文件目录
add_custom_command(TARGET QtFileTransferManager POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/ftp_servers.ini"
        "$<TARGET_FILE_DIR:QtFileTransferManager>/ftp_servers.ini"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/machine_servers.ini"
        "$<TARGET_FILE_DIR:QtFileTransferManager>/machine_servers.ini"
    COMMENT "复制配置文件到输出目录..."
)
