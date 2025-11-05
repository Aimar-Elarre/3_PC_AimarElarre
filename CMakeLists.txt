cmake_minimum_required(VERSION 3.16)
project(OpenGLApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Estructura de carpetas
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external")

# Salidas organizadas
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

# GLFW
add_subdirectory(${EXTERNAL_DIR}/glfw)

# GLAD
add_library(glad "${EXTERNAL_DIR}/glad/src/glad.c")
target_include_directories(glad PUBLIC "${EXTERNAL_DIR}/glad/include")

# Proyecto principal
file(GLOB PROJECT_SOURCES "${SRC_DIR}/*.cpp")
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${INCLUDE_DIR}
    ${EXTERNAL_DIR}/glad/include
    ${EXTERNAL_DIR}/glfw/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE glad glfw)
