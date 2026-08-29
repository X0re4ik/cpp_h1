cmake_minimum_required(VERSION 3.22)

project(HOMEWORK2 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(HOMEWORK2 h1)
set(MAIN_CMD "cmd/main.cpp")



option(BUILD_TESTS "Build tests" OFF)
message(STATUS "[INFO] Build Test: ${BUILD_TESTS}")
message(STATUS "[INFO] Get all translation units excludes tests")

SET(SOURCES "src/logger/logger.cpp")

message(DEBUG "[DEBUG] Translation Units: ${SOURCES}")

message(STATUS "[INFO] Create objects files")
add_executable(${HOMEWORK2} ${SOURCES} ${MAIN_CMD})

message(STATUS "[INFO] Set include directory base is 'include'")
target_include_directories(${HOMEWORK2}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set(COMPILE_OPTIONS "-g -fpermissive -Wall -Wextra -Werror -Wpedantic")
message(STATUS "[INFO] Set compile options: ${COMPILE_OPTIONS}")
target_compile_options(${HOMEWORK2}
    PRIVATE
    -g -fpermissive -Wall -Wextra -Werror -Wpedantic
)

set(CPP_STANDARD 20)
message(STATUS "[INFO] Standard Version: C++${CPP_STANDARD}")
set_target_properties(${HOMEWORK2}
    PROPERTIES
    CXX_STANDARD ${CPP_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# ==== FetchContent ==== 
message(STATUS "[INFO] Load external dependence with FetchContent")
include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.x
)
message(DEBUG "spdlog_POPULATED=${spdlog_POPULATED}")
FetchContent_MakeAvailable(spdlog)
message(STATUS "[INFO] spdlog 🟢")

if(BUILD_TESTS)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    FetchContent_MakeAvailable(googletest)
    message(DEBUG "googletest_POPULATED=${googletest_POPULATED}")
    message(STATUS "[INFO] googletest 🟢")
endif()
# ==== END ====

# ==== target_link_libraries ====
set(LINKED_LIBRARIES spdlog)
message(STATUS "Link libraries: ${LINKED_LIBRARIES}")
target_link_libraries(${HOMEWORK2}
    PRIVATE
    ${LINKED_LIBRARIES}
)

if(BUILD_TESTS)
    enable_testing()
    set(TEST_EXECUTABLE ${HOMEWORK2}_test)
    add_executable(
        ${TEST_EXECUTABLE}
        ${SOURCES}
        "src/test_logger.cpp"
    )
    target_include_directories(${TEST_EXECUTABLE}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    list(APPEND LINKED_LIBRARIES GTest::gtest_main)
    target_link_libraries(${TEST_EXECUTABLE}
        PRIVATE
        ${LINKED_LIBRARIES}
    )
    include(GoogleTest)
    gtest_discover_tests(${TEST_EXECUTABLE})
endif()

