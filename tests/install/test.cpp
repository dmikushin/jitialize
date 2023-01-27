// REQUIRES: install
//
// clean before, if not there may be unconsistencies 
// RUN: rm -fr build.ninja CMakeCache.txt CMakeFiles cmake_install.cmake InstallTest rules.ninja
//
// RUN: cmake -DCMAKE_CXX_COMPILER=%clang++ -DJitialize_DIR=%install_dir/lib/cmake %S -G Ninja 
// RUN: cmake --build . 
// RUN: ./InstallTest > %t.out
// RUN: %FileCheck %s < %t.out

#include <jitialize/jit.h>
#include <jitialize/code_cache.h>

#include <functional>
#include <cstdio>

using namespace std::placeholders;

void test(int a) {
  printf("this is a test %d!\n", a);
}

int main() {
  jitialize::Cache<> C;
  auto test_jit0 = jitialize::jit(test, 0);
  auto const &test_jit1 = C.jit(test, 1);

  // CHECK: this is a test 0!
  // CHECK: this is a test 1!
  test_jit0();
  test_jit1();

  return 0;
}
