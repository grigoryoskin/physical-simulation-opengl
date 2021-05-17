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


# Configure assets header file
configure_file(${CMAKE_SOURCE_DIR}/src/utils/RootDir.h.in ${CMAKE_SOURCE_DIR}/src/utils/RootDir.h)
include_directories(${CMAKE_BINARY_DIR}/)

add_executable(${TARGET_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${COMMON_HEADER_FILES} ${COMMON_SOURCE_FILES})

# Find OpenGL installed on system.
find_package(OpenGL REQUIRED)

# Find Eigen installed on system.
find_package (Eigen3 3.3 REQUIRED NO_MODULE)

# Run cmake file in subdirectory.
add_subdirectory(external/glfw)
# Link a directory with generated glfw.
target_link_directories(${TARGET_NAME} PRIVATE external/glfw/src)

# Add GLAD library.
add_library(GLAD "external/glad/src/glad.c")

# Put all libraries into a variable
set(LIBS OpenGL::GL glfw GLAD Eigen3::Eigen)

# Define the include DIRs to search for headers.
include_directories(
    PUBLIC ${CMAKE_SOURCE_DIR}/external/glfw/include
	PUBLIC ${CMAKE_SOURCE_DIR}/external/glad/include
)

# Link libraries to executable target.
target_link_libraries(${TARGET_NAME} ${LIBS})
