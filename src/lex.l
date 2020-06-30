%{
#include "lex.h"
#include "y.tab.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
%}

%pointer

isuff [iu](8|16|32|64)

fexp [eE][-+]?[0-9]+
fsuff f(32|64|80)

%%

[ \t\n\r]+	|
"//".*	|
"/*"([^*]|\*[^/])*"*/"	{ }

"fn"	{ return FN; }
"ns"	{ return NS; }
"->"	{ return ARROW; }

"if"	{ return IF; }
"else"	{ return ELSE; }
"while"	{ return WHILE; }

"break"	{ return BREAK; }
"continue"	{ return CONTINUE; }
"return"	{ return RETURN; }

"ptr"	{ return PTR; }
"mut"	{ return MUT; }
"vol"	{ return VOL; }

"bool"	{ return BOOL; }
"void"	{ return VOID; }
"struct"	{ return STRUCT; }
"union"	{ return UNION; }

"u8"	{ return U8; }
"u16"	{ return U16; }
"u32"	{ return U32; }
"u64"	{ return U64; }

"i8"	{ return I8; }
"i16"	{ return I16; }
"i32"	{ return I32; }
"i64"	{ return I64; }

"f32"	{ return F32; }
"f64"	{ return F64; }
"f80"	{ return F80; }

[()[\]{};,]	{ return *yytext; }
[-+*/%&^|<=>!~.]	{ return *yytext; }

"+="	{ return ADDEQ; }
"-="	{ return SUBEQ; }
"*="	{ return MULEQ; }
"/="	{ return DIVEQ; }
"%="	{ return MODEQ; }
"<<="	{ return LSHEQ; }
">>="	{ return RSHEQ; }
"&="	{ return ANDEQ; }
"^="	{ return XOREQ; }
"|="	{ return IOREQ; }

"||"	{ return LOGICAL_OR; }
"&&"	{ return LOGICAL_AND; }
"=="	{ return EQUAL; }
"!="	{ return NOT_EQUAL; }
"<="	{ return LTE; }
">="	{ return GTE; }

"<<"	{ return LSH; }
">>"	{ return RSH; }
"++"	{ return INCR; }
"--"	{ return DECR; }

[a-zA-Z_][a-zA-Z0-9_]*	{ return IDENTIFIER; }

[1-9][0-9]*{isuff}?	{ return DEC_INTEGER; }
0[0-7]*{isuff}?	{ return OCT_INTEGER; }
0b[01]+{isuff}?	{ return BIN_INTEGER; }
0x[0-9a-fA-F]+{isuff}?	{ return HEX_INTEGER; }

[0-9]+\.[0-9]*{fexp}?{fsuff}?	{ return FLOAT; }
\.[0-9]+{fexp}?{fsuff}?	{ return FLOAT; }
[0-9]+{fexp}{fsuff}?	{ return FLOAT; }

\"([^"\\]|\\.)*\"	{ return STRING; }
'([^'\\]|\\.)'	{ return CHARACTER; }

%%
#pragma GCC diagnostic pop

int yywrap(void) {
	return 1;
}
