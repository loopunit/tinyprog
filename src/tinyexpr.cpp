/*
 * TINYEXPR - Tiny recursive descent parser and evaluation engine in C
 *
 * Copyright (c) 2015-2018 Lewis Van Winkle
 *
 * http://CodePlea.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgement in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/* COMPILE TIME OPTIONS */

/* Exponentiation associativity:
For a^b^c = (a^b)^c and -a^b = (-a)^b do nothing.
For a^b^c = a^(b^c) and -a^b = -(a^b) uncomment the next line.*/
/* #define TE_POW_FROM_RIGHT */

/* Logarithms
For log = base 10 log do nothing
For log = natural log uncomment the next line. */
/* #define TE_NAT_LOG */

#include "tinyexpr.h"

#include <unordered_map>
#include <vector>
#include <memory>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>

#if (TE_COMPILER_ENABLED)
template <typename T_VECTOR>
struct te_native_builtins;

template <>
struct te_native_builtins<double>
{
	static double te_pi(void)
	{
		return 3.14159265358979323846;
	}

	static double te_e(void)
	{
		return 2.71828182845904523536;
	}

	static double te_fac(double a)
	{ /* simplest version of fac */
		if (a < 0.0)
			return te_traits::nan();
		if (a > UINT_MAX)
			return INFINITY;
		unsigned int	  ua	 = (unsigned int)(a);
		unsigned long int result = 1, i;
		for (i = 1; i <= ua; i++)
		{
			if (i > ULONG_MAX / result)
				return INFINITY;
			result *= i;
		}
		return (double)result;
	}

	static double te_ncr(double n, double r)
	{
		if (n < 0.0 || r < 0.0 || n < r)
			return te_traits::nan();
		if (n > UINT_MAX || r > UINT_MAX)
			return INFINITY;
		unsigned long int un = (unsigned int)(n), ur = (unsigned int)(r), i;
		unsigned long int result = 1;
		if (ur > un / 2)
			ur = un - ur;
		for (i = 1; i <= ur; i++)
		{
			if (result > ULONG_MAX / (un - ur + i))
				return INFINITY;
			result *= un - ur + i;
			result /= i;
		}
		return result;
	}

	static double te_npr(double n, double r)
	{
		return te_ncr(n, r) * te_fac(r);
	}

	static double te_fabs(double n)
	{
		return ::fabs(n);
	}

	static double te_acos(double n)
	{
		return ::acos(n);
	}

	static double te_cosh(double n)
	{
		return ::cosh(n);
	}

	static double te_cos(double n)
	{
		return ::cos(n);
	}

	static double te_exp(double n)
	{
		return ::exp(n);
	}

	static double te_asin(double n)
	{
		return ::asin(n);
	}

	static double te_sinh(double n)
	{
		return ::sinh(n);
	}

	static double te_sin(double n)
	{
		return ::sin(n);
	}

	static double te_sqrt(double n)
	{
		return ::sqrt(n);
	}

	static double te_log(double n)
	{
		return ::log(n);
	}

	static double te_log10(double n)
	{
		return ::log10(n);
	}

	static double te_atan(double n)
	{
		return ::atan(n);
	}

	static double te_tanh(double n)
	{
		return ::tanh(n);
	}

	static double te_fmod(double n, double m)
	{
		return ::fmod(n, m);
	}

	static double te_tan(double n)
	{
		return ::tan(n);
	}

	static double te_atan2(double n, double m)
	{
		return ::atan2(n, m);
	}

	static double te_pow(double n, double m)
	{
		return ::pow(n, m);
	}

	static double te_floor(double d)
	{
		return ::floor(d);
	}

	static double te_ceil(double d)
	{
		return ::ceil(d);
	}

	static double te_add(double a, double b)
	{
		return a + b;
	}

	static double te_sub(double a, double b)
	{
		return a - b;
	}

	static double te_mul(double a, double b)
	{
		return a * b;
	}

	static double te_divide(double a, double b)
	{
		return a / b;
	}

	static double te_negate(double a)
	{
		return -a;
	}

	static double te_comma(double a, double b)
	{
		(void)a;
		return b;
	}

	static double te_greater(double a, double b)
	{
		return a > b;
	}

	static double te_greater_eq(double a, double b)
	{
		return a >= b;
	}

	static double te_lower(double a, double b)
	{
		return a < b;
	}

	static double te_lower_eq(double a, double b)
	{
		return a <= b;
	}

	static double te_equal(double a, double b)
	{
		return a == b;
	}

	static double te_not_equal(double a, double b)
	{
		return a != b;
	}

	static double te_logical_and(double a, double b)
	{
		return a != 0.0 && b != 0.0;
	}

	static double te_logical_or(double a, double b)
	{
		return a != 0.0 || b != 0.0;
	}

	static double te_logical_not(double a)
	{
		return a == 0.0;
	}

	static double te_logical_notnot(double a)
	{
		return a != 0.0;
	}

	static double te_negate_logical_not(double a)
	{
		return -(a == 0.0);
	}

	static double te_negate_logical_notnot(double a)
	{
		return -(a != 0.0);
	}

	static double te_nul()
	{
		return 0.0f;
	}

