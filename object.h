#ifndef OBJECT_H
#define OBJECT_H

#include <sys/types.h>
#include "types.h"

/* Type Predicates */
bool iscons(ref_t obj);
bool isfixnum(ref_t obj);
bool isfunction(ref_t obj);
bool isinteger(ref_t obj);
bool islist(ref_t obj);
bool ismacro(ref_t obj);
bool isnil(ref_t obj);
bool isspecialform(ref_t obj);
bool isstring(ref_t obj);
bool issymbol(ref_t obj);
bool istrue(ref_t obj);

/* Type Checks */
ref_t check_function(ref_t obj);
ref_t check_integer(ref_t obj);
ref_t check_list(ref_t obj);
ref_t check_symbol(ref_t obj);

/* Constructors */
ref_t integer(int i);

/* Functions */
ref_t getbody(ref_t obj);
ref_t getclosure(ref_t obj);
fn_t getfn(ref_t obj);
ref_t getformals(ref_t obj);
size_t getarity(ref_t obj);
bool hasrest(ref_t obj);
bool isbuiltin(ref_t obj);
ref_t set_type_macro(ref_t obj);
ref_t set_type_special_form(ref_t obj);

/* Integers */
int intvalue(ref_t obj);
ref_t integer_add(ref_t x, ref_t y);
ref_t integer_sub(ref_t x, ref_t y);
ref_t integer_mul(ref_t x, ref_t y);
ref_t integer_div(ref_t x, ref_t y);

/* Lists */
ref_t car(ref_t list);
ref_t cadr(ref_t list);
ref_t caddr(ref_t list);
ref_t cdr(ref_t list);
ref_t cddr(ref_t list);
void set_car(ref_t cons, ref_t value);
void set_cdr(ref_t cons, ref_t value);

/* Symbols */
bool has_function(ref_t sym);
ref_t get_function(ref_t sym);
void set_function(ref_t sym, ref_t func);

bool has_value(ref_t sym);
ref_t get_value(ref_t sym);
void set_value(ref_t sym, ref_t func);

/* Misc */
int length(ref_t obj);
const char *strvalue(ref_t obj);

#endif
