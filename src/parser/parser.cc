#include "parser.hpp"
#include "lexer.hpp"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "ast.hpp"

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
