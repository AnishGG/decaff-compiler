#include <iostream>
#include <cstdio>
#include "ast.h"
#include "Visitor.h"
#include "PrintVisitor.cpp"

extern "C" FILE *yyin; 
extern "C" int yyparse(); 

ASTProgram *start;

int main (const int argc, const char ** argv){
    FILE *input;
    yyin = NULL;
    if (argc < 2){
        std::cerr <<"Please provide input file\n";
        exit(-1);
    } 
    else{
        input = fopen(argv[1], "r");
        if(input == NULL){
            std::cerr <<"Could not read the provided file\n";
            exit(-1);
        }
        yyin = input;
    }
    do{
        yyparse();
    } 
    while (!feof(yyin));
    PrintVisitor * visitor = new PrintVisitor();
    visitor->visit(start);
    return 0;
}

