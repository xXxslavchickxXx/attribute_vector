cmake_minimum_required(VERSION 3.20)

# Задаем версию стандарта
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# НАЗВАНИЕ ПРОЕКТА
project(attribute_vector VERSION 1.0 LANGUAGES CXX)

# ==============================================================================
# ОПРЕДЕЛЕНИЕ ФУНКЦИЙ (менеджер зависимостей с проверкой обновлений)
# ==============================================================================
function(add_external_dependency LIB_NAME GIT_URL)
    # Если таргет уже есть, то выходим из функции
    if(TARGET ${LIB_NAME})
        message(STATUS "[AUTO-DEP] Поток сборки: ${LIB_NAME} уже предоставлен главным проектом. Пропускаем.")
        return()
    endif()

    set(LOCAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/${LIB_NAME}")
    set(NEED_DOWNLOAD FALSE)

    if(EXISTS "${LOCAL_PATH}/CMakeLists.txt")
        message(STATUS "[AUTO-DEP] ${LIB_NAME} найден локально. Проверяем актуальность версии...")
        
        # 1. Получаем хэш локального коммита
        execute_process(
            COMMAND git rev-parse HEAD
            WORKING_DIRECTORY "${LOCAL_PATH}"
            OUTPUT_VARIABLE LOCAL_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        
        # 2. Получаем хэш последнего коммита с сервера
        execute_process(
            COMMAND git ls-remote ${GIT_URL} refs/heads/main
            OUTPUT_VARIABLE REMOTE_LS
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        
        # Вырезаем чистый хэш из строки ответа ls-remote
        string(REGEX MATCH "^[0-9a-f]+" REMOTE_HASH "${REMOTE_LS}")

        # 3. Сравниваем хэши
        if(NOT "${LOCAL_HASH}" STREQUAL "${REMOTE_HASH}" AND NOT "${REMOTE_HASH}" STREQUAL "")
            message(STATUS "[AUTO-DEP] Обнаружена новая версия ${LIB_NAME} на GitHub! Перекачиваем...")
            set(NEED_DOWNLOAD TRUE)
            file(REMOVE_RECURSE "${LOCAL_PATH}")
        endif()
    else()
        set(NEED_DOWNLOAD TRUE)
    endif()

    # Логика подключения или скачивания
    if(NOT NEED_DOWNLOAD)
        message(STATUS "[AUTO-DEP] Версия ${LIB_NAME} актуальна. Подключаем...")
        add_subdirectory("${LOCAL_PATH}" "${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}_build")
    else()
        message(STATUS "[AUTO-DEP] Скачиваем/Обновляем ${LIB_NAME} с GitHub...")

        # Настройка .gitignore
        set(GITIGNORE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/.gitignore")
        set(IGNORE_ENTRY "libs/${LIB_NAME}/")
        if(EXISTS "${GITIGNORE_FILE}")
            file(READ "${GITIGNORE_FILE}" GITIGNORE_CONTENT)
            string(FIND "${GITIGNORE_CONTENT}" "${IGNORE_ENTRY}" ENTRY_FOUND)
            if(ENTRY_FOUND EQUAL -1)
                file(APPEND "${GITIGNORE_FILE}" "\n${IGNORE_ENTRY}\n")
            endif()
        else()
            file(WRITE "${GITIGNORE_FILE}" "${IGNORE_ENTRY}\n")
        endif()

        # Скачивание через FetchContent
        include(FetchContent)
        FetchContent_Declare(
            ${LIB_NAME}
            GIT_REPOSITORY ${GIT_URL}
            GIT_TAG        main
            SOURCE_DIR     "${LOCAL_PATH}"
            BINARY_DIR     "${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}_build"
        )
        FetchContent_MakeAvailable(${LIB_NAME})
    endif()
endfunction()

# ==============================================================================
# НАСТРОЙКА ФАЙЛОВ И ОПЦИЙ ПРОЕКТА (Заполнять сюда)
# ==============================================================================

# может вы хотите какие то файлы в папках автоматически находить
file(GLOB_RECURSE ${CMAKE_PROJECT_NAME}_FILES
    "include/*.h"
    "include/*.inl"
)

# 1. Путь к файлу-точке входа для приложения-примера (например: main.cpp)
set("${CMAKE_PROJECT_NAME}_EXAMPLE_FILEPATH" example.cpp)

# 2. Ручное указание файлов твоей библиотеки (если хочешь перечислить конкретные файлы)
set("${CMAKE_PROJECT_NAME}_LIB_H"   "")
set("${CMAKE_PROJECT_NAME}_LIB_CPP" "")

# 3. Внешние либы для линковки к твоей основной библиотеке (например: gpuBuffers)
find_package(glm)
set("${CMAKE_PROJECT_NAME}_LIBS" glm::glm)

# 4. Либы, нужные ТОЛЬКО для запуска приложения-примера (например: shaderLib)
set("${CMAKE_PROJECT_NAME}_EXAMPLE_LIBS" "")

# Опции сборки
option(${CMAKE_PROJECT_NAME}_BUILD_EXAMPLE "Построить пример для проекта" ON)
option(${CMAKE_PROJECT_NAME}_ASSET_TO_BUILD "Копировать ассеты в папку сборки" OFF)

# ==============================================================================
# АВТОМАТИЧЕСКИЙ СБОР ФАЙЛОВ С ДИСКА (Если ручные списки выше пусты)
# ==============================================================================
if("${${CMAKE_PROJECT_NAME}_LIB_H}" STREQUAL "")
    file(GLOB_RECURSE DETECTED_H 
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.inl"
    )
    if(DETECTED_H)
        set("${CMAKE_PROJECT_NAME}_LIB_H" ${DETECTED_H})
    endif()
endif()

if("${${CMAKE_PROJECT_NAME}_LIB_CPP}" STREQUAL "")
    file(GLOB_RECURSE DETECTED_CPP 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    )
    if(DETECTED_CPP)
        set("${CMAKE_PROJECT_NAME}_LIB_CPP" ${DETECTED_CPP})
    endif()
endif()

# ==============================================================================
# БЕЗОПАСНАЯ СБОРКА БИБЛИОТЕКИ
# ==============================================================================
if(${CMAKE_PROJECT_NAME}_LIB_H OR ${CMAKE_PROJECT_NAME}_LIB_CPP)
    
    # Автоматическое определение типа библиотеки
    if(NOT ${CMAKE_PROJECT_NAME}_LIB_CPP)
        set(LIB_TYPE "INTERFACE")
        set(SCOPE_TYPE "INTERFACE")
    else()
        set(LIB_TYPE "STATIC")
        set(SCOPE_TYPE "PUBLIC")
    endif()
    
    message(STATUS "[INFO] Библиотека '${CMAKE_PROJECT_NAME}' инициализирована как: ${LIB_TYPE}")

    if(LIB_TYPE STREQUAL "INTERFACE")
        add_library(${CMAKE_PROJECT_NAME} INTERFACE)
        target_sources(${CMAKE_PROJECT_NAME} INTERFACE ${${CMAKE_PROJECT_NAME}_LIB_H})
    else()
        add_library(${CMAKE_PROJECT_NAME} STATIC)
        # Безопасно добавляем исходники, отсекая пустые списки
        if(${CMAKE_PROJECT_NAME}_LIB_H)
            target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${${CMAKE_PROJECT_NAME}_LIB_H})
        endif()
        if(${CMAKE_PROJECT_NAME}_LIB_CPP)
            target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${${CMAKE_PROJECT_NAME}_LIB_CPP})
        endif()
    endif()

    target_include_directories(${CMAKE_PROJECT_NAME} ${SCOPE_TYPE}
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    )

    if(${CMAKE_PROJECT_NAME}_LIBS)
        target_link_libraries(${CMAKE_PROJECT_NAME} ${SCOPE_TYPE} ${${CMAKE_PROJECT_NAME}_LIBS})
    endif()

else()
    message(STATUS "[NOTICE] Исходные файлы библиотеки не обнаружены. Таргет библиотеки '${CMAKE_PROJECT_NAME}' создан не будет.")
endif()

# ==============================================================================
# БЕЗОПАСНАЯ СБОРКА ПРИЛОЖЕНИЯ-ПРИМЕРА
# ==============================================================================
if(${CMAKE_PROJECT_NAME}_BUILD_EXAMPLE)
    
    set(CAN_BUILD_EXAMPLE TRUE)

    # Мягкая проверка: задан ли путь?
    if("${${CMAKE_PROJECT_NAME}_EXAMPLE_FILEPATH}" STREQUAL "")
        message(STATUS "[NOTICE] Пример не будет собран: Переменная '${CMAKE_PROJECT_NAME}_EXAMPLE_FILEPATH' пуста.")
        set(CAN_BUILD_EXAMPLE FALSE)
    # Мягкая проверка: существует ли файл физически?
    elseif(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${${CMAKE_PROJECT_NAME}_EXAMPLE_FILEPATH}")
        message(STATUS "[NOTICE] Пример не будет собран: Файл '${${CMAKE_PROJECT_NAME}_EXAMPLE_FILEPATH}' не найден на диске.")
        set(CAN_BUILD_EXAMPLE FALSE)
    endif()

    # Сборка таргета только при полной готовности
    if(CAN_BUILD_EXAMPLE)
        message(STATUS "[INFO] Сборка приложения-примера '${CMAKE_PROJECT_NAME}_example' успешно запущена.")
        
        add_executable(${CMAKE_PROJECT_NAME}_example ${${CMAKE_PROJECT_NAME}_EXAMPLE_FILEPATH})
            
        if(TARGET ${CMAKE_PROJECT_NAME})
            target_link_libraries(${CMAKE_PROJECT_NAME}_example PRIVATE ${CMAKE_PROJECT_NAME})
        endif()
            
        if(${CMAKE_PROJECT_NAME}_EXAMPLE_LIBS)
            target_link_libraries(${CMAKE_PROJECT_NAME}_example PRIVATE ${${CMAKE_PROJECT_NAME}_EXAMPLE_LIBS})
        endif()

        # Логика копирования ассетов
        if(${CMAKE_PROJECT_NAME}_ASSET_TO_BUILD)
            if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
                add_custom_command(TARGET ${CMAKE_PROJECT_NAME}_example POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
                        "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}_example>/assets"
                )
            else()
                message(WARNING "[WARNING] Папка assets не найдена, копирование в билд пропущено.")
            endif()
        endif()
    endif()
endif()