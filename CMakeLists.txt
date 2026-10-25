cmake_minimum_required(VERSION 3.16)

project(algorithm_practice_notebook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(task_store src/task_store.cpp)
target_include_directories(task_store PUBLIC src)

add_executable(task_notebook src/main.cpp)
target_link_libraries(task_notebook PRIVATE task_store)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

add_executable(task_notebook_gui WIN32 gui/main.cpp)
target_link_libraries(task_notebook_gui PRIVATE task_store Qt6::Widgets Qt6::Network)
