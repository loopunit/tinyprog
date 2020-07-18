/*
 * TINYEXPR - Tiny recursive descent parser and evaluation engine in C
 *
 * Copyright (c) 2020 Nathan Rausch
 * http://loopunit.com
 *
 * Copyright (c) 2015-2018 Lewis Van Winkle
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

namespace te
{
#if (TE_COMPILER_ENABLED)
	template<typename T_VECTOR>
	struct compiler_builtins : native_builtins<T_VECTOR>
	{
		using t_impl = native_builtins<T_VECTOR>;

		static inline const variable* find_function_by_addr(const void* addr)
		{
			for (auto var = &t_impl::functions[0]; var->name != 0; ++var)
			{
				if (var->address == addr)
				{
					return var;
				}
			}
			return nullptr;
		}

		static inline const variable* find_operator_by_addr(const void* addr)
		{
			for (auto var = &t_impl::operators[0]; var->name != 0; ++var)
			{
				if (var->address == addr)
				{
					return var;
				}
			}
			return nullptr;
		}

		static inline const variable* find_any_by_addr(const void* addr)
		{
			const variable* var = find_function_by_addr(addr);
			if (!var)
			{
				var = find_operator_by_addr(addr);
				if (!var)
				{
					return t_impl::find_builtin("nul");
				}
			}
			return var;
		}
	};

	template<typename T_TRAITS>
	struct native
	{
		using t_traits	 = T_TRAITS;
		using t_atom	 = typename T_TRAITS::t_atom;
		using t_vector	 = typename T_TRAITS::t_vector;
		using t_builtins = compiler_builtins<t_vector>;

		struct expr_native
		{
			int type;
			union
			{
				t_atom		  value;
				const t_atom* bound;
				const void*	  function;
			};
			void* parameters[1];
		};

#	ifndef INFINITY
#		define INFINITY (1.0 / 0.0)
#	endif

		typedef t_vector (*te_fun2)(t_vector, t_vector);

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
				t_atom		  value;
				const t_atom* bound;
				const void*	  function;
			};
			void* context;

			const variable* lookup;
			int				lookup_len;
		};

#	define IS_PURE(TYPE)	  (((TYPE)&TE_FLAG_PURE) != 0)
#	define IS_FUNCTION(TYPE) (((TYPE)&TE_FUNCTION0) != 0)
#	define IS_CLOSURE(TYPE)  (((TYPE)&TE_CLOSURE0) != 0)
#	define NEW_EXPR(type, ...)                                                                                        \
		[&]() {                                                                                                        \
			const expr_native* _args[] = {__VA_ARGS__};                                                             \
			return new_expr((type), _args);                                                                            \
		}()

		static expr_native* new_expr(const int type, const expr_native* parameters[])
		{
			const int arity = eval_details::arity(type);
			const int psize = sizeof(void*) * arity;
			const int size	= (sizeof(expr_native) - sizeof(void*)) + psize + (IS_CLOSURE(type) ? sizeof(void*) : 0);
			expr_native* ret   = (expr_native*)malloc(size);
			memset(ret, 0, size);
			if (arity && parameters)
			{
				memcpy(ret->parameters, parameters, psize);
			}
			ret->type  = type;
			ret->bound = 0;
			return ret;
		}

		static void free_parameters(expr_native* n)
		{
			if (!n)
				return;
			switch (eval_details::type_mask(n->type))
			{
			case TE_FUNCTION7:
			case TE_CLOSURE7:
				free_native((expr_native*)n->parameters[6]); /* Falls through. */
			case TE_FUNCTION6:
			case TE_CLOSURE6:
				free_native((expr_native*)n->parameters[5]); /* Falls through. */
			case TE_FUNCTION5:
			case TE_CLOSURE5:
				free_native((expr_native*)n->parameters[4]); /* Falls through. */
			case TE_FUNCTION4:
			case TE_CLOSURE4:
				free_native((expr_native*)n->parameters[3]); /* Falls through. */
			case TE_FUNCTION3:
			case TE_CLOSURE3:
				free_native((expr_native*)n->parameters[2]); /* Falls through. */
			case TE_FUNCTION2:
			case TE_CLOSURE2:
				free_native((expr_native*)n->parameters[1]); /* Falls through. */
			case TE_FUNCTION1:
			case TE_CLOSURE1:
				free_native((expr_native*)n->parameters[0]);
			}
		}

		static void free_native(expr_native* n)
		{
			if (!n)
				return;
			free_parameters(n);
			free(n);
		}

		static const variable* find_lookup(const variable* lookup, int lookup_len, const char* name, int len)
		{
			if (!lookup)
				return 0;

			const variable* var	  = lookup;
			int				iters = lookup_len;
			for (; iters; ++var, --iters)
			{
				if (strncmp(name, var->name, len) == 0 && var->name[len] == '\0')
				{
					return var;
				}
			}
			return 0;
		}

		static const variable* find_lookup(const state* s, const char* name, int len)
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
					s->value = (t_atom)strtod(s->next, (char**)&s->next);
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

						const variable* var = find_lookup(s, start, static_cast<int>(s->next - start));
						if (!var)
							var = t_builtins::find_builtin(start, static_cast<int>(s->next - start));

						if (!var)
						{
							s->type = TOK_ERROR;
						}
						else
						{
							switch (eval_details::type_mask(var->type))
							{
							case TE_VARIABLE:
								s->type	 = TOK_VARIABLE;
								s->bound = (const t_atom*)var->address;
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
							s->function = t_builtins::find_builtin_address("add");
							break;
						case '-':
							s->type		= TOK_INFIX;
							s->function = t_builtins::find_builtin_address("sub");
							break;
						case '*':
							s->type		= TOK_INFIX;
							s->function = t_builtins::find_builtin_address("mul");
							break;
						case '/':
							s->type		= TOK_INFIX;
							s->function = t_builtins::find_builtin_address("divide");
							break;
						case '^':
							s->type		= TOK_INFIX;
							s->function = t_builtins::find_builtin_address("pow");
							break;
						case '%':
							s->type		= TOK_INFIX;
							s->function = t_builtins::find_builtin_address("fmod");
							break;
						case '!':
							if (s->next++[0] == '=')
							{
								s->type		= TOK_INFIX;
								s->function = t_builtins::find_builtin_address("not_equal");
							}
							else
							{
								s->next--;
								s->type		= TOK_INFIX;
								s->function = t_builtins::find_builtin_address("logical_not");
							}
							break;
						case '=':
							if (s->next++[0] == '=')
							{
								s->type		= TOK_INFIX;
								s->function = t_builtins::find_builtin_address("equal");
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
								s->function = t_builtins::find_builtin_address("lower_eq");
							}
							else
							{
								s->next--;
								s->type		= TOK_INFIX;
								s->function = t_builtins::find_builtin_address("lower");
							}
							break;
						case '>':
							if (s->next++[0] == '=')
							{
								s->type		= TOK_INFIX;
								s->function = t_builtins::find_builtin_address("greater_eq");
							}
							else
							{
								s->next--;
								s->type		= TOK_INFIX;
								s->function = t_builtins::find_builtin_address("greater");
							}
							break;
						case '&':
							if (s->next++[0] == '&')
							{
								s->type		= TOK_INFIX;
								s->function = t_builtins::find_builtin_address("logical_and");
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
								s->function = t_builtins::find_builtin_address("logical_or");
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

		static expr_native* base(state* s)
		{
			/* <base>      =    <constant> | <variable> | <function-0> {"(" ")"} | <function-1> <power> | <function-X>
			 * "(" <expr> {"," <expr>} ")" | "(" <list> ")" */
			expr_native* ret;
			int				arity;

			switch (eval_details::type_mask(s->type))
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
				arity = eval_details::arity(s->type);

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
				ret->value = t_builtins::nan();
				break;
			}

			return ret;
		}

		static expr_native* power(state* s)
		{
			/* <power>     =    {("-" | "+" | "!")} <base> */
			int sign = 1;
			while (s->type == TOK_INFIX && (s->function == t_builtins::find_builtin_address("add") ||
											   s->function == t_builtins::find_builtin_address("sub")))
			{
				if (s->function == t_builtins::find_builtin_address("sub"))
					sign = -sign;
				next_token(s);
			}

			int logical = 0;
			while (s->type == TOK_INFIX && (s->function == t_builtins::find_builtin_address("add") ||
											   s->function == t_builtins::find_builtin_address("sub") ||
											   s->function == t_builtins::find_builtin_address("logical_not")))
			{
				if (s->function == t_builtins::find_builtin_address("logical_not"))
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

			expr_native* ret;

			if (sign == 1)
			{
				if (logical == 0)
				{
					ret = base(s);
				}
				else if (logical == -1)
				{
					ret			  = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, base(s));
					ret->function = t_builtins::find_builtin_address("logical_not");
				}
				else
				{
					ret			  = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, base(s));
					ret->function = t_builtins::find_builtin_address("logical_notnot");
				}
			}
			else
			{
				if (logical == 0)
				{
					ret			  = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, base(s));
					ret->function = t_builtins::find_builtin_address("negate");
				}
				else if (logical == -1)
				{
					ret			  = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, base(s));
					ret->function = t_builtins::find_builtin_address("negate_logical_not");
				}
				else
				{
					ret			  = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, base(s));
					ret->function = t_builtins::find_builtin_address("negate_logical_notnot");
				}
			}

			return ret;
		}

