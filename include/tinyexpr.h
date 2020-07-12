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

#ifndef __TINYEXPR_H__
#define __TINYEXPR_H__

#include <limits>

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

struct tinyexpr_details
{
	static inline constexpr double nan		= std::numeric_limits<double>::quiet_NaN();
	static inline constexpr double infinity = std::numeric_limits<double>::infinity();

	static inline double pi(void)
	{
		return 3.14159265358979323846;
	}

	static inline double e(void)
	{
		return 2.71828182845904523536;
	}

	static inline double exp(double a)
	{
		return ::exp(a);
	}

	static inline double fac(double a)
	{
		/* simplest version of fac */
		if (a < 0.0)
			return nan;
		if (a > UINT_MAX)
			return infinity;
		unsigned int	  ua	 = (unsigned int)(a);
		unsigned long int result = 1, i;
		for (i = 1; i <= ua; i++)
		{
			if (i > ULONG_MAX / result)
				return infinity;
			result *= i;
		}
		return (double)result;
	}

	static inline double ncr(double n, double r)
	{
		if (n < 0.0 || r < 0.0 || n < r)
			return nan;
		if (n > UINT_MAX || r > UINT_MAX)
			return infinity;
		unsigned long int un = (unsigned int)(n), ur = (unsigned int)(r), i;
		unsigned long int result = 1;
		if (ur > un / 2)
			ur = un - ur;
		for (i = 1; i <= ur; i++)
		{
			if (result > ULONG_MAX / (un - ur + i))
				return infinity;
			result *= un - ur + i;
			result /= i;
		}
		return result;
	}

	static inline double npr(double n, double r)
	{
		return ncr(n, r) * fac(r);
	}

	static inline double floor(double d)
	{
		return ::floor(d);
	}

	static inline double ceil(double d)
	{
		return ::ceil(d);
	}

	static inline double fabs(double a)
	{
		return ::fabs(a);
	}

	static inline double acos(double a)
	{
		return ::acos(a);
	}

	static inline double asin(double a)
	{
		return ::asin(a);
	}

	static inline double atan(double a)
	{
		return ::atan(a);
	}

	static inline double atan2(double a, double b)
	{
		return ::atan2(a, b);
	}

	static inline double cos(double a)
	{
		return ::cos(a);
	}

	static inline double cosh(double a)
	{
		return ::cosh(a);
	}

	static inline double log(double a)
	{
		return ::log(a);
	}

	static inline double log10(double a)
	{
		return ::log10(a);
	}

	static inline double pow(double a, double b)
	{
		return ::pow(a, b);
	}

	static inline double sin(double a)
	{
		return ::sin(a);
	}

	static inline double sinh(double a)
	{
		return ::sinh(a);
	}

	static inline double sqrt(double a)
	{
		return ::sqrt(a);
	}

	static inline double tan(double a)
	{
		return ::tan(a);
	}

	static inline double tanh(double a)
	{
		return ::tanh(a);
	}

	static inline double fmod(double a, double b)
	{
		return ::fmod(a, b);
	}

	static inline double add(double a, double b)
	{
		return a + b;
	}

	static inline double sub(double a, double b)
	{
		return a - b;
	}

	static inline double mul(double a, double b)
	{
		return a * b;
	}

	static inline double divide(double a, double b)
	{
		return a / b;
	}

	static inline double negate(double a)
	{
		return -a;
	}

	static inline double comma(double a, double b)
	{
		(void)a;
		return b;
	}

	static inline double greater(double a, double b)
	{
		return a > b;
	}

	static inline double greater_eq(double a, double b)
	{
		return a >= b;
	}

	static inline double lower(double a, double b)
	{
		return a < b;
	}

	static inline double lower_eq(double a, double b)
	{
		return a <= b;
	}

	static inline double equal(double a, double b)
	{
		return a == b;
	}

	static inline double not_equal(double a, double b)
	{
		return a != b;
	}

