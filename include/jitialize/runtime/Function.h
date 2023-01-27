#ifndef FUNCTION
#define FUNCTION

#include "LLVMHolder.h"
#include <memory>

namespace jitialize {
  class Function;
}

namespace llvm {
  class Module;
}

namespace std {
  template<> struct hash<jitialize::Function>
  {
    typedef jitialize::Function argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& F) const noexcept;
  };
}

namespace jitialize {

class Context;
struct GlobalMapping;

class Function {

  // do not reorder the fields and do not add virtual methods!
  void* Address; 
  std::unique_ptr<jitialize::LLVMHolder> Holder;

  public:

  Function(void* Addr, std::unique_ptr<jitialize::LLVMHolder> H);

  void* getRawPointer() const {
    return Address;
  }

  void serialize(std::ostream&) const;
  static std::unique_ptr<Function> deserialize(std::istream&);

  bool operator==(jitialize::Function const&) const;

  llvm::Module const& getLLVMModule() const;

  static std::unique_ptr<Function> Compile(void *Addr, jitialize::Context const &C);

  friend
  std::hash<jitialize::Function>::result_type std::hash<jitialize::Function>::operator()(argument_type const& F) const noexcept;
};

}

#endif
