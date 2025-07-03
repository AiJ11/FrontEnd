
CXX = g++
CXXFLAGS = -std=c++14 -Wall -g
LEX = flex
YACC = bison

all: api_parser

parser.tab.cc parser.tab.hh: parser.yy
	$(YACC) -d parser.yy

lex.yy.c: lexer.ll parser.tab.hh
	$(LEX) -o lex.yy.c lexer.ll

api_parser: parser.tab.cc lex.yy.c ast.cc ASTVis.cc Typechecker.cc main.cc
	$(CXX) $(CXXFLAGS) -o api_parser parser.tab.cc lex.yy.c ast.cc ASTVis.cc Typechecker.cc main.cc

clean:
	rm -f api_parser lex.yy.c parser.tab.cc parser.tab.hh
	rm -f *.o *~

.PHONY: all clean