	static inline double logical_and(double a, double b)
	{
		return a != 0.0 && b != 0.0;
	}

	static inline double logical_or(double a, double b)
	{
		return a != 0.0 || b != 0.0;
	}

	static inline double logical_not(double a)
	{
		return a == 0.0;
	}

	static inline double logical_notnot(double a)
	{
		return a != 0.0;
	}

	static inline double negate_logical_not(double a)
	{
		return -(a == 0.0);
	}

	static inline double negate_logical_notnot(double a)
	{
		return -(a != 0.0);
	}
};

struct tinyexpr_defines
{
	enum
	{
		TE_VARIABLE = 0,

		TE_FUNCTION0 = 8,
		TE_FUNCTION1,
		TE_FUNCTION2,
		TE_FUNCTION3,
		TE_FUNCTION4,
		TE_FUNCTION5,
		TE_FUNCTION6,
		TE_FUNCTION7,

		TE_CLOSURE_INIT
	};

	enum
	{
		TE_CLOSURE0 = TE_CLOSURE_INIT,
		TE_CLOSURE1,
		TE_CLOSURE2,
		TE_CLOSURE3,
		TE_CLOSURE4,
		TE_CLOSURE5,
		TE_CLOSURE6,
		TE_CLOSURE7,

		TE_FLAG_PURE = 32
	};

	enum
	{
		TE_CONSTANT = 1
	};
};

struct tinyexpr_common : public tinyexpr_defines
{
	using details = tinyexpr_details;

	struct te_expr
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

	struct te_variable
	{
		const char* name;
		const void* address;
		int			type;
		void*		context;
	};

	/* Parses the input expression, evaluates it, and frees it. */
	/* Returns NaN on error. */
	static inline double te_interp(const char* expression, int* error);

	/* Parses the input expression and binds variables. */
	/* Returns NULL on error. */
	static inline te_expr* te_compile(const char* expression, const te_variable* variables, int var_count, int* error);

	///* Evaluates the expression. */
	// static inline double te_eval(const te_expr* n);

	/* Prints debugging information on the syntax tree. */
	static inline void te_print(const te_expr* n);

	///* Frees the expression. */
	///* This is safe to call on NULL pointers. */
	// static inline void te_free(te_expr* n);
};

struct tinyexpr_eval
{
	using details = tinyexpr_details;
	using common  = tinyexpr_common;

#define TYPE_MASK(TYPE) ((TYPE)&0x0000001F)

#define IS_PURE(TYPE)	  (((TYPE)&common::TE_FLAG_PURE) != 0)
#define IS_FUNCTION(TYPE) (((TYPE)&common::TE_FUNCTION0) != 0)
#define IS_CLOSURE(TYPE)  (((TYPE)&common::TE_CLOSURE0) != 0)
#define ARITY(TYPE)		  (((TYPE) & (common::TE_FUNCTION0 | common::TE_CLOSURE0)) ? ((TYPE)&0x00000007) : 0)

	static inline double te_eval(const common::te_expr* n)
	{
#define TE_FUN(...) ((double (*)(__VA_ARGS__))n->function)
#define M(e)		te_eval((const common::te_expr*)n->parameters[e])

		if (!n)
			return details::nan;

		switch (TYPE_MASK(n->type))
		{
		case common::TE_CONSTANT:
			return n->value;
		case common::TE_VARIABLE:
			return *n->bound;

		case common::TE_FUNCTION0:
		case common::TE_FUNCTION1:
		case common::TE_FUNCTION2:
		case common::TE_FUNCTION3:
		case common::TE_FUNCTION4:
		case common::TE_FUNCTION5:
		case common::TE_FUNCTION6:
		case common::TE_FUNCTION7:
			switch (ARITY(n->type))
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
				return details::nan;
			}

		case common::TE_CLOSURE0:
		case common::TE_CLOSURE1:
		case common::TE_CLOSURE2:
		case common::TE_CLOSURE3:
		case common::TE_CLOSURE4:
		case common::TE_CLOSURE5:
		case common::TE_CLOSURE6:
		case common::TE_CLOSURE7:
			switch (ARITY(n->type))
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
				return details::nan;
			}

