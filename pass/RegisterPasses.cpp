#include "StaticPasses.h"

#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;
using namespace jitialize;

static void callback(const PassManagerBuilder &,
                     legacy::PassManagerBase &PM) {
  PM.add(jitialize::createRegisterBitcodePass());
}

RegisterStandardPasses Register(PassManagerBuilder::EP_OptimizerLast, callback);
RegisterStandardPasses RegisterO0(PassManagerBuilder::EP_EnabledOnOptLevel0, callback);
