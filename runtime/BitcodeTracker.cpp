#include <jitialize/runtime/BitcodeTracker.h>

#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Support/raw_ostream.h>

#include <jitialize/exceptions.h>

using namespace jitialize;
using namespace llvm;

namespace jitialize {
  DefineJitializeException(BitcodeNotRegistered, "Cannot find bitcode.");
  DefineJitializeException(BitcodeParseError, "Cannot parse bitcode for: ");
}

BitcodeTracker& BitcodeTracker::GetTracker() {
  static BitcodeTracker TheTracker;
  return TheTracker;
}

bool BitcodeTracker::hasGlobalMapping(void* FPtr) const {
  auto InfoPtr = Functions.find(FPtr);
  return InfoPtr != Functions.end();
}

LayoutInfo const & BitcodeTracker::getLayoutInfo(jitialize::layout_id id) const {
  auto InfoPair = Layouts.find(id);
  assert(InfoPair != Layouts.end());
  return InfoPair->second;
}

void* BitcodeTracker::getAddress(std::string const &Name) {
  auto Addr = NameToAddress.find(Name);
  if(Addr == NameToAddress.end())
    return nullptr;
  return Addr->second;
}

std::tuple<const char*, GlobalMapping*> BitcodeTracker::getNameAndGlobalMapping(void* FPtr) {
  auto InfoPtr = Functions.find(FPtr);
  if(InfoPtr == Functions.end()) {
    throw jitialize::BitcodeNotRegistered();
  }

  return std::make_tuple(InfoPtr->second.Name, InfoPtr->second.Globals);
}

std::unique_ptr<llvm::Module> BitcodeTracker::getModuleWithContext(void* FPtr, llvm::LLVMContext &C) {
  auto InfoPtr = Functions.find(FPtr);
  if(InfoPtr == Functions.end()) {
    throw jitialize::BitcodeNotRegistered();
  }

  auto &Info = InfoPtr->second;

  llvm::StringRef BytecodeStr(Info.Bitcode, Info.BitcodeLen);
  std::unique_ptr<llvm::MemoryBuffer> Buf(llvm::MemoryBuffer::getMemBuffer(BytecodeStr));
  auto ModuleOrErr =
      llvm::parseBitcodeFile(Buf->getMemBufferRef(), C);

  if (ModuleOrErr.takeError()) {
    throw jitialize::BitcodeParseError(Info.Name);
  }

  return std::move(ModuleOrErr.get());
}

BitcodeTracker::ModuleContextPair BitcodeTracker::getModule(void* FPtr) {

  std::unique_ptr<llvm::LLVMContext> Context(new llvm::LLVMContext());
  auto Module = getModuleWithContext(FPtr, *Context);
  return ModuleContextPair(std::move(Module), std::move(Context));
}

// function to interface with the generated code
extern "C" {
void jitialize_register(void* FPtr, const char* Name, GlobalMapping* Globals, const char* Bitcode, size_t BitcodeLen) {
  BitcodeTracker::GetTracker().registerFunction(FPtr, Name, Globals, Bitcode, BitcodeLen);
}
void jitialize_register_layout(layout_id Id, size_t N) {
  BitcodeTracker::GetTracker().registerLayout(Id, N);
}
}