		default:
			return details::nan;
		}

#undef TE_FUN
#undef M
	}
};

struct tinyexpr_compiler
{
	using details = tinyexpr_details;
	using common  = tinyexpr_common;
	using eval	  = tinyexpr_eval;

	typedef double (*te_fun2)(double, double);

	enum
	{
		TOK_NULL = common::TE_CLOSURE7 + 1,
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

		const common::te_variable* lookup;
		int						   lookup_len;
	};

#define NEW_EXPR(type, ...)                                                                                            \
	[&]() {                                                                                                            \
		const common::te_expr* _args[] = {__VA_ARGS__};                                                                \
		return new_expr((type), _args);                                                                                \
	}()

	static inline common::te_expr* new_expr(const int type, const common::te_expr* parameters[])
	{
		const int arity = ARITY(type);
		const int psize = sizeof(void*) * arity;
		const int size	= (sizeof(common::te_expr) - sizeof(void*)) + psize + (IS_CLOSURE(type) ? sizeof(void*) : 0);
		common::te_expr* ret = (common::te_expr*)malloc(size);
		memset(ret, 0, size);
		if (arity && parameters)
		{
			memcpy(ret->parameters, parameters, psize);
		}
		ret->type  = type;
		ret->bound = 0;
		return ret;
	}

	static inline void te_free_parameters(common::te_expr* n)
	{
		if (!n)
			return;

		switch (TYPE_MASK(n->type))
		{
		case common::TE_FUNCTION7:
		case common::TE_CLOSURE7:
			te_free((common::te_expr*)n->parameters[6]); /* Falls through. */
		case common::TE_FUNCTION6:
		case common::TE_CLOSURE6:
			te_free((common::te_expr*)n->parameters[5]); /* Falls through. */
		case common::TE_FUNCTION5:
		case common::TE_CLOSURE5:
			te_free((common::te_expr*)n->parameters[4]); /* Falls through. */
		case common::TE_FUNCTION4:
		case common::TE_CLOSURE4:
			te_free((common::te_expr*)n->parameters[3]); /* Falls through. */
		case common::TE_FUNCTION3:
		case common::TE_CLOSURE3:
			te_free((common::te_expr*)n->parameters[2]); /* Falls through. */
		case common::TE_FUNCTION2:
		case common::TE_CLOSURE2:
			te_free((common::te_expr*)n->parameters[1]); /* Falls through. */
		case common::TE_FUNCTION1:
		case common::TE_CLOSURE1:
			te_free((common::te_expr*)n->parameters[0]);
		}
	}

	static inline void te_free(common::te_expr* n)
	{
		if (!n)
			return;
		te_free_parameters(n);
		free(n);
	}

