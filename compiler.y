%{
    #include <iostream>
    #include "ast.h"
    #include "Visitor.h"
    #include <string>
    #include <vector>

    extern "C" int yylex();
    extern "C" int yyparse();
    extern "C" FILE * yyin;
    extern int yylineno;
    extern ASTProgram *start;
    void yyerror(const char *s);
%}

%union {
    char *sval;
    int ival;
    char cval;
    ASTProgram *program;
    std::vector<FieldDecl *> *field_decl_list;
    std::vector<TypeIdentifier *> *type_identifier_list;
    TypeIdentifier *type_identifier;
    BlockStatement *block;
    VarDecl *var_decl;
    DataType type;
    Statement *statement;
    FieldDecl *field_decl;
    std::vector<VarIdentifier *> *identifier_list;
    std::vector<ArrIdentifier *> *identifier_array_list;
    std::vector<CalloutArg *> *callout_arg_list;
    MethodDecl *method_decl;
    AssignOp assign_op;
    Location *location;
    Expression *expr;
    CalloutArg *callout_arg;
    ArrIdentifier *identifier_array;
    std::vector<MethodDecl *> *method_decl_list;
    MethodCall *method_call;
    std::vector<Expression *> *expr_list;
    std::vector<Statement *> *statement_list;
    std::vector<VarDecl *> *var_decl_list;
    LitExpression *literal;
}

%token COMMA SEMICOLON
%token OP CP LSB RSB OB CB
%token BOOL_LITERAL INT_LITERAL
%token BREAK CONTINUE
%token CALLOUT
%token ELSE IF FOR
%token FALSE TRUE
%token RETURN VOID CLASS

%nonassoc LTE LT GTE GT

%left EQUAL NOT_EQUAL
%left CONDITIONAL_OR CONDITIONAL_AND
%left SUB ADD 
%left MUL DIV PERCENTAGE

%precedence NOT UMINUS
%token ASSIGN ADD_ASSIGN SUB_ASSIGN

%token <sval> IDENTIFIER
%token <ival> INT_VALUE
%token <sval> STRING_VALUE
%token <cval> CHAR_VALUE

%type <program> program
%type <field_decl_list> field_decl_list
%type <field_decl> field_decl
%type <identifier_list> identifier_list
%type <identifier_array_list> identifier_array_list
%type <identifier_array> identifier_array
%type <method_decl_list> method_decl_list
%type <method_decl> method_decl
%type <type_identifier_list> type_identifier_list
%type <type_identifier> type_identifier
%type <block> block
%type <statement_list> statement_list
%type <var_decl_list> var_decl_list
%type <var_decl> var_decl
%type <type> type
%type <statement> statement
%type <assign_op> assign_op
%type <method_call> method_call
%type <expr_list> expr_list
%type <callout_arg_list> callout_arg_list
%type <location> location
%type <expr> expr
%type <callout_arg> callout_arg
%type <literal> literal
%%

program : CLASS IDENTIFIER OB field_decl_list method_decl_list CB { std::string x = std::string($2); $$ = new ASTProgram(x, $4, $5); start = $$; }
        | CLASS IDENTIFIER OB field_decl_list CB  { std::string x = std::string($2); $$ = new ASTProgram(x, $4, NULL); start = $$; }
        | CLASS IDENTIFIER OB method_decl_list CB { std::string x = std::string($2); $$ = new ASTProgram(x, NULL, $4); start = $$; }
        | CLASS IDENTIFIER OB CB { std::string x = std::string($2); $$ = new ASTProgram(x, NULL, NULL); start = $$; }
        ;

field_decl_list : field_decl { std::vector<FieldDecl *> *it = new std::vector<FieldDecl *>(); $$ = it; $$->push_back($1); }
                | field_decl_list field_decl { $1->push_back($2); $$ = $1; }
                ;

field_decl : type identifier_list SEMICOLON { FieldDecl *it = new FieldDecl($2, $1); $$ = it; }
           | type identifier_array_list SEMICOLON { FieldDecl *it = new FieldDecl($2, $1); $$ = it; }
           ;

identifier_list : IDENTIFIER {std::vector<VarIdentifier *> *it = new std::vector<VarIdentifier *>(); $$ = it; $$->push_back(new VarIdentifier(std::string($1))); }
                | identifier_list COMMA IDENTIFIER { VarIdentifier *it = new VarIdentifier(std::string($3)); $1->push_back(it); $$ = $1; }
                ;

