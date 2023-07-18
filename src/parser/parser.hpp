#pragma once
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
#include <string>
#include <utility>
#include <vector>
#include "ast.hpp"

#ifndef PARSER_
#define PARSER_

/*
 *  CurTok/getNextToken - Provide a simple token buffer.
 *  CurTok is the current token the parser is looking at.
 *  getNextToken() reads another token from the lexer and updates
 *  CurTok with its results
 */
extern int CurTok;
extern int getNextToken();

extern std::map<char, int> BinopPrecedence;
extern int GetTokPrecedence();
extern std::unique_ptr<ast::ExprAST> LogError(const char* Str);
extern std::unique_ptr<ast::PrototypeAST> LogErrorP(const char* Str);
extern std::unique_ptr<ast::ExprAST> ParseExpression();
extern std::unique_ptr<ast::ExprAST> ParseNumberExpression();
extern std::unique_ptr<ast::ExprAST> ParseParenExpr();
extern std::unique_ptr<ast::ExprAST> ParseIdentifierExpr();
extern std::unique_ptr<ast::ExprAST> ParsePrimary();
extern std::unique_ptr<ast::ExprAST> ParseBinOpRHS(int ExexPrec, std::unique_ptr<ast::ExprAST> LHS);
extern std::unique_ptr<ast::PrototypeAST> ParsePrototype();
extern std::unique_ptr<ast::FunctionAST> ParseDefinition();
extern std::unique_ptr<ast::FunctionAST> ParseTopLevelExpr();
extern std::unique_ptr<ast::PrototypeAST> ParseExtern();

// Top level parsing
extern void HandleDefinition();
extern void HandleExtern();
extern void HandleTopLevelExpression();
extern void MainLoop();

#endif /* PARSER_ */
