#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#ifndef LEXER_
#define LEXER_
enum Token
{
  tok_eof = -1,

  // commands
  tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5,
};

extern std::string IdentifierStr; // filled in if tok_identifier
extern double NumVal;             // filled in if tok_number
                                
extern int gettok();
#endif /* LEXER_ */
