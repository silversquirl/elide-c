// vim: noet

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// Type equality checks {{{

bool vtype_eq(struct val_type *x, struct val_type *y);

bool rtype_eq(struct ref_type *x, struct ref_type *y) {
	if (!x || !y) return false;

	if (x->vol != y->vol) return false;
	if (x->mut != y->mut) return false;

	return vtype_eq(&x->to, &y->to);
}

bool vtype_eq(struct val_type *x, struct val_type *y) {
	if (!x || !y) return false;

	if (x->t != y->t) return false;

	switch (x->t) {
	case TYPE_PTR:
		return rtype_eq(x->ptr, y->ptr);

	case TYPE_FUNC:
		if (x->func.nargs != y->func.nargs) return false;
		if (!vtype_eq(x->func.ret_type, y->func.ret_type)) return false;
		for (size_t i = 0; i < x->func.nargs; ++i) {
			if (!rtype_eq(&x->func.args[i], &y->func.args[i])) return false;
		}
		return true;

	case TYPE_VOID:
	case TYPE_BOOL:
		return true;

	case TYPE_INT:
		return x->int_ == y->int_;

	case TYPE_FLOAT:
		return x->float_ == y->float_;

	case TYPE_NEWTYPE:
		return !strcmp(x->newtype_name, y->newtype_name);

	case TYPE_STRUCT:
	case TYPE_UNION:
		if (x->composite.nfields != y->composite.nfields) return false;
		for (size_t i = 0; i < x->composite.nfields; ++i) {
			if (strcmp(x->composite.fields[i].name, y->composite.fields[i].name)) return false;
			if (!vtype_eq(x->composite.fields[i].type, y->composite.fields[i].type)) return false;
		}
		return true;
	}
}

// }}}

bool _cast_valid(struct val_type from, struct val_type to) {
	// FIXME newtypes

	if (from.t == TYPE_VOID) return false;
	
	if (vtype_eq(&from, &to)) return true;

	switch (to.t) {
	case TYPE_VOID:
		return true;

	case TYPE_PTR:
	case TYPE_INT:
	case TYPE_FLOAT:
		return from.t == to.t;

	case TYPE_BOOL:
		return from.t == TYPE_INT || from.t == TYPE_FLOAT;

	default:
		return false;
	}
}

struct {
	// Stack of functions
	size_t nfuncs;
	size_t alloc;
	struct {
		// Function return type
		struct val_type ret;

		// Arguments (taken from the def itself)
		size_t nargs;
		struct {
			const char *name;
			struct ref_type type;
		} *args;

		// Stack of scopes
		size_t nscopes;
		size_t scopes_alloc;
		struct {
			const char *name;
			struct ref_type type;
		} *scopes;
	} *funcs;
} _func_stack;

#define cur_func (_func_stack.funcs[_func_stack.nfuncs-1])


#define VALTYPE 0
#define REFTYPE (1<<0)
#define REF_MUT (1<<1)
#define REF_VOL (1<<2)

