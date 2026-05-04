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
# Таргет 2: Qt5-приложение (UI + локализация, линкуется к библиотеке)
# ------------------------------------------------------------------
find_package(Qt5 REQUIRED COMPONENTS Widgets Core)

add_executable(teletraffic_calc WIN32
    main.cpp
    mainwindow.cpp
    mainwindow.h
    app_locale.cpp
    app_locale.h
)

target_link_libraries(teletraffic_calc PRIVATE
    teletraffic_core
    Qt5::Widgets
    Qt5::Core
)

# ------------------------------------------------------------------
# Копировать JSON-файлы локализации рядом с .exe после сборки
# ------------------------------------------------------------------
add_custom_command(TARGET teletraffic_calc POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/locale_ru.json"
        "$<TARGET_FILE_DIR:teletraffic_calc>/locale_ru.json"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/locale_en.json"
        "$<TARGET_FILE_DIR:teletraffic_calc>/locale_en.json"
    COMMENT "Copying locale files..."
)

# Для Windows: после сборки выполните в папке сборки:
#   windeployqt teletraffic_calc.exe
