%{
#include "lex.h"
int yyerror(const char *s);
%}

%token IDENTIFIER DEC_INTEGER OCT_INTEGER BIN_INTEGER HEX_INTEGER FLOAT STRING CHARACTER
%token FN NS ARROW
%token IF ELSE WHILE
%token BREAK CONTINUE RETURN

%token PTR MUT VOL
%token BOOL VOID STRUCT UNION
%token U8 U16 U32 U64
%token I8 I16 I32 I64
%token F32 F64 F80

%token ADDEQ SUBEQ MULEQ DIVEQ MODEQ LSHEQ RSHEQ ANDEQ XOREQ IOREQ
%token LOGICAL_OR LOGICAL_AND EQUAL NOT_EQUAL LTE GTE
%token LSH RSH INCR DECR

%%

toplevels : toplevel toplevels | ;

toplevel
	: global_function
	| global_variable
	| namespace
	;

global_function
	: FN identifier '(' named_arguments ')' func_ret expr
	| FN identifier '(' maybe_named_arguments ')' func_ret ';'
	;
named_arguments
	: identifier ref_type ',' named_arguments
	| identifier ref_type
	|
	;
maybe_named_arguments
	: identifier ref_type ',' maybe_named_arguments
	| ref_type ',' maybe_named_arguments
	| identifier ref_type
	| ref_type
	|
	;
func_ret
	: ARROW val_type
	|
	;

global_variable
	: identifier ref_type ';'
	| identifier ref_type '=' expr ';'
	;

namespace
	: NS identifier '{' toplevels '}'
	;

ref_type
	: val_type
	;
val_type
	: PTR ref_type
	| function_type
	| VOID
	| int_type
	| float_type
	| identifier
	| composite_type
	;

function_type
	: FN '(' maybe_named_arguments ')' func_ret
	;

int_type
	: BOOL
	| U8 | U16 | U32 | U64
	| I8 | I16 | I32 | I64
	;

float_type
	: F32 | F64 | F80
	;

composite_type
	: STRUCT '{' fields '}'
	| UNION '{' fields '}'
fields
	: identifier ref_type ';' fields
	|
	;

expr
	: if
	| while
	| break
	| CONTINUE
	| return
	| op_sequence

if
	: IF '(' expr ')' expr else
	;
else
	: ELSE expr
	|
	;

while
	: WHILE '(' expr ')' expr
	;

break
	: BREAK
	| BREAK expr
	;

return
	: RETURN
	| RETURN expr
	;

op_sequence
	: op_sequence ';' op_assign
	| op_assign
	;

op_assign
	: op_lor assignop op_assign
	| op_lor
	;
assignop
	: '='
	| ADDEQ
	| SUBEQ
	| MULEQ
	| DIVEQ
	| MODEQ
	| LSHEQ
	| RSHEQ
	| ANDEQ
	| XOREQ
	| IOREQ
	;

op_lor
	: op_lor LOGICAL_OR op_land
	| op_land
	;

op_land
	: op_land LOGICAL_AND op_eq
	| op_eq
	;

op_eq
	: op_eq EQUAL op_cmp
	| op_eq NOT_EQUAL op_cmp
	| op_cmp
	;

op_cmp
	: op_cmp cmpop op_ior
	| op_ior
	;
cmpop
	: '<'
	| '>'
	| LTE
	| GTE
	;

op_ior
	: op_ior '|' op_xor
	| op_xor
	;

op_xor
	: op_xor '|' op_and
	| op_and
	;

op_and
	: op_and '&' op_shift
	| op_shift
	;

op_shift
	: op_shift LSH op_add
	| op_shift RSH op_add
	| op_add
	;

op_add
	: op_add '+' op_mul
	| op_add '-' op_mul
	| op_mul
	;

op_mul
	: op_mul '*' op_prefix
	| op_mul '/' op_prefix
	| op_mul '%' op_prefix
	| op_prefix
	;

op_prefix
	: prefixop op_prefix
	| '(' val_type ')' op_prefix
	| op_postfix
	;
prefixop
	: PTR
	| INCR
	| DECR
	| '!'
	| '~'
	| '+'
	| '-'

op_postfix
	: op_postfix INCR
	| op_postfix DECR
	| op_postfix '.' identifier
	| op_postfix '(' exprs ')'
	| literal
	| '(' expr ')'
	| '[' expr ']'
	;
exprs
	: expr ',' exprs
	| expr
	|
	;

literal
	: integer
	| float_
	| string
	| character
	| literal_array
	| literal_composite
	| literal_function
	;
literal_array
	: val_type '[' exprs ']'
	;
literal_composite
	: val_type '{' exprs '}'
	;
literal_function
	: FN '(' named_arguments ')' func_ret expr
	;

// Lexical elements
identifier
	: IDENTIFIER
	;

integer
	: DEC_INTEGER
	| OCT_INTEGER
	| BIN_INTEGER
	| HEX_INTEGER
	;

float_
	: FLOAT
	;

string
	: STRING
	;

character
	: CHARACTER
	;

%%
