// vim: noet

#ifndef AST_H
#define AST_H

enum float_type {
	F_32,
	F_64,
	F_80,
};

struct int_type {
	enum {
		I_8,
		I_16,
		I_32,
		I_64,
	} size;

	enum {
		SIGNED,
		UNSIGNED,
	} signedness;
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
	} type;

	union {
		struct ptr_type *ptr;

		struct {
			size_t nargs;
			struct ptr_type *args;

			struct val_type *ret_type;
		} func;

		enum float_type float_;
		struct int_type int_;

		// TODO: I don't know how we're representing newtypes, I've just put
		// a numeric ID here for now
		uint64_t newtype_id;

		struct {
			size_t nfields;
			struct {
				const char *name;
				const val_type type;
			} *fields;
		} struct_;
	};
};

struct ptr_type {
	bool vol, mut;
	struct val_type to;
};

struct ast_expr {
	enum {
		EXPR_BLOCK,
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
		EXPR_STRUCT_LIT,
		EXPR_FIELD_ACCESS,
	} type;

	// NULL if at root
	struct ast_expr *parent;

	union {
		struct {
			size_t nlocals;
			struct {
				const char *name;
				struct ptr_type type;
			} *locals;

			size_t nexprs;
			struct ast_expr *exprs;
		} block;

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
			} type;

			struct val_ptr *x, *y;
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
			} type;

			struct val_ptr *x;
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
			// NULL indicates void
			struct ast_expr *val;
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
				struct ptr_type type;
			} *args;

			struct val_type ret;

			struct ast_expr *body;
		} func;

		struct {
			struct int_type type;
			union {
				uint64_t u;
				int64_t i;
			};
		} int_lit;

		struct {
			enum float_type float_;
			long double x;
		} float_lit;

		struct {
			size_t nelems;
			struct ast_expr *elems;
		} arr_lit;

		struct {
			// We use a normal val_type here because you explicitly specify
			// the type of a struct literal and e.g. a `foo` is incompatible
			// with a `struct { int x }` even if it's newtyped
			struct val_type type;
			// Must be as many as there are fields
			struct ast_expr *vals;
		} struct_lit;

		struct {
			struct ast_expr *aggr;
			const char *field;
		} field_access;
	}
};

struct ast_expr_top {
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
				struct ptr_type type;
			} *args;

			struct val_type ret;

			struct ast_expr *body;
		} func;

		struct {
			struct ptr_type type;
			const char *name;
			struct ast_expr *val;
		} decl;

		struct {
			size_t nexprs;
			struct ast_expr_top *exprs;
		} namespace;
	};
};

#endif
