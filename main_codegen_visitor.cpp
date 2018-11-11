#include <iostream>
#include <cstdio>
#include "ast.h"
#include "Visitor.h"
#include "CodeGenVisitor.h"

extern "C" FILE *yyin; 
extern "C" int yyparse(); 

ASTProgram * start;

int main (const int argc, const char ** argv) {
    if (argc < 2) {
        yyin = NULL;
        std::cerr <<"fatal error: no input files" << std::endl;
        return -1;
    } else {
        FILE * infile = fopen(argv[1], "r");
        if (!infile) {
            std::cerr <<"Error reading file " << argv[1] << std::endl;
            return -1;
        }
        yyin = infile;
    }
    if (!yyparse()) {
        CodeGenVisitor * visitor = new CodeGenVisitor(start);
        visitor->codeGen();
    }
    return 0;
}

