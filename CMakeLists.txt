cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(attribute_vector VERSION 1.0 LANGUAGES CXX)

# Опция: собирать тесты или нет?
option(BUILD_TESTS "Build tests" OFF)

# 1. Создаем либу
add_library(attribute_vector INTERFACE)

# 2. Добавляем пути к заголовкам
target_include_directories(attribute_vector INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/proxy/multi
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/proxy/single
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/tags
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/io
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/type_traits
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/versionedVector
)

# 3. Зависимости
find_package(glm REQUIRED)
target_link_libraries(attribute_vector INTERFACE glm::glm)

# 4. Тесты
if(BUILD_TESTS)
    enable_testing()
    
    add_executable(attribute_vector_tests tests/test.cpp)
    target_link_libraries(attribute_vector_tests PRIVATE attribute_vector)
    
    # Добавляем тест в CTest
    add_test(NAME AttributeVectorTests COMMAND attribute_vector_tests)
endif()

# 5. Установка
install(DIRECTORY 
    ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION include
    FILES_MATCHING 
        PATTERN "*.h"
        PATTERN "*.inl"
        PATTERN "tests" EXCLUDE
)