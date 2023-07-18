#include <iostream>
#include "lexer.hpp"
#include "ast.hpp"
#include "parser.hpp"

int
main(void)
{
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40;

  fprintf(stderr, "ready... ");
  getNextToken();

  MainLoop();

  return 0;
}
