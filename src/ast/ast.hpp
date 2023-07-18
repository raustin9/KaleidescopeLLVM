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

#pragma once
#ifndef AST_
#define AST_

extern std::unique_ptr<llvm::LLVMContext> TheContext;
extern std::unique_ptr<llvm::Module> TheModule;
extern std::unique_ptr<llvm::IRBuilder<>> Builder;
extern std::map <std::string, llvm::Value*> NamedValues;
extern llvm::Value* LogErrorV();

namespace ast
{
  class ExprAST
  {
    public:
      virtual ~ExprAST() = default;
      virtual llvm::Value *codegen() = 0;
  };

  class NumberExprAST : public ExprAST
  {
    double Val;
    
    public:
      NumberExprAST(double Val) : Val(Val) {}
      llvm::Value *codegen() override;
  };

  class VariableExprAST : public ExprAST 
  {
    std::string Name;

    public:
      VariableExprAST(const std::string &Name) : Name(Name) {}
      llvm::Value *codegen() override;
  };

  class BinaryExprAST : public ExprAST 
  {
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;

    public:
      BinaryExprAST(
        char Op,
        std::unique_ptr<ExprAST> LHS,
        std::unique_ptr<ExprAST> RHS
      ) : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
      llvm::Value *codegen() override;
  };

  class CallExprAST : public ExprAST
  {
    std::string Callee;
    std::vector< std::unique_ptr<ExprAST> > Args;

    public:
      CallExprAST(
        const std::string &Callee,
        std::vector<std::unique_ptr<ExprAST> > Args
      ) : Callee(Callee), Args(std::move(Args)) {}
      llvm::Value *codegen() override;
  };

  // PrototypeAST
  // Represents the prototype for a function
  // captures the name, and its argument names (and implicitly the number of arguments)
  class PrototypeAST
  {
    std::string Name;
    std::vector<std::string> Args;

    public:
      PrototypeAST(
        const std::string &Name,
        std::vector<std::string> Args
      ) : Name(Name), Args(std::move(Args)) {}

      const std::string &getName() const { return Name; }
      llvm::Function *codegen();
  };

  class FunctionAST
  {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;

    public:
      FunctionAST(
        std::unique_ptr<PrototypeAST> Proto,
        std::unique_ptr<ExprAST> Body
      ) : Proto(std::move(Proto)), Body(std::move(Body)) {}
      llvm::Function *codegen();
  };
} /* End ast namespace  */
     
#endif /* AST_ */