	static constexpr inline te_variable functions[] = {
		/* must be in alphabetical order */
		{"abs", te_fabs, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"acos", te_acos, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"asin", te_asin, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"atan", te_atan, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"atan2", te_atan2, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"ceil", te_ceil, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"cos", te_cos, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"cosh", te_cosh, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"e", te_e, TE_FUNCTION0 | TE_FLAG_PURE, 0},
		{"exp", te_exp, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"fac", te_fac, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"floor", te_floor, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"ln", te_log, TE_FUNCTION1 | TE_FLAG_PURE, 0},
#	ifdef TE_NAT_LOG
		{"log", te_log, TE_FUNCTION1 | TE_FLAG_PURE, 0},
#	else
		{"log", te_log10, TE_FUNCTION1 | TE_FLAG_PURE, 0},
#	endif
		{"log10", te_log10, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"ncr", te_ncr, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"npr", te_npr, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"pi", te_pi, TE_FUNCTION0 | TE_FLAG_PURE, 0},
		{"pow", te_pow, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"sin", te_sin, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"sinh", te_sinh, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"sqrt", te_sqrt, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"tan", te_tan, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"tanh", te_tanh, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{0, 0, 0, 0}};

	static constexpr inline te_variable operators[] = {
		/* must be in alphabetical order */
		{"add", te_add, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"comma", te_comma, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"divide", te_divide, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"equal", te_equal, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"fmod", te_fmod, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"greater", te_greater, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"greater_eq", te_greater_eq, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"logical_and", te_logical_and, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"logical_not", te_logical_not, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"logical_notnot", te_logical_notnot, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"logical_or", te_logical_or, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"lower", te_lower, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"lower_eq", te_lower_eq, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"mul", te_mul, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"negate", te_negate, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"negate_logical_not", te_negate_logical_not, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"negate_logical_notnot", te_negate_logical_notnot, TE_FUNCTION1 | TE_FLAG_PURE, 0},
		{"not_equal", te_not_equal, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"pow", te_pow, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{"sub", te_sub, TE_FUNCTION2 | TE_FLAG_PURE, 0},
		{0, 0, 0, 0}};

	static const te_variable* find_builtin_function(const char* name, int len)
	{
		int imin = 0;
		int imax = sizeof(functions) / sizeof(te_variable) - 2;

		/*Binary search.*/
		while (imax >= imin)
		{
			const int i = (imin + ((imax - imin) / 2));
			int		  c = strncmp(name, functions[i].name, len);
			if (!c)
				c = '\0' - functions[i].name[len];
			if (c == 0)
			{
				return functions + i;
			}
			else if (c > 0)
			{
				imin = i + 1;
			}
			else
			{
				imax = i - 1;
			}
		}
		return nullptr;
	}

	static const te_variable* find_builtin_operator(const char* name, int len)
	{
		int imin = 0;
		int imax = sizeof(operators) / sizeof(te_variable) - 2;

		/*Binary search.*/
		while (imax >= imin)
		{
			const int i = (imin + ((imax - imin) / 2));
			int		  c = strncmp(name, operators[i].name, len);
			if (!c)
				c = '\0' - operators[i].name[len];

			if (c == 0)
			{
				return operators + i;
			}
			else if (c > 0)
			{
				imin = i + 1;
			}
			else
			{
				imax = i - 1;
			}
		}
		return nullptr;
	}

	static const te_variable* find_builtin(const char* name, int len)
	{
		auto res = find_builtin_function(name, len);
		if (!res)
		{
			res = find_builtin_operator(name, len);
		}
		return res;
	}

	static const te_variable* find_builtin(const char* name)
	{
		return find_builtin(name, static_cast<int>(::strlen(name)));
	}

	static const te_variable* find_function_by_addr(const void* addr)
	{
		for (auto var = &functions[0]; var->name != 0; ++var)
		{
			if (var->address == addr)
			{
				return var;
			}
		}
		return nullptr;
	}

	static const te_variable* find_operator_by_addr(const void* addr)
	{
		for (auto var = &operators[0]; var->name != 0; ++var)
		{
			if (var->address == addr)
			{
				return var;
			}
		}
		return nullptr;
	}

	static const te_variable* find_any_by_addr(const void* addr)
	{
		const te_variable* var = find_function_by_addr(addr);
		if (!var)
		{
			var = find_operator_by_addr(addr);
			if (!var)
			{
				return find_builtin("nul");
			}
		}
		return var;
	}
};

struct te_native
{
	struct te_expr_native
	{
		int type;
		union
		{
			double		  value;
			const double* bound;
			const void*	  function;
		};
		void* parameters[1];
	};

#ifndef INFINITY
#	define INFINITY (1.0 / 0.0)
#endif

	typedef double (*te_fun2)(double, double);

	enum
	{
		TOK_NULL = TE_CLOSURE7 + 1,
		TOK_ERROR,
		TOK_END,
		TOK_SEP,
		TOK_OPEN,
		TOK_CLOSE,
		TOK_NUMBER,
		TOK_VARIABLE,
		TOK_INFIX
	};

	struct state
	{
		const char* start;
		const char* next;
		int			type;
		union
		{
			double		  value;
			const double* bound;
			const void*	  function;
		};
		void* context;

		const te_variable* lookup;
		int				   lookup_len;
	};

#define IS_PURE(TYPE)	  (((TYPE)&TE_FLAG_PURE) != 0)
#define IS_FUNCTION(TYPE) (((TYPE)&TE_FUNCTION0) != 0)
#define IS_CLOSURE(TYPE)  (((TYPE)&TE_CLOSURE0) != 0)
#define NEW_EXPR(type, ...)                                                                                            \
	[&]() {                                                                                                            \
		const te_expr_native* _args[] = {__VA_ARGS__};                                                                 \
		return new_expr((type), _args);                                                                                \
	}()

	static te_expr_native* new_expr(const int type, const te_expr_native* parameters[])
	{
		const int arity		= te_arity(type);
		const int psize		= sizeof(void*) * arity;
		const int size		= (sizeof(te_expr_native) - sizeof(void*)) + psize + (IS_CLOSURE(type) ? sizeof(void*) : 0);
		te_expr_native* ret = (te_expr_native*)malloc(size);
		memset(ret, 0, size);
		if (arity && parameters)
		{
			memcpy(ret->parameters, parameters, psize);
		}
		ret->type  = type;
		ret->bound = 0;
		return ret;
	}

	static void te_free_parameters(te_expr_native* n)
	{
		if (!n)
			return;
		switch (te_type_mask(n->type))
		{
		case TE_FUNCTION7:
		case TE_CLOSURE7:
			te_free_native((te_expr_native*)n->parameters[6]); /* Falls through. */
		case TE_FUNCTION6:
		case TE_CLOSURE6:
			te_free_native((te_expr_native*)n->parameters[5]); /* Falls through. */
		case TE_FUNCTION5:
		case TE_CLOSURE5:
			te_free_native((te_expr_native*)n->parameters[4]); /* Falls through. */
		case TE_FUNCTION4:
		case TE_CLOSURE4:
			te_free_native((te_expr_native*)n->parameters[3]); /* Falls through. */
		case TE_FUNCTION3:
		case TE_CLOSURE3:
			te_free_native((te_expr_native*)n->parameters[2]); /* Falls through. */
		case TE_FUNCTION2:
		case TE_CLOSURE2:
			te_free_native((te_expr_native*)n->parameters[1]); /* Falls through. */
		case TE_FUNCTION1:
		case TE_CLOSURE1:
			te_free_native((te_expr_native*)n->parameters[0]);
		}
	}

	static void te_free_native(te_expr_native* n)
	{
		if (!n)
			return;
		te_free_parameters(n);
		free(n);
	}

	static const te_variable* find_lookup(const te_variable* lookup, int lookup_len, const char* name, int len)
	{
		if (!lookup)
			return 0;

		const te_variable* var	 = lookup;
		int				   iters = lookup_len;
		for (; iters; ++var, --iters)
		{
			if (strncmp(name, var->name, len) == 0 && var->name[len] == '\0')
			{
				return var;
			}
		}
		return 0;
	}

	static const te_variable* find_lookup(const state* s, const char* name, int len)
	{
		return find_lookup(s->lookup, s->lookup_len, name, len);
	}

	static void next_token(state* s)
	{
		s->type = TOK_NULL;

		do
		{
			if (!*s->next)
			{
				s->type = TOK_END;
				return;
			}

			/* Try reading a number. */
			if ((s->next[0] >= '0' && s->next[0] <= '9') || s->next[0] == '.')
			{
				s->value = strtod(s->next, (char**)&s->next);
				s->type	 = TOK_NUMBER;
			}
			else
			{
				/* Look for a variable or builtin function call. */
				if (s->next[0] >= 'a' && s->next[0] <= 'z')
				{
					const char* start;
					start = s->next;
					while ((s->next[0] >= 'a' && s->next[0] <= 'z') || (s->next[0] >= '0' && s->next[0] <= '9') ||
						   (s->next[0] == '_'))
						s->next++;

					const te_variable* var = find_lookup(s, start, static_cast<int>(s->next - start));
					if (!var)
						var = te_native_builtins<te_traits::t_vector>::find_builtin(start, static_cast<int>(s->next - start));

					if (!var)
					{
						s->type = TOK_ERROR;
					}
					else
					{
						switch (te_type_mask(var->type))
						{
						case TE_VARIABLE:
							s->type	 = TOK_VARIABLE;
							s->bound = (const double*)var->address;
							break;

						case TE_CLOSURE0:
						case TE_CLOSURE1:
						case TE_CLOSURE2:
						case TE_CLOSURE3: /* Falls through. */
						case TE_CLOSURE4:
						case TE_CLOSURE5:
						case TE_CLOSURE6:
						case TE_CLOSURE7:			   /* Falls through. */
							s->context = var->context; /* Falls through. */

						case TE_FUNCTION0:
						case TE_FUNCTION1:
						case TE_FUNCTION2:
						case TE_FUNCTION3: /* Falls through. */
						case TE_FUNCTION4:
						case TE_FUNCTION5:
						case TE_FUNCTION6:
						case TE_FUNCTION7: /* Falls through. */
							s->type		= var->type;
							s->function = var->address;
							break;
						}
					}
				}
				else
				{
					/* Look for an operator or special character. */
					switch (s->next++[0])
					{
					case '+':
						s->type		= TOK_INFIX;
						s->function = te_native_builtins<te_traits::t_vector>::find_builtin("add")->address;
						break;
					case '-':
						s->type		= TOK_INFIX;
						s->function = te_native_builtins<te_traits::t_vector>::find_builtin("sub")->address;
						break;
					case '*':
						s->type		= TOK_INFIX;
						s->function = te_native_builtins<te_traits::t_vector>::find_builtin("mul")->address;
						break;
					case '/':
						s->type		= TOK_INFIX;
						s->function = te_native_builtins<te_traits::t_vector>::find_builtin("divide")->address;
						break;
					case '^':
						s->type		= TOK_INFIX;
						s->function = te_native_builtins<te_traits::t_vector>::find_builtin("pow")->address;
						break;
					case '%':
						s->type		= TOK_INFIX;
						s->function = te_native_builtins<te_traits::t_vector>::find_builtin("fmod")->address;
						break;
					case '!':
						if (s->next++[0] == '=')
						{
							s->type		= TOK_INFIX;
							s->function = te_native_builtins<te_traits::t_vector>::find_builtin("not_equal")->address;
						}
						else
						{
							s->next--;
							s->type		= TOK_INFIX;
							s->function = te_native_builtins<te_traits::t_vector>::find_builtin("logical_not")->address;
						}
						break;
					case '=':
						if (s->next++[0] == '=')
						{
							s->type		= TOK_INFIX;
							s->function = te_native_builtins<te_traits::t_vector>::find_builtin("equal")->address;
						}
						else
						{
							s->type = TOK_ERROR;
						}
						break;
					case '<':
						if (s->next++[0] == '=')
						{
							s->type		= TOK_INFIX;
							s->function = te_native_builtins<te_traits::t_vector>::find_builtin("lower_eq")->address;
						}
						else
						{
							s->next--;
							s->type		= TOK_INFIX;
							s->function = te_native_builtins<te_traits::t_vector>::find_builtin("lower")->address;
						}
						break;
					case '>':
						if (s->next++[0] == '=')
						{
							s->type		= TOK_INFIX;
							s->function = te_native_builtins<te_traits::t_vector>::find_builtin("greater_eq")->address;
						}
						else
						{
							s->next--;
							s->type		= TOK_INFIX;
							s->function = te_native_builtins<te_traits::t_vector>::find_builtin("greater")->address;
						}
						break;
					case '&':
						if (s->next++[0] == '&')
						{
							s->type		= TOK_INFIX;
							s->function = te_native_builtins<te_traits::t_vector>::find_builtin("logical_and")->address;
						}
						else
						{
							s->type = TOK_ERROR;
						}
						break;
					case '|':
						if (s->next++[0] == '|')
						{
							s->type		= TOK_INFIX;
							s->function = te_native_builtins<te_traits::t_vector>::find_builtin("logical_or")->address;
						}
						else
						{
							s->type = TOK_ERROR;
						}
						break;
					case '(':
						s->type = TOK_OPEN;
						break;
					case ')':
						s->type = TOK_CLOSE;
						break;
					case ',':
						s->type = TOK_SEP;
						break;
					case ' ':
					case '\t':
					case '\n':
					case '\r':
						break;
					default:
						s->type = TOK_ERROR;
						break;
					}
				}
			}
		} while (s->type == TOK_NULL);
	}

	//static te_expr_native* list(state* s);
	//static te_expr_native* expr(state* s);
	//static te_expr_native* power(state* s);

	static te_expr_native* base(state* s)
	{
		/* <base>      =    <constant> | <variable> | <function-0> {"(" ")"} | <function-1> <power> | <function-X> "("
		 * <expr> {"," <expr>} ")" | "(" <list> ")" */
		te_expr_native* ret;
		int				arity;

		switch (te_type_mask(s->type))
		{
		case TOK_NUMBER:
			ret		   = new_expr(TE_CONSTANT, 0);
			ret->value = s->value;
			next_token(s);
			break;

		case TOK_VARIABLE:
			ret		   = new_expr(TE_VARIABLE, 0);
			ret->bound = s->bound;
			next_token(s);
			break;

		case TE_FUNCTION0:
		case TE_CLOSURE0:
			ret			  = new_expr(s->type, 0);
			ret->function = s->function;
			if (IS_CLOSURE(s->type))
				ret->parameters[0] = s->context;
			next_token(s);
			if (s->type == TOK_OPEN)
			{
				next_token(s);
				if (s->type != TOK_CLOSE)
				{
					s->type = TOK_ERROR;
				}
				else
				{
					next_token(s);
				}
			}
			break;

		case TE_FUNCTION1:
		case TE_CLOSURE1:
			ret			  = new_expr(s->type, 0);
			ret->function = s->function;
			if (IS_CLOSURE(s->type))
				ret->parameters[1] = s->context;
			next_token(s);
			ret->parameters[0] = power(s);
			break;

		case TE_FUNCTION2:
		case TE_FUNCTION3:
		case TE_FUNCTION4:
		case TE_FUNCTION5:
		case TE_FUNCTION6:
		case TE_FUNCTION7:
		case TE_CLOSURE2:
		case TE_CLOSURE3:
		case TE_CLOSURE4:
		case TE_CLOSURE5:
		case TE_CLOSURE6:
		case TE_CLOSURE7:
			arity = te_arity(s->type);

			ret			  = new_expr(s->type, 0);
			ret->function = s->function;
			if (IS_CLOSURE(s->type))
				ret->parameters[arity] = s->context;
			next_token(s);

			if (s->type != TOK_OPEN)
			{
				s->type = TOK_ERROR;
			}
			else
			{
				int i;
				for (i = 0; i < arity; i++)
				{
					next_token(s);
					ret->parameters[i] = expr(s);
					if (s->type != TOK_SEP)
					{
						break;
					}
				}
				if (s->type != TOK_CLOSE || i != arity - 1)
				{
					s->type = TOK_ERROR;
				}
				else
				{
					next_token(s);
				}
			}

			break;

		case TOK_OPEN:
			next_token(s);
			ret = list(s);
			if (s->type != TOK_CLOSE)
			{
				s->type = TOK_ERROR;
			}
			else
			{
				next_token(s);
			}
			break;

		default:
			ret		   = new_expr(0, 0);
			s->type	   = TOK_ERROR;
			ret->value = te_traits::nan();
			break;
		}

		return ret;
	}

	static te_expr_native* power(state* s)
	{
		/* <power>     =    {("-" | "+" | "!")} <base> */
		int sign = 1;
		while (s->type == TOK_INFIX &&
			   (s->function == te_native_builtins<te_traits::t_vector>::find_builtin("add")->address ||
				   s->function == te_native_builtins<te_traits::t_vector>::find_builtin("sub")->address))
		{
			if (s->function == te_native_builtins<te_traits::t_vector>::find_builtin("sub")->address)
				sign = -sign;
			next_token(s);
		}

		int logical = 0;
		while (s->type == TOK_INFIX &&
			   (s->function == te_native_builtins<te_traits::t_vector>::find_builtin("add")->address ||
				   s->function == te_native_builtins<te_traits::t_vector>::find_builtin("sub")->address ||
				   s->function == te_native_builtins<te_traits::t_vector>::find_builtin("logical_not")->address))
		{
			if (s->function == te_native_builtins<te_traits::t_vector>::find_builtin("logical_not")->address)
			{
				if (logical == 0)
				{
					logical = -1;
				}
				else
				{
					logical = -logical;
				}
			}
			next_token(s);
		}

		te_expr_native* ret;

		if (sign == 1)
		{
			if (logical == 0)
			{
				ret = base(s);
			}
			else if (logical == -1)
			{
				ret			  = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, base(s));
				ret->function = te_native_builtins<te_traits::t_vector>::find_builtin("logical_not")->address;
			}
			else
			{
				ret			  = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, base(s));
				ret->function = te_native_builtins<te_traits::t_vector>::find_builtin("logical_notnot")->address;
			}
		}
		else
		{
			if (logical == 0)
			{
				ret			  = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, base(s));
				ret->function = te_native_builtins<te_traits::t_vector>::find_builtin("negate")->address;
			}
			else if (logical == -1)
			{
				ret			  = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, base(s));
				ret->function = te_native_builtins<te_traits::t_vector>::find_builtin("negate_logical_not")->address;
			}
			else
			{
				ret			  = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, base(s));
				ret->function = te_native_builtins<te_traits::t_vector>::find_builtin("negate_logical_notnot")->address;
			}
		}

		return ret;
	}

