#include "ast.hpp"
#include "parser.hpp"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

std::unique_ptr<llvm::LLVMContext> TheContext;
std::unique_ptr<llvm::Module> TheModule;
std::unique_ptr<llvm::IRBuilder<>> Builder;
std::map <std::string, llvm::Value*> NamedValues;

// Emit the error and then return NULL
llvm::Value*
LogErrorV(const char* Str) {
  LogError(Str);
  return nullptr;
}

// Expression Code Generation

// Generate code for a number expression
llvm::Value* 
ast::NumberExprAST::codegen() {
  return llvm::ConstantFP::get(*TheContext, llvm::APFloat(Val));
}

// Generate code for a variable expr
llvm::Value* ast::VariableExprAST::codegen() {
  // Look up this variable up in the function
  llvm::Value *V = NamedValues[Name];
  if (!V)
    LogErrorV("Unknown variable name");

  return V;
}

// Generate code for binary operations
llvm::Value*
ast::BinaryExprAST::codegen() {
  llvm::Value* L = LHS->codegen();
  llvm::Value* R = RHS->codegen();
  if (!L || !R)
    return nullptr;

  switch (Op) {
    case '+':
      return Builder->CreateFAdd(L, R, "addtmp");
    case '-':
      return Builder->CreateFSub(L, R, "subtmp");
    case '*':
      return Builder->CreateFMul(L, R, "multmp");
    case '<':
      L = Builder->CreateFCmpULT(L, R, "cmptmp");
      // convert bool 0/1 to double 0.0/1.0
      return Builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*TheContext), "booltmp");
    default:
      return LogErrorV("invalid binary operator");
      
  }
}

llvm::Value*
ast::CallExprAST::codegen() {
  // Look up the function in the global module table
  llvm::Function *CalleeF = TheModule->getFunction(Callee);
  if (!CalleeF)
    return LogErrorV("Unknown function referenced");

  // If argument mismatch error
  if (CalleeF->arg_size() != Args.size())
    return LogErrorV("Incorrect num of arguments passed");

  std::vector<llvm::Value*> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    ArgsV.push_back(Args[i]->codegen());
    if (!ArgsV.back())
      return nullptr;
  }

  return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

// Code generation for functions

// code gen for function prototypes
llvm::Function*
ast::PrototypeAST::codegen() {
  // make the function type: double(double, double) etc
  std::vector<llvm::Type*> Doubles(Args.size(), llvm::Type::getDoubleTy(*TheContext));

  llvm::FunctionType *FT = 
    llvm::FunctionType::get(llvm::Type::getDoubleTy(*TheContext), Doubles, false);

  llvm::Function *F = 
    llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, TheModule.get());

  unsigned Idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(Args[Idx++]);
  return F;
}

// code gen for functions
llvm::Function*
ast::FunctionAST::codegen() {
  // Check for an existing function from a previour 'extern' declaration
  llvm::Function *TheFunction = TheModule->getFunction(Proto->getName());

  if (!TheFunction)
    TheFunction = Proto->codegen();

  if (!TheFunction)
    return nullptr;

  if (!TheFunction->empty())
    return (llvm::Function*)LogErrorV("Function cannot be redefined");

  // create a basic block to start insertion into
  llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", TheFunction);
  Builder->SetInsertPoint(BB);

  // record the function arguments in the NamedValues map
  NamedValues.clear();
  for (auto &Arg : TheFunction->args())
    NamedValues[std::string(Arg.getName())] = &Arg;

  if (llvm::Value* RetVal = Body->codegen()) {
    // Finish off the function
    Builder->CreateRet(RetVal);

    // Validate generated code, checking for consistency
    llvm::verifyFunction(*TheFunction);

    return TheFunction;
  }

  // Error reading body, remove function
  TheFunction->eraseFromParent();
  return nullptr;
}
