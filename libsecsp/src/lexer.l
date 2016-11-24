%{
#include "parser.tab.h"

/*
 * http://stackoverflow.com/a/8024849
 */
#define YY_USER_ACTION yylloc->first_line = yylloc->last_line = yylineno; \
    yylloc->first_column = yycolumn; yylloc->last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
%}

%option reentrant noyywrap yylineno
%option bison-bridge bison-locations
%%

"\n"	{ yylloc->last_line++; yylloc->last_column = 0; }
"("	{ return TOK_LPAREN; }
")"	{ return TOK_RPAREN; }
"{"	{ return TOK_LBRACE; }
"}"	{ return TOK_RBRACE;}
","	{ return TOK_COMMA; }
"&"	{ return TOK_AND; }
"|"	{ return TOK_OR; }
"~"	{ return TOK_NOT; }
"^"	{ return TOK_XOR; }
"="	{ return TOK_EQUALS; }
";" { return TOK_SEMICOLON; }

"block" { return TOK_BLOCK; }
"abstract" { return TOK_ABSTRACT; }
"inherits" { return TOK_INHERITS; }
"role" { return TOK_ROLE; }
"role_attribute" { return TOK_ROLE_ATTRIBUTE; }
"type" { return TOK_TYPE; }
"type_attribute" { return TOK_TYPE_ATTRIBUTE; }
"user" { return TOK_USER; }

[a-z]+	{ yylval->name = strdup(yytext); return TOK_NAME; }
[0-9]+	{ yylval->number = atol(yytext); return TOK_NUMBER; }
[\s\t ]+	{}
%%