#	ifdef TE_POW_FROM_RIGHT
		static expr_native* factor(state* s)
		{
			/* <factor>    =    <power> {"^" <power>} */
			expr_native* ret = power(s);

			const void*		left_function = NULL;
			expr_native* insertion	   = 0;

			if (ret->type == (TE_FUNCTION1 | TE_FLAG_PURE) &&
				(ret->function == t_builtins::find_builtin_address("negate") ||
					ret->function == t_builtins::find_builtin_address("logical_not") ||
					ret->function == t_builtins::find_builtin_address("logical_notnot") ||
					ret->function == t_builtins::find_builtin_address("negate_logical_not") ||
					ret->function == t_builtins::find_builtin_address("negate_logical_notnot")))
			{
				left_function	   = ret->function;
				expr_native* se = ret->parameters[0];
				free(ret);
				ret = se;
			}

			while (s->type == TOK_INFIX && (s->function == t_builtins::find_builtin_address("pow")))
			{
				te_fun2 t = s->function;
				next_token(s);

				if (insertion)
				{
					/* Make exponentiation go right-to-left. */
					expr_native* insert = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, insertion->parameters[1], power(s));
					insert->function	   = t;
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
#	else
		static expr_native* factor(state* s)
		{
			/* <factor>    =    <power> {"^" <power>} */
			expr_native* ret = power(s);

			while (s->type == TOK_INFIX && (s->function == t_builtins::find_builtin_address("pow")))
			{
				te_fun2 t = (te_fun2)s->function;
				next_token(s);
				ret			  = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, power(s));
				ret->function = t;
			}

			return ret;
		}
#	endif

		static expr_native* term(state* s)
		{
			/* <term>      =    <factor> {("*" | "/" | "%") <factor>} */
			expr_native* ret = factor(s);

			while (s->type == TOK_INFIX && (s->function == t_builtins::find_builtin_address("mul") ||
											   s->function == t_builtins::find_builtin_address("divide") ||
											   s->function == t_builtins::find_builtin_address("fmod")))
			{
				te_fun2 t = (te_fun2)s->function;
				next_token(s);
				ret			  = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, factor(s));
				ret->function = t;
			}

			return ret;
		}

		static expr_native* sum_expr(state* s)
		{
			/* <expr>      =    <term> {("+" | "-") <term>} */
			expr_native* ret = term(s);

			while (s->type == TOK_INFIX && (s->function == t_builtins::find_builtin_address("add") ||
											   s->function == t_builtins::find_builtin_address("sub")))
			{
				te_fun2 t = (te_fun2)s->function;
				next_token(s);
				ret			  = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, term(s));
				ret->function = t;
			}

			return ret;
		}

		static expr_native* test_expr(state* s)
		{
			/* <expr>      =    <sum_expr> {(">" | ">=" | "<" | "<=" | "==" | "!=") <sum_expr>} */
			expr_native* ret = sum_expr(s);

			while (s->type == TOK_INFIX && (s->function == t_builtins::find_builtin_address("greater") ||
											   s->function == t_builtins::find_builtin_address("greater_eq") ||
											   s->function == t_builtins::find_builtin_address("lower") ||
											   s->function == t_builtins::find_builtin_address("lower_eq") ||
											   s->function == t_builtins::find_builtin_address("equal") ||
											   s->function == t_builtins::find_builtin_address("not_equal")))
			{
				te_fun2 t = (te_fun2)s->function;
				next_token(s);
				ret			  = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, sum_expr(s));
				ret->function = t;
			}

			return ret;
		}

		static expr_native* expr(state* s)
		{
			/* <expr>      =    <test_expr> {("&&" | "||") <test_expr>} */
			expr_native* ret = test_expr(s);

			while (s->type == TOK_INFIX && (s->function == t_builtins::find_builtin_address("logical_and") ||
											   s->function == t_builtins::find_builtin_address("logical_or")))
			{
				te_fun2 t = (te_fun2)s->function;
				next_token(s);
				ret			  = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, test_expr(s));
				ret->function = t;
			}

			return ret;
		}

		static expr_native* list(state* s)
		{
			/* <list>      =    <expr> {"," <expr>} */
			expr_native* ret = expr(s);

			while (s->type == TOK_SEP)
			{
				next_token(s);
				ret			  = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, expr(s));
				ret->function = t_builtins::find_builtin_address("comma");
			}

			return ret;
		}

		static t_vector eval_native(const expr_native* n)
		{
#	define TE_FUN(...) ((t_vector(*)(__VA_ARGS__))n->function)
#	define M(e)		eval_native((const expr_native*)n->parameters[e])

			if (!n)
				return t_builtins::nan();

			switch (eval_details::type_mask(n->type))
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
				switch (eval_details::arity(n->type))
				{
				case 0:
					return TE_FUN(void)();
				case 1:
					return TE_FUN(t_vector)(M(0));
				case 2:
					return TE_FUN(t_vector, t_vector)(M(0), M(1));
				case 3:
					return TE_FUN(t_vector, t_vector, t_vector)(M(0), M(1), M(2));
				case 4:
					return TE_FUN(t_vector, t_vector, t_vector, t_vector)(M(0), M(1), M(2), M(3));
				case 5:
					return TE_FUN(t_vector, t_vector, t_vector, t_vector, t_vector)(M(0), M(1), M(2), M(3), M(4));
				case 6:
					return TE_FUN(t_vector, t_vector, t_vector, t_vector, t_vector, t_vector)(
						M(0), M(1), M(2), M(3), M(4), M(5));
				case 7:
					return TE_FUN(t_vector, t_vector, t_vector, t_vector, t_vector, t_vector, t_vector)(
						M(0), M(1), M(2), M(3), M(4), M(5), M(6));
				default:
					return t_builtins::nan();
				}

			case TE_CLOSURE0:
			case TE_CLOSURE1:
			case TE_CLOSURE2:
			case TE_CLOSURE3:
			case TE_CLOSURE4:
			case TE_CLOSURE5:
			case TE_CLOSURE6:
			case TE_CLOSURE7:
				switch (eval_details::arity(n->type))
				{
				case 0:
					return TE_FUN(void*)(n->parameters[0]);
				case 1:
					return TE_FUN(void*, t_vector)(n->parameters[1], M(0));
				case 2:
					return TE_FUN(void*, t_vector, t_vector)(n->parameters[2], M(0), M(1));
				case 3:
					return TE_FUN(void*, t_vector, t_vector, t_vector)(n->parameters[3], M(0), M(1), M(2));
				case 4:
					return TE_FUN(void*, t_vector, t_vector, t_vector, t_vector)(
						n->parameters[4], M(0), M(1), M(2), M(3));
				case 5:
					return TE_FUN(void*, t_vector, t_vector, t_vector, t_vector, t_vector)(
						n->parameters[5], M(0), M(1), M(2), M(3), M(4));
				case 6:
					return TE_FUN(void*, t_vector, t_vector, t_vector, t_vector, t_vector, t_vector)(
						n->parameters[6], M(0), M(1), M(2), M(3), M(4), M(5));
				case 7:
					return TE_FUN(void*, t_vector, t_vector, t_vector, t_vector, t_vector, t_vector, t_vector)(
						n->parameters[7], M(0), M(1), M(2), M(3), M(4), M(5), M(6));
				default:
					return t_builtins::nan();
				}

			default:
				return t_builtins::nan();
			}

