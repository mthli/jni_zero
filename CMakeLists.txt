cmake_minimum_required(VERSION 3.4.1)
project(jni_zero LANGUAGES CXX)

# C++ standard requirements.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add source files.
set(SOURCE_FILES
    core.cc
    logging.cc
    scoped_java_ref.cc
    )

# Create target.
add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC .)
