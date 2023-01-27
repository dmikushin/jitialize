// RUN: %clangxx %cxxflags %include_flags %ld_flags %s -Xclang -load -Xclang %lib_pass -o %t
// RUN: %t > %t.out
// RUN: %FileCheck %s < %t.out

#include <jitialize/jit.h>

#include <functional>
#include <cstdio>

using namespace std::placeholders;

int add (int a, int *b) {
  return a+*b;
}

int main() {
  int b = 1;
  jitialize::FunctionWrapper<int(int)> inc = jitialize::jit(add, _1, &b);

  // CHECK: inc(4) is 5
  // CHECK: inc(5) is 6
  // CHECK: inc(6) is 7
  // CHECK: inc(7) is 8
  for(int v = 4; v != 8; ++v)
    printf("inc(%d) is %d\n", v, inc(v));

  return 0;
}
