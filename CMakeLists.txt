cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(physical_simulation)
set (CMAKE_CXX_STANDARD 11)

if(NOT DEFINED TARGET_NAME) 
	set (TARGET_NAME mass_spring)
endif()

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/src/${TARGET_NAME}/*.c
	${CMAKE_SOURCE_DIR}/src/${TARGET_NAME}/*.cpp)
	
# Add header files
file(GLOB_RECURSE HEADER_FILES 
	${CMAKE_SOURCE_DIR}/src/${TARGET_NAME}/*.h
	${CMAKE_SOURCE_DIR}/src/${TARGET_NAME}/*.hpp)

file(GLOB_RECURSE COMMON_HEADER_FILES 
	${CMAKE_SOURCE_DIR}/utils/*.h
	${CMAKE_SOURCE_DIR}/utils/*.hpp)

file(GLOB_RECURSE COMMON_SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/utils/*.c
	${CMAKE_SOURCE_DIR}/utils/*.cpp)

link_directories(${CMAKE_SOURCE_DIR}/lib)

# Configure assets header file
configure_file(${CMAKE_SOURCE_DIR}/src/utils/RootDir.h.in ${CMAKE_SOURCE_DIR}/src/utils/RootDir.h)
include_directories(${CMAKE_BINARY_DIR}/)

add_executable(${TARGET_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${COMMON_HEADER_FILES} ${COMMON_SOURCE_FILES})

# OpenGL
find_package(OpenGL REQUIRED)

# GLFW
find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

find_package (Eigen3 3.3 REQUIRED NO_MODULE)

# GLAD
add_library(GLAD "thirdparty/glad.c")

# Put all libraries into a variable
set(LIBS OpenGL::GL GLAD Eigen3::Eigen)

# Define the include DIRs
include_directories(
    PUBLIC ${CMAKE_SOURCE_DIR}/
	PUBLIC ${CMAKE_SOURCE_DIR}/include
)

# Define the link libraries
target_link_libraries(${TARGET_NAME} ${LIBS})
target_link_libraries(${TARGET_NAME} ${CMAKE_SOURCE_DIR}/lib/glfw3.dylib)