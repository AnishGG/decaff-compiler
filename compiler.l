%{
    #include <iostream>
    #include <cstdlib>
    #include "ast.h"
    #include "Visitor.h"
    #include "compiler.tab.h"
    #define YY_DECL extern "C" int yylex()
    char * translateString(char *, int);
%}

%option noyywrap
%option yylineno
%option nounput

UNDERSCOREDALPHA [a-zA-Z_]
CHAR ([\40-\176]{-}['"\\])|\\n|\\t|\\\\|\\\"|\\\'
HEXADECIMAL [0-9a-fA-F]
DIGIT [0-9]
ALPHA [a-zA-Z]

%%

\/\/.*\n {
}

\, {
    return COMMA;
}

\; {
    return SEMICOLON;
}

\( {
    return OP;
}

\) {
    return CP;
}

\[ {
    return LSB;
}

\] {
    return RSB;
}

\{  {
    return OB;
}

\}  {
    return CB;
}

\+\= {
    return ADD_ASSIGN;
}

\+ {
    return ADD;
}

\-\= {
    return SUB_ASSIGN;
}

\- {
    return SUB;
}

\* {
    return MUL;
}

\/ {
    return DIV;
}

\% {
    return PERCENTAGE;
}

\!\= {
    return NOT_EQUAL;
}

\! {
    return NOT;
}

\<\= {
    return LTE;
}

\< {
    return LT;
}

\>\= {
    return GTE;
}

\> {
    return GT;
}

\=\= {
    return EQUAL;
}

\= {
    return ASSIGN;
}

\|\| {
    return CONDITIONAL_OR;
}

\&\& {
    return CONDITIONAL_AND;
}

boolean {
    return BOOL_LITERAL;
}

break {
    return BREAK;
}

callout {
    return CALLOUT;
}

class {
    return CLASS;
}

continue {
    return CONTINUE;
}

else {
    return ELSE;
}

false {
    return FALSE;
}

for {
    return FOR;
}

if {
    return IF;
}

int {
    return INT_LITERAL;
}

return {
    return RETURN;
}

true {
    return TRUE;
}

void {
    return VOID;
}

{UNDERSCOREDALPHA}({UNDERSCOREDALPHA}|{DIGIT})* {
    yylval.sval = strdup(yytext);
    return IDENTIFIER;
}

{DIGIT}+ {
    yylval.ival = atoi(yytext);
    return INT_VALUE;
}

0x{HEXADECIMAL}+ {
    yylval.ival = atoi(yytext);
    return INT_VALUE;
}

\"{CHAR}*\" {
    yylval.sval = translateString(yytext + 1, strlen(yytext) - 2);
    return STRING_VALUE;
}

\'{CHAR}\' {
    yylval.cval = yytext[1];
    return CHAR_VALUE;
}

[\t\ \n] {
}

. {
    std::cerr << "UNRECOGNIZED TEXT ENCOUNTERED" << yytext;
}

%%

char * translateString(char * string, int size) {
	char * ch = string;
	char * newString = (char *) malloc(size + 1);
	char * temp = newString;
	int i;
	for (i = 0; i < size; i++) {
		switch (*ch) {
			case '\\':
				i++;
				ch++;
				switch (*ch) {
					case 'a':
						*temp = '\a';
						break;
					case 'b':
						*temp = '\b';
						break;
					case 'f':
						*temp = '\f';
						break;
					case 'n':
						*temp = '\n';
						break;
					case 'r':
						*temp = '\r';
						break;
					case 't':
						*temp = '\t';
						break;
					case 'v':
						*temp = '\v';
						break;
					case '\\':
						*temp = '\\';
						break;
					case '\'':
						*temp = '\'';
						break;
					case '\"':
						*temp = '\"';
						break;
					case '?':
						*temp = '\?';
						break;
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
						//TODO: Fill
						break;
					case 'x':
						//TODO: Fill
						break;
					default:
						break;
				}
				break;
			default: 
				*temp = *ch;
				break;
		}
		ch++;
		temp++;
	}
	*temp = '\0';
	return newString;
}
