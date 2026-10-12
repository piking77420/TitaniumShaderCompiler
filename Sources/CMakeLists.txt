cmake_minimum_required(VERSION 3.26)

project(TitaniumShaderCompiler LANGUAGES CXX)

add_library(TitaniumShaderCompiler STATIC
    ShaderCompiler.cpp
    ShaderCompiler.hpp
)

target_compile_features(TitaniumShaderCompiler PUBLIC cxx_std_23)

target_include_directories(TitaniumShaderCompiler
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Find DXC
find_path(DXC_INCLUDE_DIR
    NAMES dxc/dxcapi.h
    HINTS
        "$ENV{VULKAN_SDK}/Include"
        "$ENV{VULKAN_SDK}/include"
    REQUIRED
)

find_library(DXC_LIBRARY
    NAMES dxcompiler
    HINTS
        "$ENV{VULKAN_SDK}/Lib"
        "$ENV{VULKAN_SDK}/lib"
    REQUIRED
)

find_file(DXC_DLL
    NAMES dxcompiler.dll
    PATHS "$ENV{VULKAN_SDK}/Bin"
    NO_DEFAULT_PATH
    REQUIRED
)

message(STATUS "DXC_DLL = ${DXC_DLL}")


if(NOT DXC_INCLUDE_DIR OR NOT DXC_LIBRARY)
    message(FATAL_ERROR "DXC compiler SDK not found")
endif()

target_include_directories(TitaniumShaderCompiler
    PRIVATE
        "${DXC_INCLUDE_DIR}"
)

target_link_libraries(TitaniumShaderCompiler
    PRIVATE
        "${DXC_LIBRARY}"
)