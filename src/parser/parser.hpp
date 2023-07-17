#pragma once
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
extern std::unique_ptr<ast::ExprAST> ParsePrototype();
extern std::unique_ptr<ast::ExprAST> ParseDefinition();
extern std::unique_ptr<ast::ExprAST> ParsePrototype();
extern std::unique_ptr<ast::ExprAST> ParsePrototype();

// Top level parsing
extern void HandleDefinition();
extern void HandleExtern();
extern void HandleTopLevelExpression();
extern void MainLoop();

#endif /* PARSER_ */