#ifdef TE_POW_FROM_RIGHT
	static te_expr_native* factor(state* s)
	{
		/* <factor>    =    <power> {"^" <power>} */
		te_expr_native* ret = power(s);

		const void*		left_function = NULL;
		te_expr_native* insertion	  = 0;

		if (ret->type == (TE_FUNCTION1 | TE_FLAG_PURE) &&
			(ret->function == te_native_builtins<te_traits::t_vector>::find_builtin("negate")->address ||
				ret->function == te_native_builtins<te_traits::t_vector>::find_builtin("logical_not")->address ||
				ret->function == te_native_builtins<te_traits::t_vector>::find_builtin("logical_notnot")->address ||
				ret->function == te_native_builtins<te_traits::t_vector>::find_builtin("negate_logical_not")->address ||
				ret->function ==
					te_native_builtins<te_traits::t_vector>::find_builtin("negate_logical_notnot")->address))
		{
			left_function	   = ret->function;
			te_expr_native* se = ret->parameters[0];
			free(ret);
			ret = se;
		}

		while (s->type == TOK_INFIX &&
			   (s->function == te_native_builtins<te_traits::t_vector>::find_builtin("pow")->address))
		{
			te_fun2 t = s->function;
			next_token(s);

			if (insertion)
			{
				/* Make exponentiation go right-to-left. */
				te_expr_native* insert	 = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, insertion->parameters[1], power(s));
				insert->function		 = t;
				insertion->parameters[1] = insert;
				insertion				 = insert;
			}
			else
			{
				ret			  = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, power(s));
				ret->function = t;
				insertion	  = ret;
			}
		}

		if (left_function)
		{
			ret			  = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, ret);
			ret->function = left_function;
		}

		return ret;
	}
