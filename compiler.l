%{
    #include"compiler.tab.h"
%}
digit [0-9]
letter [a-zA-Z]
hexdigit {digit}|[a-fA-F]
whitespace [\n\t ]+
comments \/\/.*
string (\\n|\\t|\\'|\\\\|\\\"|[^\\"'])
character \'[ -~]\'

%%
"Program"   {return PROGRAM;}
"class" {return CLASS;}
"void"  {return VOID;}
"int"|"boolean" {return D_TYPE;}
"break" {return BREAK;}
"for"   {return FOR;}
"continue"  {return CONTINUE;}
"return"    {return RETURN;}
"if"    {return IF;}
"else"  {return ELSE;}
"callout"   {return CALLOUT;}
"true"|"false"    {return BOOL_LITERAL;}
"{"     {return OB;}
"}"     {return CB;}
"["     {return LSB;}
"]"     {return RSB;}
"("     {return OP;}
")"     {return CP;}
";"     {return SEMICOLON;}
"+"     {return ADD;}
"-"     {return SUB;}
"*"     {return MUL;}
"%"     {return PERCENTAGE;}
"/"     {return DIV;}
"<"     {return LT;}
">"     {return GT;}
"<="    {return LTE;}
">="    {return GTE;}
"=="    {return EQUAL;}
"!="    {return NOT_EQUAL;}
"="     {return ASSIGN;}
"+="    {return ADD_ASSIGN;}
"-="    {return SUB_ASSIGN;}
"&&"    {return CONDITIONAL_AND;}
"||"    {return CONDITIONAL_OR;}
","     {return COMMA;}
"!"     {return NOT;}
{whitespace}   /* ignoring whitespace */ 
{comments}   /* ignoring comments */
{digit}+|"0x"{hexdigit}+    {return INT_LITERAL;}
\"{string}*\"    {return STRING;}
{character}     {printf("%c\n", *yytext);return CHAR_LITERAL;}
{letter}[0-9a-zA-Z]*    {printf("id??%d\n", *yytext);return ID;}

%%
