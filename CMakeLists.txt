cmake_minimum_required(VERSION 3.16)

project(computer_graphics_lab5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_executable(computer_graphics_lab5
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mountainwidget.cpp
    mountainwidget.h
)

target_link_libraries(computer_graphics_lab5
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
)