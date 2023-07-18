#include <iostream>
#include <llvm/Support/raw_ostream.h>
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

  fprintf(stderr, "ready >> ");
  getNextToken();

  InitializeModule();

  MainLoop();

  TheModule->print(llvm::errs(), nullptr);

  return 0;
}
