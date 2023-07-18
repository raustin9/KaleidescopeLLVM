#include "parser.hpp"
#include "lexer.hpp"
#include "ast.hpp"

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

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <new>
#include <string>
#include <utility>
#include <vector>

std::map<char, int> BinopPrecedence;
int CurTok;
int getNextToken()
{
  return CurTok = gettok();
}


// LogError* - These are helper functions for error handling
std::unique_ptr<ast::ExprAST> LogError(const char *Str)
{
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}

std::unique_ptr<ast::PrototypeAST> LogErrorP(const char *Str)
{
  LogError(Str);
  return nullptr;
}

std::unique_ptr<ast::ExprAST> ParseNumberExpression()
{
  auto Result = std::make_unique<ast::NumberExprAST>(NumVal);
  getNextToken();
  return std::move(Result);
}

std::unique_ptr<ast::ExprAST> ParseParenExpr()
{
  getNextToken(); // eat (
  auto V = ParseExpression();
  if (!V)
    return nullptr;

  if (CurTok != ')')
    return LogError("expected ')'");

  getNextToken(); // eat ')'
  return V;
}

std::unique_ptr<ast::ExprAST> ParseIdentifierExpr()
{
  std::string IdName = IdentifierStr;

  getNextToken(); // eat the identifier

  if (CurTok != '(') // simple variable reference
    return std::make_unique<ast::VariableExprAST>(IdName);

  // Call
  getNextToken();
  std::vector<std::unique_ptr<ast::ExprAST> > Args;
  if (CurTok != ')')
  {
    while(1)
    {
      if (auto Arg = ParseExpression())
        Args.push_back(std::move(Arg));
      else
        return nullptr;

      if (CurTok == ')')
        break;

      if (CurTok != ',')
        return LogError("Expected ')' or ',' in argument list");
      getNextToken();
    }
  }

  // Eat the ')'
  getNextToken();

  return std::make_unique<ast::CallExprAST>(IdName, std::move(Args));
}

std::unique_ptr<ast::ExprAST> ParsePrimary()
{
  switch (CurTok)
  {
    default: 
      return LogError("unknown token when expecting an expression");
    case tok_identifier:
      return ParseIdentifierExpr();
    case tok_number:
      return ParseNumberExpression();
    case '(':
      return ParseParenExpr();
  }
}

int GetTokPrecedence()
{
  if (!isascii(CurTok))
    return -1;

  // Make sure it is a declared binop
  int TokPrec = BinopPrecedence[CurTok];
  if (TokPrec <=0) return -1;
  return TokPrec;
}

std::unique_ptr<ast::ExprAST> ParseExpression()
{
  auto LHS = ParsePrimary();
  if (!LHS)
    return nullptr;

  return ParseBinOpRHS(0, std::move(LHS));
}

std::unique_ptr<ast::ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ast::ExprAST> LHS)
{
  // If this is a binop, find the precedence
  while (1)
  {
    int TokPrec = GetTokPrecedence();

    // If this is binop that binds at least as tightly as 
    // the current binop, consume it, otherwise we are done
    if (TokPrec < ExprPrec)
      return LHS;

    // We now know this is a binop
    int BinOp = CurTok;
    getNextToken(); // eat the binop
    
    // Parse the primary expression after the binary operator
    auto RHS = ParsePrimary();
    if (!RHS)
      return nullptr;

    // If the binop binds less tightly with RHS than the operator after RHS,
    // let the pending operator take RHS as its LHS
    int NextPrec = GetTokPrecedence();
    if (TokPrec < NextPrec)
    {
      RHS = ParseBinOpRHS(TokPrec+1, std::move(RHS));
      if (!RHS)
        return nullptr;
    }

    // merge LHS/RHS
    LHS = 
      std::make_unique<ast::BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
  }
}

std::unique_ptr<ast::PrototypeAST> ParsePrototype()
{
  if (CurTok != tok_identifier)
    return LogErrorP("Expected function name in prototype");

  std::string FnName = IdentifierStr;
  getNextToken();

  if (CurTok != '(')
    return LogErrorP("Expected '(' in prototype");

  // Read the list of argument names
  std::vector<std::string> ArgNames;
  while(getNextToken() == tok_identifier)
    ArgNames.push_back(IdentifierStr);

  if (CurTok != ')')
    return LogErrorP("Expected ')' in prototype");

  // Success
  getNextToken(); // eat ')'

  return std::make_unique<ast::PrototypeAST>(FnName, std::move(ArgNames));
}

std::unique_ptr<ast::FunctionAST> ParseDefinition()
{
  getNextToken();
  auto Proto = ParsePrototype();
  if (!Proto)
    return nullptr;

  if (auto E = ParseExpression())
    return std::make_unique<ast::FunctionAST>(std::move(Proto), std::move(E));
  return nullptr;
}

std::unique_ptr<ast::PrototypeAST> ParseExtern()
{
  getNextToken(); // eat extern
  return ParsePrototype();
}

std::unique_ptr<ast::FunctionAST> ParseTopLevelExpr()
{
  if (auto E = ParseExpression())
  {
    auto Proto = std::make_unique<ast::PrototypeAST>("", std::vector<std::string>());
    return std::make_unique<ast::FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

void
HandleDefinition()
{
  if (ParseDefinition())
  {
    fprintf(stderr, "Parsed a function definition\n");
  }
  else
  {
    // skip token for error recovery
    getNextToken();
  }
}

void
HandleExtern()
{
  if (ParseExtern())
  {
    fprintf(stderr, "Parsed an extern\n");
  }
  else
  {
    // skip the next token for error recovery
    getNextToken();
  }
}

void
HandleTopLevelExpression()
{
  if (ParseTopLevelExpr())
  {
    fprintf(stderr, "Parsed a top-level expr\n");
  }
  else
  {
    // skip token for error recovery
    getNextToken();
  }
}

// driver  loop
void
MainLoop()
{
  while(1)
  {
    fprintf(stderr, "ready> ");
    switch (CurTok)
    {
      case tok_eof:
        return;
      case ';': // ignore top-level semicolons
        getNextToken();
        break;
      case tok_def: 
        HandleDefinition();
        break;
      case tok_extern:
        HandleExtern();
        break;
      default:
        HandleTopLevelExpression();
        break;
    }
  }
}
