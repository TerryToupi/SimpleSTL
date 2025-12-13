cmake_minimum_required(VERSION 3.20)
project(SimpleSTL LANGUAGES CXX)

file(GLOB_RECURSE FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h")

add_library(SimpleSTL INTERFACE ${FILES})

target_include_directories(SimpleSTL
    INTERFACE 
        "${CMAKE_CURRENT_SOURCE_DIR}/Include"
)

target_compile_features(${PROJECT_NAME}
    INTERFACE 
        cxx_std_20
)

add_subdirectory(Benchmark)
