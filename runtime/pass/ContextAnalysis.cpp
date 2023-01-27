#include <jitialize/runtime/RuntimePasses.h>

using namespace llvm;
using namespace jitialize;

char jitialize::ContextAnalysis::ID = 0;

llvm::Pass* jitialize::createContextAnalysisPass(jitialize::Context const &C) {
  return new ContextAnalysis(C);
}

static RegisterPass<jitialize::ContextAnalysis> X("", "", true, true);
