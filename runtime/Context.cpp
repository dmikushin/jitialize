#include "jitialize/runtime/Context.h"

using namespace jitialize;

Context& Context::setParameterIndex(unsigned param_idx) {
  return setArg<ForwardArgument>(param_idx);
}

Context& Context::setParameterInt(int64_t val) {
  return setArg<IntArgument>(val);
}

Context& Context::setParameterFloat(double val) {
  return setArg<FloatArgument>(val);
}

Context& Context::setParameterPointer(const void* val) {
  return setArg<PtrArgument>(val);
}

Context& Context::setParameterStruct(serialized_arg arg) {
  return setArg<StructArgument>(std::move(arg));
}

Context& Context::setParameterModule(jitialize::Function const &F) {
  return setArg<ModuleArgument>(F);
}

bool Context::operator==(const Context& Other) const {
  if(getOptLevel() != Other.getOptLevel())
    return false;
  if(size() != Other.size())
    return false;

  for(auto this_it = begin(), other_it = Other.begin();
      this_it != end(); ++this_it, ++other_it) {
    ArgumentBase &ThisArg = **this_it;
    ArgumentBase &OtherArg = **other_it;
    if(!(ThisArg == OtherArg))
      return false;
  }

  return true;
}
