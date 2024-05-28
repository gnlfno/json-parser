cmake_minimum_required(VERSION 3.18)
project(JsonParser LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find all .cpp files in the current directory
file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Define the executable target
add_executable(JsonParser ${SOURCES})


