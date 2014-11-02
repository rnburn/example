#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

Module* makeLLVMModule();

int main(int argc, char**argv) {
  Module* Mod = makeLLVMModule();

  Mod->dump();
  delete Mod;
  return 0;
}

Module* makeLLVMModule() {
  Module* mod = new Module("test", getGlobalContext());

  auto string_initializer = llvm::ConstantDataArray::getString(llvm::getGlobalContext()
                                                             , "abc");
  auto global_variable = 
      new llvm::GlobalVariable(*mod
                             , string_initializer->getType()
                             , true
                             , llvm::GlobalValue::PrivateLinkage
                             , string_initializer);

  Constant* noise = mod->getOrInsertFunction("noise"
                                           , llvm::Type::getVoidTy(llvm::getGlobalContext())
                                           , llvm::Type::getInt8PtrTy(llvm::getGlobalContext())
                                           , NULL);

  Constant* main_function_const = mod->getOrInsertFunction("main"
                                                   , IntegerType::get(getGlobalContext(), 32)
                                                   , NULL);

  Function* main_function = cast<Function>(main_function_const);
  main_function->setCallingConv(CallingConv::C);
  auto main_block = BasicBlock::Create(getGlobalContext(), "entry", main_function);
  IRBuilder<> main_builder(main_block);

  auto sptr = main_builder.CreateGlobalStringPtr("arfarfyipwoof");
  main_builder.CreateCall(noise, sptr);

  main_builder.CreateRet(ConstantInt::get(Type::getInt32Ty(getGlobalContext()), 0)); 

  return mod;
}
