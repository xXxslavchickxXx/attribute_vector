cmake_minimum_required(VERSION 3.20)

# Задаем версию стандарта
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# НАЗВАНИЕ ПРОЕКТА
project(attribute_vector VERSION 1.0 LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchDependency.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildLib.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildExample.cmake)

# может вы хотите какие то файлы в папках автоматически находить
file(GLOB_RECURSE ${PROJECT_NAME}_FILES
    "include/*.h"
    "include/*.inl"
)

message(STATUS "Found files: ${${PROJECT_NAME}_FILES}")

# 1. Путь к файлу-точке входа для приложения-примера (например: main.cpp)
set(${PROJECT_NAME}_EXAMPLE_FILEPATH example.cpp)

# 2. Ручное указание файлов твоей библиотеки (если хочешь перечислить конкретные файлы)
set(${PROJECT_NAME}_LIB_H ${${PROJECT_NAME}_FILES})
set(${PROJECT_NAME}_LIB_CPP "")

# 3. Подключение инклудов
set(${PROJECT_NAME}_INCLUDE "")

# 3. Внешние либы для линковки к твоей основной библиотеке (например: gpuBuffers)
find_package(glm)
set(${PROJECT_NAME}_LIBS glm::glm)

# 4. Либы, нужные ТОЛЬКО для запуска приложения-примера (например: shaderLib)
set(${PROJECT_NAME}_EXAMPLE_LIBS "")

# Опции сборки
option(${PROJECT_NAME}_BUILD_EXAMPLE "Построить пример для проекта" ON)
option(${PROJECT_NAME}_ASSET_TO_BUILD "Копировать ассеты в папку сборки" OFF)

# Собираем либу
create_library(attribute_vector
    HEADERS ${${PROJECT_NAME}_LIB_H}
    LIBS ${${PROJECT_NAME}_LIBS}
)

if(${PROJECT_NAME}_BUILD_EXAMPLE)
    create_executable(${PROJECT_NAME}_example
        SOURCE ${${PROJECT_NAME}_EXAMPLE_FILEPATH}
        LIBS ${PROJECT_NAME}
    )
endif()