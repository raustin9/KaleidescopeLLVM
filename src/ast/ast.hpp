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

namespace ast
{
  class ExprAST
  {
    public:
      virtual ~ExprAST() = default;
  };

  class NumberExprAST : public ExprAST
  {
    double Val;
    
    public:
      NumberExprAST(double Val) : Val(Val) {}
  };

  class VariableExprAST : public ExprAST 
  {
    std::string Name;

    public:
      VariableExprAST(const std::string &Name) : Name(Name) {}
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
  };
} /* End ast namespace  */
     
#endif /* AST_ */