#else
	static te_expr_native* factor(state* s)
	{
		/* <factor>    =    <power> {"^" <power>} */
		te_expr_native* ret = power(s);

		while (s->type == TOK_INFIX &&
			   (s->function == te_native_builtins<te_traits::t_vector>::find_builtin("pow")->address))
		{
			te_fun2 t = (te_fun2)s->function;
			next_token(s);
			ret = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, power(s));
			ret->function = t;
		}

		return ret;
	}
#endif

	static te_expr_native* term(state* s)
	{
		/* <term>      =    <factor> {("*" | "/" | "%") <factor>} */
		te_expr_native* ret = factor(s);

		while (s->type == TOK_INFIX &&
			   (s->function == te_native_builtins<te_traits::t_vector>::find_builtin("mul")->address ||
				   s->function == te_native_builtins<te_traits::t_vector>::find_builtin("divide")->address ||
				   s->function == te_native_builtins<te_traits::t_vector>::find_builtin("fmod")->address))
		{
			te_fun2 t = (te_fun2)s->function;
			next_token(s);
			ret			  = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, factor(s));
			ret->function = t;
		}

		return ret;
	}

	static te_expr_native* sum_expr(state* s)
	{
		/* <expr>      =    <term> {("+" | "-") <term>} */
		te_expr_native* ret = term(s);

		while (s->type == TOK_INFIX &&
			   (s->function == te_native_builtins<te_traits::t_vector>::find_builtin("add")->address ||
				   s->function == te_native_builtins<te_traits::t_vector>::find_builtin("sub")->address))
		{
			te_fun2 t = (te_fun2)s->function;
			next_token(s);
			ret			  = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, term(s));
			ret->function = t;
		}

		return ret;
	}

	static te_expr_native* test_expr(state* s)
	{
		/* <expr>      =    <sum_expr> {(">" | ">=" | "<" | "<=" | "==" | "!=") <sum_expr>} */
		te_expr_native* ret = sum_expr(s);

		while (s->type == TOK_INFIX &&
			   (s->function == te_native_builtins<te_traits::t_vector>::find_builtin("greater")->address ||
				   s->function == te_native_builtins<te_traits::t_vector>::find_builtin("greater_eq")->address ||
				   s->function == te_native_builtins<te_traits::t_vector>::find_builtin("lower")->address ||
				   s->function == te_native_builtins<te_traits::t_vector>::find_builtin("lower_eq")->address ||
				   s->function == te_native_builtins<te_traits::t_vector>::find_builtin("equal")->address ||
				   s->function == te_native_builtins<te_traits::t_vector>::find_builtin("not_equal")->address))
		{
			te_fun2 t = (te_fun2)s->function;
			next_token(s);
			ret			  = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, sum_expr(s));
			ret->function = t;
		}

		return ret;
	}

	static te_expr_native* expr(state* s)
	{
		/* <expr>      =    <test_expr> {("&&" | "||") <test_expr>} */
		te_expr_native* ret = test_expr(s);

		while (s->type == TOK_INFIX &&
			   (s->function == te_native_builtins<te_traits::t_vector>::find_builtin("logical_and")->address ||
				   s->function == te_native_builtins<te_traits::t_vector>::find_builtin("logical_or")->address))
		{
			te_fun2 t = (te_fun2)s->function;
			next_token(s);
			ret			  = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, test_expr(s));
			ret->function = t;
		}

		return ret;
	}

	static te_expr_native* list(state* s)
	{
		/* <list>      =    <expr> {"," <expr>} */
		te_expr_native* ret = expr(s);

		while (s->type == TOK_SEP)
		{
			next_token(s);
			ret			  = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, expr(s));
			ret->function = te_native_builtins<te_traits::t_vector>::find_builtin("comma")->address;
		}

		return ret;
	}

