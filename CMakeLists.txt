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
    "include/proxy/base/*.h"
)

set(${PROJECT_NAME}_TESTS
    "tests/BaseProxy.cpp"
    "tests/AVTest.cpp"
    #"tests/AliasGenTests.cpp"
    #"tests/ClassGenTests.cpp"
)

message(STATUS "Found files: ${${PROJECT_NAME}_FILES}")

# 1. Путь к файлу-точке входа для приложения-примера (например: main.cpp)
set(${PROJECT_NAME}_EXAMPLE_FILEPATH
example.cpp)
#tests/test.cpp)

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
option(${PROJECT_NAME}_BUILD_EXAMPLE "Построить пример для проекта" OFF)
option(${PROJECT_NAME}_ASSET_TO_BUILD "Копировать ассеты в папку сборки" OFF)
option(${PROJECT_NAME}_BUILD_TESTS "собирать тесты" OFF)

# Собираем либу
create_library(attribute_vector
    HEADERS ${${PROJECT_NAME}_LIB_H}
    LIBS ${${PROJECT_NAME}_LIBS}
)

if(${PROJECT_NAME}_BUILD_EXAMPLE)
    create_executable(${PROJECT_NAME}_tests
        SOURCE ${${PROJECT_NAME}_EXAMPLE_FILEPATH}
        LIBS ${PROJECT_NAME}
    )
    set_source_files_properties(${${PROJECT_NAME}_EXAMPLE_FILEPATH} PROPERTIES
        OBJECT_DEPENDS "${${PROJECT_NAME}_LIB_H}"
    )
endif()

create_executable(${PROJECT_NAME}_example
    SOURCE example.cpp
    LIBS ${PROJECT_NAME}
)

if(${PROJECT_NAME}_BUILD_TESTS)
    if(${PROJECT_NAME}_TESTS)        
        find_package(GTest CONFIG REQUIRED)        
        
        list(GET ${PROJECT_NAME}_TESTS 0 FIRST_TEST_FILE)        
        list(REMOVE_AT ${PROJECT_NAME}_TESTS 0)

        create_executable(run_tests
            SOURCE ${FIRST_TEST_FILE}
            LIBS 
                ${PROJECT_NAME}
                GTest::gtest
                GTest::gtest_main
        )        
        
        if(TARGET run_tests AND ${PROJECT_NAME}_TESTS)            
            target_sources(run_tests PRIVATE ${${PROJECT_NAME}_TESTS})        
        endif()        
        
        if(TARGET run_tests)            
            enable_testing()            
            include(GoogleTest)
            gtest_discover_tests(run_tests)        
        else()            
            message(WARNING "[WARNING] Таргет run_tests не был создан.")        
        endif()
        
    else()        
        message(STATUS "[NOTICE] Тесты не будут собраны: папка tests/ пуста.")    
    endif()
endif()