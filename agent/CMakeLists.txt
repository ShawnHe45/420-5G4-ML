cmake_minimum_required(VERSION 3.21)
project(bibliotheque)

file(GLOB SOURCES "*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

# Specifie qu'on veut la version C++17 du langage
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# flag pour tous les warnings possible
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

set(CORE_SOURCES book.cpp user.cpp library.cpp filemanager.cpp)

add_library(bibliotheque_core STATIC ${CORE_SOURCES})
target_include_directories(bibliotheque_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(bibliotheque_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(bibliotheque main.cpp)
target_link_libraries(bibliotheque PRIVATE bibliotheque_core)
target_compile_options(bibliotheque PRIVATE -Wall -Wextra -Wpedantic)

option(BUILD_PYTHON_BINDINGS "Build the pybind11 module" OFF)

if(BUILD_PYTHON_BINDINGS)
    find_package(pybind11 CONFIG REQUIRED)
    
    pybind11_add_module(bibliotheque_py MODULE bindings.cpp)
    target_link_libraries(bibliotheque_py PRIVATE bibliotheque_core)

    set_target_properties(bibliotheque_py PROPERTIES
            OUTPUT_NAME bibliotheque_core
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/python")
endif()
