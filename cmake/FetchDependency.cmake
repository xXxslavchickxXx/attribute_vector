if(__FETCH_DEPENDENCY_INCLUDED)
    return()
endif()
set(__FETCH_DEPENDENCY_INCLUDED TRUE)

function(add_external_dependency LIB_NAME GIT_URL)
	if(TARGET ${LIB_NAME})
		message(STATUS "[AUTO-DEP] ${LIB_NAME} уже предоставлен. Пропускаем.")
		return()
	endif()
    # Параметры: LIB_NAME, GIT_URL, GIT_TAG (опционально)
    set(options "")
    set(oneValueArgs GIT_TAG)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Если тег не указан, используем "main"
    if(NOT ARG_GIT_TAG)
        set(ARG_GIT_TAG "main")
    endif()
    
    # Если таргет уже есть, выходим
    if(TARGET ${LIB_NAME})
        message(STATUS "[AUTO-DEP] ${LIB_NAME} уже предоставлен. Пропускаем.")
        return()
    endif()

    set(LOCAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/${LIB_NAME}")
    set(NEED_DOWNLOAD FALSE)

    if(EXISTS "${LOCAL_PATH}/CMakeLists.txt")
        message(STATUS "[AUTO-DEP] ${LIB_NAME} найден локально. Проверяем актуальность...")
        
        # Получаем локальный хэш
        execute_process(
            COMMAND git rev-parse HEAD
            WORKING_DIRECTORY "${LOCAL_PATH}"
            OUTPUT_VARIABLE LOCAL_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        
        # Пробуем как ветку
        execute_process(
            COMMAND git ls-remote ${GIT_URL} refs/heads/${ARG_GIT_TAG}
            OUTPUT_VARIABLE REMOTE_LS
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        
        # Если не ветка, пробуем как тег
        if(NOT REMOTE_LS)
            execute_process(
                COMMAND git ls-remote ${GIT_URL} refs/tags/${ARG_GIT_TAG}
                OUTPUT_VARIABLE REMOTE_LS
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )
        endif()
        
        # Если не тег, пробуем как коммит
        if(NOT REMOTE_LS)
            execute_process(
                COMMAND git ls-remote ${GIT_URL} ${ARG_GIT_TAG}
                OUTPUT_VARIABLE REMOTE_LS
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )
        endif()
        
        string(REGEX MATCH "^[0-9a-f]+" REMOTE_HASH "${REMOTE_LS}")

        if(NOT "${LOCAL_HASH}" STREQUAL "${REMOTE_HASH}" AND NOT "${REMOTE_HASH}" STREQUAL "")
            message(STATUS "[AUTO-DEP] Новая версия ${LIB_NAME} (${ARG_GIT_TAG})! Перекачиваем...")
            set(NEED_DOWNLOAD TRUE)
            file(REMOVE_RECURSE "${LOCAL_PATH}")
        elseif(NOT REMOTE_HASH)
            message(WARNING "[AUTO-DEP] Не удалось определить хэш для ${LIB_NAME}:${ARG_GIT_TAG}")
        endif()
    else()
        set(NEED_DOWNLOAD TRUE)
    endif()

    if(NOT NEED_DOWNLOAD)
        message(STATUS "[AUTO-DEP] Версия ${LIB_NAME} (${ARG_GIT_TAG}) актуальна. Подключаем...")
        add_subdirectory("${LOCAL_PATH}" "${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}_build")
    else()
        message(STATUS "[AUTO-DEP] Скачиваем ${LIB_NAME} (${ARG_GIT_TAG})...")

        # Добавляем в .gitignore
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

        include(FetchContent)
        FetchContent_Declare(
            ${LIB_NAME}
            GIT_REPOSITORY ${GIT_URL}
            GIT_TAG        ${ARG_GIT_TAG}
            SOURCE_DIR     "${LOCAL_PATH}"
            BINARY_DIR     "${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}_build"
        )
        FetchContent_MakeAvailable(${LIB_NAME})
    endif()
endfunction()