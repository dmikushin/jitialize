#include <llvm/IR/Metadata.h>
#include <llvm/IR/Module.h>
#include <llvm/ADT/SmallVector.h>

#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Bitcode/BitcodeReader.h>

#include <string>

#include <jitialize/runtime/Utils.h>

using namespace llvm;

static const char JitializeMD[] = "jitialize::jit";
static const char EntryTag[] = "entry";

std::string jitialize::GetEntryFunctionName(Module const &M) {
  NamedMDNode* MD = M.getNamedMetadata(JitializeMD);

  for(MDNode *Operand : MD->operands()) {
    if(Operand->getNumOperands() != 2)
      continue;
    MDString* Entry = dyn_cast<MDString>(Operand->getOperand(0));
    MDString* Name = dyn_cast<MDString>(Operand->getOperand(1));

    if(!Entry || !Name || Entry->getString() != EntryTag)
      continue;

    return Name->getString().str();
  }

  llvm_unreachable("No entry function in jitialize::jit module!");
  return "";
}

void jitialize::MarkAsEntry(llvm::Function &F) {
  Module &M = *F.getParent();
  LLVMContext &Ctx = F.getContext();
  NamedMDNode* MD = M.getOrInsertNamedMetadata(JitializeMD);
  MDNode* Node = MDNode::get(Ctx, { MDString::get(Ctx, EntryTag),
                                    MDString::get(Ctx, F.getName())});
  MD->addOperand(Node);
}

void jitialize::UnmarkEntry(llvm::Module &M) {
  NamedMDNode* MD = M.getOrInsertNamedMetadata(JitializeMD);
  M.eraseNamedMetadata(MD);
}

std::unique_ptr<llvm::Module>
jitialize::CloneModuleWithContext(llvm::Module const &LM, llvm::LLVMContext &C) {
  // I have not found a better way to do this withouth having to fully reimplement
  // CloneModule

  std::string buf;

  // write module
  {
    llvm::raw_string_ostream stream(buf);
    llvm::WriteBitcodeToFile(LM, stream);
    stream.flush();
  }

  // read the module
  auto MemBuf = llvm::MemoryBuffer::getMemBuffer(llvm::StringRef(buf));
  auto ModuleOrError = llvm::parseBitcodeFile(*MemBuf, C);
  if(ModuleOrError.takeError())
    return nullptr;

  auto LMCopy = std::move(ModuleOrError.get());
  return LMCopy;
}
