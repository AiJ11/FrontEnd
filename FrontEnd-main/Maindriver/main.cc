
#include "ast.hh"
#include "ASTVis.hpp"
#include "Typechecker.hh"
#include <fstream>
#include <iostream>

extern FILE* yyin;
extern int yyparse();
extern Spec* astRoot;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) { std::cerr << "Cannot open " << argv[1] << '\n'; return 1; }

    if (yyparse() != 0 || !astRoot) {
        std::cerr << "Parsing failed.\n";
        return 1;
    }

    std::cout << "Generated AST:\n";
    PrintVisitor printer;
    astRoot->accept(printer);

    TypeChecker tc;
    if (tc.typeCheckSpec(*astRoot))
        std::cout << "\nTypechecking success!\n";
    else
        std::cerr << "\nTypechecking failed.\n";

    fclose(yyin);
    return 0;
}
