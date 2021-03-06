%{
#include <stdio.h>
#include <string.h>

#include "type.hpp"
#include "symbol_table.hpp"


static void count(void);
static int comment(void);

#include "grammar.tab.h"
%}

D[0-9]
L[a-zA-Z_]
%option yylineno
%%
"/*"		{ comment(); }
"//".*          { count(); }
"float"		{ count(); return(TOKEN_FLOAT); }
"if"		{ count(); return(TOKEN_IF); }
"else"		{ count(); return(TOKEN_ELSE); }
"int"		{ count(); return(TOKEN_INT); }
"return"	{ count(); return(TOKEN_RETURN); }
"void"		{ count(); return(TOKEN_VOID); }
"while"		{ count(); return(TOKEN_WHILE); }
"for"		{ count(); return(TOKEN_FOR); }
"map"           { count(); return(TOKEN_MAP); }
"reduce"        { count(); return(TOKEN_REDUCE); }
"extern"        { count(); return(TOKEN_EXTERN); }
"do"            { count(); return(TOKEN_DO); }
[$]?{L}({L}|{D})*	{ count(); yylval.str=strdup(yytext);
                            return(TOKEN_IDENTIFIER); }
{D}+    	{ count(); yylval.i=atoi(yytext); return(TOKEN_CONSTANTI); }
{D}+"."{D}*     { count(); yylval.f=atof(yytext); return(TOKEN_CONSTANTF); }
"+="		{ count(); return(TOKEN_ADD_ASSIGN); }
"-="		{ count(); return(TOKEN_SUB_ASSIGN); }
"*="		{ count(); return(TOKEN_MUL_ASSIGN); }
"++"		{ count(); return(TOKEN_INC_OP); }
"--"		{ count(); return(TOKEN_DEC_OP); }
"<="		{ count(); return(TOKEN_LE_OP); }
">="		{ count(); return(TOKEN_GE_OP); }
"=="		{ count(); return(TOKEN_EQ_OP); }
"!="		{ count(); return(TOKEN_NE_OP); }
";"		{ count(); return(';'); }
"{"      	{ count(); st_push(); return('{'); }
"}"      	{ count(); st_pop(); return('}'); }
","		{ count(); return(','); }
"/"		{ count(); return('/'); }
"="		{ count(); return('='); }
"("		{ count(); return('('); }
")"		{ count(); return(')'); }
"["     	{ count(); return('['); }
"]"     	{ count(); return(']'); }
"."		{ count(); return('.'); }
"!"		{ count(); return('!'); }
"-"		{ count(); return('-'); }
"+"		{ count(); return('+'); }
"*"		{ count(); return('*'); }
"<"		{ count(); return('<'); }
">"		{ count(); return('>'); }
[ \t\v\n\f]	{ count(); }
.		{ /* ignore bad characters */ }

%%
int yywrap()
{
	return 1;
}
/* This function reads a comment. Comments are not part of the grammar. */
static int comment(void)
{
	char c, c1;
loop:
	while ((c = yyinput()) != '*' && c != 0);
	if ((c1 = yyinput()) != '/' && c != 0) {
		unput(c1);
		goto loop;
	}
	return 0;
}

int yycolno = 0;

/* This function counts the number of character, for debugging purposes */
static void count(void)
{
    int i;
    for (i = 0; yytext[i] != '\0'; i++) {
	if (yytext[i] == '\n')
	    yycolno = 0;
	else if (yytext[i] == '\t')
	    yycolno += 8 - (yycolno % 8);
	else
	    yycolno++;
    }
}

