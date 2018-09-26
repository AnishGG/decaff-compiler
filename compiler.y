%{
#include<stdio.h>
void yyerror(char* s);
int yylex();
int yyparse();
%}
/* token declarations */

%token INT_LITERAL BOOL_LITERAL CHAR_LITERAL STRING ID D_TYPE
%token OB CB LSB RSB OP CP
%token ADD SUB MUL DIV PERCENTAGE
%token LT GT LTE GTE
%token SEMICOLON COMMA
%token ASSIGN EQUAL NOT_EQUAL
%token CONDITIONAL_AND CONDITIONAL_OR NOT
%token ADD_ASSIGN SUB_ASSIGN
%token CALLOUT 
%token PROGRAM CLASS VOID RETURN
%token BREAK CONTINUE
%token FOR IF ELSE EOL

/* Precedence */
%left EQUAL NOT_EQUAL
%left CONDITIONAL_AND CONDITIONAL_OR
%left LT GT LTE GTE
%left SUB ADD
%left MUL DIV PERCENTAGE
%nonassoc NOT

/* Grammar rules */
%%

expression:
          location
          | method_call
          | literal
          | expression operation expression
          | NOT expression
          | SUB expression
          | OP expression CP
          ;

operation:
        ADD
        |   SUB
        |   MUL
        |   DIV
        |   PERCENTAGE
        |   LT
        |   GT
        |   LTE
        |   GTE
        |   EQUAL
        |   NOT_EQUAL
        |   CONDITIONAL_AND
        |   CONDITIONAL_OR
        ;

literal:
       INT_LITERAL      
       |    CHAR_LITERAL 
       |    BOOL_LITERAL
       ;

method_call:
           CALLOUT OP STRING callout_arguments CP  
           |    ID OP method_arguments CP
           ;

method_arguments:
                expression
                | method_arguments COMMA expression
                |   /* epsilon */
                ;

callout_arguments:
                 COMMA callout_argument
                 |  /* epsilon */
                 ;

callout_argument:
                expression callout_arguments
                STRING callout_arguments
                ;

location:
        ID
        | ID LSB expression RSB

%%
int main(int argc, char **argv){
    yyparse();
    printf("Parsing over\n");
}

void yyerror(char *s){
    fprintf(stderr, "error: %s\n", s);
}