uint8_t annotate_type(struct ast_expr *e) {
	uint8_t x_tflags; // fuck C
	switch (e->t) {
	// EXPR_BINOP {{{
	case EXPR_BINOP:
		x_tflags = annotate_type(e->binop.x);
		uint8_t y_tflags = annotate_type(e->binop.y);

		if (e->unop.t != BINOP_SEQOP) {
			if (!vtype_eq(&e->binop.x->type, &e->binop.y->type)) {
				// XXX error
			}

			if (e->binop.x->type.t == TYPE_VOID || e->binop.y->type.t == TYPE_VOID) {
				// XXX error
			}
		}

		switch (e->binop.t) {
		case BINOP_ADD:
			if (e->binop.x->type.t == TYPE_PTR) {
				if (e->binop.y->type.t != TYPE_INT) {
					// XXX error
				}
				e->type = e->binop.x->type;
				return VALTYPE;
			}

			if (e->binop.y->type.t == TYPE_PTR) {
				if (e->binop.x->type.t != TYPE_INT) {
					// XXX error
				}
				e->type = e->binop.y->type;
				return VALTYPE;
			}

			if (!vtype_eq(&e->binop.x->type, &e->binop.y->type)) {
				// XXX error
			}

			if (e->binop.x->type.t != TYPE_INT
					&& e->binop.x->type.t != TYPE_FLOAT) {
				// XXX error
			}

			e->type = e->binop.x->type;
			return VALTYPE;

		case BINOP_SUB:
			if (!vtype_eq(&e->binop.x->type, &e->binop.y->type)) {
				// XXX error
			}

			if (e->binop.x->type.t != TYPE_INT
					&& e->binop.x->type.t != TYPE_FLOAT
					&& e->binop.x->type.t != TYPE_PTR) {
				// XXX error
			}

			if (e->binop.x->type.t == TYPE_PTR) {
				e->type.t = TYPE_INT;
				e->type.int_ = U_64;
			} else {
				e->type = e->binop.x->type;
			}
			return VALTYPE;

		case BINOP_MUL:
		case BINOP_DIV:
		case BINOP_MOD:
		case BINOP_LSHIFT:
		case BINOP_RSHIFT:
			if (!vtype_eq(&e->binop.x->type, &e->binop.y->type)) {
				// XXX error
			}

			if (e->binop.x->type.t != TYPE_INT
					&& e->binop.x->type.t != TYPE_FLOAT) {
				// XXX error
			}

			e->type = e->binop.x->type;
			return VALTYPE;

		case BINOP_BOOL_AND:
		case BINOP_BOOL_OR:
			if (!vtype_eq(&e->binop.x->type, &e->binop.y->type)) {
				// XXX error
			}

			if (e->binop.x->type.t != TYPE_BOOL) {
				// XXX error
			}

			e->type.t = TYPE_BOOL;
			return VALTYPE;

		case BINOP_BIN_AND:
		case BINOP_BIN_OR:
		case BINOP_BIN_XOR:
			if (!vtype_eq(&e->binop.x->type, &e->binop.y->type)) {
				// XXX error
			}

			if (e->binop.x->type.t != TYPE_INT) {
				// XXX error
			}

			e->type = e->binop.x->type;
			return VALTYPE;

		case BINOP_EQUAL:
		case BINOP_NEQUAL:
			if (!vtype_eq(&e->binop.x->type, &e->binop.y->type)) {
				// XXX error
			}

			e->type.t = TYPE_BOOL;
			return VALTYPE;

		case BINOP_ASSIGN:
			if (!vtype_eq(&e->binop.x->type, &e->binop.y->type)) {
				// XXX error
			}

			if (!(x_tflags & REFTYPE) || !(x_tflags & REF_MUT)) {
				// XXX error
			}
			e->type = e->binop.x->type;
			return x_tflags;

		case BINOP_GT:
		case BINOP_LT:
		case BINOP_GTE:
		case BINOP_LTE:
			if (!vtype_eq(&e->binop.x->type, &e->binop.y->type)) {
				// XXX error
			}

			if (e->binop.x->type.t != TYPE_INT
					&& e->binop.x->type.t != TYPE_FLOAT
					&& e->binop.x->type.t != TYPE_PTR) {
				// XXX error
			}

			e->type.t = TYPE_BOOL;
			return VALTYPE;

		case BINOP_SEQOP:
			// TODO: I feel like enforcing x to have type void. What does
			// vktec think?
			e->type = e->binop.y->type;
			return y_tflags;
		}
	// }}}

	// EXPR_UNOP {{{
	case EXPR_UNOP:
		x_tflags = annotate_type(e->unop.x);

		if (e->unop.x->type.t == TYPE_VOID) {
			// XXX error
		}

		switch (e->unop.t) {
		case UNOP_REF:
			if (!(x_tflags & REFTYPE)) {
				// XXX error
			}
			e->type.t = TYPE_PTR;
			e->type.ptr = malloc(sizeof *e->type.ptr);
			e->type.ptr->mut = x_tflags & REF_MUT;
			e->type.ptr->vol = x_tflags & REF_VOL;
			e->type.ptr->to = e->unop.x->type;
			return VALTYPE;

		case UNOP_DEREF:
			if (e->unop.x->type.t != TYPE_PTR) {
				// XXX error
			}
			struct ref_type type = *e->unop.x->type.ptr;
			e->type = type.to;
			uint8_t ret = REFTYPE;
			if (type.mut) ret |= REF_MUT;
			if (type.vol) ret |= REF_VOL;
			return ret;

		case UNOP_PREINC:
		case UNOP_POSTINC:
		case UNOP_PREDEC:
		case UNOP_POSTDEC:
			if (!(x_tflags & REFTYPE) || !(x_tflags & REF_MUT)) {
				 // XXX error
			}
		case UNOP_PLUS:
			if (e->unop.x->type.t != TYPE_INT && e->unop.x->type.t != TYPE_FLOAT) {
				// XXX error
			}
			e->type = e->unop.x->type;
			return VALTYPE;

		case UNOP_MINUS:
			if (e->unop.x->type.t != TYPE_INT && e->unop.x->type.t != TYPE_FLOAT) {
				// XXX error
			}
			if (e->unop.x->type.t == TYPE_INT && !(e->unop.x->type.int_ & I_SIGNED)) {
				// XXX error
			}
			e->type = e->unop.x->type;
			return VALTYPE;

		case UNOP_BIN_NOT:
			if (e->unop.x->type.t != TYPE_INT) {
				// XXX error
			}
			e->type = e->unop.x->type;
			return VALTYPE;

		case UNOP_BOOL_NOT:
			if (e->unop.x->type.t != TYPE_BOOL) {
				// XXX error
			}
			e->type.t = TYPE_BOOL;
			return VALTYPE;

		case UNOP_SIZEOF:
			e->type.t = TYPE_INT;
			e->type.int_ = U_64;
			return VALTYPE;
		}
	// }}}

	// EXPR_CALL {{{
	case EXPR_CALL:
		annotate_type(e->call.func);
		if (e->call.func->type.t != TYPE_FUNC) {
			// XXX error
		}

		for (size_t i = 0; i < e->call.func->type.func.nargs; ++i) {
			annotate_type(e->call.args + i);
			if (!vtype_eq(&e->call.func->type.func.args[i].to, &e->call.args[i].type)) {
				// XXX error
			}
		}
		e->type = *e->call.func->type.func.ret_type;
		return VALTYPE;
	// }}}

	// EXPR_IF {{{
	case EXPR_IF:
		annotate_type(e->if_.cond);
		annotate_type(e->if_.t);

		if (e->if_.cond->type.t != TYPE_BOOL) {
			 // XXX error
		}

		if (e->if_.f) annotate_type(e->if_.f);

		if (vtype_eq(&e->if_.f->type, &e->if_.t->type)) {
			e->type = e->if_.t->type;
		} else {
			e->type.t = TYPE_VOID;
		}

		return VALTYPE;
	// }}}

	// EXPR_WHILE {{{
	case EXPR_WHILE:
		annotate_type(e->while_.cond);
		annotate_type(e->while_.body);
		if (e->if_.cond->type.t != TYPE_BOOL) {
			 // XXX error
		}
		e->type.t = TYPE_VOID;
		return VALTYPE;
	// }}}

	// EXPR_BREAK {{{
	case EXPR_BREAK:
		e->type.t = TYPE_VOID;
		return VALTYPE;
	// }}}

	// EXPR_CONTINUE {{{
	case EXPR_CONTINUE:
		e->type.t = TYPE_VOID;
		return VALTYPE;
	// }}}

	// EXPR_RETURN {{{
	case EXPR_RETURN:
		if (e->return_.val) annotate_type(e->return_.val);

		struct val_type ret_type = e->return_.val ? e->return_.val->type : (struct val_type){.t=TYPE_VOID};

		if (!vtype_eq(&cur_func.ret, &ret_type)) {
			// XXX error
		}

		e->type.t = TYPE_VOID;
		return VALTYPE;
	// }}}

	// EXPR_FUNC {{{
	case EXPR_FUNC:
		if (_func_stack.nfuncs == _func_stack.alloc) {
			_func_stack.alloc *= 2;
			_func_stack.funcs = realloc(_func_stack.funcs, _func_stack.alloc * sizeof _func_stack.funcs[0]);
		}
		_func_stack.funcs[_func_stack.nfuncs].nargs = e->func.nargs;
		_func_stack.funcs[_func_stack.nfuncs].args = (void *)e->func.args; // FUCK C
		_func_stack.funcs[_func_stack.nfuncs].ret = e->func.ret;

		++_func_stack.nfuncs;
		annotate_type(e->func.body);
		--_func_stack.nfuncs;

		e->type.t = TYPE_FUNC;
		e->type.func.nargs = e->func.nargs;
		e->type.func.args = malloc(e->func.nargs * sizeof e->type.func.args[0]);
		for (size_t i = 0; i < e->func.nargs; ++i) {
			e->type.func.args[i] = e->func.args[i].type;
		}
		e->type.func.ret_type = &e->func.ret;
		return REFTYPE;
	// }}}

	// EXPR_INT_LIT {{{
	case EXPR_INT_LIT:
		e->type.t = TYPE_INT;
		e->type.int_ = e->int_lit.type;
		return VALTYPE;
	// }}}

	// EXPR_FLOAT_LIT {{{
	case EXPR_FLOAT_LIT:
		e->type.t = TYPE_FLOAT;
		e->type.float_ = e->float_lit.type;
		return VALTYPE;
	// }}}

	// EXPR_ARR_LIT {{{
	case EXPR_ARR_LIT:
		// TODO
		return VALTYPE;
	// }}}

	// EXPR_COMPOSITE_LIT {{{
	case EXPR_COMPOSITE_LIT:
		// TODO
		return VALTYPE;
	// }}}

	case EXPR_BOOL_LIT:
		e->type.t = TYPE_BOOL;
		return VALTYPE;

	// EXPR_FIELD_ACCESS {{{
	case EXPR_FIELD_ACCESS:;
		uint8_t aggr_tflags = annotate_type(e->field_access.aggr);
		struct val_type aggr_type = e->field_access.aggr->type; // FIXME: newtypes
		if (aggr_type.t != TYPE_STRUCT
				&& aggr_type.t != TYPE_UNION) {
			// XXX error
		}

		for (size_t i = 0; i < aggr_type.composite.nfields; ++i) {
			if (!strcmp(aggr_type.composite.fields[i].name, e->field_access.field)) {
				e->type = *aggr_type.composite.fields[i].type;
				return aggr_tflags;
			}
		}

		// XXX error
	// }}}

	// EXPR_LET {{{
	case EXPR_LET:
		annotate_type(e->let.val);
		uint8_t body_tflags = annotate_type(e->let.body);
		if (e->let.deferred) annotate_type(e->let.deferred);

		if (!vtype_eq(&e->let.val->type, &e->let.type.to)) {
			// XXX error
		}

		e->type = e->let.body->type;
		return body_tflags;
	// }}}

	// EXPR_CAST {{{
	case EXPR_CAST:
		annotate_type(e->cast.val);
		struct val_type to = e->cast.type, from = e->cast.val->type;
		if (_cast_valid(e->cast.val->type, e->cast.type)) {
			e->type = e->cast.type;
		} else {
			// XXX error
		}
		return VALTYPE;
	// }}}
	
	// EXPR_IDENT {{{
	case EXPR_IDENT:
		for (size_t i = 0; i < cur_func.nargs; ++i) {
			if (!strcmp(cur_func.args[i].name, e->ident)) {
				struct ref_type type = cur_func.args[i].type;
				e->type = type.to;
				uint8_t ret = REFTYPE;
				if (type.mut) ret |= REF_MUT;
				if (type.vol) ret |= REF_VOL;
				return ret;
			}
		}
		for (size_t i = 0; i < cur_func.nscopes; ++i) {
			if (!strcmp(cur_func.scopes[i].name, e->ident)) {
				struct ref_type type = cur_func.scopes[i].type;
				e->type = type.to;
				uint8_t ret = REFTYPE;
				if (type.mut) ret |= REF_MUT;
				if (type.vol) ret |= REF_VOL;
				return ret;
			}
		}
		// XXX error
	// }}}
	}
}