#define TE_FUN(...) ((double (*)(__VA_ARGS__))n->function)
#define M(e)		te_eval_native((const te_expr_native*)n->parameters[e])

	static double te_eval_native(const te_expr_native* n)
	{
		if (!n)
			return te_traits::nan();

		switch (te_type_mask(n->type))
		{
		case TE_CONSTANT:
			return n->value;
		case TE_VARIABLE:
			return *n->bound;

		case TE_FUNCTION0:
		case TE_FUNCTION1:
		case TE_FUNCTION2:
		case TE_FUNCTION3:
		case TE_FUNCTION4:
		case TE_FUNCTION5:
		case TE_FUNCTION6:
		case TE_FUNCTION7:
			switch (te_arity(n->type))
			{
			case 0:
				return TE_FUN(void)();
			case 1:
				return TE_FUN(double)(M(0));
			case 2:
				return TE_FUN(double, double)(M(0), M(1));
			case 3:
				return TE_FUN(double, double, double)(M(0), M(1), M(2));
			case 4:
				return TE_FUN(double, double, double, double)(M(0), M(1), M(2), M(3));
			case 5:
				return TE_FUN(double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4));
			case 6:
				return TE_FUN(double, double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4), M(5));
			case 7:
				return TE_FUN(double, double, double, double, double, double, double)(
					M(0), M(1), M(2), M(3), M(4), M(5), M(6));
			default:
				return te_traits::nan();
			}

		case TE_CLOSURE0:
		case TE_CLOSURE1:
		case TE_CLOSURE2:
		case TE_CLOSURE3:
		case TE_CLOSURE4:
		case TE_CLOSURE5:
		case TE_CLOSURE6:
		case TE_CLOSURE7:
			switch (te_arity(n->type))
			{
			case 0:
				return TE_FUN(void*)(n->parameters[0]);
			case 1:
				return TE_FUN(void*, double)(n->parameters[1], M(0));
			case 2:
				return TE_FUN(void*, double, double)(n->parameters[2], M(0), M(1));
			case 3:
				return TE_FUN(void*, double, double, double)(n->parameters[3], M(0), M(1), M(2));
			case 4:
				return TE_FUN(void*, double, double, double, double)(n->parameters[4], M(0), M(1), M(2), M(3));
			case 5:
				return TE_FUN(void*, double, double, double, double, double)(
					n->parameters[5], M(0), M(1), M(2), M(3), M(4));
			case 6:
				return TE_FUN(void*, double, double, double, double, double, double)(
					n->parameters[6], M(0), M(1), M(2), M(3), M(4), M(5));
			case 7:
				return TE_FUN(void*, double, double, double, double, double, double, double)(
					n->parameters[7], M(0), M(1), M(2), M(3), M(4), M(5), M(6));
			default:
				return te_traits::nan();
			}

		default:
			return te_traits::nan();
		}
	}

