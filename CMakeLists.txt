# 1. Set the minimum required CMake version
cmake_minimum_required(VERSION 3.10)

# 2. Define the project name and language
project(MyProject VERSION 1.0 LANGUAGES CXX)

# 3. Specify the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 4. Define the executable and its source files
add_executable(citterfish src/main.cpp src/board.cpp src/zobrist.cpp)
target_compile_options(citterfish PRIVATE
    -Ofast
)