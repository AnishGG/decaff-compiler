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

expr:
          location
          | method_call
          | literal
          | expr ADD expr
          | expr SUB expr
          | expr MUL expr
          | expr DIV expr
          | expr PERCENTAGE expr
          | expr LT expr
          | expr GT expr
          | expr LTE expr
          | expr GTE expr
          | expr EQUAL expr
          | expr NOT_EQUAL expr
          | expr CONDITIONAL_AND expr
          | expr CONDITIONAL_OR expr
          | NOT expr
          | SUB expr
          | OP expr CP
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
                expr
                | method_arguments COMMA expr
                |   /* epsilon */
                ;

callout_arguments:
                 COMMA callout_argument
                 |  /* epsilon */
                 ;

callout_argument:
                expr callout_arguments
                STRING callout_arguments
                ;

location:
        ID
        | ID LSB expr RSB

%%
int main(int argc, char **argv){
    yyparse();
    printf("Parsing over\n");
}

void yyerror(char *s){
    fprintf(stderr, "error: %s\n", s);
}
