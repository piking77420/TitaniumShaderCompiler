cmake_minimum_required(VERSION 3.26)

project(UnitTest VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
include(GoogleTest)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.18.0
    GIT_SHALLOW TRUE
)

# Prevent GoogleTest from overriding the MSVC runtime configuration.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

if (WIN32)
    set(TEST_SOURCES
    window_test.cpp)
endif()

add_executable(
        ${PROJECT_NAME}
        ${TEST_SOURCES}
    )

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    TitaniumShaderCompiler
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(
    ${PROJECT_NAME}
    DISCOVERY_MODE PRE_TEST
)

if(WIN32)
message(STATUS "Copying DXC DLL")
    add_custom_command(
        TARGET UnitTest
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DXC_DLL}"
            "$<TARGET_FILE_DIR:UnitTest>"
        VERBATIM
    )
endif()



