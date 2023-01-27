// RUN: %not %clangxx %cxxflags -O2 %include_flags %ld_flags %s -Xclang -load -Xclang %lib_pass -o %t 2> %t.log
// RUN: %FileCheck %s < %t.log

#include <jitialize/jit.h>
#include <jitialize/options.h>

using namespace std::placeholders;

int foo(int, int, int) {
  return 0;
}

int main(int, char** argv) {

  auto foo_ = jitialize::jit(foo, 1, 2);
  foo_(); // CHECK: jitialize::jit: not providing enough argument to actual call
  return 0;
}
