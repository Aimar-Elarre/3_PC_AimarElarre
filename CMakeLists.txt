cmake_minimum_required(VERSION 3.16)

project(OpenGLApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(INCLUDE_DIR ${ROOT_DIR}/include)
set(SRC_DIR ${ROOT_DIR}/src)
set(EXTERNAL_DIR ${ROOT_DIR}/external)

if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Usa siempre: cmake -S . -B build")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT EXISTS ${SRC_DIR})
    message(FATAL_ERROR "No existe src/")
endif()

if(NOT EXISTS ${INCLUDE_DIR})
    message(FATAL_ERROR "No existe include/")
endif()

if(NOT EXISTS ${EXTERNAL_DIR}/glfw)
    message(FATAL_ERROR "No existe external/glfw")
endif()

if(NOT EXISTS ${EXTERNAL_DIR}/glad/src/glad.c)
    message(FATAL_ERROR "No existe external/glad/src/glad.c")
endif()

add_subdirectory(${EXTERNAL_DIR}/glfw)

add_library(glad ${EXTERNAL_DIR}/glad/src/glad.c)
target_include_directories(glad PUBLIC ${EXTERNAL_DIR}/glad/include)

file(GLOB_RECURSE CPP_FILES ${SRC_DIR}/*.cpp)
file(GLOB_RECURSE H_FILES ${INCLUDE_DIR}/*.h)

add_executable(OpenGLApp ${CPP_FILES} ${H_FILES})

source_group("Source Files" FILES ${CPP_FILES})
source_group("Header Files" FILES ${H_FILES})

target_include_directories(OpenGLApp PRIVATE
    ${INCLUDE_DIR}
    ${EXTERNAL_DIR}/glad/include
    ${EXTERNAL_DIR}/glfw/include
    ${EXTERNAL_DIR}/glm
)

target_link_libraries(OpenGLApp PRIVATE glad glfw opengl32)