#	undef TE_FUN
#	undef M
		}

		static void optimize(expr_native* n)
		{
			/* Evaluates as much as possible. */
			if (n->type == TE_CONSTANT)
				return;
			if (n->type == TE_VARIABLE)
				return;

			/* Only optimize out functions flagged as pure. */
			if (IS_PURE(n->type))
			{
				const int arity = eval_details::arity(n->type);
				int		  known = 1;
				int		  i;
				for (i = 0; i < arity; ++i)
				{
					optimize((expr_native*)n->parameters[i]);
					if (((expr_native*)(n->parameters[i]))->type != TE_CONSTANT)
					{
						known = 0;
					}
				}
				if (known)
				{
					const t_vector value = eval_native(n);
					free_parameters(n);
					n->type	 = TE_CONSTANT;
					n->value = value;
				}
			}
		}

		static expr_native* compile_native(
			const char* expression, const variable* variables, int var_count, int* error)
		{
			state s;
			s.start = s.next = expression;
			s.lookup		 = variables;
			s.lookup_len	 = var_count;

			next_token(&s);
			expr_native* root = list(&s);

			if (s.type != TOK_END)
			{
				free_native(root);
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

		static t_vector interp_native(const char* expression, int* error)
		{
			expr_native* n = compile_native(expression, 0, 0, error);
			t_vector		ret;
			if (n)
			{
				ret = eval_native(n);
				free_native(n);
			}
			else
			{
				ret = t_traits::nan();
			}
			return ret;
		}

		static void pn(const expr_native* n, int depth)
		{
			int i, arity;
			printf("%*s", depth, "");

			switch (type_mask(n->type))
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
				arity = arity(n->type);
				printf("f%d", arity);
				for (i = 0; i < arity; i++)
				{
					printf(" %p", n->parameters[i]);
				}
				printf("\n");
				for (i = 0; i < arity; i++)
				{
					pn((const expr_native*)n->parameters[i], depth + 1);
				}
				break;
			}
		}

		static void print(const expr_native* n)
		{
			pn(n, 0);
		}

		////

		static const variable* find_bind_by_addr(const void* addr, const variable* lookup, int lookup_len)
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

		static const variable* find_closure_by_addr(const void* addr, const variable* lookup, int lookup_len)
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

		static const variable* find_bind_or_any_by_addr(const void* addr, const variable* lookup, int lookup_len)
		{
			auto res = t_builtins::find_any_by_addr(addr);
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

	template<typename T_TRAITS>
	struct portable
	{
		using t_traits		 = T_TRAITS;
		using t_atom		 = typename T_TRAITS::t_atom;
		using t_vector		 = typename T_TRAITS::t_vector;
		using expr_native = typename native<t_traits>::expr_native;

		using name_map = std::unordered_map<const void*, std::string>;

		using index_map = std::unordered_map<const void*, int>;

		struct expr_portable_expression_build_indexer
		{
			name_map	 name_map;
			index_map index_map;
			int			 index_counter = 0;
		};

		struct expr_portable_expression_build_bindings
		{
			std::vector<const void*>
									 index_to_address; // this contains the native function/value address as originally compiled
			std::vector<std::string> index_to_name;
			std::vector<const char*> index_to_name_c_str;
		};

		struct compiled_expr
		{
			expr_portable_expression_build_indexer	m_indexer;
			expr_portable_expression_build_bindings m_bindings;
			std::unique_ptr<unsigned char>			m_build_buffer;
			size_t									m_build_buffer_size;
		};

		static size_t export_estimate(const expr_native* n,
			size_t&											   export_size,
			const variable*									   lookup,
			int												   lookup_len,
			name_map&									   name_map,
			index_map&									   index_map,
			int&											   index_counter)
		{
#	define M(e)                                                                                                       \
		export_estimate(                                                                                            \
			(const expr_native*)n->parameters[e],                                                    \
			export_size,                                                                                               \
			lookup,                                                                                                    \
			lookup_len,                                                                                                \
			name_map,                                                                                                  \
			index_map,                                                                                                 \
			index_counter)

			if (!n)
				return export_size;

			export_size += sizeof(expr_native);

			auto handle_addr = [&](const variable* var) -> bool {
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

			switch (eval_details::type_mask(n->type))
			{
			case TE_CONSTANT:
			{
				return export_size;
			}

			case TE_VARIABLE:
			{
				auto res = handle_addr(native<t_traits>::find_bind_by_addr(n->bound, lookup, lookup_len));
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
				auto res = handle_addr(native<t_traits>::find_bind_or_any_by_addr(n->function, lookup, lookup_len));
				assert(res);
				export_size += sizeof(n->parameters[0]) * (eval_details::arity(n->type));

				for (int i = 0; i < eval_details::arity(n->type); ++i)
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
				auto res = handle_addr(native<t_traits>::find_bind_or_any_by_addr(n->function, lookup, lookup_len));
				assert(res);

				export_size += sizeof(n->parameters[0]) * eval_details::arity(n->type);

				for (int i = 0; i < eval_details::arity(n->type); ++i)
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
		static size_t export_write(const expr_native* n,
			size_t&											export_size,
			const variable*									lookup,
			int												lookup_len,
			const unsigned char*							out_buffer,
			T_REGISTER_FUNC									register_func)
		{
#	define M(e)                                                                                                       \
		export_write(                                                                                               \
			(const expr_native*)n->parameters[e], export_size, lookup, lookup_len, out_buffer, register_func)

			if (!n)
				return export_size;

			auto n_out = (expr_portable<t_traits>*)(out_buffer + export_size);

			export_size += sizeof(expr_native);
			n_out->type = n->type;
			switch (eval_details::type_mask(n->type))
			{
			case TE_CONSTANT:
			{
				n_out->value = n->value;
				return export_size;
			}

			case TE_VARIABLE:
			{
				register_func(n->bound, n_out, native<t_traits>::find_bind_by_addr(n->bound, lookup, lookup_len));
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
				register_func(
					n->function, n_out, native<t_traits>::find_bind_or_any_by_addr(n->function, lookup, lookup_len));

				export_size += sizeof(n->parameters[0]) * eval_details::arity(n->type);

				for (int i = 0; i < eval_details::arity(n->type); ++i)
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
				register_func(
					n->function, n_out, native<t_traits>::find_bind_or_any_by_addr(n->function, lookup, lookup_len));

				export_size += sizeof(n->parameters[0]) * eval_details::arity(n->type);

				for (int i = 0; i < eval_details::arity(n->type); ++i)
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

		static t_vector eval_compare(const expr_native* n,
			const expr_portable<t_traits>*				   n_portable,
			const unsigned char*						   expr_buffer,
			const void* const							   expr_context[])
		{
#	define TE_FUN(...) ((t_vector(*)(__VA_ARGS__))expr_context[n_portable->function])

#	define M(e)                                                                                                       \
		eval_compare((const expr_native*)n->parameters[e],                                                          \
			(const expr_portable*)&expr_buffer[n_portable->parameters[e]],                                             \
			expr_buffer,                                                                                               \
			expr_context)

			if (!n)
				return t_traits::nan();

			assert(n->type == n_portable->type);

			switch (type_mask(n_portable->type))
			{
			case TE_CONSTANT:
				return n_portable->value;
			case TE_VARIABLE:
				assert(n->bound == expr_context[n_portable->bound]);
				return t_traits::load_atom(*((const t_atom*)(expr_context[n_portable->bound])));

			case TE_FUNCTION0:
			case TE_FUNCTION1:
			case TE_FUNCTION2:
			case TE_FUNCTION3:
			case TE_FUNCTION4:
			case TE_FUNCTION5:
			case TE_FUNCTION6:
			case TE_FUNCTION7:

				assert(n->function == expr_context[n_portable->function]);

				switch (arity(n_portable->type))
				{
				case 0:
					return TE_FUN(void)();
				case 1:
					return TE_FUN(t_vector)(M(0));
				case 2:
					return TE_FUN(t_vector, t_vector)(M(0), M(1));
				case 3:
					return TE_FUN(t_vector, t_vector, t_vector)(M(0), M(1), M(2));
				case 4:
					return TE_FUN(t_vector, t_vector, t_vector, t_vector)(M(0), M(1), M(2), M(3));
				case 5:
					return TE_FUN(t_vector, t_vector, t_vector, t_vector, t_vector)(M(0), M(1), M(2), M(3), M(4));
				case 6:
					return TE_FUN(t_vector, t_vector, t_vector, t_vector, t_vector, t_vector)(
						M(0), M(1), M(2), M(3), M(4), M(5));
				case 7:
					return TE_FUN(t_vector, t_vector, t_vector, t_vector, t_vector, t_vector, t_vector)(
						M(0), M(1), M(2), M(3), M(4), M(5), M(6));
				default:
					return t_traits::nan();
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
				assert(n->parameters[arity(n->type)] == expr_context[n_portable->parameters[arity(n->type)]]);

				auto arity_params = (void*)expr_context[n_portable->parameters[arity(n->type)]];

				switch (arity(n_portable->type))
				{
				case 0:
					return TE_FUN(void*)(arity_params);
				case 1:
					return TE_FUN(void*, t_vector)(arity_params, M(0));
				case 2:
					return TE_FUN(void*, t_vector, t_vector)(arity_params, M(0), M(1));
				case 3:
					return TE_FUN(void*, t_vector, t_vector, t_vector)(arity_params, M(0), M(1), M(2));
				case 4:
					return TE_FUN(void*, t_vector, t_vector, t_vector, t_vector)(arity_params, M(0), M(1), M(2), M(3));
				case 5:
					return TE_FUN(void*, t_vector, t_vector, t_vector, t_vector, t_vector)(
						arity_params, M(0), M(1), M(2), M(3), M(4));
				case 6:
					return TE_FUN(void*, t_vector, t_vector, t_vector, t_vector, t_vector, t_vector)(
						arity_params, M(0), M(1), M(2), M(3), M(4), M(5));
				case 7:
					return TE_FUN(void*, t_vector, t_vector, t_vector, t_vector, t_vector, t_vector, t_vector)(
						arity_params, M(0), M(1), M(2), M(3), M(4), M(5), M(6));
				default:
					return t_traits::nan();
				}
			}

			default:
				return t_traits::nan();
			}
#	undef TE_FUN
#	undef M
		}
	};

	namespace details
	{
		template<typename T_TRAITS>
		compiled_expr compile(const char* expression, const variable* variables, int var_count, int* error)
		{
			typename native<T_TRAITS>::expr_native* native_expr =
				native<T_TRAITS>::compile_native(expression, variables, var_count, error);

			if (native_expr)
			{
				auto expr = new typename portable<T_TRAITS>::compiled_expr;

				size_t export_size = 0;
				portable<T_TRAITS>::export_estimate(native_expr,
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
				portable<T_TRAITS>::export_write(native_expr,
					actual_export_size,
					variables,
					var_count,
					expr->m_build_buffer.get(),
					[&](const void* addr, expr_portable<T_TRAITS>* out, const variable* v) -> void {
						assert(v != nullptr);
						auto itor = expr->m_indexer.index_map.find(addr);
						assert(itor != expr->m_indexer.index_map.end());
						out->function = itor->second;

						if (v->type >= TE_CLOSURE0 && v->type <= TE_CLOSURE7)
						{
							auto itor2 = expr->m_indexer.index_map.find(v->context);
							assert(itor2 != expr->m_indexer.index_map.end());
							out->parameters[eval_details::arity(v->type)] = itor2->second;
						}
					});

				native<T_TRAITS>::free_native(native_expr);
				return expr;
			}
			return nullptr;
		}

		template<typename T_TRAITS>
		size_t get_binding_array_size(const compiled_expr _n)
		{
			auto n = (const portable<T_TRAITS>::compiled_expr*)_n;
			if (n)
			{
				return n->m_bindings.index_to_address.size();
			}
			return 0;
		}

		template<typename T_TRAITS>
		const void* const* get_binding_addresses(const compiled_expr _n)
		{
			auto n = (const portable<T_TRAITS>::compiled_expr*)_n;
			if (n && (n->m_bindings.index_to_address.size() > 0))
			{
				return &(*n->m_bindings.index_to_address.cbegin());
			}
			return nullptr;
		}

		template<typename T_TRAITS>
		const char* const* get_binding_names(const compiled_expr _n)
		{
			auto n = (const portable<T_TRAITS>::compiled_expr*)_n;
			if (n)
			{
				return &(*n->m_bindings.index_to_name_c_str.cbegin());
			}
			return nullptr;
		}

		template<typename T_TRAITS>
		size_t get_expr_data_size(const compiled_expr _n)
		{
			auto n = (const portable<T_TRAITS>::compiled_expr*)_n;
			if (n)
			{
				return n->m_build_buffer_size;
			}
			return 0;
		}

		template<typename T_TRAITS>
		const unsigned char* get_expr_data(const compiled_expr _n)
		{
			auto n = (const portable<T_TRAITS>::compiled_expr*)_n;
			if (n)
			{
				return n->m_build_buffer.get();
			}
			return nullptr;
		}

		template<typename T_TRAITS>
		void te_free(compiled_expr _n)
		{
			auto n = (typename portable<T_TRAITS>::compiled_expr*)_n;
			if (n)
			{
				delete n;
			}
		}
	} // namespace details

	compiled_expr compile(const char* expression, const variable* variables, int var_count, int* error)
	{
		return details::compile<env_traits>(expression, variables, var_count, error);
	}

	size_t get_binding_array_size(const compiled_expr n)
	{
		return details::get_binding_array_size<env_traits>(n);
	}

	const void* const* get_binding_addresses(const compiled_expr n)
	{
		return details::get_binding_addresses<env_traits>(n);
	}

	const char* const* get_binding_names(const compiled_expr n)
	{
		return details::get_binding_names<env_traits>(n);
	}

	size_t get_expr_data_size(const compiled_expr n)
	{
		return details::get_expr_data_size<env_traits>(n);
	}

	const unsigned char* get_expr_data(const compiled_expr n)
	{
		return details::get_expr_data<env_traits>(n);
	}

	void te_free(compiled_expr n)
	{
		details::te_free<env_traits>(n);
	}
}

#endif // #if (TE_COMPILER_ENABLED)
