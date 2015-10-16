%{

/******************************************************************************
 *  File          : lexer.l
 *  Project       : Pazcal Compiler
 *  Written by    : Lolos Konstantinos, Podimata Charikleia
 *                  (lolos.kostis@gmail.com, charapod@gmail.com)
 *  Date          : 2014-2015
 *  Description   : Lexer definition to be used by flex
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "general.h"
#include "error.h"
#include "symbol.h"
#include "./gc/include/gc.h"
#include "parser.hpp"

#define T_eof      0

int lineno = 1;
void yyerror (const char * msg);
%}

%option nounput
%option noinput

D [0-9]
l [a-z]
L [A-Z]
W [ \t\r]
ESC [ntr0\\\'\"]
PRINT [^\x00-\x1f]

%%

"and"       {return T_and;}
"bool"      {return T_bool;}
"break"     {return T_break;}
"case"      {return T_case;}
"char"      {return T_char;}
"const"     {return T_const;}
"continue"  {return T_cont;}
"default"   {return T_default;}
"do"        {return T_do;}
"DOWNTO"    {return T_downto;}
"else"      {return T_else;}
"false"     {return T_false;}
"FOR"       {return T_for;}
"FORM"      {return T_form;}
"FUNC"      {return T_func;}
"if"        {return T_if;}
"int"       {return T_int;}
"MOD"       {return T_mod;}
"NEXT"      {return T_next;}
"not"       {return T_not;}
"or"        {return T_or;}
"PROC"      {return T_proc;}
"PROGRAM"   {return T_program;}
"REAL"      {return T_real;}
"return"    {return T_return;}
"STEP"      {return T_step;}
"switch"    {return T_switch;}
"TO"        {return T_to;}
"true"      {return T_true;}
"while"     {return T_while;}
"WRITE"     {return T_write;}
"WRITELN"   {return T_writeln;}
"WRITESP"   {return T_writesp;}
"WRITESPLN" {return T_writespln;}


"'\\n'"                         { yylval.c = '\n';      return T_char_lit; /* characters */}
"'\\t'"                         { yylval.c = '\t';      return T_char_lit; }
"'\\r'"                         { yylval.c = '\r';      return T_char_lit; }
"'\\0'"                         { yylval.c = '\0';      return T_char_lit; }
"'\\\'"                         { yylval.c = '\\';      return T_char_lit; }
"'\\''"                         { yylval.c = '\'';      return T_char_lit; }
"'\\\"'"                        { yylval.c = '\"';      return T_char_lit; }
\'{PRINT}\'                     { yylval.c = yytext[1]; return T_char_lit; }
\"([^\"\n\\]|\\{ESC})*\"        { yylval.n = strdup(yytext+1);             /* string literals */
                                  yylval.n[strlen(yylval.n)-1] = '\0';
                                  return T_string_lit; }
({L}|{l})({L}|{l}|{D}|_)*       { yylval.n = strdup(yytext);               /* identifiers */
                                  return T_identifier; }
({D}+)({L}|{l}|_)+              { fprintf(stderr, "Lexical error\n");      /* 9gh => error */
                                  exit(1); }
[1-9]{D}*                       { yylval.i = atoi(yytext);                 /* integer literals */
                                  return T_int_lit; }
0                               { yylval.i = atoi(yytext);
                                  return T_int_lit; }
(0)({D}+)                       { fprintf(stderr, "Lexical error\n");      /* 083 => error */
                                  exit(1); }
{D}+(\.{D}+([Ee][\-\+]?{D}+)?)? { sscanf(yytext, "%Lf", &yylval.r);        /* real literals */
                                  return T_real_lit; }


"=="                         {return T_eqeq;}
"!="                         {return T_noteq;}
">="                         {return T_geq;}
"<="                         {return T_leq;}
"&&"                         {return T_andand;}
"||"                         {return T_oror;}
"++"                         {return T_plusplus;}
"--"                         {return T_minmin;}
"+="                         {return T_pluseq;}
"-="                         {return T_mineq;}
"*="                         {return T_muleq;}
"/="                         {return T_diveq;}
"%="                         {return T_modeq;}


[\&\;\.\(\)\:\,\[\]\{\}]     {return yytext[0];  /* separators */ }
[\>\<\+\-\*\/\%\!\=]         {return yytext[0];  /* operators  */ }


\/\/.*                       { /* single line comments */ }
"/*"([^*]+|\*+[^\*\/])*\*+\/ { 
    /* multi line comments */
    for(unsigned int i = 0; i < strlen(yytext); i++)
        if (yytext[i] == '\n')
            lineno++;
}
{W}+                         { /* whitespace */ }
\n                           { lineno++; /* new line counter */ }
<<EOF>>                      { return T_eof; /* end of file */}
.                            { fprintf(stderr, "Line %d: Lexical error\n", lineno); exit(1); }

%%
