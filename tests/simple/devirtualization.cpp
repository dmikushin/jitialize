// RUN: %clangxx %cxxflags -O2 %include_flags %ld_flags %s -Xclang -load -Xclang %lib_pass -o %t
// RUN: %t "%t.ll" > %t.out
// RUN: %FileCheck %s < %t.out
// RUN: %FileCheck --check-prefix=CHECK-IR %s < %t.ll

#include <jitialize/jit.h>
#include <jitialize/options.h>

#include <functional>
#include <cstdio>

// only one function
// reading from a global variable
// CHECK-IR-NOT: = tail call


using namespace std::placeholders;

struct Foo {
  virtual int JITIALIZE_EXPOSE doit() { return 1; }
  virtual ~Foo() = default;
};

struct Bar : Foo {
  int JITIALIZE_EXPOSE doit() override  { return 2; }
};

int doit(Foo* f) {
  return f->doit();
}

int main(int argc, char** argv) {
  Foo* f = nullptr;
  if(argc == 1)
    f = new Foo();
  else
    f = new Bar();

  jitialize::FunctionWrapper<int(void)> jitialize_doit = jitialize::jit(doit, f, jitialize::options::dump_ir(argv[1]));

  // CHECK: doit() is 2
  printf("doit() is %d\n", jitialize_doit());

  delete f;

  return 0;
}
