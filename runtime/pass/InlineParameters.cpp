#include <jitialize/runtime/RuntimePasses.h>
#include <jitialize/runtime/Utils.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Linker/Linker.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/SmallSet.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Support/raw_ostream.h>
#include <numeric>
#include <iostream>

#include "InlineParametersHelper.h"

using namespace llvm;
using jitialize::HighLevelLayout;

char jitialize::InlineParameters::ID = 0;

llvm::Pass* jitialize::createInlineParametersPass(llvm::StringRef Name) {
  return new InlineParameters(Name);
}

HighLevelLayout GetNewLayout(jitialize::Context const &C, HighLevelLayout &HLL) {

  assert(C.size() == HLL.Args_.size());

  size_t NNewArgs = 0;
  for(auto const &Arg : C)
    if(auto const *Map = Arg->as<jitialize::ForwardArgument>())
      NNewArgs = std::max<size_t>(NNewArgs, Map->get()+1);

  HighLevelLayout NewHLL(HLL);
  NewHLL.Args_.clear();
  NewHLL.Args_.resize(NNewArgs, HighLevelLayout::HighLevelArg());

  SmallSet<unsigned, 8> VisitedArgs;

  // only forwarded params are kept
  for(size_t arg = 0; arg != HLL.Args_.size(); ++arg) {
    if(auto const *Map = C.getArgumentMapping(arg).as<jitialize::ForwardArgument>()) {
      if(!VisitedArgs.insert(Map->get()).second)
        continue;
      NewHLL.Args_[Map->get()] = HLL.Args_[arg];
    }
  }

  // set the param_idx once all the parameter sizes are known
  for(size_t new_arg = 0, ParamIdx = 0; new_arg != NewHLL.Args_.size(); ++new_arg) {
    NewHLL.Args_[new_arg].FirstParamIdx_ = ParamIdx;
    ParamIdx += NewHLL.Args_[new_arg].Types_.size();
  }
  return NewHLL;
}

FunctionType* GetWrapperTy(HighLevelLayout &HLL) {
  SmallVector<Type*, 8> Args;
  if(HLL.StructReturn_)
    Args.push_back(HLL.StructReturn_);
  for(auto &HLArg : HLL.Args_)
    Args.insert(Args.end(), HLArg.Types_.begin(), HLArg.Types_.end());
  return FunctionType::get(HLL.Return_, Args, false);
}

void GetInlineArgs(jitialize::Context const &C,
                   Function& F, HighLevelLayout &FHLL,
                   Function &Wrapper, HighLevelLayout &WrapperHLL,
                   SmallVectorImpl<Value*> &Args, IRBuilder<> &B) {

  LLVMContext &Ctx = F.getContext();
  DataLayout const &DL = F.getParent()->getDataLayout();

  if(FHLL.StructReturn_)
    Args.push_back(&*Wrapper.arg_begin());

  for(size_t i = 0, n = C.size(); i != n; ++i) {
    auto const &Arg = C.getArgumentMapping(i);
    auto &ArgInF = FHLL.Args_[i];

    switch(Arg.kind()) {

      case jitialize::ArgumentBase::AK_Forward: {
        auto Forward = GetForwardArgs(ArgInF, FHLL, Wrapper, WrapperHLL);
        Args.insert(Args.end(), Forward.begin(), Forward.end());
      } break;
      case jitialize::ArgumentBase::AK_Int:
      case jitialize::ArgumentBase::AK_Float: {
        Args.push_back(jitialize::GetScalarArgument(Arg, ArgInF.Types_[0]));
      } break;

      case jitialize::ArgumentBase::AK_Ptr: {
        auto const *Ptr = Arg.as<jitialize::PtrArgument>();
        Type* PtrTy = FHLL.Args_[i].Types_[0];

        Constant* PtrVal = jitialize::GetScalarArgument(Arg, PtrTy);
        if(Constant* LinkedPtr = jitialize::LinkPointerIfPossible(*Wrapper.getParent(), *Ptr, PtrTy))
          PtrVal = LinkedPtr;

        Args.push_back(PtrVal);
      } break;

      case jitialize::ArgumentBase::AK_Struct: {
        auto const *Struct = Arg.as<jitialize::StructArgument>();
        auto &ArgInF = FHLL.Args_[i];

        if(ArgInF.StructByPointer_) {
          // struct is passed trough a pointer
          AllocaInst* ParamAlloc = jitialize::GetStructAlloc(B, DL, *Struct, ArgInF.Types_[0]);
          Args.push_back(ParamAlloc);
        } else {
          // struct is passed by value (may be many values)
          size_t N = ArgInF.Types_.size();
          for(size_t ParamIdx = 0, RawOffset = 0; ParamIdx != N; ++ParamIdx) {
            Type* FieldTy = ArgInF.Types_[ParamIdx];
            const char* RawField = &Struct->get()[RawOffset];

            Constant* FieldValue;
            size_t RawSize;
            std::tie(FieldValue, RawSize) = jitialize::GetConstantFromRaw(DL, FieldTy, (uint8_t const*)RawField);

            Args.push_back(FieldValue);
            RawOffset += RawSize;
          }
        }
      } break;

      case jitialize::ArgumentBase::AK_Module: {

        auto &ArgInF = FHLL.Args_[i];
        assert(ArgInF.Types_.size() == 1);

        jitialize::Function const &Function = Arg.as<jitialize::ModuleArgument>()->get();
        llvm::Module const& FunctionModule = Function.getLLVMModule();
        auto FunctionName = jitialize::GetEntryFunctionName(FunctionModule);

        std::unique_ptr<llvm::Module> LM =
            jitialize::CloneModuleWithContext(FunctionModule, Wrapper.getContext());

        assert(LM);

        jitialize::UnmarkEntry(*LM);

        llvm::Module* M = Wrapper.getParent();
        if(Linker::linkModules(*M, std::move(LM), Linker::OverrideFromSrc,
                                [](Module &, const StringSet<> &){})) {
          llvm::report_fatal_error("Failed to link with another module!", true);
        }

        llvm::Function* FunctionInM = M->getFunction(FunctionName);
        FunctionInM->setLinkage(Function::PrivateLinkage);

        Args.push_back(FunctionInM);

      } break;
    }
  }
}