	static inline const common::te_variable functions[] = {
		/* must be in alphabetical order */
		{"abs", details::fabs, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
		{"acos", details::acos, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
		{"asin", details::asin, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
		{"atan", details::atan, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
		{"atan2", details::atan2, common::TE_FUNCTION2 | common::TE_FLAG_PURE, 0},
		{"ceil", details::ceil, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
		{"cos", details::cos, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
		{"cosh", details::cosh, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
		{"e", details::e, common::TE_FUNCTION0 | common::TE_FLAG_PURE, 0},
		{"exp", details::exp, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
		{"fac", details::fac, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
		{"floor", details::floor, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
		{"ln", details::log, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
#ifdef TE_NAT_LOG
		{"log", details::log, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
#else
		{"log", details::log10, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
#endif
		{"log10", details::log10, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
		{"ncr", details::ncr, common::TE_FUNCTION2 | common::TE_FLAG_PURE, 0},
		{"npr", details::npr, common::TE_FUNCTION2 | common::TE_FLAG_PURE, 0},
		{"pi", details::pi, common::TE_FUNCTION0 | common::TE_FLAG_PURE, 0},
		{"pow", details::pow, common::TE_FUNCTION2 | common::TE_FLAG_PURE, 0},
		{"sin", details::sin, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
		{"sinh", details::sinh, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
		{"sqrt", details::sqrt, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
		{"tan", details::tan, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
		{"tanh", details::tanh, common::TE_FUNCTION1 | common::TE_FLAG_PURE, 0},
		{0, 0, 0, 0}};

	static inline const common::te_variable* find_builtin(const char* name, int len)
	{
		int imin = 0;
		int imax = sizeof(functions) / sizeof(common::te_variable) - 2;

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

		return 0;
	}

	static inline const common::te_variable* find_lookup(const state* s, const char* name, int len)
	{
		int						   iters;
		const common::te_variable* var;
		if (!s->lookup)
			return 0;

		for (var = s->lookup, iters = s->lookup_len; iters; ++var, --iters)
		{
			if (strncmp(name, var->name, len) == 0 && var->name[len] == '\0')
			{
				return var;
			}
		}
		return 0;
	}

	static inline void next_token(state* s)
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

					const common::te_variable* var = find_lookup(s, start, s->next - start);
					if (!var)
						var = find_builtin(start, s->next - start);

					if (!var)
					{
						s->type = TOK_ERROR;
					}
					else
					{
						switch (TYPE_MASK(var->type))
						{
						case common::TE_VARIABLE:
							s->type	 = TOK_VARIABLE;
							s->bound = (const double*)var->address;
							break;

						case common::TE_CLOSURE0:
						case common::TE_CLOSURE1:
						case common::TE_CLOSURE2:
						case common::TE_CLOSURE3: /* Falls through. */
						case common::TE_CLOSURE4:
						case common::TE_CLOSURE5:
						case common::TE_CLOSURE6:
						case common::TE_CLOSURE7:	   /* Falls through. */
							s->context = var->context; /* Falls through. */

						case common::TE_FUNCTION0:
						case common::TE_FUNCTION1:
						case common::TE_FUNCTION2:
						case common::TE_FUNCTION3: /* Falls through. */
						case common::TE_FUNCTION4:
						case common::TE_FUNCTION5:
						case common::TE_FUNCTION6:
						case common::TE_FUNCTION7: /* Falls through. */
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
						s->function = details::add;
						break;
					case '-':
						s->type		= TOK_INFIX;
						s->function = details::sub;
						break;
					case '*':
						s->type		= TOK_INFIX;
						s->function = details::mul;
						break;
					case '/':
						s->type		= TOK_INFIX;
						s->function = details::divide;
						break;
					case '^':
						s->type		= TOK_INFIX;
						s->function = pow;
						break;
					case '%':
						s->type		= TOK_INFIX;
						s->function = details::fmod;
						break;
					case '!':
						if (s->next++[0] == '=')
						{
							s->type		= TOK_INFIX;
							s->function = details::not_equal;
						}
						else
						{
							s->next--;
							s->type		= TOK_INFIX;
							s->function = details::logical_not;
						}
						break;
					case '=':
						if (s->next++[0] == '=')
						{
							s->type		= TOK_INFIX;
							s->function = details::equal;
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
							s->function = details::lower_eq;
						}
						else
						{
							s->next--;
							s->type		= TOK_INFIX;
							s->function = details::lower;
						}
						break;
					case '>':
						if (s->next++[0] == '=')
						{
							s->type		= TOK_INFIX;
							s->function = details::greater_eq;
						}
						else
						{
							s->next--;
							s->type		= TOK_INFIX;
							s->function = details::greater;
						}
						break;
					case '&':
						if (s->next++[0] == '&')
						{
							s->type		= TOK_INFIX;
							s->function = details::logical_and;
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
							s->function = details::logical_or;
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

	static inline common::te_expr* base(state* s)
	{
		/* <base>      =    <constant> | <variable> | <function-0> {"(" ")"} | <function-1> <power> | <function-X> "("
		 * <expr> {"," <expr>} ")" | "(" <list> ")" */
		common::te_expr* ret;
		int				 arity;

		switch (TYPE_MASK(s->type))
		{
		case TOK_NUMBER:
			ret		   = new_expr(common::TE_CONSTANT, 0);
			ret->value = s->value;
			next_token(s);
			break;

		case TOK_VARIABLE:
			ret		   = new_expr(common::TE_VARIABLE, 0);
			ret->bound = s->bound;
			next_token(s);
			break;

		case common::TE_FUNCTION0:
		case common::TE_CLOSURE0:
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

		case common::TE_FUNCTION1:
		case common::TE_CLOSURE1:
			ret			  = new_expr(s->type, 0);
			ret->function = s->function;
			if (IS_CLOSURE(s->type))
				ret->parameters[1] = s->context;
			next_token(s);
			ret->parameters[0] = power(s);
			break;

		case common::TE_FUNCTION2:
		case common::TE_FUNCTION3:
		case common::TE_FUNCTION4:
		case common::TE_FUNCTION5:
		case common::TE_FUNCTION6:
		case common::TE_FUNCTION7:
		case common::TE_CLOSURE2:
		case common::TE_CLOSURE3:
		case common::TE_CLOSURE4:
		case common::TE_CLOSURE5:
		case common::TE_CLOSURE6:
		case common::TE_CLOSURE7:
			arity = ARITY(s->type);

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
			ret->value = details::nan;
			break;
		}

		return ret;
	}

	static inline common::te_expr* power(state* s)
	{
		/* <power>     =    {("-" | "+" | "!")} <base> */
		int sign = 1;
		while (s->type == TOK_INFIX && (s->function == details::add || s->function == details::sub))
		{
			if (s->function == details::sub)
				sign = -sign;
			next_token(s);
		}

		int logical = 0;
		while (s->type == TOK_INFIX &&
			   (s->function == details::add || s->function == details::sub || s->function == details::logical_not))
		{
			if (s->function == details::logical_not)
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

		common::te_expr* ret;

		if (sign == 1)
		{
			if (logical == 0)
			{
				ret = base(s);
			}
			else if (logical == -1)
			{
				ret			  = NEW_EXPR(common::TE_FUNCTION1 | common::TE_FLAG_PURE, base(s));
				ret->function = details::logical_not;
			}
			else
			{
				ret			  = NEW_EXPR(common::TE_FUNCTION1 | common::TE_FLAG_PURE, base(s));
				ret->function = details::logical_notnot;
			}
		}
		else
		{
			if (logical == 0)
			{
				ret			  = NEW_EXPR(common::TE_FUNCTION1 | common::TE_FLAG_PURE, base(s));
				ret->function = details::negate;
			}
			else if (logical == -1)
			{
				ret			  = NEW_EXPR(common::TE_FUNCTION1 | common::TE_FLAG_PURE, base(s));
				ret->function = details::negate_logical_not;
			}
			else
			{
				ret			  = NEW_EXPR(common::TE_FUNCTION1 | common::TE_FLAG_PURE, base(s));
				ret->function = details::negate_logical_notnot;
			}
		}

		return ret;
	}

#ifdef TE_POW_FROM_RIGHT
	static inline te_expr* factor(state* s)
	{
		/* <factor>    =    <power> {"^" <power>} */
		common::te_expr* ret = power(s);

		const void*		 left_function = NULL;
		common::te_expr* insertion	   = 0;

		if (ret->type == (TE_FUNCTION1 | TE_FLAG_PURE) &&
			(ret->function == details::negate || ret->function == details::logical_not ||
				ret->function == details::logical_notnot || ret->function == details::negate_logical_not ||
				ret->function == details::negate_logical_notnot))
		{
			left_function = ret->function;
			te_expr* se	  = ret->parameters[0];
			free(ret);
			ret = se;
		}

		while (s->type == TOK_INFIX && (s->function == pow))
		{
			te_fun2 t = s->function;
			next_token(s);

			if (insertion)
			{
				/* Make exponentiation go right-to-left. */
				te_expr* insert			 = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, insertion->parameters[1], power(s));
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
	static inline common::te_expr* factor(state* s)
	{
		/* <factor>    =    <power> {"^" <power>} */
		common::te_expr* ret = power(s);

		while (s->type == TOK_INFIX && (s->function == pow))
		{
			te_fun2 t = (te_fun2)s->function;
			next_token(s);
			ret = NEW_EXPR(common::TE_FUNCTION2 | common::TE_FLAG_PURE, ret, power(s));
			ret->function = t;
		}

		return ret;
	}
#endif

	static inline common::te_expr* term(state* s)
	{
		/* <term>      =    <factor> {("*" | "/" | "%") <factor>} */
		common::te_expr* ret = factor(s);

		while (s->type == TOK_INFIX &&
			   (s->function == details::mul || s->function == details::divide || s->function == details::fmod))
		{
			te_fun2 t = (te_fun2)s->function;
			next_token(s);
			ret			  = NEW_EXPR(common::TE_FUNCTION2 | common::TE_FLAG_PURE, ret, factor(s));
			ret->function = t;
		}

		return ret;
	}

	static inline common::te_expr* sum_expr(state* s)
	{
		/* <expr>      =    <term> {("+" | "-") <term>} */
		common::te_expr* ret = term(s);

		while (s->type == TOK_INFIX && (s->function == details::add || s->function == details::sub))
		{
			te_fun2 t = (te_fun2)s->function;
			next_token(s);
			ret			  = NEW_EXPR(common::TE_FUNCTION2 | common::TE_FLAG_PURE, ret, term(s));
			ret->function = t;
		}

		return ret;
	}

	static inline common::te_expr* test_expr(state* s)
	{
		/* <expr>      =    <sum_expr> {(">" | ">=" | "<" | "<=" | "==" | "!=") <sum_expr>} */
		common::te_expr* ret = sum_expr(s);

		while (s->type == TOK_INFIX && (s->function == details::greater || s->function == details::greater_eq ||
										   s->function == details::lower || s->function == details::lower_eq ||
										   s->function == details::equal || s->function == details::not_equal))
		{
			te_fun2 t = (te_fun2)s->function;
			next_token(s);
			ret			  = NEW_EXPR(common::TE_FUNCTION2 | common::TE_FLAG_PURE, ret, sum_expr(s));
			ret->function = t;
		}

		return ret;
	}

	static inline common::te_expr* expr(state* s)
	{
		/* <expr>      =    <test_expr> {("&&" | "||") <test_expr>} */
		common::te_expr* ret = test_expr(s);

		while (s->type == TOK_INFIX && (s->function == details::logical_and || s->function == details::logical_or))
		{
			te_fun2 t = (te_fun2)s->function;
			next_token(s);
			ret			  = NEW_EXPR(common::TE_FUNCTION2 | common::TE_FLAG_PURE, ret, test_expr(s));
			ret->function = t;
		}

		return ret;
	}

	static inline common::te_expr* list(state* s)
	{
		/* <list>      =    <expr> {"," <expr>} */
		common::te_expr* ret = expr(s);

		while (s->type == TOK_SEP)
		{
			next_token(s);
			ret			  = NEW_EXPR(common::TE_FUNCTION2 | common::TE_FLAG_PURE, ret, expr(s));
			ret->function = details::comma;
		}

		return ret;
	}

	static inline void optimize(common::te_expr* n)
	{
		/* Evaluates as much as possible. */
		if (n->type == common::TE_CONSTANT)
			return;
		if (n->type == common::TE_VARIABLE)
			return;

		/* Only optimize out functions flagged as pure. */
		if (IS_PURE(n->type))
		{
			const int arity = ARITY(n->type);
			int		  known = 1;
			int		  i;
			for (i = 0; i < arity; ++i)
			{
				optimize((common::te_expr*)n->parameters[i]);
				if (((common::te_expr*)(n->parameters[i]))->type != common::TE_CONSTANT)
				{
					known = 0;
				}
			}
			if (known)
			{
				const double value = eval::te_eval(n);
				te_free_parameters(n);
				n->type	 = common::TE_CONSTANT;
				n->value = value;
			}
		}
	}

	static inline common::te_expr* te_compile(
		const char* expression, const common::te_variable* variables, int var_count, int* error)
	{
		state s;
		s.start = s.next = expression;
		s.lookup		 = variables;
		s.lookup_len	 = var_count;

		next_token(&s);
		common::te_expr* root = list(&s);

		if (s.type != TOK_END)
		{
			te_free(root);
			if (error)
			{
				*error = (s.next - s.start);
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

	static inline double te_interp(const char* expression, int* error)
	{
		common::te_expr* n = te_compile(expression, 0, 0, error);
		double			 ret;
		if (n)
		{
			ret = eval::te_eval(n);
			te_free(n);
		}
		else
		{
			ret = details::nan;
		}
		return ret;
	}

	static inline void pn(const common::te_expr* n, int depth)
	{
		int i, arity;
		printf("%*s", depth, "");

		switch (TYPE_MASK(n->type))
		{
		case common::TE_CONSTANT:
			printf("%f\n", n->value);
			break;
		case common::TE_VARIABLE:
			printf("bound %p\n", n->bound);
			break;

		case common::TE_FUNCTION0:
		case common::TE_FUNCTION1:
		case common::TE_FUNCTION2:
		case common::TE_FUNCTION3:
		case common::TE_FUNCTION4:
		case common::TE_FUNCTION5:
		case common::TE_FUNCTION6:
		case common::TE_FUNCTION7:
		case common::TE_CLOSURE0:
		case common::TE_CLOSURE1:
		case common::TE_CLOSURE2:
		case common::TE_CLOSURE3:
		case common::TE_CLOSURE4:
		case common::TE_CLOSURE5:
		case common::TE_CLOSURE6:
		case common::TE_CLOSURE7:
			arity = ARITY(n->type);
			printf("f%d", arity);
			for (i = 0; i < arity; i++)
			{
				printf(" %p", n->parameters[i]);
			}
			printf("\n");
			for (i = 0; i < arity; i++)
			{
				pn((const common::te_expr*)n->parameters[i], depth + 1);
			}
			break;
		}
	}

	static inline void te_print(const common::te_expr* n)
	{
		pn(n, 0);
	}
};

struct tinyexpr : public tinyexpr_defines
{
	using details  = tinyexpr_details;
	using common   = tinyexpr_common;
	using eval	   = tinyexpr_eval;
	using compiler = tinyexpr_compiler;

	using te_variable = common::te_variable;
	using te_expr	  = common::te_expr;

	static inline common::te_expr* te_compile(
		const char* expression, const common::te_variable* variables, int var_count, int* error)
	{
		return compiler::te_compile(
			expression, variables, var_count, error);
	}

	static inline double te_interp(const char* expression, int* error)
	{
		return compiler::te_interp(expression, error);
	}

	static inline double te_eval(const common::te_expr* n)
	{
		return eval::te_eval(n);
	}

	static inline void te_free(common::te_expr* n)
	{
		compiler::te_free(n);
	}
};

#endif /*__TINYEXPR_H__*/
