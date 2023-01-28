add_library(JitializeRuntime SHARED
  BitcodeTracker.cpp
  Context.cpp
  Function.cpp
  InitNativeTarget.cpp
  Utils.cpp
  pass/ContextAnalysis.cpp
  pass/DevirtualizeConstant.cpp
  pass/InlineParameters.cpp
  pass/InlineParametersHelper.cpp
)

llvm_config(JitializeRuntime core codegen interpreter support mcjit native executionengine passes objcarcopts)


set(JITIALIZE_RUNTIME ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libJitializeRuntime${CMAKE_SHARED_LIBRARY_SUFFIX} PARENT_SCOPE)

install(TARGETS JitializeRuntime
        LIBRARY DESTINATION lib)
