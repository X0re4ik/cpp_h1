cmake_minimum_required(VERSION 3.22)

project(HOMEWORK1 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

SET(HOMEWORK1 h1)

add_executable(${HOMEWORK1}
    src/main.cpp
)

target_compile_options(${HOMEWORK1}
  PRIVATE
    -g
    -fpermissive
    -Wall
    -Wextra
    -Werror
    -Wpedantic
)

set_target_properties(${HOMEWORK1}
  PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

if(USE_CLANG_FORMAT_CHECK)
    find_program(CLANG_FORMAT_PATH NAMES clang-format-21 REQUIRED)
    message(STATUS "Found clang-format: ${CLANG_FORMAT_PATH}")

    file(GLOB_RECURSE CLANG_FORMAT_SRC CONFIGURE_DEPENDS
        src/*.h
        src/*.cpp
        src/*.hpp
    )

    add_custom_target(clang-format-fix
        COMMAND
            ${CLANG_FORMAT_PATH} -style=file -i ${CLANG_FORMAT_SRC}
        WORKING_DIRECTORY
            ${CMAKE_SOURCE_DIR}
        COMMENT
            "Formatting ${HOMEWORK1} with ${CLANG_FORMAT_PATH} ..."
    )

    add_custom_target(clang-format
        COMMAND
            python3.10 run-clang-format.py --clang-format-executable=${CLANG_FORMAT_PATH} --style=file ${CLANG_FORMAT_SRC}
        WORKING_DIRECTORY
            ${CMAKE_SOURCE_DIR}
        COMMENT
            "Checking format ${HOMEWORK1} with ${CLANG_FORMAT_PATH} ..."
    )
    add_dependencies(${HOMEWORK1} clang-format)
endif()

if(USE_CLANG_FORMAT_FIX)
    find_program(CLANG_FORMAT_PATH NAMES clang-format-21 REQUIRED)
    message(STATUS "Found clang-format: ${CLANG_FORMAT_PATH}")

    file(GLOB_RECURSE CLANG_FORMAT_SRC CONFIGURE_DEPENDS
        src/*.h
        src/*.cpp
        src/*.hpp
    )

    add_custom_target(clang-format-fix
        COMMAND
            ${CLANG_FORMAT_PATH} -style=file -i ${CLANG_FORMAT_SRC}
        WORKING_DIRECTORY
            ${CMAKE_SOURCE_DIR}
        COMMENT
            "Formatting ${HOMEWORK1} with ${CLANG_FORMAT_PATH} ..."
    )

    add_dependencies(${HOMEWORK1} clang-format-fix)
endif()

# 
if(USE_CLANG_TIDY)
    find_program(CLANG_TIDY_PATH NAMES clang-tidy-21 REQUIRED)
    file(GLOB_RECURSE CLANG_FORMAT_SRC CONFIGURE_DEPENDS
        src/*.h
        src/*.cpp
        src/*.hpp
    )
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_PATH}")
    set(CLANG_TIDY_COMMAND ${CLANG_TIDY_PATH} "--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy" ${CLANG_FORMAT_SRC})
    message(STATUS "Clang tidy command: ${CLANG_TIDY_COMMAND}")
    add_custom_target(
        clang-tidy-check
        COMMAND
            ${CLANG_TIDY_COMMAND}
        WORKING_DIRECTORY
            ${CMAKE_SOURCE_DIR}
        COMMENT
            "Tidy ${HOMEWORK1} with ${CLANG_FORMAT_PATH} ..."
    )
    add_dependencies(${HOMEWORK1} clang-tidy-check)
    set_target_properties(
        ${HOMEWORK1} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
endif()