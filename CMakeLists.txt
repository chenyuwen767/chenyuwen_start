cmake_minimum_required(VERSION 3.16)

project(algorithm_practice_notebook VERSION 0.3.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Network)

add_executable(task_notebook_gui WIN32
    gui/main.cpp
    src/app_paths.cpp
    src/task_export.cpp
    src/task_store.cpp
)
target_include_directories(task_notebook_gui PRIVATE src)
target_link_libraries(task_notebook_gui PRIVATE Qt6::Widgets Qt6::Network)
target_compile_definitions(task_notebook_gui PRIVATE APP_VERSION="${PROJECT_VERSION}")

if (MSVC)
    target_compile_options(task_notebook_gui PRIVATE /W4 /permissive-)
else()
    target_compile_options(task_notebook_gui PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS task_notebook_gui RUNTIME DESTINATION .)

include(CTest)
if (BUILD_TESTING)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Test)

    add_executable(test_task_store
        tests/test_task_store.cpp
        src/task_store.cpp
    )
    target_include_directories(test_task_store PRIVATE src tests)
    target_link_libraries(test_task_store PRIVATE Qt6::Test)

    add_executable(test_app_paths
        tests/test_app_paths.cpp
        src/app_paths.cpp
    )
    target_include_directories(test_app_paths PRIVATE src tests)
    target_link_libraries(test_app_paths PRIVATE Qt6::Test)

    add_executable(test_task_export
        tests/test_task_export.cpp
        src/task_export.cpp
    )
    target_include_directories(test_task_export PRIVATE src)
    target_link_libraries(test_task_export PRIVATE Qt6::Test)

    if (MSVC)
        target_compile_options(test_task_store PRIVATE /W4 /permissive-)
        target_compile_options(test_app_paths PRIVATE /W4 /permissive-)
        target_compile_options(test_task_export PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_task_store PRIVATE -Wall -Wextra -Wpedantic)
        target_compile_options(test_app_paths PRIVATE -Wall -Wextra -Wpedantic)
        target_compile_options(test_task_export PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME task_store COMMAND test_task_store)
    add_test(NAME app_paths COMMAND test_app_paths)
    add_test(NAME task_export COMMAND test_task_export)
endif()
