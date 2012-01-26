#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "eval.h"
#include "display.h"
#include "proc.h"

#define is_application(exp)			is_pair(exp)
#define is_definition(exp)			is_tagged_list(exp, "define")
#define is_lambda(exp)				is_tagged_list(exp, "lambda")
#define is_self_evaluating(exp)		(is_number(exp) || is_string(exp))
#define is_quoted(exp)				is_tagged_list(exp, "quote")
#define is_variable(exp)			is_symbol(exp)

#define lambda_parameters(exp)		cadr(exp)
#define lambda_body(exp)			cddr(exp)
#define make_lambda(p, b)			cons(symbol("lambda"), cons(p, b))

#define operator(exp)				car(exp)
#define operands(exp)				cdr(exp)
#define is_no_operands(ops)			is_null(ops)
#define first_operand(ops)			car(ops)
#define rest_operands(ops)			cdr(ops)

#define make_procedure(p, b, e) \
	cons(symbol("procedure"), cons(p, cons(b, cons(e, nil))))

#define is_last_exp(seq)			is_null(cdr(seq))
#define first_exp(seq)				car(seq)
#define rest_exps(seq)				cdr(seq)

#define is_if(exp)					is_tagged_list(exp, "if")
#define if_predicate(exp)			cadr(exp)
#define if_consequent(exp)			caddr(exp)

object *apply(object *procedure, object *arguments);
object *apply_primitive_procedure(object *proc, object *args);
object *definition_value(object *exp);
object *definition_variable(object *exp);
object *eval_definition(object *exp, object *env);
object *eval_if(object *exp, object *env);
object *eval_sequence(object *exps, object *env);
object *extend_environment(object *vars, object *vals, object *base_env);
object *if_alternative(object *exp);
object *list_of_values(object *exps, object *env);
object *lookup_variable_value(object *exp, object *env);

object *apply(object *proc, object *args) {
	if (is_primitive_procedure(proc))
		return apply_primitive_procedure(proc, args);
	else if (is_compound_procedure(proc))
		return eval_sequence(procedure_body(proc),
							 extend_environment(procedure_parameters(proc),
												args,
												procedure_environment(proc)));
	return error("Unknown procedure type -- APPLY", proc);
}

object *apply_primitive_procedure(object *proc, object *args) {
	return proc->data.proc(args);
}

void define_variable(object *var, object *val, object *env) {
	for (; env; env = cdr(env)) {
		if (var == caar(env)) {
			set_cdr(car(env), val);
			return;
		}
		if (!cdr(env))
			break;
	}
	set_cdr(env, cons(cons(var, val), nil));
}

object *definition_variable(object *exp) {
	if (is_symbol(cadr(exp)))
		return cadr(exp);
	return caadr(exp);
}

object *definition_value(object *exp) {
	if (is_symbol(cadr(exp)))
		return caddr(exp);
	return make_lambda(cdadr(exp), cddr(exp));
}

object *eval(object *exp, object *env) {
	if (is_self_evaluating(exp))
		return exp;
	else if (is_variable(exp))
		return lookup_variable_value(exp, env);
	else if (is_quoted(exp))
		return cadr(exp);
	else if (is_definition(exp))
		return eval_definition(exp, env);
	else if (is_if(exp))
		return eval_if(exp, env);
	else if (is_lambda(exp))
		return make_procedure(lambda_parameters(exp), lambda_body(exp), env);
	else if (is_application(exp))
		return apply(eval(operator(exp), env),
					 list_of_values(operands(exp), env));
	else
		return error("Unknown expression type -- EVAL", exp);
}

object *eval_definition(object *exp, object *env) {
	define_variable(definition_variable(exp),
					eval(definition_value(exp), env),
					env);
	return definition_variable(exp);
}

object *eval_if(object *exp, object *env) {
	if (is_true(eval(if_predicate(exp), env)))
		return eval(if_consequent(exp), env);
	else
		return eval(if_alternative(exp), env);
}

object *eval_sequence(object *exps, object *env) {
	if (is_last_exp(exps))
		return eval(first_exp(exps), env);
	eval(first_exp(exps), env);
	return eval_sequence(rest_exps(exps), env);
}

object *extend_environment(object *vars, object *vals, object *base_env) {
	while (vars && vals) {
		base_env = cons(cons(car(vars), car(vals)), base_env);
		vars = cdr(vars);
		vals = cdr(vals);
	}
	if (vars)
		return error("Too few arguments supplied", vars);
	if (vals)
		return error("Too many arguments supplied", vars);
	return base_env;
}

object *if_alternative(object *exp) {
	if (cdddr(exp))
		return cadddr(exp);
	else
		return false;
}

object *list_of_values(object *exps, object *env) {
	if (is_no_operands(exps))
		return nil;
	return cons(eval(first_operand(exps), env),
				list_of_values(rest_operands(exps), env));
}

object *lookup_variable_value(object *var, object *env) {
	while (env) {
		if (var == caar(env))
			return cdar(env);
		env = cdr(env);
	}
	return error("Unbound variable", var);
}

object *setup_environment(void) {
	object *env;

	env = primitive_procedures();
	env = cons(cons(true, true), env);
	env = cons(cons(false, false), env);
	return env;
}