void RemapAttributes(Function const &F, HighLevelLayout const& HLL, Function &Wrapper, HighLevelLayout const& NewHLL) {
  auto FAttributes = F.getAttributes();

  auto FunAttrs = FAttributes.getFnAttributes();
  for(Attribute Attr : FunAttrs)
    Wrapper.addFnAttr(Attr);

  for(size_t new_arg = 0; new_arg != NewHLL.Args_.size(); ++new_arg) {
    auto const &NewArg = NewHLL.Args_[new_arg];
    auto const &OrgArg = HLL.Args_[NewArg.Position_];

    for(size_t field = 0; field != NewArg.Types_.size(); ++field) {
      Wrapper.addParamAttrs(field + NewArg.FirstParamIdx_,
                             FAttributes.getParamAttributes(field + OrgArg.FirstParamIdx_));
    }
  }
}

Function* CreateWrapperFun(Module &M, Function &F, HighLevelLayout &HLL, jitialize::Context const &C) {
  LLVMContext &CC = M.getContext();

  HighLevelLayout NewHLL(GetNewLayout(C, HLL));
  FunctionType *WrapperTy = GetWrapperTy(NewHLL);

  Function* Wrapper = Function::Create(WrapperTy, Function::ExternalLinkage, "", &M);

  BasicBlock* BB = BasicBlock::Create(CC, "", Wrapper);
  IRBuilder<> B(BB);

  SmallVector<Value*, 8> Args;
  GetInlineArgs(C, F, HLL, *Wrapper, NewHLL, Args, B);

  Value* Call = B.CreateCall(&F, Args);

  if(HLL.StructReturn_) {
    Wrapper->arg_begin()->addAttr(Attribute::StructRet);
  }

  if(Call->getType()->isVoidTy()) {
    B.CreateRetVoid();
  } else {
    B.CreateRet(Call);
  }

  RemapAttributes(F, HLL, *Wrapper, NewHLL);

  return Wrapper;
}

bool jitialize::InlineParameters::runOnModule(llvm::Module &M) {
  jitialize::Context const &C = getAnalysis<ContextAnalysis>().getContext();
  llvm::Function* F = M.getFunction(TargetName_);
  assert(F);

  HighLevelLayout HLL(C, *F);
  llvm::Function* WrapperFun = CreateWrapperFun(M, *F, HLL, C);

  // privatize F, steal its name, copy its attributes, and its cc
  F->setLinkage(llvm::Function::PrivateLinkage);
  WrapperFun->takeName(F);
  WrapperFun->setCallingConv(CallingConv::C);

  // add metadata to identify the entry function
  jitialize::MarkAsEntry(*WrapperFun);


  return true;
}

static RegisterPass<jitialize::InlineParameters> X("","",false, false);
