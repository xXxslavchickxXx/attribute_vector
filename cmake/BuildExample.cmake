if(__LIB_EXECUTABLE_INCLUDED)
    return()
endif()
set(__LIB_EXECUTABLE_INCLUDED TRUE)

function(create_executable TARGET_NAME)
    set(options "")
    set(oneValueArgs SOURCE COPY_ASSETS)
    set(multiValueArgs LIBS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Проверка: указан ли источник?
    if(NOT ARG_SOURCE)
        message(STATUS "[NOTICE] Исполняемый файл '${TARGET_NAME}' не будет собран: не указан SOURCE.")
        return()
    endif()
    
    # Проверка: существует ли файл?
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_SOURCE}")
        message(STATUS "[NOTICE] Исполняемый файл '${TARGET_NAME}' не будет собран: файл '${ARG_SOURCE}' не найден.")
        return()
    endif()
    
    message(STATUS "[INFO] Сборка исполняемого файла '${TARGET_NAME}' успешно запущена.")
    
    # Создаём исполняемый файл
    add_executable(${TARGET_NAME} ${ARG_SOURCE})
    
    # Линкуем библиотеки (теперь работает со списком)
    if(ARG_LIBS)
        target_link_libraries(${TARGET_NAME} PRIVATE ${ARG_LIBS})
    endif()
    
    # Копирование ассетов
    if(ARG_COPY_ASSETS)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
                    "$<TARGET_FILE_DIR:${TARGET_NAME}>/assets"
                COMMENT "Copying assets to ${TARGET_NAME}"
            )
        else()
            message(WARNING "[WARNING] Папка assets не найдена, копирование в билд пропущено.")
        endif()
    endif()
endfunction()