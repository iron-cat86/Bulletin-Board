cmake_minimum_required(VERSION 3.16)

project(bulletinApp VERSION 1.0)

find_package(Qt5 COMPONENTS Widgets Core REQUIRED)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

add_executable(bulletinApp
    main.cpp 
    mainwindow.cpp
    board.cpp
    bull_thread.cpp
    mainwindow.h 
    board.h
    bull_thread.h
)
target_link_libraries(bulletinApp PRIVATE Qt5::Widgets Qt5::Core)
