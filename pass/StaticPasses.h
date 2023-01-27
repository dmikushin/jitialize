#ifndef STATIC_PASSES
#define STATIC_PASSES

#include <llvm/Pass.h>

namespace jitialize {
  llvm::Pass* createRegisterBitcodePass();
  llvm::Pass* createRegisterLayoutPass();
}

#endif
