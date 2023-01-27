// RUN: %not %clangxx %cxxflags -O2 %include_flags %ld_flags %s -Xclang -load -Xclang %lib_pass -o %t 2> %t.log
// RUN: %FileCheck %s < %t.log

#include <jitialize/jit.h>
#include <jitialize/options.h>

// CHECK: An jitialize::jit option is expected

using namespace std::placeholders;

int foo(int) {
  return 0;
}

int main(int, char** argv) {

  auto foo_ = jitialize::jit(foo, 1, 2);
  return 0;
}
