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

Program:
       CLASS PROGRAM OB field_decl method_decls CB  
       ;

field_decl:
           field_decl D_TYPE var SEMICOLON
           |    /* epsilon */
           ;

var:
   ID 
   | var COMMA ID
   | ID LSB INT_LITERAL RSB
   | var COMMA ID LSB INT_LITERAL RSB
   ;

method_decls:
            method_decl method_decls
            |   /* epsilon */
            ;

method_decl:
           D_TYPE ID arguments block
           |    VOID ID arguments block
           ;

arguments:
         OP CP  
         |  OP D_TYPE ID argument CP
         ;

argument:
        COMMA D_TYPE ID argument
        | /* epsilon */
        ;

block:
     OB var_decls statements CB
     ;

var_decls:
         var_decl SEMICOLON var_decls
         | /* epsilon */
         ;

var_decl:
        D_TYPE ID  
        | var_decl COMMA ID 
        ;

statements:
          statements statement
          | /* epsilon */
          ;

statement:
         location assign_op expr SEMICOLON
         |  method_call SEMICOLON
         |  IF OP expr CP block
         |  IF OP expr CP block ELSE block
         |  FOR ID ASSIGN expr COMMA expr block
         |  RETURN expr SEMICOLON
         |  RETURN SEMICOLON    /* for void methods */
         |  BREAK SEMICOLON
         |  CONTINUE SEMICOLON
         |  block
         ;

assign_op:
         ASSIGN 
         |  ADD_ASSIGN
         |  SUB_ASSIGN
         ;

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
                |   STRING callout_arguments  
                ;

location:
        ID
        | ID LSB expr RSB

%%
int main(int argc, char **argv){
    yyparse();
    printf("Parsing over successfully\n");
}

void yyerror(char *s){
    fprintf(stderr, "error: %s\n", s);
}
