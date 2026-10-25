cmake_minimum_required(VERSION 3.16)

project(algorithm_practice_notebook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

add_executable(task_notebook_gui WIN32
    gui/main.cpp
    src/task_store.cpp
)
target_include_directories(task_notebook_gui PRIVATE src)
target_link_libraries(task_notebook_gui PRIVATE Qt6::Widgets Qt6::Network)
