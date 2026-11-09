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
    bezierwidget.cpp
    bezierwidget.h
)

target_link_libraries(computer_graphics_lab5
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
)

include(CTest)
if(BUILD_TESTING)
    add_executable(lab5_algorithms_test
        tests/algorithms.cpp
        mountainwidget.cpp
        mountainwidget.h
    )
    target_include_directories(lab5_algorithms_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(lab5_algorithms_test PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
    add_test(NAME lab5_algorithms COMMAND lab5_algorithms_test)
    set_tests_properties(lab5_algorithms PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()
