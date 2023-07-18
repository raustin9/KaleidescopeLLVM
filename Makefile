CC=clang++
LLVM=`llvm-config --cxxflags --ldflags --system-libs --libs core`
CFLAGS=-Wall -g -Isrc/lexer -Isrc/ast -Isrc/parser $(LLVM)
BIN=bin/
LEX=lexer.cc
AST=ast.cc
EXECS=$(BIN)kaleidescope
LIB=lib/parser.a lib/ast.a lib/lexer.a 

all: $(EXECS)
	
clean:
	rm -f obj/* bin/* lib/*

$(BIN)kaleidescope: src/kaleidescope.cc $(LIB)
	$(CC) $(CFLAGS) -o $@ $< $(LIB)


lib/lexer.a: obj/lexer.o
	ar ru $@ $<
	ranlib $@

obj/lexer.o: src/lexer/lexer.cc
	$(CC) $(CFLAGS) -c -o $@ $<

lib/parser.a: obj/parser.o
	ar ru $@ $<
	ranlib $@

obj/parser.o: src/parser/parser.cc
	$(CC) $(CFLAGS)  $(LLVM) -c -o $@ $< 

lib/ast.a: obj/ast.o
	ar ru $@ $<
	ranlib $@

obj/ast.o: src/ast/ast.cc
	$(CC) $(CFLAGS) $(LLVM) -c -o $@ $<
