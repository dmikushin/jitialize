add_llvm_component_library(JitializePass MODULE
  RegisterPasses.cpp
  Jitialize.cpp
  Layout.cpp
  Utils.cpp
  MayAliasTracer.cpp
  PLUGIN_TOOL clang
  )

set(JITIALIZE_PASS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/JitializePass${CMAKE_SHARED_LIBRARY_SUFFIX} PARENT_SCOPE)

install(TARGETS JitializePass
        LIBRARY DESTINATION lib)

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if(APPLE)
    set_target_properties(JitializePass PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif(APPLE)
