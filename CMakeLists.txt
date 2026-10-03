cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_COMPILER clang++)

project(h1 VERSION 1.0 LANGUAGES CXX)

message(STATUS "CXX Compiler ID:      ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CXX Compiler Path:    ${CMAKE_CXX_COMPILER}")
message(STATUS "CXX Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(H1_CORE_SOURCES
    src/logger/logger.cpp
    src/calculator/calc.cpp
    src/calculator/di.cpp
    src/json_parser/impl.cpp
    src/application.cpp
    src/calculation/service.cpp
    src/argparse/argparse.cpp
    # db::pg
    src/db/pg/pg_connector.cpp
    src/db/pg/pg_params.cpp
    src/db/pg/pg_result.cpp
    # db
    src/db/manager.cpp
    # lcache
    src/local_cache/local_cache.cpp
    # config
    src/config/config.cpp
)

add_library(h1_core STATIC ${H1_CORE_SOURCES})

target_include_directories(h1_core
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(
    ${PROJECT_NAME}
    cmd/main.cpp
)

set(H1_COMPILE_OPTIONS
    -Wall
    -Wextra
    -Werror
    -Wpedantic
)
message(STATUS "[INFO] Set compile options: ${H1_COMPILE_OPTIONS}")
target_compile_options(${PROJECT_NAME}
    PRIVATE
    ${H1_COMPILE_OPTIONS}
)
target_compile_options(h1_core
    PRIVATE
    ${H1_COMPILE_OPTIONS}
)

target_compile_features(h1_core PUBLIC cxx_std_20)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

message(STATUS "[INFO] Load external dependence with FetchContent")
include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.17.0
)
message(DEBUG "spdlog_POPULATED=${spdlog_POPULATED}")
FetchContent_MakeAvailable(spdlog)
message(STATUS "[INFO] spdlog 🟢")

FetchContent_Declare(
    cpp_h1_lib
    GIT_REPOSITORY https://github.com/X0re4ik/cpp_h1_lib.git
    GIT_TAG v1.1.11-header-only
)
message(DEBUG "cpp_h1_lib_POPULATED=${cpp_h1_lib_POPULATED}")
FetchContent_MakeAvailable(cpp_h1_lib)
message(STATUS "[INFO] cpp_h1_lib 🟢")

FetchContent_Declare(
    json
    URL
    https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)
FetchContent_MakeAvailable(json)
message(DEBUG "json_POPULATED=${json_POPULATED}")
message(STATUS "[INFO] json 🟢")

FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG v3.2
)
FetchContent_MakeAvailable(argparse)
message(DEBUG "argparse_POPULATED=${argparse_POPULATED}")
message(STATUS "[INFO] argparse 🟢")
FetchContent_Declare(
    libenvpp
    GIT_REPOSITORY https://github.com/ph3at/libenvpp.git
    GIT_TAG v1.5.3
)
FetchContent_MakeAvailable(libenvpp)
message(DEBUG "libenvpp_POPULATED=${argparse_POPULATED}")
message(STATUS "[INFO] libenvpp 🟢")

message(STATUS "[INFO] Load external dependence with `find_package`")
message(STATUS "[INFO] REQUIRED: PostgreSQL")
find_package(PostgreSQL REQUIRED)

target_link_libraries(
    h1_core
    PUBLIC
    H1
    nlohmann_json::nlohmann_json
    PostgreSQL::PostgreSQL
    libenvpp
    PRIVATE
    argparse
    spdlog::spdlog
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    h1_core
)


message(STATUS "Use Clang Format Fix: ${USE_CLANG_FORMAT_FIX}")
if(USE_CLANG_FORMAT_FIX)
    find_program(CLANG_FORMAT_PATH NAMES clang-format-21 clang-format REQUIRED)

    file(GLOB_RECURSE CLANG_FORMAT_SRC CONFIGURE_DEPENDS
        include/*.h
        include/*.hpp
        src/*.h
        src/*.hpp
        src/*.cpp
        cmd/*.h
        cmd/*.hpp
        cmd/*.cpp
    )

    set(
        CLANG_FORMAT_COMMAND
        ${CLANG_FORMAT_PATH}
        -style=file
        -i ${CLANG_FORMAT_SRC}
    )

    string(JOIN " " CLANG_FORMAT_PRETTY ${CLANG_FORMAT_COMMAND})
    message(STATUS "${CLANG_FORMAT_PRETTY}")

    add_custom_target(clang-format-fix
        COMMAND
        ${CLANG_FORMAT_COMMAND}
        WORKING_DIRECTORY
        ${CMAKE_SOURCE_DIR}
        COMMENT
        "Formatting ${PROJECT_NAME} with ${CLANG_FORMAT_PATH} ..."
    )
endif()

message(STATUS "Use Clang Tidy: ${USE_CLANG_TIDY}")
if(USE_CLANG_TIDY)
    find_program(CLANG_TIDY_PATH NAMES clang-tidy-21 clang-tidy REQUIRED)
    file(GLOB_RECURSE CLANG_TIDY_SRC CONFIGURE_DEPENDS
        src/*.cpp
        cmd/*.cpp
    )

    set(
        CLANG_TIDY_COMMAND
        ${CLANG_TIDY_PATH}
        "-p=${CMAKE_BINARY_DIR}"
        "--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy"
        ${CLANG_TIDY_SRC}
    )

    string(JOIN " " CLANG_TIDY_COMMAND_PRETTY ${CLANG_TIDY_COMMAND})
    message(STATUS "${CLANG_TIDY_COMMAND_PRETTY}")

    add_custom_target(
        clang-tidy-check
        COMMAND
        ${CLANG_TIDY_COMMAND}
        WORKING_DIRECTORY
        ${CMAKE_SOURCE_DIR}
        COMMENT
        "Tidy ${PROJECT_NAME} with ${CLANG_TIDY_PATH} ..."
    )
endif()

include(CTest)
message(STATUS "Build Test: ${BUILD_TESTING}")

if(BUILD_TESTING)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    FetchContent_MakeAvailable(googletest)
    message(DEBUG "googletest_POPULATED=${googletest_POPULATED}")
    message(STATUS "[INFO] googletest 🟢")

    add_executable(h1_test
        src/test_logger.cpp
        src/test_calculator.cpp
        src/test_json_parser.cpp
        #src/test_application.cpp
    )

    target_compile_features(h1_test PRIVATE cxx_std_20)
    target_compile_options(h1_test
        PRIVATE
        ${H1_COMPILE_OPTIONS}
    )

    target_link_libraries(h1_test
        PRIVATE
        h1_core
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(h1_test)
endif()

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
