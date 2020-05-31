cmake_minimum_required (VERSION 3.8)

project (InteractiveGLSL)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -pthread")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -O2")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
endif()
find_package(OpenMP)
if (OPENMP_FOUND)
    message(STATUS "OpenMP: enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")      # Parallel compilation
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")    # Enable exception
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251")  # Disable warning for improper text encoding
    add_definitions("-D_CRT_SECURE_NO_WARNINGS")
    add_definitions("-D_USE_MATH_DEFINES")
endif()

if (APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo")
endif()


set(SRC_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE PATH "")

file(GLOB SRC_FILES
	"core/*.cpp"
	"core/*.h"
    "opnegl-wrapper/*.cpp"
    "opengl-wrapper/*.h"
    "glad/glad.h"
    "glad/glad.cpp")

file(GLOB SRC_FILES_RELATIVE RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	${SRC_FILES})


file(GLOB SHADER_FILES
	"shaders/*.vert"
	"shaders/*.frag")


find_package(OpenGL REQUIRED)
find_package(GLFW3 REQUIRED)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(main_app "InteractiveGLSL")

add_executable(${main_app} "main.cpp" ${SRC_FILES} ${SHADER_FILES})
include_directories(${SRC_INCLUDE_DIR}  ${GLFW3_INCLUDE_DIR})
target_link_libraries(${main_app} ${OPENGL_LIBRARIES} ${GLFW3_LIBRARY})

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON) 
source_group("Shader Files" FILES ${SHADER_FILES})

foreach(src IN LISTS SRC_FILES_RELATIVE)
	get_filename_component(src_dir "${src}" PATH)
	string(REPLACE "/" "\\" src_dir ${src_dir})
    source_group("Source Files\\${src_dir}" FILES "${src}")
endforeach()