#undef TE_FUN
#undef M

	static void optimize(te_expr_native* n)
	{
		/* Evaluates as much as possible. */
		if (n->type == TE_CONSTANT)
			return;
		if (n->type == TE_VARIABLE)
			return;

		/* Only optimize out functions flagged as pure. */
		if (IS_PURE(n->type))
		{
			const int arity = te_arity(n->type);
			int		  known = 1;
			int		  i;
			for (i = 0; i < arity; ++i)
			{
				optimize((te_expr_native*)n->parameters[i]);
				if (((te_expr_native*)(n->parameters[i]))->type != TE_CONSTANT)
				{
					known = 0;
				}
			}
			if (known)
			{
				const double value = te_eval_native(n);
				te_free_parameters(n);
				n->type	 = TE_CONSTANT;
				n->value = value;
			}
		}
	}

	static te_expr_native* te_compile_native(const char* expression, const te_variable* variables, int var_count, int* error)
	{
		state s;
		s.start = s.next = expression;
		s.lookup		 = variables;
		s.lookup_len	 = var_count;

		next_token(&s);
		te_expr_native* root = list(&s);

		if (s.type != TOK_END)
		{
			te_free_native(root);
			if (error)
			{
				*error = static_cast<int>(s.next - s.start);
				if (*error == 0)
					*error = 1;
			}
			return 0;
		}
		else
		{
			optimize(root);
			if (error)
				*error = 0;

			return root;
		}
	}

	static double te_interp_native(const char* expression, int* error)
	{
		te_expr_native* n = te_compile_native(expression, 0, 0, error);
		double			ret;
		if (n)
		{
			ret = te_eval_native(n);
			te_free_native(n);
		}
		else
		{
			ret = te_traits::nan();
		}
		return ret;
	}

	static void pn(const te_expr_native* n, int depth)
	{
		int i, arity;
		printf("%*s", depth, "");

		switch (te_type_mask(n->type))
		{
		case TE_CONSTANT:
			printf("%f\n", n->value);
			break;
		case TE_VARIABLE:
			printf("bound %p\n", n->bound);
			break;

		case TE_FUNCTION0:
		case TE_FUNCTION1:
		case TE_FUNCTION2:
		case TE_FUNCTION3:
		case TE_FUNCTION4:
		case TE_FUNCTION5:
		case TE_FUNCTION6:
		case TE_FUNCTION7:
		case TE_CLOSURE0:
		case TE_CLOSURE1:
		case TE_CLOSURE2:
		case TE_CLOSURE3:
		case TE_CLOSURE4:
		case TE_CLOSURE5:
		case TE_CLOSURE6:
		case TE_CLOSURE7:
			arity = te_arity(n->type);
			printf("f%d", arity);
			for (i = 0; i < arity; i++)
			{
				printf(" %p", n->parameters[i]);
			}
			printf("\n");
			for (i = 0; i < arity; i++)
			{
				pn((const te_expr_native*)n->parameters[i], depth + 1);
			}
			break;
		}
	}

	static void te_print(const te_expr_native* n)
	{
		pn(n, 0);
	}

	////

	static const te_variable* find_bind_by_addr(const void* addr, const te_variable* lookup, int lookup_len)
	{
		for (int i = 0; i < lookup_len; ++i)
		{
			if (lookup[i].address == addr)
			{
				return &lookup[i];
			}
		}
		return nullptr;
	}

	static const te_variable* find_closure_by_addr(const void* addr, const te_variable* lookup, int lookup_len)
	{
		for (int i = 0; i < lookup_len; ++i)
		{
			if (lookup[i].context == addr)
			{
				return &lookup[i];
			}
		}
		return nullptr;
	}

	static const te_variable* find_bind_or_any_by_addr(const void* addr, const te_variable* lookup, int lookup_len)
	{
		auto res = te_native_builtins<te_traits::t_vector>::find_any_by_addr(addr);
		if (!res)
		{
			res = find_bind_by_addr(addr, lookup, lookup_len);
			if (!res)
			{
				// maybe this is a closure?
				res = find_closure_by_addr(addr, lookup, lookup_len);
			}
		}
		return res;
	}
};

struct te_portable
{
	using te_name_map = std::unordered_map<const void*, std::string>;

	using te_index_map = std::unordered_map<const void*, int>;

	struct te_expr_portable_expression_build_indexer
	{
		te_name_map	 name_map;
		te_index_map index_map;
		int			 index_counter = 0;
	};

	struct te_expr_portable_expression_build_bindings
	{
		std::vector<const void*>
								 index_to_address; // this contains the native function/value address as originally compiled
		std::vector<std::string> index_to_name;
		std::vector<const char*> index_to_name_c_str;
	};

	struct te_compiled_expr
	{
		te_expr_portable_expression_build_indexer  m_indexer;
		te_expr_portable_expression_build_bindings m_bindings;
		std::unique_ptr<unsigned char>			   m_build_buffer;
		size_t									   m_build_buffer_size;
	};

