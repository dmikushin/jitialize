cmake_minimum_required(VERSION 3.13.4)

project(jitialize)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  message(FATAL_ERROR "Do not set the build directory equal to the source directory!")
endif()

message(STATUS "CPU Architecture: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
if (${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "x86_64")
	add_definitions(-D_X86)
elseif (${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "aarch64")
	add_definitions(-D_AARCH64)
endif()

find_package(LLVM CONFIG REQUIRED PATHS "/usr/lib/llvm-12/lib/cmake" NO_DEFAULT_PATH)
find_package(Clang CONFIG REQUIRED PATHS "/usr/lib/llvm-12/lib/cmake" NO_DEFAULT_PATH)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in ${LLVM_DIR}")
message(STATUS "LLVM Root: ${LLVM_TOOLS_BINARY_DIR}")
message(STATUS "LLVM Include dirs: ${LLVM_INCLUDE_DIRS}")

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(AddLLVM)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
include_directories(include)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

set(JITIALIZE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Suppress superfluous randlib warnings about "*.a" having no symbols on MacOSX.
if (APPLE)
	set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
	set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
	set(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
	set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
endif()

add_subdirectory(cmake)
add_subdirectory(include)
add_subdirectory(pass)
add_subdirectory(runtime)
add_subdirectory(example)
add_subdirectory(benchmark)

add_custom_target(jitialize-core DEPENDS JitializePass JitializeRuntime)

include(Python)
find_python_module(lit REQUIRED)

set(ALL_TESTS ${CMAKE_CURRENT_BINARY_DIR}/tests/)

add_custom_target(check
    COMMAND lit ${ALL_TESTS} -v --shuffle
    DEPENDS JitializePass JitializeRuntime)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tests/lit.cfg.in" "${CMAKE_CURRENT_BINARY_DIR}/tests/lit.cfg")

add_custom_target(docker
    COMMAND docker build -f ${CMAKE_CURRENT_SOURCE_DIR}/docker/Dockerfile -t ${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/docker
    COMMAND docker run -it --rm --user "${UID}:${GID}" -v${CMAKE_CURRENT_SOURCE_DIR}:/project ${PROJECT_NAME} sh /project/docker/scripts/build.sh
    COMMENT "Building ${PROJECT_NAME} in a Docker container")

