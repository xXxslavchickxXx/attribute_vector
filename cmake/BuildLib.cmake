if(__LIB_CREATED_INCLUDED)
    return()
endif()
set(__LIB_CREATED_INCLUDED TRUE)

# Функция для сборки либы
function(create_library TARGET_NAME)
    cmake_parse_arguments(ARG "" "HEADERS;SOURCES;INCLUDE_DIRS;LIBS" "" ${ARGN})
    
    # Автоматическое определение типа библиотеки
    if(NOT ARG_SOURCES)
        set(LIB_TYPE "INTERFACE")
        set(SCOPE_TYPE "INTERFACE")
    else()
        set(LIB_TYPE "STATIC")
        set(SCOPE_TYPE "PUBLIC")
    endif()
    
    message(STATUS "[INFO] Библиотека '${TARGET_NAME}' инициализирована как: ${LIB_TYPE}")

    if(LIB_TYPE STREQUAL "INTERFACE")
        add_library(${TARGET_NAME} INTERFACE)
        if(ARG_HEADERS)
            target_sources(${TARGET_NAME} INTERFACE ${ARG_HEADERS})
        endif()
    else()
        add_library(${TARGET_NAME} STATIC)
        if(ARG_HEADERS)
            target_sources(${TARGET_NAME} PRIVATE ${ARG_HEADERS})
        endif()
        if(ARG_SOURCES)
            target_sources(${TARGET_NAME} PRIVATE ${ARG_SOURCES})
        endif()
    endif()

    # Добавляем include директории
    if(NOT ARG_INCLUDE_DIRS)
        set(ARG_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")
    endif()
    target_include_directories(${TARGET_NAME} ${SCOPE_TYPE} ${ARG_INCLUDE_DIRS})

    # Линкуем библиотеки
    if(ARG_LIBS)
        target_link_libraries(${TARGET_NAME} ${SCOPE_TYPE} ${ARG_LIBS})
    endif()
endfunction()