identifier_array_list : identifier_array { std::vector<ArrIdentifier *> *it = new std::vector<ArrIdentifier *>(); $$ = it; $$->push_back($1); }
                      | identifier_array_list COMMA identifier_array { $1->push_back($3); $$ = $1; }
                      ;

identifier_array : IDENTIFIER LSB INT_VALUE RSB { ArrIdentifier *it = new ArrIdentifier(std::string($1), $3); $$ = it; }
                 ;

method_decl_list : method_decl { std::vector<MethodDecl *> *it = new std::vector<MethodDecl*>(); $$ = it; $$->push_back($1); }
                 | method_decl_list method_decl { $1->push_back($2); $$ = $1; }
                 ;

method_decl : type IDENTIFIER OP type_identifier_list CP block { MethodDecl *it = new MethodDecl(std::string($2), $1, $4, $6); $$ = it; }
            | type IDENTIFIER OP CP block { MethodDecl *it = new MethodDecl(std::string($2), $1, NULL, $5); $$ = it; }
            | VOID IDENTIFIER OP type_identifier_list CP block { MethodDecl *it = new MethodDecl(std::string($2), DataType::void_literal, $4, $6); $$ = it; }
            | VOID IDENTIFIER OP CP block { MethodDecl *it = new MethodDecl(std::string($2), DataType::void_literal, NULL, $5); $$ = it; }
            ;

type_identifier_list : type_identifier { std::vector<TypeIdentifier *> *it = new std::vector<TypeIdentifier *>(); $$ = it; $$->push_back($1); }
                     | type_identifier_list COMMA type_identifier { $1->push_back($3); $$ = $1; }
                     ;

type_identifier : type IDENTIFIER { TypeIdentifier *it = new TypeIdentifier(std::string($2), $1); $$ = it;}
                ;

block : OB var_decl_list statement_list CB { BlockStatement *it = new BlockStatement($3, $2); $$ = it; } 
      | OB var_decl_list CB { BlockStatement *it = new BlockStatement(NULL, $2); $$ = it;}
      | OB statement_list CB { BlockStatement *it = new BlockStatement($2, NULL); $$ = it;}
      | OB CB { BlockStatement *it = new BlockStatement(NULL, NULL); $$ = it; }
      ;

statement_list : statement { std::vector<Statement *> *it = new std::vector<Statement *>(); $$ = it; $$->push_back($1); }
               | statement_list statement { $1->push_back($2); $$ = $1; }
               ;

var_decl_list : var_decl { std::vector<VarDecl *> *it = new std::vector<VarDecl *>(); $$ = it; $$->push_back($1); }
              | var_decl_list var_decl { $1->push_back($2); $$ = $1; }
              ;

var_decl : type identifier_list SEMICOLON { VarDecl *it = new VarDecl($2, $1); $$ = it;}
         ;

type : INT_LITERAL { $$ = DataType::int_literal; }
     | BOOL_LITERAL { $$ = DataType::bool_literal; }
     ;

statement : SEMICOLON { $$ = NULL; }
          | location assign_op expr SEMICOLON { AssignStatement *it = new AssignStatement($2, $1, $3); $$ = it; }
          | method_call SEMICOLON { $$ = $1; }
          | IF OP expr CP block ELSE block { IfStatement *it = new IfStatement($3, $5, $7); $$ = it; }
          | IF OP expr CP block { IfStatement *it = new IfStatement($3, $5, NULL); $$ = it; }
          | FOR IDENTIFIER ASSIGN expr COMMA expr block { ForStatement *it = new ForStatement($4, $6, $7, std::string($2)); $$ = it; }
          | RETURN expr SEMICOLON { RetStatement *it = new RetStatement($2); $$ = it; }
          | RETURN SEMICOLON { RetStatement *it = new RetStatement(NULL); $$ = it; }
          | BREAK SEMICOLON { BreakStatement *it = new BreakStatement(); $$ = it; }
          | CONTINUE SEMICOLON { ConStatement *it = new ConStatement(); $$ = it; }
          | block { $$ = $1; }
          ;

