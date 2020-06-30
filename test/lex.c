#include "vtest.h"
#include "testhelper.h"
#include "lex.h"
#include "y.tab.h"

#define _assert_toks(source, ...) do { \
		yyin = stropen(source); \
		vassert_not_null(yyin); \
		int toks[] = {__VA_ARGS__}, *tok = toks; \
		do vassert_eq(yylex(), *tok); while (*tok++); \
	} while (0)
#define assert_toks(...) _assert_toks(__VA_ARGS__, 0)

VTEST(test_whitespace) {
	// Should reach EOF immediately
	assert_toks(" \t\n\n\r\n\t\t       ");
}

VTEST(test_comment) {
	assert_toks(
		"// hello, world!!!! 123 1.1.1 \"'\n"
		"/* foo bar 'aaaaaaaa' \" // baz\n"
		"123 'aaaaa' // asdfasdf\n"
		"*/    \t1\n",
		DEC_INTEGER
	);
}

VTEST(test_keyword) {
	assert_toks(
		"fn ns if else while break continue return ptr mut vol",
		FN, NS, IF, ELSE, WHILE, BREAK, CONTINUE, RETURN, PTR, MUT, VOL
	);
}

VTEST(test_type) {
	assert_toks("bool void struct union", BOOL, VOID, STRUCT, UNION);
	assert_toks("u8 u16 u32 u64", U8, U16, U32, U64);
	assert_toks("i8 i16 i32 i64", I8, I16, I32, I64);
	assert_toks("f32 f64 f80", F32, F64, F80);
}

VTEST(test_symbol) {
	assert_toks("()[]{};,->", '(', ')', '[', ']', '{', '}', ';', ',', ARROW);
	assert_toks(" ( ) [ ] { } ; , -> ", '(', ')', '[', ']', '{', '}', ';', ',', ARROW);
}

VTEST(test_operator) {
	assert_toks("=+-*/%&^|<>!~=.=", '=', '+', '-', '*', '/', '%', '&', '^', '|', '<', '>', '!', '~', '=', '.', '=');
	assert_toks(" + - * / % & ^ | < = > ! ~ . ", '+', '-', '*', '/', '%', '&', '^', '|', '<', '=', '>', '!', '~', '.');

	assert_toks("+=-=*=/=%=<<=>>=&=^=|=", ADDEQ, SUBEQ, MULEQ, DIVEQ, MODEQ, LSHEQ, RSHEQ, ANDEQ, XOREQ, IOREQ);
	assert_toks(" += -= *= /= %= <<= >>= &= ^= |= ", ADDEQ, SUBEQ, MULEQ, DIVEQ, MODEQ, LSHEQ, RSHEQ, ANDEQ, XOREQ, IOREQ);

	assert_toks("||&&==!=<=>=", LOGICAL_OR, LOGICAL_AND, EQUAL, NOT_EQUAL, LTE, GTE);
	assert_toks(" || && == != <= >= ", LOGICAL_OR, LOGICAL_AND, EQUAL, NOT_EQUAL, LTE, GTE);
	assert_toks(" | | & & = = ! = < = > = ", '|', '|', '&', '&', '=', '=', '!', '=', '<', '=', '>', '=');

	assert_toks("<<>>++--", LSH, RSH, INCR, DECR);
	assert_toks(" << >> ++ -- ", LSH, RSH, INCR, DECR);
	assert_toks(" < < > > + + - - ", '<', '<', '>', '>', '+', '+', '-', '-');
}

VTEST(test_literal) {
	yyin = stropen(
		"hello foo_bar i123\n"
		"fnns ifelse whilebreak continuereturn ptrmutvol mybool boolvoid\n"

		"123 0123 0b1010 0x123456789abcdef\n"

		"0. 0.0 .0 0e1\n"
		"0.e1 0.0e1 .0e1\n"
		"0.f32 0.f64 0.f80\n"
		"0.0f32 0.0f64 0.0f80\n"
		".0f32 .0f64 .0f80\n"
		"0e1f32 0e1f64 0e1f80\n"
		"0.e1f32 0.0e1f64 .0e1f80\n"

		"\"abcdef\" \"foo\\\"bar\\n\\u101a\"\n"
		"'a' '\\n' '\\''\n"

		"123foo 1.foo foo.1\n"
	);
	vassert_not_null(yyin);

	struct {int tok; const char *text;} tokens[] = {
		{IDENTIFIER, "hello"},
		{IDENTIFIER, "foo_bar"},
		{IDENTIFIER, "i123"},

		{IDENTIFIER, "fnns"},
		{IDENTIFIER, "ifelse"},
		{IDENTIFIER, "whilebreak"},
		{IDENTIFIER, "continuereturn"},
		{IDENTIFIER, "ptrmutvol"},
		{IDENTIFIER, "mybool"},
		{IDENTIFIER, "boolvoid"},

		{DEC_INTEGER, "123"},
		{OCT_INTEGER, "0123"},
		{BIN_INTEGER, "0b1010"},
		{HEX_INTEGER, "0x123456789abcdef"},

		{FLOAT, "0."},
		{FLOAT, "0.0"},
		{FLOAT, ".0"},
		{FLOAT, "0e1"},
		{FLOAT, "0.e1"},
		{FLOAT, "0.0e1"},
		{FLOAT, ".0e1"},
		{FLOAT, "0.f32"},
		{FLOAT, "0.f64"},
		{FLOAT, "0.f80"},
		{FLOAT, "0.0f32"},
		{FLOAT, "0.0f64"},
		{FLOAT, "0.0f80"},
		{FLOAT, ".0f32"},
		{FLOAT, ".0f64"},
		{FLOAT, ".0f80"},
		{FLOAT, "0e1f32"},
		{FLOAT, "0e1f64"},
		{FLOAT, "0e1f80"},
		{FLOAT, "0.e1f32"},
		{FLOAT, "0.0e1f64"},
		{FLOAT, ".0e1f80"},

		{STRING, "\"abcdef\""},
		{STRING, "\"foo\\\"bar\\n\\u101a\""},
		{CHARACTER, "'a'"},
		{CHARACTER, "'\\n'"},
		{CHARACTER, "'\\''"},

		{DEC_INTEGER, "123"}, {IDENTIFIER, "foo"},
		{FLOAT, "1."}, {IDENTIFIER, "foo"},
		{IDENTIFIER, "foo"}, {FLOAT, ".1"},

		{0, ""},
	}, *tok = tokens;

	do {
		vassert_eq(yylex(), tok->tok);
		vassert_eq_s(yytext, tok->text);
	} while (tok++->tok);
}

VTESTS_BEGIN
	test_whitespace,
	test_comment,
	test_keyword,
	test_type,
	test_symbol,
	test_operator,
	test_literal,
VTESTS_END
