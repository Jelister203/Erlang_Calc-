cmake_minimum_required(VERSION 3.14)
project(teletraffic_calc VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# ------------------------------------------------------------------
# Таргет 1: расчётная библиотека (не зависит от Qt)
# Подключение в сторонний проект:
#   add_subdirectory(path/to/teletraffic_calc)
#   target_link_libraries(your_target PRIVATE teletraffic_core)
# ------------------------------------------------------------------
add_library(teletraffic_core STATIC
    calc_core.cpp
    calc_core.h
)

target_include_directories(teletraffic_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ------------------------------------------------------------------
# Таргет 2: Qt5-приложение (только UI, линкуется к библиотеке)
# ------------------------------------------------------------------
find_package(Qt5 REQUIRED COMPONENTS Widgets)

add_executable(teletraffic_calc WIN32
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(teletraffic_calc PRIVATE
    teletraffic_core
    Qt5::Widgets
)

# Для Windows: после сборки выполните в папке сборки:
#   windeployqt teletraffic_calc.exe
