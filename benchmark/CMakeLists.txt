option(JITIALIZE_BENCHMARK "Enable benchmarking" OFF)

if(JITIALIZE_BENCHMARK)
    set(BENCHMARK_ENABLE_TESTING OFF)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/benchmark)

    set(CMAKE_CXX_COMPILER ${LLVM_TOOLS_BINARY_DIR}/clang++)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -disable-O0-optnone -Xclang -load -Xclang ${JITIALIZE_PASS}")

    add_executable(jitialize-benchmark benchmark.cpp)
    add_dependencies(jitialize-benchmark jitialize-core)

    link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

    target_link_libraries(jitialize-benchmark JitializeRuntime pthread benchmark::benchmark)
endif()
