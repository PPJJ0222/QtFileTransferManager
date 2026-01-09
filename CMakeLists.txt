cmake_minimum_required(VERSION 3.16)
project(QtFileTransferManager)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/6.10.1/msvc2022_64")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Concurrent)

# 资源文件
set(RESOURCES
    resources/resources.qrc
)

add_executable(QtFileTransferManager
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/FilePanel.cpp
    src/ui/ProgressBar.cpp
    src/ui/MachineConfigDialog.cpp
    src/ui/StyleManager.cpp
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
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
)
