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
#include <cassert>
#include <vector>
#include <algorithm>
#include <string_view>
#include <unordered_map>

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
		TE_FUNCTION_MAX,
		TE_CLOSURE_INIT = TE_FUNCTION_MAX
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
		TE_CLOSURE_MAX,

		TE_FLAG_PURE = 32
	};

	enum
	{
		TE_CONSTANT = 1
	};

	static inline int type_mask(const int i)
	{
		return i & 0x0000001F;
	}

	static inline bool is_pure(const int i)
	{
		return (i & TE_FLAG_PURE) != 0;
	}

	static inline bool is_function(const int i)
	{
		return (i & TE_FUNCTION0) != 0;
	}

	static inline bool is_closure(const int i)
	{
		return (i & TE_CLOSURE0) != 0;
	}

	static inline int arity(const int i)
	{
		return (i & (TE_FUNCTION0 | TE_CLOSURE0)) ? (i & 0x00000007) : 0;
	}
};

struct tinyexpr_common : public tinyexpr_defines
{
	struct te_expr
	{
		int type;
		union
		{
			double		  value;
			const double* bound_variable;
			const void*	  bound_function;
		};
		void* parameters[1];
	};

	struct te_variable
	{
		std::string_view name;
		const void*		 address;
		int				 type;
		void*			 closure_context;
	};
};

struct tinyexpr_eval
{
	using details	  = tinyexpr_details;
	using common	  = tinyexpr_common;