assign_op : SUB_ASSIGN { auto it = AssignOp::sub_assign; $$ = it;}
          | ASSIGN { auto it = AssignOp::assign; $$ = it; }
          | ADD_ASSIGN { auto it = AssignOp::add_assign; $$ = it; }
          ;

method_call : IDENTIFIER OP expr_list CP { SmplMethod *it = new SmplMethod(std::string($1), $3); $$ = it; }
            | IDENTIFIER OP CP { SmplMethod *it = new SmplMethod(std::string($1), NULL); $$ = it; }
            | CALLOUT OP STRING_VALUE COMMA callout_arg_list CP { CalloutMethod *it = new CalloutMethod(std::string($3), $5); $$ = it; }
            | CALLOUT OP STRING_VALUE CP { CalloutMethod *it = new CalloutMethod(std::string($3), NULL); $$ = it; }
            ;
  
expr_list : expr { std::vector<Expression *> *it = new std::vector<Expression *>(); $$ = it; $$->push_back($1); }
          | expr_list COMMA expr { $1->push_back($3); $$ = $1; }
          ;

callout_arg_list : callout_arg { std::vector<CalloutArg *> *it = new std::vector<CalloutArg *>(); $$ = it; $$->push_back($1); }
                 | callout_arg_list COMMA callout_arg { $1->push_back($3); $$ = $1; }
                 ;

location : IDENTIFIER { VarLocation *it = new VarLocation(std::string($1)); $$ = it; }
         | IDENTIFIER LSB expr RSB { ArrayLocation *it = new ArrayLocation(std::string($1), $3); $$ = it; }
         ;

expr : location { auto it = $1; $$ = it; }
     | method_call { auto it = $1; $$ = it; }
     | literal { auto it = $1; $$ = it; }
     | expr CONDITIONAL_OR expr { BinOpExpression *it = new BinOpExpression($1, $3, BinOp::conditional_or); $$ = it; }
     | expr CONDITIONAL_AND expr { BinOpExpression *it = new BinOpExpression($1, $3, BinOp::conditional_and); $$ = it; }
     | expr EQUAL expr { BinOpExpression *it = new BinOpExpression($1, $3, BinOp::equal); $$ = it;}
     | expr NOT_EQUAL expr { $$ = new BinOpExpression($1, $3, BinOp::not_equal); }
     | expr LT expr { $$ = new BinOpExpression($1, $3, BinOp::lt); }
     | expr LTE expr { $$ = new BinOpExpression($1, $3, BinOp::lte); }
     | expr GTE expr { $$ = new BinOpExpression($1, $3, BinOp::gte); }
     | expr GT expr { $$ = new BinOpExpression($1, $3, BinOp::gt); }
     | expr ADD expr { $$ = new BinOpExpression($1, $3, BinOp::add); }
     | expr SUB expr { $$ = new BinOpExpression($1, $3, BinOp::sub); }
     | expr MUL expr { $$ = new BinOpExpression($1, $3, BinOp::mul); }
     | expr DIV expr { BinOpExpression *it = new BinOpExpression($1, $3, BinOp::div); $$ = it; }
     | expr PERCENTAGE expr { BinOpExpression *it = new BinOpExpression($1, $3, BinOp::percentage); $$ = it; }
     | NOT expr { UnaryOpExpression *it = new UnaryOpExpression($2, UnOp::not_operator); $$ = it; }
     | SUB expr %prec UMINUS { UnaryOpExpression *it = new UnaryOpExpression($2, UnOp::sub); $$ = it; }
     | OP expr CP { $$ = $2; }
     ;

callout_arg : expr  { ExpressionCalloutArg *it = new ExpressionCalloutArg($1); $$ = it; }
            | STRING_VALUE { StringCalloutArg *it = new StringCalloutArg(std::string($1)); $$ = it; }
            ;

literal : INT_VALUE { IntLitExpression *it = new IntLitExpression($1); $$ = it;}
        | CHAR_VALUE { CharacterLitExpression *it = new CharacterLitExpression($1); $$ = it; }
        | TRUE { TrueLitExpression *it = new TrueLitExpression(); $$ = it; }
        | FALSE { FalseLitExpression *it = new FalseLitExpression(); $$ = it; }
        ;

%%


void yyerror (const char *s) {
    std::cerr << "Parse Error on Line : " << yylineno << "\n" << "Message : " << s << std::endl;
    exit(-1);
}

