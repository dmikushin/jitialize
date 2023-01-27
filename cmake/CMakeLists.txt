configure_file(${CMAKE_CURRENT_SOURCE_DIR}/JitializeConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/JitializeConfig.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/JitializeConfig.cmake
        DESTINATION lib/cmake)