	static size_t te_export_estimate(const te_native::te_expr_native* n,
		size_t&												   export_size,
		const te_variable*									   lookup,
		int													   lookup_len,
		te_name_map&										   name_map,
		te_index_map&										   index_map,
		int&												   index_counter)
	{
#	define M(e)                                                                                                       \
		te_export_estimate((const te_native::te_expr_native*)n->parameters[e],                                         \
			export_size,                                                                                               \
			lookup,                                                                                                    \
			lookup_len,                                                                                                \
			name_map,                                                                                                  \
			index_map,                                                                                                 \
			index_counter)

		if (!n)
			return export_size;

		export_size += sizeof(te_native::te_expr_native);

		auto handle_addr = [&](const te_variable* var) -> bool {
			if (var)
			{
				auto itor = name_map.find(var->address);
				if (itor == name_map.end())
				{
					name_map.emplace(std::make_pair(var->address, std::string(var->name)));
					index_map.insert(std::make_pair(var->address, index_counter++));
				}

				if (var->type >= TE_CLOSURE0 && var->type <= TE_CLOSURE7)
				{
					auto itor = name_map.find(var->context);
					if (itor == name_map.end())
					{
						name_map.emplace(std::make_pair(var->context, std::string(var->name) + "_closure"));
						index_map.insert(std::make_pair(var->context, index_counter++));
					}
				}

				return true;
			}
			return false;
		};

		switch (te_type_mask(n->type))
		{
		case TE_CONSTANT:
		{
			return export_size;
		}

		case TE_VARIABLE:
		{
			auto res = handle_addr(te_native::find_bind_by_addr(n->bound, lookup, lookup_len));
			assert(res);
			return export_size;
		}

		case TE_FUNCTION0:
		case TE_FUNCTION1:
		case TE_FUNCTION2:
		case TE_FUNCTION3:
		case TE_FUNCTION4:
		case TE_FUNCTION5:
		case TE_FUNCTION6:
		case TE_FUNCTION7:
		{
			auto res = handle_addr(te_native::find_bind_or_any_by_addr(n->function, lookup, lookup_len));
			assert(res);
			export_size += sizeof(n->parameters[0]) * (te_arity(n->type));

			for (int i = 0; i < te_arity(n->type); ++i)
			{
				M(i);
			}
		}
		break;

		case TE_CLOSURE0:
		case TE_CLOSURE1:
		case TE_CLOSURE2:
		case TE_CLOSURE3:
		case TE_CLOSURE4:
		case TE_CLOSURE5:
		case TE_CLOSURE6:
		case TE_CLOSURE7:
		{
			auto res = handle_addr(te_native::find_bind_or_any_by_addr(n->function, lookup, lookup_len));
			assert(res);

			export_size += sizeof(n->parameters[0]) * te_arity(n->type);

			for (int i = 0; i < te_arity(n->type); ++i)
			{
				M(i);
			}
		}
		break;
		}
		return export_size;
#	undef TE_FUN
#	undef M
	}

	template<typename T_REGISTER_FUNC>
	static size_t te_export_write(const te_native::te_expr_native* n,
		size_t&												export_size,
		const te_variable*									lookup,
		int													lookup_len,
		const unsigned char*								out_buffer,
		T_REGISTER_FUNC										register_func)
	{
#	define M(e)                                                                                                       \
		te_export_write((const te_native::te_expr_native*)n->parameters[e],                                            \
			export_size,                                                                                               \
			lookup,                                                                                                    \
			lookup_len,                                                                                                \
			out_buffer,                                                                                                \
			register_func)

		if (!n)
			return export_size;

		te_expr_portable* n_out = (te_expr_portable*)(out_buffer + export_size);

		export_size += sizeof(te_native::te_expr_native);
		n_out->type = n->type;
		switch (te_type_mask(n->type))
		{
		case TE_CONSTANT:
		{
			n_out->value = n->value;
			return export_size;
		}

		case TE_VARIABLE:
		{
			register_func(n->bound, n_out, te_native::find_bind_by_addr(n->bound, lookup, lookup_len));
			return export_size;
		}

		case TE_FUNCTION0:
		case TE_FUNCTION1:
		case TE_FUNCTION2:
		case TE_FUNCTION3:
		case TE_FUNCTION4:
		case TE_FUNCTION5:
		case TE_FUNCTION6:
		case TE_FUNCTION7:
		{
			register_func(n->function, n_out, te_native::find_bind_or_any_by_addr(n->function, lookup, lookup_len));

			export_size += sizeof(n->parameters[0]) * te_arity(n->type);

			for (int i = 0; i < te_arity(n->type); ++i)
			{
				n_out->parameters[i] = export_size;
				M(i);
			}
		}
		break;

		case TE_CLOSURE0:
		case TE_CLOSURE1:
		case TE_CLOSURE2:
		case TE_CLOSURE3:
		case TE_CLOSURE4:
		case TE_CLOSURE5:
		case TE_CLOSURE6:
		case TE_CLOSURE7:
		{
			register_func(n->function, n_out, te_native::find_bind_or_any_by_addr(n->function, lookup, lookup_len));

			// register_func(n->parameters[te_arity(n->type)],
			//	n_out->parameters[te_arity(n->type)],
			//	te_native::find_bind_or_any_by_addr(n->parameters[te_arity(n->type)], lookup, lookup_len));

			export_size += sizeof(n->parameters[0]) * te_arity(n->type);

			for (int i = 0; i < te_arity(n->type); ++i)
			{
				n_out->parameters[i] = export_size;
				M(i);
			}
		}
		break;
		}
		return export_size;
#	undef TE_FUN
#	undef M
	}

	static double te_eval_compare(const te_native::te_expr_native* n,
		const te_expr_portable*								n_portable,
		const unsigned char*								expr_buffer,
		const void* const									expr_context[])
	{
#	define TE_FUN(...) ((double (*)(__VA_ARGS__))expr_context[n_portable->function])

#	define M(e)                                                                                                       \
		te_eval_compare((const te_native::te_expr_native*)n->parameters[e],                                            \
			(const te_expr_portable*)&expr_buffer[n_portable->parameters[e]],                                          \
			expr_buffer,                                                                                               \
			expr_context)

		if (!n)
			return te_traits::nan();

		assert(n->type == n_portable->type);

		switch (te_type_mask(n_portable->type))
		{
		case TE_CONSTANT:
			return n_portable->value;
		case TE_VARIABLE:
			assert(n->bound == expr_context[n_portable->bound]);
			return *((const double*)(expr_context[n_portable->bound]));

		case TE_FUNCTION0:
		case TE_FUNCTION1:
		case TE_FUNCTION2:
		case TE_FUNCTION3:
		case TE_FUNCTION4:
		case TE_FUNCTION5:
		case TE_FUNCTION6:
		case TE_FUNCTION7:

			assert(n->function == expr_context[n_portable->function]);

			switch (te_arity(n_portable->type))
			{
			case 0:
				return TE_FUN(void)();
			case 1:
				return TE_FUN(double)(M(0));
			case 2:
				return TE_FUN(double, double)(M(0), M(1));
			case 3:
				return TE_FUN(double, double, double)(M(0), M(1), M(2));
			case 4:
				return TE_FUN(double, double, double, double)(M(0), M(1), M(2), M(3));
			case 5:
				return TE_FUN(double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4));
			case 6:
				return TE_FUN(double, double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4), M(5));
			case 7:
				return TE_FUN(double, double, double, double, double, double, double)(
					M(0), M(1), M(2), M(3), M(4), M(5), M(6));
			default:
				return te_traits::nan();
			}