	static inline double te_eval(const common::te_expr* n)
	{
#define TE_FUN(...) ((double (*)(__VA_ARGS__))n->bound_function)
#define M(e)		te_eval((const common::te_expr*)n->parameters[e])

		if (!n)
			return details::nan;

		switch (common::type_mask(n->type))
		{
		case common::TE_CONSTANT:
			return n->value;

		case common::TE_VARIABLE:
			return *n->bound_variable;

		case common::TE_FUNCTION0:
		case common::TE_FUNCTION1:
		case common::TE_FUNCTION2:
		case common::TE_FUNCTION3:
		case common::TE_FUNCTION4:
		case common::TE_FUNCTION5:
		case common::TE_FUNCTION6:
		case common::TE_FUNCTION7:
			switch (common::arity(n->type))
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
			switch (common::arity(n->type))
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

template<typename T_REGISTRY>
struct tinyexpr_compiler
{
	using details	  = tinyexpr_details;
	using common	  = tinyexpr_common;
	using eval		  = tinyexpr_eval;
	using te_registry = typename T_REGISTRY;

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
			const double* bound_variable;
			const void*	  bound_function;
		};
		void* context;

		te_registry* registry;
	};

	static inline common::te_expr* new_expr_impl(state* s, const int type, const common::te_expr* parameters[])
	{
		const int arity = common::arity(type);
		const int psize = sizeof(void*) * arity;
		const int size =
			(sizeof(common::te_expr) - sizeof(void*)) + psize + (common::is_closure(type) ? sizeof(void*) : 0);
		common::te_expr* ret = (common::te_expr*)malloc(size);
		memset(ret, 0, size);
		if (arity && parameters)
		{
			memcpy(ret->parameters, parameters, psize);
		}
		ret->type			= type;
		ret->bound_variable = 0; // TODO: clear safety?

		s->registry->track_expr(ret, size);
		return ret;
	}

	static inline common::te_expr* new_expr(state* s, const int type)
	{
		return new_expr_impl(s, type, nullptr);
	}

	template<typename... T_ARGS>
	static inline common::te_expr* new_expr(state* s, const int type, T_ARGS... args)
	{
		const common::te_expr* parameters[] = {args...};
		return new_expr_impl(s, type, parameters);
	}

	static inline int ta_num_parameters_for_type(int type_mask) noexcept
	{
		// assume this order for optimizing the conditions
		static_assert(common::TE_CLOSURE0 >= common::TE_FUNCTION_MAX);

		if (type_mask >= common::TE_FUNCTION1)
		{
			if (type_mask < common::TE_FUNCTION_MAX)
			{
				return (type_mask - static_cast<int>(common::TE_FUNCTION0));
			}
			else if (type_mask > common::TE_CLOSURE0)
			{
				assert(type_mask < common::TE_CLOSURE_MAX);
				return (type_mask - static_cast<int>(common::TE_CLOSURE0));
			}
		}

		return 0;
	}

	static inline void te_free_parameters(common::te_expr* n)
	{
		if (!n)
		{
			return;
		}

		for (int i = ta_num_parameters_for_type(common::type_mask(n->type)); i > 0; --i)
		{
			te_free((common::te_expr*)n->parameters[i - 1]);
		}
	}

	static inline void te_free(common::te_expr* n)
	{
		if (!n)
		{
			return;
		}

		te_free_parameters(n);
		free(n);
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

			// Try reading a number
			if ((s->next[0] >= '0' && s->next[0] <= '9') || s->next[0] == '.')
			{
				s->value = strtod(s->next, (char**)&s->next);
				s->type	 = TOK_NUMBER;
			}
			else
			{
				// Look for a variable or builtin function call.
				if (s->next[0] >= 'a' && s->next[0] <= 'z')
				{
					const char* start;
					start = s->next;
					while ((s->next[0] >= 'a' && s->next[0] <= 'z') || (s->next[0] >= '0' && s->next[0] <= '9') ||
						   (s->next[0] == '_'))
						s->next++;

					const common::te_variable* var =
						s->registry->get_variable(std::string_view(start, s->next - start));

					if (!var)
					{
						s->type = TOK_ERROR;
					}
					else
					{
						switch (common::type_mask(var->type))
						{
						case common::TE_VARIABLE:
							s->type			  = TOK_VARIABLE;
							s->bound_variable = (const double*)var->address;
							break;

						case common::TE_CLOSURE0:
						case common::TE_CLOSURE1:
						case common::TE_CLOSURE2:
						case common::TE_CLOSURE3:
						case common::TE_CLOSURE4:
						case common::TE_CLOSURE5:
						case common::TE_CLOSURE6:
						case common::TE_CLOSURE7:
							s->context = var->closure_context;

						case common::TE_FUNCTION0:
						case common::TE_FUNCTION1:
						case common::TE_FUNCTION2:
						case common::TE_FUNCTION3:
						case common::TE_FUNCTION4:
						case common::TE_FUNCTION5:
						case common::TE_FUNCTION6:
						case common::TE_FUNCTION7:
							s->type			  = var->type;
							s->bound_function = var->address;
							break;
						}
					}
				}
				else
				{
					// Look for an operator or special character.
					switch (s->next++[0])
					{
					case '+':
						s->type			  = TOK_INFIX;
						s->bound_function = s->registry->get_operator("add");
						break;
					case '-':
						s->type			  = TOK_INFIX;
						s->bound_function = s->registry->get_operator("sub");
						break;
					case '*':
						s->type			  = TOK_INFIX;
						s->bound_function = s->registry->get_operator("mul");
						break;
					case '/':
						s->type			  = TOK_INFIX;
						s->bound_function = s->registry->get_operator("divide");
						break;
					case '^':
						s->type			  = TOK_INFIX;
						s->bound_function = s->registry->get_operator("pow");
						break;
					case '%':
						s->type			  = TOK_INFIX;
						s->bound_function = s->registry->get_operator("fmod");
						break;
					case '!':
						if (s->next++[0] == '=')
						{
							s->type			  = TOK_INFIX;
							s->bound_function = s->registry->get_operator("not_equal");
						}
						else
						{
							s->next--;
							s->type			  = TOK_INFIX;
							s->bound_function = s->registry->get_operator("logical_not");
						}
						break;
					case '=':
						if (s->next++[0] == '=')
						{
							s->type			  = TOK_INFIX;
							s->bound_function = s->registry->get_operator("equal");
						}
						else
						{
							s->type = TOK_ERROR;
						}
						break;
					case '<':
						if (s->next++[0] == '=')
						{
							s->type			  = TOK_INFIX;
							s->bound_function = s->registry->get_operator("lower_eq");
						}
						else
						{
							s->next--;
							s->type			  = TOK_INFIX;
							s->bound_function = s->registry->get_operator("lower");
						}
						break;
					case '>':
						if (s->next++[0] == '=')
						{
							s->type			  = TOK_INFIX;
							s->bound_function = s->registry->get_operator("greater_eq");
						}
						else
						{
							s->next--;
							s->type			  = TOK_INFIX;
							s->bound_function = s->registry->get_operator("greater");
						}
						break;
					case '&':
						if (s->next++[0] == '&')
						{
							s->type			  = TOK_INFIX;
							s->bound_function = s->registry->get_operator("logical_and");
						}
						else
						{
							s->type = TOK_ERROR;
						}
						break;
					case '|':
						if (s->next++[0] == '|')
						{
							s->type			  = TOK_INFIX;
							s->bound_function = s->registry->get_operator("logical_or");
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

		switch (common::type_mask(s->type))
		{
		case TOK_NUMBER:
			ret		   = new_expr(s, common::TE_CONSTANT);
			ret->value = s->value;
			next_token(s);
			break;

		case TOK_VARIABLE:
			ret					= new_expr(s, common::TE_VARIABLE);
			ret->bound_variable = s->bound_variable;
			next_token(s);
			break;

		case common::TE_FUNCTION0:
		case common::TE_CLOSURE0:
			ret					= new_expr(s, s->type);
			ret->bound_function = s->bound_function;
			if (common::is_closure(s->type))
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
			ret					= new_expr(s, s->type);
			ret->bound_function = s->bound_function;
			if (common::is_closure(s->type))
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
			arity = common::arity(s->type);

			ret					= new_expr(s, s->type);
			ret->bound_function = s->bound_function;
			if (common::is_closure(s->type))
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
			ret		   = new_expr(s, 0);
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
		while (s->type == TOK_INFIX && (s->bound_function == s->registry->get_operator("add") ||
										   s->bound_function == s->registry->get_operator("sub")))
		{
			if (s->bound_function == s->registry->get_operator("sub"))
				sign = -sign;
			next_token(s);
		}

		int logical = 0;
		while (s->type == TOK_INFIX && (s->bound_function == s->registry->get_operator("add") ||
										   s->bound_function == s->registry->get_operator("sub") ||
										   s->bound_function == s->registry->get_operator("logical_not")))
		{
			if (s->bound_function == s->registry->get_operator("logical_not"))
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
				ret					= new_expr(s, common::TE_FUNCTION1 | common::TE_FLAG_PURE, base(s));
				ret->bound_function = s->registry->get_operator("logical_not");
			}
			else
			{
				ret					= new_expr(s, common::TE_FUNCTION1 | common::TE_FLAG_PURE, base(s));
				ret->bound_function = s->registry->get_operator("logical_notnot");
			}
		}
		else
		{
			if (logical == 0)
			{
				ret					= new_expr(s, common::TE_FUNCTION1 | common::TE_FLAG_PURE, base(s));
				ret->bound_function = s->registry->get_operator("negate");
			}
			else if (logical == -1)
			{
				ret					= new_expr(s, common::TE_FUNCTION1 | common::TE_FLAG_PURE, base(s));
				ret->bound_function = s->registry->get_operator("negate_logical_not");
			}
			else
			{
				ret					= new_expr(s, common::TE_FUNCTION1 | common::TE_FLAG_PURE, base(s));
				ret->bound_function = s->registry->get_operator("negate_logical_notnot");
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
			(ret->bound_function == s->registry->get_operator("negate") ||
				ret->bound_function == s->registry->get_operator("logical_not") ||
				ret->bound_function == s->registry->get_operator("logical_notnot") ||
				ret->bound_function == s->registry->get_operator("negate_logical_not") ||
				ret->bound_function == s->registry->get_operator("negate_logical_notnot")))
		{
			left_function = ret->bound_function;
			te_expr* se	  = ret->parameters[0];
			free(ret);
			ret = se;
		}

		while (s->type == TOK_INFIX && (s->bound_function == pow))
		{
			te_fun2 t = s->bound_function;
			next_token(s);

			if (insertion)
			{
				/* Make exponentiation go right-to-left. */
				te_expr* insert			 = new_expr(s, TE_FUNCTION2 | TE_FLAG_PURE, insertion->parameters[1], power(s));
				insert->bound_function	 = t;
				insertion->parameters[1] = insert;
				insertion				 = insert;
			}
			else
			{
				ret					= new_expr(s, TE_FUNCTION2 | TE_FLAG_PURE, ret, power(s));
				ret->bound_function = t;
				insertion			= ret;
			}
		}

		if (left_function)
		{
			ret					= new_expr(s, TE_FUNCTION1 | TE_FLAG_PURE, ret);
			ret->bound_function = left_function;
		}

		return ret;
	}
#else
	static inline common::te_expr* factor(state* s)
	{
		/* <factor>    =    <power> {"^" <power>} */
		common::te_expr* ret = power(s);

		while (s->type == TOK_INFIX && (s->bound_function == s->registry->get_operator("pow")))
		{
			te_fun2 t = (te_fun2)s->bound_function;
			next_token(s);
			ret					= new_expr(s, common::TE_FUNCTION2 | common::TE_FLAG_PURE, ret, power(s));
			ret->bound_function = t;
		}

		return ret;
	}
#endif

	static inline common::te_expr* term(state* s)
	{
		/* <term>      =    <factor> {("*" | "/" | "%") <factor>} */
		common::te_expr* ret = factor(s);

		while (s->type == TOK_INFIX && (s->bound_function == s->registry->get_operator("mul") ||
										   s->bound_function == s->registry->get_operator("divide") ||
										   s->bound_function == s->registry->get_operator("fmod")))
		{
			te_fun2 t = (te_fun2)s->bound_function;
			next_token(s);
			ret					= new_expr(s, common::TE_FUNCTION2 | common::TE_FLAG_PURE, ret, factor(s));
			ret->bound_function = t;
		}

		return ret;
	}

	static inline common::te_expr* sum_expr(state* s)
	{
		/* <expr>      =    <term> {("+" | "-") <term>} */
		common::te_expr* ret = term(s);

		while (s->type == TOK_INFIX && (s->bound_function == s->registry->get_operator("add") ||
										   s->bound_function == s->registry->get_operator("sub")))
		{
			te_fun2 t = (te_fun2)s->bound_function;
			next_token(s);
			ret					= new_expr(s, common::TE_FUNCTION2 | common::TE_FLAG_PURE, ret, term(s));
			ret->bound_function = t;
		}

		return ret;
	}

	static inline common::te_expr* test_expr(state* s)
	{
		/* <expr>      =    <sum_expr> {(">" | ">=" | "<" | "<=" | "==" | "!=") <sum_expr>} */
		common::te_expr* ret = sum_expr(s);

		while (s->type == TOK_INFIX && (s->bound_function == s->registry->get_operator("greater") ||
										   s->bound_function == s->registry->get_operator("greater_eq") ||
										   s->bound_function == s->registry->get_operator("lower") ||
										   s->bound_function == s->registry->get_operator("lower_eq") ||
										   s->bound_function == s->registry->get_operator("equal") ||
										   s->bound_function == s->registry->get_operator("not_equal")))
		{
			te_fun2 t = (te_fun2)s->bound_function;
			next_token(s);
			ret					= new_expr(s, common::TE_FUNCTION2 | common::TE_FLAG_PURE, ret, sum_expr(s));
			ret->bound_function = t;
		}

		return ret;
	}

	static inline common::te_expr* expr(state* s)
	{
		/* <expr>      =    <test_expr> {("&&" | "||") <test_expr>} */
		common::te_expr* ret = test_expr(s);

		while (s->type == TOK_INFIX && (s->bound_function == s->registry->get_operator("logical_and") ||
										   s->bound_function == s->registry->get_operator("logical_or")))
		{
			te_fun2 t = (te_fun2)s->bound_function;
			next_token(s);
			ret					= new_expr(s, common::TE_FUNCTION2 | common::TE_FLAG_PURE, ret, test_expr(s));
			ret->bound_function = t;
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
			ret					= new_expr(s, common::TE_FUNCTION2 | common::TE_FLAG_PURE, ret, expr(s));
			ret->bound_function = s->registry->get_operator("comma");
		}

		return ret;
	}

	static inline void optimize(te_registry* registry, common::te_expr* n)
	{
		/* Evaluates as much as possible. */
		if (n->type == common::TE_CONSTANT)
			return;
		if (n->type == common::TE_VARIABLE)
			return;

		/* Only optimize out functions flagged as pure. */
		if (common::is_pure(n->type))
		{
			const int arity = common::arity(n->type);
			int		  known = 1;
			int		  i;
			for (i = 0; i < arity; ++i)
			{
				optimize(registry, (common::te_expr*)n->parameters[i]);
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

	static inline common::te_expr* te_compile(te_registry* registry, const char* expression, int* error)
	{
		state s;
		s.start = s.next = expression;
		s.registry		 = registry;

		next_token(&s);
		common::te_expr* root = list(&s);

		if (s.type != TOK_END)
		{
			te_free(root);
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
			optimize(registry, root);
			if (error)
				*error = 0;
			return root;
		}
	}

	static inline void pn(te_registry* registry, const common::te_expr* n, int depth, size_t& total_size)
	{
		int i, arity;
		const auto expr_size = registry->lookup_size(n);
		total_size += expr_size;

		printf("Expression: %p\n", n);
		printf("\tName: %s\n\tSize: %lld\n", registry->lookup_name(n).data(), expr_size);

		switch (common::type_mask(n->type))
		{
		case common::TE_CONSTANT:
			printf("\tType: constant\n\t\tValue: %f\n", n->value);
			// nothing needed, the value can be used as-is
			break;
		
		case common::TE_VARIABLE:
			printf("\tType: variable\n\t\tAddress: %p\n\t\tName: %s\n", n->bound_variable, registry->lookup_name(n->bound_variable).data());
			// add variable address/name to remapping table
			break;

		case common::TE_FUNCTION0:
		case common::TE_FUNCTION1:
		case common::TE_FUNCTION2:
		case common::TE_FUNCTION3:
		case common::TE_FUNCTION4:
		case common::TE_FUNCTION5:
		case common::TE_FUNCTION6:
		case common::TE_FUNCTION7:
			arity = common::arity(n->type);
			printf("\tType: function\n\t\tAddress: %p\n\t\tName: %s\n\t\tArgs: %d\n",
				n->bound_function, registry->lookup_name(n->bound_function).data(),
				arity);

			// Add function address/name to remapping table

			for (i = 0; i < arity; i++)
			{
				printf("\t\t\tAddress: %p\n\t\t\tName: %s\n", n->parameters[i], registry->lookup_name(n->parameters[i]).data());
			}
			for (i = 0; i < arity; i++)
			{
				pn(registry, (const common::te_expr*)n->parameters[i], depth + 1, total_size);
			}
			break;

		case common::TE_CLOSURE0:
		case common::TE_CLOSURE1:
		case common::TE_CLOSURE2:
		case common::TE_CLOSURE3:
		case common::TE_CLOSURE4:
		case common::TE_CLOSURE5:
		case common::TE_CLOSURE6:
		case common::TE_CLOSURE7:
			arity = common::arity(n->type);
			printf("\tType: closure\n\t\tAddress: %p\n\t\tName: %s\n\t\tArgs: %d\n",
				n->bound_function,
				registry->lookup_name(n->bound_function).data(),
				arity);
			for (i = 0; i < arity; i++)
			{
				printf("\t\t\tAddress: %p\n\t\t\tName: %s\n",
					n->parameters[i],
					registry->lookup_name(n->parameters[i]).data());
			}
			for (i = 0; i < arity; i++)
			{
				pn(registry, (const common::te_expr*)n->parameters[i], depth + 1, total_size);
			}
			break;
		
		default:
			assert(0);
		}
	}

	static inline size_t te_print(te_registry* registry, const common::te_expr* n)
	{
		size_t total_size = 0;
		pn(registry, n, 0, total_size);
		return total_size;
	}
};

template<typename T_VARIABLE>
struct tinyexpr_registry
{
	using te_variable = typename T_VARIABLE;
	using details	  = tinyexpr_details;
	using defines	  = tinyexpr_defines;
	using common	  = tinyexpr_common;

	std::vector<te_variable> m_variable_cache;

	auto find_variable(const te_variable& v)
	{
		return std::find_if(m_variable_cache.begin(), m_variable_cache.end(), [&v](const te_variable& existing) {
			if (v.name.size() == existing.name.size())
			{
				if (_strnicmp(v.name.data(), existing.name.data(), v.name.size()) == 0)
				{
					assert(v.address == existing.address);
					return true;
				}
			}
			return false;
		});
	}

	auto find_variable(std::string_view v_name)
	{
		return std::find_if(m_variable_cache.begin(), m_variable_cache.end(), [&v_name](const te_variable& existing) {
			if (v_name.size() == existing.name.size())
			{
				if (_strnicmp(v_name.data(), existing.name.data(), v_name.size()) == 0)
				{
					return true;
				}
			}
			return false;
		});
	}

	auto find_variable(std::string_view v_name) const
	{
		return std::find_if(m_variable_cache.cbegin(), m_variable_cache.cend(), [&v_name](const te_variable& existing) {
			if (v_name.size() == existing.name.size())
			{
				if (_strnicmp(v_name.data(), existing.name.data(), v_name.size()) == 0)
				{
					return true;
				}
			}
			return false;
		});
	}

	void register_variable(const te_variable& var)
	{
		auto itor = find_variable(var);
		if (itor == m_variable_cache.end())
		{
			m_variable_cache.push_back(var);
		}
	}

	void release_variable(std::string_view var)
	{
		auto itor = find_variable(var);
		if (itor != m_variable_cache.end())
		{
			m_variable_cache.erase(itor);
		}
	}

	te_variable* get_variables()
	{
		return m_variable_cache.size() > 0 ? &m_variable_cache[0] : 0;
	}

	const te_variable* get_variables() const
	{
		return m_variable_cache.size() > 0 ? &m_variable_cache[0] : 0;
	}

	int get_var_count() const
	{
		return static_cast<int>(m_variable_cache.size());
	}

	const void* get_operator(std::string_view name)
	{
		std::tuple<std::string_view, const void*> functions[] = {
			{"add", details::add},
			{"sub", details::sub},
			{"mul", details::mul},
			{"divide", details::divide},
			{"pow", details::pow},
			{"fmod", details::fmod},
			{"not_equal", details::not_equal},
			{"logical_not", details::logical_not},
			{"logical_notnot", details::logical_notnot},
			{"equal", details::equal},
			{"lower_eq", details::lower_eq},
			{"lower", details::lower},
			{"greater_eq", details::greater_eq},
			{"greater", details::greater},
			{"logical_and", details::logical_and},
			{"logical_or", details::logical_or},
			{"comma", details::comma},
			{"negate", details::negate},
			{"negate_logical_not", details::negate_logical_not},
			{"negate_logical_notnot", details::negate_logical_notnot},

		};

		auto functions_end = &functions[(sizeof(functions) / sizeof(std::tuple<const char*, const void*>)) - 1];

		auto itor = std::find_if(&functions[0], functions_end, [&name](auto& existing) {
			if (name.length() == std::get<0>(existing).length())
			{
				if (_strnicmp(name.data(), std::get<0>(existing).data(), name.length()) == 0)
				{
					return true;
				}
			}
			return false;
		});

		auto res = (itor != functions_end) ? std::get<1>(*itor) : 0;
		assert(res != 0);
		
		auto name_itor = m_name_map.find(res);
		if (name_itor == m_name_map.end())
		{
			m_name_map.emplace(std::make_pair(res, std::string(name)));
		}
		
		return res;
	}

	const te_variable* get_builtin(std::string_view name) const
	{
		static const te_variable functions[] = {
			{"abs", details::fabs, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
			{"acos", details::acos, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
			{"asin", details::asin, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
			{"atan", details::atan, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
			{"atan2", details::atan2, defines::TE_FUNCTION2 | defines::TE_FLAG_PURE, 0},
			{"ceil", details::ceil, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
			{"cos", details::cos, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
			{"cosh", details::cosh, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
			{"e", details::e, defines::TE_FUNCTION0 | defines::TE_FLAG_PURE, 0},
			{"exp", details::exp, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
			{"fac", details::fac, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
			{"floor", details::floor, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
			{"ln", details::log, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
#ifdef TE_NAT_LOG
			{"log", details::log, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
#else
			{"log", details::log10, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
#endif
			{"log10", details::log10, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
			{"ncr", details::ncr, defines::TE_FUNCTION2 | defines::TE_FLAG_PURE, 0},
			{"npr", details::npr, defines::TE_FUNCTION2 | defines::TE_FLAG_PURE, 0},
			{"pi", details::pi, defines::TE_FUNCTION0 | defines::TE_FLAG_PURE, 0},
			{"pow", details::pow, defines::TE_FUNCTION2 | defines::TE_FLAG_PURE, 0},
			{"sin", details::sin, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
			{"sinh", details::sinh, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
			{"sqrt", details::sqrt, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
			{"tan", details::tan, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
			{"tanh", details::tanh, defines::TE_FUNCTION1 | defines::TE_FLAG_PURE, 0},
			{"", 0, 0, 0}};

		auto functions_end = &functions[(sizeof(functions) / sizeof(te_variable)) - 1];

		auto itor = std::find_if(&functions[0], functions_end, [&name](const te_variable& existing) {
			if (name.length() == existing.name.length())
			{
				if (_strnicmp(name.data(), existing.name.data(), name.length()) == 0)
				{
					return true;
				}
			}
			return false;
		});

		return (itor != functions_end) ? itor : 0;
	}

	std::unordered_map<const void*, std::string> m_name_map;
	std::unordered_map<const void*, size_t> m_size_map;

	const te_variable* get_variable(std::string_view name)
	{
		auto itor = find_variable(name);
		if (itor != m_variable_cache.end())
		{
			auto var = &(*itor);

			auto itor = m_name_map.find(var->address);
			if (itor == m_name_map.end())
			{
				m_name_map.emplace(std::make_pair(var->address, std::string(name)));
			}
			return var;
		}

		auto var = get_builtin(name);

		if (var)
		{
			auto itor = m_name_map.find(var->address);
			if (itor == m_name_map.end())
			{
				m_name_map.emplace(std::make_pair(var->address, std::string(name)));
			}
		}
		return var;
	}

	std::string_view lookup_name(const void* addr)
	{
		auto itor = m_name_map.find(addr);
		if (itor != m_name_map.end())
		{
			return itor->second;
		}
		return "nope";
	}

	size_t lookup_size(const void* addr) 
	{
		auto itor = m_size_map.find(addr);
		if (itor != m_size_map.end())
		{
			return itor->second;
		}
		return 0;
	}

	void track_expr(void* expr, size_t size) 
	{
		auto itor = m_name_map.find(expr);
		if (itor == m_name_map.end())
		{
			m_name_map.emplace(std::make_pair(expr, std::string("te_expr")));
			m_size_map.insert(std::make_pair(expr, size));
		}
	}
};

struct tinyexpr : public tinyexpr_defines
{
	using common = tinyexpr_common;

	using te_variable = common::te_variable;

	struct te_expr
	{
		std::string		 m_expression_src;
		common::te_expr* m_expr;
	};

	using te_registry = tinyexpr_registry<te_variable>;
	using details	  = tinyexpr_details;
	using eval		  = tinyexpr_eval;
	using compiler	  = tinyexpr_compiler<te_registry>;

	te_registry m_registry;

	template<typename... T_VARS>
	void register_variables(T_VARS... vars)
	{
		(m_registry.register_variable(vars), ...);
	}

	void register_variables(std::initializer_list<te_variable> vars)
	{
		for (const auto& v : vars)
		{
			m_registry.register_variable(v);
		}
	}

	template<typename... T_VARS>
	void release_variables(T_VARS... vars)
	{
		(m_registry.release_variable(vars), ...);
	}

	void release_variables(std::initializer_list<std::string_view> vars)
	{
		for (const auto& v : vars)
		{
			m_registry.release_variable(v);
		}
	}

	inline te_expr* te_compile(const char* expression, int* error)
	{
		common::te_expr* raw_expr = compiler::te_compile(&m_registry, expression, error);
		if (raw_expr)
		{
			return new te_expr {expression, raw_expr};
		}
		return nullptr;
	}

	inline void te_print(const te_expr* expr)
	{
		compiler::te_print(&m_registry, expr->m_expr);
	}

	inline double te_interp(const char* expression, int* error)
	{
		auto   n = te_compile(expression, error);
		double ret;
		if (n)
		{
			ret = eval::te_eval(n->m_expr);
			te_free(n);
		}
		else
		{
			ret = details::nan;
		}
		return ret;
	}

	inline double te_eval(const te_expr* n)
	{
		printf("\n***************************************\n");
		printf("Evaluating expression:\n");
		printf("%s\n\n", n->m_expression_src.c_str());
		auto total_size = compiler::te_print(&m_registry, n->m_expr);

		printf("Total size: %4lld***********************\n", total_size);
		return eval::te_eval(n->m_expr);
	}

	inline void te_free(te_expr* n)
	{
		compiler::te_free(n->m_expr);
		delete n;
	}
};

#endif /*__TINYEXPR_H__*/
