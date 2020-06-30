#ifndef LEX_H
#define LEX_H

#include <stdio.h>

int yylex(void);
extern FILE *yyin;
extern char *yytext;
extern int yyleng;

#endif