		case TE_CLOSURE0:
		case TE_CLOSURE1:
		case TE_CLOSURE2:
		case TE_CLOSURE3:
		case TE_CLOSURE4:
		case TE_CLOSURE5:
		case TE_CLOSURE6:
		case TE_CLOSURE7:
		{
			assert(n->function == expr_context[n_portable->function]);
			assert(n->parameters[te_arity(n->type)] == expr_context[n_portable->parameters[te_arity(n->type)]]);

			auto arity_params = (void*)expr_context[n_portable->parameters[te_arity(n->type)]];

			switch (te_arity(n_portable->type))
			{
			case 0:
				return TE_FUN(void*)(arity_params);
			case 1:
				return TE_FUN(void*, double)(arity_params, M(0));
			case 2:
				return TE_FUN(void*, double, double)(arity_params, M(0), M(1));
			case 3:
				return TE_FUN(void*, double, double, double)(arity_params, M(0), M(1), M(2));
			case 4:
				return TE_FUN(void*, double, double, double, double)(arity_params, M(0), M(1), M(2), M(3));
			case 5:
				return TE_FUN(void*, double, double, double, double, double)(
					arity_params, M(0), M(1), M(2), M(3), M(4));
			case 6:
				return TE_FUN(void*, double, double, double, double, double, double)(
					arity_params, M(0), M(1), M(2), M(3), M(4), M(5));
			case 7:
				return TE_FUN(void*, double, double, double, double, double, double, double)(
					arity_params, M(0), M(1), M(2), M(3), M(4), M(5), M(6));
			default:
				return te_traits::nan();
			}
		}

		default:
			return te_traits::nan();
		}
#	undef TE_FUN
#	undef M
	}
};

te_compiled_expr te_compile(const char* expression, const te_variable* variables, int var_count, int* error)
{
	te_native::te_expr_native* native_expr = te_native::te_compile_native(expression, variables, var_count, error);
	if (native_expr)
	{
		auto expr = new te_portable::te_compiled_expr;

		size_t export_size = 0;
		te_portable::te_export_estimate(native_expr,
			export_size,
			variables,
			var_count,
			expr->m_indexer.name_map,
			expr->m_indexer.index_map,
			expr->m_indexer.index_counter);

		expr->m_bindings.index_to_address.resize(expr->m_indexer.index_counter);
		for (const auto& itor : expr->m_indexer.index_map)
		{
			expr->m_bindings.index_to_address[itor.second] = itor.first;
		}

		expr->m_bindings.index_to_name.resize(expr->m_indexer.index_counter);
		expr->m_bindings.index_to_name_c_str.resize(expr->m_indexer.index_counter);
		for (int i = 0; i < expr->m_indexer.index_counter; ++i)
		{
			auto itor = expr->m_indexer.name_map.find(expr->m_bindings.index_to_address[i]);
			assert(itor != expr->m_indexer.name_map.end());
			expr->m_bindings.index_to_name[i]		= itor->second;
			expr->m_bindings.index_to_name_c_str[i] = expr->m_bindings.index_to_name[i].c_str();
		}

		expr->m_build_buffer.reset(new uint8_t[export_size]);
		::memset(expr->m_build_buffer.get(), 0x0, export_size);
		expr->m_build_buffer_size = export_size;

		size_t actual_export_size = 0;
		te_portable::te_export_write(native_expr,
			actual_export_size,
			variables,
			var_count,
			expr->m_build_buffer.get(),
			[&](const void* addr, te_expr_portable* out, const te_variable* v) -> void {
				assert(v != nullptr);
				auto itor = expr->m_indexer.index_map.find(addr);
				assert(itor != expr->m_indexer.index_map.end());
				out->function = itor->second;

				if (v->type >= TE_CLOSURE0 && v->type <= TE_CLOSURE7)
				{
					auto itor2 = expr->m_indexer.index_map.find(v->context);
					assert(itor2 != expr->m_indexer.index_map.end());
					out->parameters[te_arity(v->type)] = itor2->second;
				}
			});

		te_native::te_free_native(native_expr);
		return expr;
	}
	return nullptr;
}

size_t te_get_binding_array_size(const te_compiled_expr _n)
{
	auto n = (const te_portable::te_compiled_expr*)_n;
	if (n)
	{
		return n->m_bindings.index_to_address.size();
	}
	return 0;
}

const void* const* te_get_binding_addresses(const te_compiled_expr _n)
{
	auto n = (const te_portable::te_compiled_expr*)_n;
	if (n && (n->m_bindings.index_to_address.size() > 0))
	{
		return &(*n->m_bindings.index_to_address.cbegin());
	}
	return nullptr;
}

const char* const* te_get_binding_names(const te_compiled_expr _n)
{
	auto n = (const te_portable::te_compiled_expr*)_n;
	if (n)
	{
		return &(*n->m_bindings.index_to_name_c_str.cbegin());
	}
	return nullptr;
}

size_t te_get_expr_data_size(const te_compiled_expr _n)
{
	auto n = (const te_portable::te_compiled_expr*)_n;
	if (n)
	{
		return n->m_build_buffer_size;
	}
	return 0;
}

const unsigned char* te_get_expr_data(const te_compiled_expr _n)
{
	auto n = (const te_portable::te_compiled_expr*)_n;
	if (n)
	{
		return n->m_build_buffer.get();
	}
	return nullptr;
}

void te_free(te_compiled_expr _n)
{
	auto n = (te_portable::te_compiled_expr*)_n;
	if (n)
	{
		delete n;
	}
}
#endif // #if (TE_COMPILER_ENABLED)
