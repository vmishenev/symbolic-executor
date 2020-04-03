/* Mini Calculator */
/* calc.lex */
%option noyywrap
%{
#include "heading.h"
#include "tok.h"
int yyerror(char *s);
//int yylineno = 1;
%}

digit		[0-9]
integer_const	{digit}+

%%
"func"																										{return FUNC;}
"if"																										{return IF;}
"else"																										{return ELSE;}
"return"																									{return RETURN;}
":="																										{return ASSIGN;}
"assert"																									{return ASSERT;}
"||" 																										{return or_const;}
"&&"																										{return and_const;}
"=="																										{return eq_const;}
"!="																										{return neq_const;}
"<="																										{return lte_const;}
">="																										{return gte_const;}																									


";"|"="|","|"{"|"}"|"("|")"|"["|"]"|"*"|"+"|"-"|"/"|"?"|":"|"&"|"|"|"^"|"!"|"~"|"%"|"<"|">"					{return yytext[0];}
"*="|"/="|"+="|"%="|">>="|"-="|"<<="|"&="|"^="|"|="															{return PUNC;}
[0-9]+																										{yylval.int_val = atoi(yytext); return int_const;}
"'"."'"																										{yylval.op_val = new std::string(yytext); return char_const;}
[a-zA-z_][a-zA-z_0-9]*			 																			{yylval.op_val = new std::string(yytext); return id;}
\"(\\.|[^\"])*\"			 																				{yylval.op_val = new std::string(yytext); return str_const;}


[ \t]*		{}
[\n]		{ yylineno++;	}

.		{ std::cerr << "SCANNER "; yyerror(""); exit(1);	}

