// RUN: %not %clangxx %cxxflags %include_flags %ld_flags %s -Xclang -load -Xclang %lib_pass -o %t

#include <jitialize/jit.h>

#include <cstdlib>
#include <cstdio>
#include <vector>

using namespace std::placeholders;

float mul(float a, float b) {
  return a*b;
}

int accumulate(std::vector<int> const &vec, int acum, int(*fun)(int)) {
  int a = acum;
  for(int e : vec)
    a += fun(e);
  return a;
}

int main(int argc, char** argv) {

  jitialize::FunctionWrapper<float(float)> mul_by_two = jitialize::jit(mul, _1, 2.0);
  jitialize::FunctionWrapper<int(std::vector<int> const&)> mul_vector_by_two = jitialize::jit(accumulate, _1, 0, mul_by_two);

  return 0;
}
