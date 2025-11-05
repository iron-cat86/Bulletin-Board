cmake_minimum_required(VERSION 3.16)

project(bulletinApp VERSION 1.0)

find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

qt5_add_resources(bulletinApp_RESOURCES app.qrc)

add_executable(bulletinApp
    main.cpp 
    mainwindow.cpp
    board.cpp
    bull_thread.cpp
    task_thread.cpp
    task_thread.h
    mainwindow.h 
    board.h
    bull_thread.h
    qrc_app.cpp
    ${bulletinApp_RESOURCES}
)
target_link_libraries(bulletinApp PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui)

find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})

# Создайте новый файл в вашем проекте, например, tests/bulletins_app_tests.cpp
set(TEST_SOURCES
    tests/bulletins_app_tests.cpp
    board.cpp
    bull_thread.cpp
    task_thread.cpp
    task_thread.h
    board.h
    bull_thread.h
)

# 3. Создаем новый исполняемый файл для тестов
add_executable(bulletinApp_tests ${TEST_SOURCES})

# 4. Линкуем тестовый исполняемый файл с библиотеками GTest и Qt
target_link_libraries(bulletinApp_tests PRIVATE
    ${GTEST_LIBRARIES}
    Qt5::Core
    Qt5::Widgets
)

# Включаем автоматический запуск тестов при выполнении 'make test'
include(GoogleTest)
gtest_discover_tests(bulletinApp_tests)
