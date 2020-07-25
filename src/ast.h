// vim: noet

#ifndef AST_H
#define AST_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

enum float_type {
	F_32,
	F_64,
	F_80,
};

#define I_SIGNED 0x100

enum int_type {
	U_8  = 8,
	U_16 = 16,
	U_32 = 32,
	U_64 = 64,

	I_8  = 8  | I_SIGNED,
	I_16 = 16 | I_SIGNED,
	I_32 = 32 | I_SIGNED,
	I_64 = 64 | I_SIGNED,
};

struct val_type {
	enum {
		TYPE_PTR,
		TYPE_FUNC,
		TYPE_VOID,
		TYPE_INT,
		TYPE_FLOAT,
		TYPE_NEWTYPE,
		TYPE_STRUCT,
		TYPE_UNION,
		TYPE_BOOL,
	} t;

	union {
		struct ref_type *ptr;

		struct {
			size_t nargs;
			struct ref_type *args;

			struct val_type *ret_type;
		} func;

		enum float_type float_;
		enum int_type int_;

		const char *newtype_name;

		struct {
			size_t nfields;
			struct {
				const char *name;
				struct val_type *type;
			} *fields;
		} composite;
	};
};

struct ref_type {
	bool vol, mut;
	struct val_type to;
};

struct ast_expr {
	enum {
		EXPR_BINOP,
		EXPR_UNOP,
		EXPR_CALL,
		EXPR_IF,
		EXPR_WHILE,
		EXPR_BREAK,
		EXPR_CONTINUE,
		EXPR_RETURN,
		EXPR_FUNC,
		EXPR_INT_LIT,
		EXPR_FLOAT_LIT,
		EXPR_ARR_LIT,
		EXPR_COMPOSITE_LIT,
		EXPR_BOOL_LIT,
		EXPR_FIELD_ACCESS,
		EXPR_LET,
		EXPR_CAST,
		EXPR_IDENT,
	} t;

	struct val_type type;

	union {
		struct {
			enum {
				BINOP_ADD,
				BINOP_SUB,
				BINOP_MUL,
				BINOP_DIV,
				BINOP_MOD,
				BINOP_BOOL_AND,
				BINOP_BOOL_OR,
				BINOP_BIN_AND,
				BINOP_BIN_OR,
				BINOP_BIN_XOR,
				BINOP_EQUAL,
				BINOP_NEQUAL,
				BINOP_ASSIGN,
				BINOP_LSHIFT,
				BINOP_RSHIFT,
				BINOP_GT,
				BINOP_LT,
				BINOP_GTE,
				BINOP_LTE,
				BINOP_SEQOP,
			} t;

			struct ast_expr *x, *y;
		} binop;

		struct {
			enum {
				UNOP_REF,
				UNOP_DEREF,
				UNOP_PREINC,
				UNOP_POSTINC,
				UNOP_PREDEC,
				UNOP_POSTDEC,
				UNOP_BOOL_NOT,
				UNOP_BIN_NOT,
				UNOP_SIZEOF,
				UNOP_PLUS,
				UNOP_MINUS,
			} t;

			struct ast_expr *x;
		} unop;

		struct {
			struct ast_expr *func;

			size_t nargs;
			struct ast_expr *args;
		} call;

		struct {
			struct ast_expr *cond;
			// f can be NULL for easiness
			struct ast_expr *t, *f;
		} if_;

		struct {
			struct ast_expr *cond;
			struct ast_expr *body;
		} while_;

		struct {
			// NULL indicates none
			const char *lbl;
		} break_;

		struct {
			// NULL indicates none
			const char *lbl;
		} continue_;

		struct {
			// NULL indicates void
			struct ast_expr *val;
		} return_;

		struct {
			size_t nargs;
			struct {
				const char *name;
				struct ref_type type;
			} *args;

			struct val_type ret;

			struct ast_expr *body;
		} func;

		struct {
			enum int_type type;
			union {
				uint64_t u;
				int64_t i;
			};
		} int_lit;

		struct {
			enum float_type type;
			long double x;
		} float_lit;

		bool bool_lit;

		struct {
			struct val_type type;
			size_t nelems;
			struct ast_expr *vals;
		} aggregate_lit;

		struct {
			struct ast_expr *aggr;
			const char *field;
		} field_access;

		struct {
			const char *name;
			struct ref_type type;
			struct ast_expr *val;
			struct ast_expr *body;
			// NULL if not present
			struct ast_expr *deferred;
		} let;

		struct {
			struct val_type type;
			struct ast_expr *val;
		} cast;

		const char *ident;
	};
};

struct ast_toplevel {
	enum {
		EXPRTOP_FUNC,
		EXPRTOP_DECL,
		EXPRTOP_NAMESPACE,
	} type;

	union {
		struct {
			const char *name;

			size_t nargs;
			struct {
				const char *name;
				struct ref_type type;
			} *args;

			struct val_type ret;

			struct ast_expr *body;
		} func;

		struct {
			struct ref_type type;
			const char *name;
			struct ast_expr *val;
		} decl;

		struct {
			size_t size;
			struct ast_toplevel *body;
		} namespace;
	};
};

#endif
