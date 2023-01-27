// RUN: %clangxx %cxxflags %include_flags %ld_flags %s -Xclang -load -Xclang %lib_pass -o %t
// RUN: %t > %t.out
// RUN: %FileCheck %s < %t.out

#include <jitialize/jit.h>

#include <functional>
#include <cstdio>

using namespace std::placeholders;

void add (int b) {
  static int a = 4;
  printf("inc(%d) is %d\n", a, a+b);
  a++;
}

int main() {
  jitialize::FunctionWrapper<void()> inc = jitialize::jit(add, 1);

  // CHECK: inc(4) is 5
  // CHECK: inc(5) is 6
  // CHECK: inc(6) is 7
  // CHECK: inc(7) is 8
  for(int v = 4; v != 8; ++v)
    inc();

  return 0;
}
