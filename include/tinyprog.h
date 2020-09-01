/*
 * TinyProg - a minimalist shader-like scripting language.
 *
 * Copyright (c) 2020 Nathan Rausch
 * http://loopunit.com
 *
 * Based on:
 * TINYEXPR - Tiny recursive descent parser and evaluation engine in C
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

#ifndef __TINYPROG_H__
#define __TINYPROG_H__

#include <limits>
#include <cctype>
#include <tuple>

#ifndef TP_TESTING
#define TP_TESTING 0
#endif // #ifndef TP_TESTING

#if TP_TESTING
#define TP_COMPILER_ENABLED 1
#define TP_STANDARD_LIBRARY 1
#endif // #if TP_TESTING

#ifndef TP_COMPILER_ENABLED
#define TP_COMPILER_ENABLED 0
#endif // #ifndef TP_COMPILER_ENABLED

#ifndef TP_STANDARD_LIBRARY
#define TP_STANDARD_LIBRARY 0
#endif // #ifndef TP_STANDARD_LIBRARY

#if (_MSVC_LANG < 201703L)
#define TP_MODERN_CPP 0
#else
#define TP_MODERN_CPP 1
#endif

#if TP_COMPILER_ENABLED
#if TP_MODERN_CPP
#include <string_view>
#else
#error C++ 17 is required for the compiler.
#endif
#endif // #if TP_COMPILER_ENABLED

namespace tp
{
	enum
	{
		VARIABLE = 0,

		CONSTANT = 1,

		FUNCTION0 = 8,
		FUNCTION1,
		FUNCTION2,
		FUNCTION3,
		FUNCTION4,
		FUNCTION5,
		FUNCTION6,
		FUNCTION7,
		FUNCTION_MAX,

		CLOSURE0 = 16,
		CLOSURE1,
		CLOSURE2,
		CLOSURE3,
		CLOSURE4,
		CLOSURE5,
		CLOSURE6,
		CLOSURE7,
		CLOSURE_MAX,

		FLAG_PURE = 32
	};

	struct variable
	{
		const char* name;
		const void* address;
		int			type;
		void*		context;
	};

	struct variable_lookup
	{
		const variable* lookup;
		int				lookup_len;
	};

	template<typename T_TRAITS>
	struct expr_portable
	{
		using t_traits = T_TRAITS;
		using t_atom   = typename T_TRAITS::t_atom;

		int type;
		union
		{
			t_atom value;
			size_t bound;
			size_t function;
		};
		size_t parameters[1];
	};

	namespace eval_details
	{
		template<typename T>
		inline T type_mask(const T t) noexcept
		{
			return ((t)&0x0000001F);
		}

		template<typename T>
		inline T arity(const T t) noexcept
		{
			return (((t) & (FUNCTION0 | CLOSURE0)) ? ((t)&0x00000007) : 0);
		}

		template<typename T_HANDLE_CONSTANT, typename T_HANDLE_VARIABLE, typename T_HANDLE_FUNCTION, typename T_HANDLE_CLOSURE, typename T_HANDLE_ERROR>
		static inline auto eval_generic(
			int type, T_HANDLE_CONSTANT handle_constant, T_HANDLE_VARIABLE handle_variable, T_HANDLE_FUNCTION handle_function, T_HANDLE_CLOSURE handle_closure,
			T_HANDLE_ERROR handle_error)
		{
			const auto t = type_mask(type);
			if (t == CONSTANT)
			{
				return handle_constant();
			}
			else if (t == VARIABLE)
			{
				return handle_variable();
			}
			else if (t >= FUNCTION0)
			{
				if (t < FUNCTION_MAX)
				{
					return handle_function(t - FUNCTION0);
				}
				if (t >= CLOSURE0)
				{
					if (t < CLOSURE_MAX)
					{
						return handle_closure(t - CLOSURE0);
					}
				}
			}

			return handle_error();
		}

		template<typename T_VECTOR, typename T_RET, typename T_EVAL_ARG>
		auto eval_function(int a, const void* fn, T_RET error_val, T_EVAL_ARG eval_arg) -> T_RET
		{
#define FUN(...) ((T_RET(*)(__VA_ARGS__))fn)
			switch (a)
			{
			case 0:
				return FUN(void)();
			case 1:
				return FUN(T_VECTOR)(eval_arg(0));
			case 2:
				return FUN(T_VECTOR, T_VECTOR)(eval_arg(0), eval_arg(1));
			case 3:
				return FUN(T_VECTOR, T_VECTOR, T_VECTOR)(eval_arg(0), eval_arg(1), eval_arg(2));
			case 4:
				return FUN(T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR)(eval_arg(0), eval_arg(1), eval_arg(2), eval_arg(3));
			case 5:
				return FUN(T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR)(eval_arg(0), eval_arg(1), eval_arg(2), eval_arg(3), eval_arg(4));
			case 6:
				return FUN(T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR)(eval_arg(0), eval_arg(1), eval_arg(2), eval_arg(3), eval_arg(4), eval_arg(5));
			case 7:
				return FUN(T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR)(
					eval_arg(0), eval_arg(1), eval_arg(2), eval_arg(3), eval_arg(4), eval_arg(5), eval_arg(6));
			}
#undef FUN
			return error_val;
		}

		template<typename T_VECTOR, typename T_RET, typename T_EVAL_ARG>
		auto eval_closure(int a, const void* fn, const void* arity_params, T_RET error_val, T_EVAL_ARG eval_arg) -> T_RET
		{
#define FUN(...) ((T_RET(*)(__VA_ARGS__))fn)
			switch (a)
			{
			case 0:
				return FUN(const void*)(arity_params);
			case 1:
				return FUN(const void*, T_VECTOR)(arity_params, eval_arg(0));
			case 2:
				return FUN(const void*, T_VECTOR, T_VECTOR)(arity_params, eval_arg(0), eval_arg(1));
			case 3:
				return FUN(const void*, T_VECTOR, T_VECTOR, T_VECTOR)(arity_params, eval_arg(0), eval_arg(1), eval_arg(2));
			case 4:
				return FUN(const void*, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR)(arity_params, eval_arg(0), eval_arg(1), eval_arg(2), eval_arg(3));
			case 5:
				return FUN(const void*, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR)(arity_params, eval_arg(0), eval_arg(1), eval_arg(2), eval_arg(3), eval_arg(4));
			case 6:
				return FUN(const void*, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR)(
					arity_params, eval_arg(0), eval_arg(1), eval_arg(2), eval_arg(3), eval_arg(4), eval_arg(5));
			case 7:
				return FUN(const void*, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR)(
					arity_params, eval_arg(0), eval_arg(1), eval_arg(2), eval_arg(3), eval_arg(4), eval_arg(5), eval_arg(6));
			}
#undef FUN
			return error_val;
		}

		template<typename T_TRAITS, typename T_ATOM, typename T_VECTOR>
		static inline auto eval_portable_impl(const expr_portable<T_TRAITS>* n_portable, const unsigned char* expr_buffer, const void* const expr_context[]) noexcept -> T_VECTOR
		{
			using t_atom   = T_ATOM;
			using t_vector = T_VECTOR;
			using t_traits = T_TRAITS;

			auto eval_arg = [&](int e) {
				return eval_portable_impl<t_traits, t_atom, t_vector>((const expr_portable<t_traits>*)&expr_buffer[n_portable->parameters[e]], expr_buffer, expr_context);
			};

			return eval_generic(
				n_portable->type, [&]() { return t_traits::load_atom(n_portable->value); },
				[&]() { return t_traits::load_atom((expr_context != nullptr) ? *((const t_vector*)(expr_context[n_portable->bound])) : t_traits::nan()); },
				[&](int a) { return eval_function<t_vector>(a, expr_context[n_portable->function], t_traits::nan(), eval_arg); },
				[&](int a) { return eval_closure<t_vector>(a, expr_context[n_portable->function], (void*)expr_context[n_portable->parameters[a]], t_traits::nan(), eval_arg); },
				[&]() { return t_traits::nan(); });
		}
	} // namespace eval_details

	enum class statement_type : int
	{
		jump,
		return_value,
		assign,
		call,
	};

	struct statement
	{
		statement_type type;
		int			   arg_a;
		int			   arg_b;
	};

#if (TP_COMPILER_ENABLED)
	struct compiled_expr
	{
		virtual ~compiled_expr() = default;

		virtual size_t				 get_binding_array_size() const = 0;
		virtual const void* const*	 get_binding_addresses() const	= 0;
		virtual const char* const*	 get_binding_names() const		= 0;
		virtual size_t				 get_data_size() const			= 0;
		virtual const unsigned char* get_data() const				= 0;
	};

	struct compiled_program
	{
		virtual ~compiled_program() = default;

		virtual size_t				 get_binding_array_size() const	  = 0;
		virtual const void* const*	 get_binding_addresses() const	  = 0;
		virtual const char* const*	 get_binding_names() const		  = 0;
		virtual size_t				 get_data_size() const			  = 0;
		virtual const unsigned char* get_data() const				  = 0;
		virtual size_t				 get_statement_array_size() const = 0;
		virtual const statement*	 get_statements() const			  = 0;
	};
#endif // #if (TP_COMPILER_ENABLED)
} // namespace tp

#if (TP_COMPILER_ENABLED)
#include <unordered_map>
#include <vector>
#include <variant>
#include <memory>
#include <cassert>

namespace tp
{
	template<typename T_TRAITS>
	struct native
	{
		using t_traits = T_TRAITS;
		using t_atom   = typename T_TRAITS::t_atom;
		using t_vector = typename T_TRAITS::t_vector;

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

		typedef t_vector (*te_fun2)(t_vector, t_vector);

		enum
		{
			TOK_NUL = CLOSURE_MAX,
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

			variable_lookup lookup;
		};

		static inline bool is_pure(int t) noexcept
		{
			return (((t)&FLAG_PURE) != 0);
		}

		static inline bool is_function(int t) noexcept
		{
			return (((t)&FUNCTION0) != 0);
		}

		static inline bool is_closure(int t) noexcept
		{
			return (((t)&CLOSURE0) != 0);
		}

#define NEW_EXPR(type, ...)                                                                                                                                                        \
	[&]() {                                                                                                                                                                        \
		const expr_native* _args[] = {__VA_ARGS__};                                                                                                                                \
		return new_expr((type), _args);                                                                                                                                            \
	}()

		static expr_native* new_expr(const int type, const expr_native* parameters[])
		{
			const int	 arity = eval_details::arity(type);
			const int	 psize = sizeof(void*) * arity;
			const int	 size  = (sizeof(expr_native) - sizeof(void*)) + psize + (is_closure(type) ? sizeof(void*) : 0);
			expr_native* ret   = (expr_native*)malloc(size);
			if (ret)
			{
				memset(ret, 0, size);
				if (arity && parameters)
				{
					memcpy(ret->parameters, parameters, psize);
				}
				ret->type  = type;
				ret->bound = 0;
			}
			return ret;
		}

		static void free_parameters(expr_native* n)
		{
			if (!n)
				return;

			for (int arity = eval_details::arity(n->type); arity > 0; --arity)
			{
				free_native((expr_native*)n->parameters[arity - 1]);
			}
		}

		static void free_native(expr_native* n)
		{
			if (!n)
				return;
			free_parameters(n);
			free(n);
		}

		static inline const void* find_wrapper(const char* name, state* s)
		{
			auto var = t_traits::find_by_name(name, int(strlen(name)), &s->lookup);
			return var ? var->address : nullptr;
		}

		static void next_token(state* s)
		{
			s->type = (int)TOK_NUL;

			do
			{
				if (!*s->next || *s->next == ';')
				{
					s->type = (int)TOK_END;
					return;
				}

				/* Try reading a number. */
				if ((s->next[0] >= '0' && s->next[0] <= '9') || s->next[0] == '.')
				{
					s->value = (t_atom)strtod(s->next, (char**)&s->next);
					s->type	 = (int)TOK_NUMBER;
				}
				else
				{
					/* Look for a variable or builtin function call. */
					if ((s->next[0] >= 'a' && s->next[0] <= 'z') || (s->next[0] == '_'))
					{
						const char* start;
						start = s->next;
						while ((s->next[0] >= 'a' && s->next[0] <= 'z') || (s->next[0] >= '0' && s->next[0] <= '9') || (s->next[0] == '_'))
							s->next++;

						const variable* var = t_traits::find_by_name(start, static_cast<int>(s->next - start), &s->lookup);

						if (!var)
						{
							s->type = (int)TOK_ERROR;
						}
						else
						{
							const auto t = eval_details::type_mask(var->type);
							if (t == CONSTANT)
							{
								s->type	 = (int)TOK_VARIABLE;
								s->bound = (const t_atom*)var->address;
							}
							else if (t == VARIABLE)
							{
								s->type	 = (int)TOK_VARIABLE;
								s->bound = (const t_atom*)var->address;
							}
							else if (t >= FUNCTION0)
							{
								if (t < FUNCTION_MAX)
								{
									s->type		= var->type;
									s->function = var->address;
								}
								else if (t >= CLOSURE0)
								{
									if (t < CLOSURE_MAX)
									{
										s->context	= var->context;
										s->type		= var->type;
										s->function = var->address;
									}
								}
							}
						}
					}
					else
					{
						/* Look for an operator or special character. */
						switch (s->next++[0])
						{
						case '+':
							s->type		= (int)TOK_INFIX;
							s->function = find_wrapper("add", s);
							break;
						case '-':
							s->type		= (int)TOK_INFIX;
							s->function = find_wrapper("sub", s);
							break;
						case '*':
							s->type		= (int)TOK_INFIX;
							s->function = find_wrapper("mul", s);
							break;
						case '/':
							s->type		= (int)TOK_INFIX;
							s->function = find_wrapper("divide", s);
							break;
						case '^':
							s->type		= (int)TOK_INFIX;
							s->function = find_wrapper("pow", s);
							break;
						case '%':
							s->type		= (int)TOK_INFIX;
							s->function = find_wrapper("fmod", s);
							break;
						case '!':
							if (s->next++[0] == '=')
							{
								s->type		= (int)TOK_INFIX;
								s->function = find_wrapper("not_equal", s);
							}
							else
							{
								s->next--;
								s->type		= (int)TOK_INFIX;
								s->function = find_wrapper("logical_not", s);
							}
							break;
						case '=':
							if (s->next++[0] == '=')
							{
								s->type		= (int)TOK_INFIX;
								s->function = find_wrapper("equal", s);
							}
							else
							{
								s->type = (int)TOK_ERROR;
							}
							break;
						case '<':
							if (s->next++[0] == '=')
							{
								s->type		= (int)TOK_INFIX;
								s->function = find_wrapper("lower_eq", s);
							}
							else
							{
								s->next--;
								s->type		= (int)TOK_INFIX;
								s->function = find_wrapper("lower", s);
							}
							break;
						case '>':
							if (s->next++[0] == '=')
							{
								s->type		= (int)TOK_INFIX;
								s->function = find_wrapper("greater_eq", s);
							}
							else
							{
								s->next--;
								s->type		= (int)TOK_INFIX;
								s->function = find_wrapper("greater", s);
							}
							break;
						case '&':
							if (s->next++[0] == '&')
							{
								s->type		= (int)TOK_INFIX;
								s->function = find_wrapper("logical_and", s);
							}
							else
							{
								s->type = (int)TOK_ERROR;
							}
							break;
						case '|':
							if (s->next++[0] == '|')
							{
								s->type		= (int)TOK_INFIX;
								s->function = find_wrapper("logical_or", s);
							}
							else
							{
								s->type = (int)TOK_ERROR;
							}
							break;
						case '(':
							s->type = (int)TOK_OPEN;
							break;
						case ')':
							s->type = (int)TOK_CLOSE;
							break;
						case ',':
							s->type = (int)TOK_SEP;
							break;
						case ' ':
						case '\t':
						case '\n':
						case '\r':
							break;
						default:
							s->type = (int)TOK_ERROR;
							break;
						}
					}
				}
			} while (s->type == (int)TOK_NUL);
		}

		static expr_native* base(state* s)
		{
			/* <base>      =    <constant> | <variable> | <function-0> {"(" ")"} | <function-1> <power> | <function-X>
			 * "(" <expr> {"," <expr>} ")" | "(" <list> ")" */
			expr_native* ret;

			const auto t = eval_details::type_mask(s->type);

			if (t == (int)TOK_NUMBER)
			{
				ret		   = new_expr(CONSTANT, 0);
				ret->value = s->value;
				next_token(s);
			}
			else if (t == (int)TOK_VARIABLE)
			{
				ret		   = new_expr(VARIABLE, 0);
				ret->bound = s->bound;
				next_token(s);
			}
			else if ((t >= FUNCTION0 && t < FUNCTION_MAX) || (t >= CLOSURE0 && t < CLOSURE_MAX))
			{
				const auto arity = eval_details::arity(s->type);
				if (arity == 0)
				{
					ret			  = new_expr(s->type, 0);
					ret->function = s->function;
					if (is_closure(s->type))
						ret->parameters[0] = s->context;
					next_token(s);
					if (s->type == (int)TOK_OPEN)
					{
						next_token(s);
						if (s->type != (int)TOK_CLOSE)
						{
							s->type = (int)TOK_ERROR;
						}
						else
						{
							next_token(s);
						}
					}
				}
				else if (arity == 1)
				{
					ret			  = new_expr(s->type, 0);
					ret->function = s->function;
					if (is_closure(s->type))
						ret->parameters[1] = s->context;
					next_token(s);
					ret->parameters[0] = power(s);
				}
				else
				{
					ret			  = new_expr(s->type, 0);
					ret->function = s->function;
					if (is_closure(s->type))
						ret->parameters[arity] = s->context;
					next_token(s);

					if (s->type != (int)TOK_OPEN)
					{
						s->type = (int)TOK_ERROR;
					}
					else
					{
						int i;
						for (i = 0; i < arity; i++)
						{
							next_token(s);
							ret->parameters[i] = expr(s);
							if (s->type != (int)TOK_SEP)
							{
								break;
							}
						}
						if (s->type != (int)TOK_CLOSE || i != arity - 1)
						{
							s->type = (int)TOK_ERROR;
						}
						else
						{
							next_token(s);
						}
					}
				}
			}
			else if (t == (int)TOK_OPEN)
			{
				next_token(s);
				ret = list(s);
				if (s->type != (int)TOK_CLOSE)
				{
					s->type = (int)TOK_ERROR;
				}
				else
				{
					next_token(s);
				}
			}
			else
			{
				ret		   = new_expr(0, 0);
				s->type	   = (int)TOK_ERROR;
				ret->value = t_traits::nan();
			}

			return ret;
		}

		static expr_native* power(state* s)
		{
			/* <power>     =    {("-" | "+" | "!")} <base> */
			int sign = 1;
			while (s->type == (int)TOK_INFIX && (s->function == find_wrapper("add", s) || s->function == find_wrapper("sub", s)))
			{
				if (s->function == find_wrapper("sub", s))
					sign = -sign;
				next_token(s);
			}

			int logical = 0;
			while (s->type == (int)TOK_INFIX && (s->function == find_wrapper("add", s) || s->function == find_wrapper("sub", s) || s->function == find_wrapper("logical_not", s)))
			{
				if (s->function == find_wrapper("logical_not", s))
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
					ret			  = NEW_EXPR(FUNCTION1 | FLAG_PURE, base(s));
					ret->function = find_wrapper("logical_not", s);
				}
				else
				{
					ret			  = NEW_EXPR(FUNCTION1 | FLAG_PURE, base(s));
					ret->function = find_wrapper("logical_notnot", s);
				}
			}
			else
			{
				if (logical == 0)
				{
					ret			  = NEW_EXPR(FUNCTION1 | FLAG_PURE, base(s));
					ret->function = find_wrapper("negate", s);
				}
				else if (logical == -1)
				{
					ret			  = NEW_EXPR(FUNCTION1 | FLAG_PURE, base(s));
					ret->function = find_wrapper("negate_logical_not", s);
				}
				else
				{
					ret			  = NEW_EXPR(FUNCTION1 | FLAG_PURE, base(s));
					ret->function = find_wrapper("negate_logical_notnot", s);
				}
			}

			return ret;
		}

#ifdef TP_POW_FROM_RIGHT
		static expr_native* factor(state* s)
		{
			/* <factor>    =    <power> {"^" <power>} */
			expr_native* ret = power(s);

			const void*	 left_function = NUL;
			expr_native* insertion	   = 0;

			if (ret->type == (FUNCTION1 | FLAG_PURE) &&
				(ret->function == find_wrapper("negate", s) || ret->function == find_wrapper("logical_not", s) || ret->function == find_wrapper("logical_notnot", s) ||
				 ret->function == find_wrapper("negate_logical_not", s) || ret->function == find_wrapper("negate_logical_notnot", s)))
			{
				left_function	= ret->function;
				expr_native* se = ret->parameters[0];
				free(ret);
				ret = se;
			}

			while (s->type == (int)TOK_INFIX && (s->function == find_wrapper("pow", s)))
			{
				te_fun2 t = s->function;
				next_token(s);

				if (insertion)
				{
					/* Make exponentiation go right-to-left. */
					expr_native* insert		 = NEW_EXPR(FUNCTION2 | FLAG_PURE, insertion->parameters[1], power(s));
					insert->function		 = t;
					insertion->parameters[1] = insert;
					insertion				 = insert;
				}
				else
				{
					ret			  = NEW_EXPR(FUNCTION2 | FLAG_PURE, ret, power(s));
					ret->function = t;
					insertion	  = ret;
				}
			}

			if (left_function)
			{
				ret			  = NEW_EXPR(FUNCTION1 | FLAG_PURE, ret);
				ret->function = left_function;
			}

			return ret;
		}
#else
		static expr_native* factor(state* s)
		{
			/* <factor>    =    <power> {"^" <power>} */
			expr_native* ret = power(s);

			while (s->type == (int)TOK_INFIX && (s->function == find_wrapper("pow", s)))
			{
				te_fun2 t = (te_fun2)s->function;
				next_token(s);
				ret			  = NEW_EXPR(FUNCTION2 | FLAG_PURE, ret, power(s));
				ret->function = t;
			}

			return ret;
		}
#endif

		static expr_native* term(state* s)
		{
			/* <term>      =    <factor> {("*" | "/" | "%") <factor>} */
			expr_native* ret = factor(s);

			while (s->type == (int)TOK_INFIX && (s->function == find_wrapper("mul", s) || s->function == find_wrapper("divide", s) || s->function == find_wrapper("fmod", s)))
			{
				te_fun2 t = (te_fun2)s->function;
				next_token(s);
				ret			  = NEW_EXPR(FUNCTION2 | FLAG_PURE, ret, factor(s));
				ret->function = t;
			}

			return ret;
		}

		static expr_native* sum_expr(state* s)
		{
			/* <expr>      =    <term> {("+" | "-") <term>} */
			expr_native* ret = term(s);

			while (s->type == (int)TOK_INFIX && (s->function == find_wrapper("add", s) || s->function == find_wrapper("sub", s)))
			{
				te_fun2 t = (te_fun2)s->function;
				next_token(s);
				ret			  = NEW_EXPR(FUNCTION2 | FLAG_PURE, ret, term(s));
				ret->function = t;
			}

			return ret;
		}

		static expr_native* test_expr(state* s)
		{
			/* <expr>      =    <sum_expr> {(">" | ">=" | "<" | "<=" | "==" | "!=") <sum_expr>} */
			expr_native* ret = sum_expr(s);

			while (s->type == (int)TOK_INFIX &&
				   (s->function == find_wrapper("greater", s) || s->function == find_wrapper("greater_eq", s) || s->function == find_wrapper("lower", s) ||
					s->function == find_wrapper("lower_eq", s) || s->function == find_wrapper("equal", s) || s->function == find_wrapper("not_equal", s)))
			{
				te_fun2 t = (te_fun2)s->function;
				next_token(s);
				ret			  = NEW_EXPR(FUNCTION2 | FLAG_PURE, ret, sum_expr(s));
				ret->function = t;
			}

			return ret;
		}

		static expr_native* expr(state* s)
		{
			/* <expr>      =    <test_expr> {("&&" | "||") <test_expr>} */
			expr_native* ret = test_expr(s);

			while (s->type == (int)TOK_INFIX && (s->function == find_wrapper("logical_and", s) || s->function == find_wrapper("logical_or", s)))
			{
				te_fun2 t = (te_fun2)s->function;
				next_token(s);
				ret			  = NEW_EXPR(FUNCTION2 | FLAG_PURE, ret, test_expr(s));
				ret->function = t;
			}

			return ret;
		}

		static expr_native* list(state* s)
		{
			/* <list>      =    <expr> {"," <expr>} */
			expr_native* ret = expr(s);

			while (s->type == (int)TOK_SEP)
			{
				next_token(s);
				ret			  = NEW_EXPR(FUNCTION2 | FLAG_PURE, ret, expr(s));
				ret->function = find_wrapper("comma", s);
			}

			return ret;
		}

		static t_vector eval_native(const expr_native* n)
		{
			if (!n)
				return t_traits::nan();

			auto eval_arg = [&](int e) {
				return eval_native((const expr_native*)n->parameters[e]);
			};

			return eval_details::eval_generic(
				n->type, [&]() { return n->value; }, [&]() { return *n->bound; },
				[&](int a) { return eval_details::eval_function<t_vector>(a, n->function, t_traits::nan(), eval_arg); },
				[&](int a) { return eval_details::eval_closure<t_vector>(a, n->function, (void*)n->parameters[a], t_traits::nan(), eval_arg); }, [&]() { return t_traits::nan(); });
		}

		static void optimize(expr_native* n)
		{
			/* Evaluates as much as possible. */
			if (n->type == CONSTANT)
		    {
				return;
			}
			
			if (n->type == VARIABLE)
		    {
				const variable* v = t_traits::find_by_addr(n->bound, nullptr);
				if (v && (v->type == CONSTANT))
				{
					free_parameters(n);
					n->type	 = CONSTANT;
					n->value = *((t_vector*)n->bound);
				}
				return;
			}

			/* Only optimize out functions flagged as pure. */
			if (is_pure(n->type))
			{
				const int arity = eval_details::arity(n->type);
				int		  known = 1;
				int		  i;
				for (i = 0; i < arity; ++i)
				{
					optimize((expr_native*)n->parameters[i]);
					if (((expr_native*)(n->parameters[i]))->type != CONSTANT)
					{
						known = 0;
					}
				}
				if (known)
				{
					const t_vector value = eval_native(n);
					free_parameters(n);
					n->type	 = CONSTANT;
					n->value = value;
				}
			}
		}

		static expr_native* compile_native(const char* expression, const variable_lookup* lookup, int* error)
		{
			state s;
			s.start = s.next	= expression;
			if (lookup)
			{
				s.lookup = *lookup;
			}
			else
			{
				s.lookup = { 0, 0 };
			}

			next_token(&s);
			expr_native* root = list(&s);

			if (s.type != (int)TOK_END)
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
			expr_native* n = compile_native(expression, 0, error);
			t_vector	 ret;
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

			const auto t = type_mask(n->type);

			if (t == CONSTANT)
			{
				printf("%f\n", n->value);
			}
			else if (t == VARIABLE)
			{
				printf("bound %p\n", n->bound);
			}
			else if ((t >= FUNCTION0 && t < FUNCTION_MAX) || (t >= CLOSURE0 && t < CLOSURE_MAX))
			{
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
			}
		}

		static void print(const expr_native* n)
		{
			pn(n, 0);
		}
	};

	template<typename T_TRAITS>
	struct portable
	{
		using t_traits	  = T_TRAITS;
		using t_atom	  = typename T_TRAITS::t_atom;
		using t_vector	  = typename T_TRAITS::t_vector;
		using expr_native = typename native<t_traits>::expr_native;

		using name_map = std::unordered_map<const void*, std::string>;

		using index_map = std::unordered_map<const void*, int>;

		struct expr_portable_expression_build_indexer
		{
			name_map  name_map;
			index_map index_map;
			int		  index_counter = 0;

			std::vector<variable> m_env_variables;

			std::vector<std::string>				m_declared_variable_names;
			std::vector<std::unique_ptr<t_atom>>	 m_declared_variable_values;

			void reset()
			{
				name_map.clear();
				index_map.clear();
				index_counter = 0;
				m_env_variables.clear();
				m_declared_variable_names.clear();
				m_declared_variable_values.clear();
			}

			struct variable_lookup_temp
			{
				std::vector<variable> data;

				variable_lookup get_lookup() const
				{
					return variable_lookup { (data.size() > 0) ? &data[0] : nullptr, int(data.size()) };
				}
			};
			
			std::unique_ptr<variable_lookup_temp> get_variable_array() const
			{
				std::unique_ptr<variable_lookup_temp> combined(new variable_lookup_temp());
				
				for (size_t v = 0; v < m_declared_variable_names.size(); ++v)
				{
					combined->data.push_back(variable{m_declared_variable_names[v].c_str(), m_declared_variable_values[v].get()});
				}

				for (auto var : m_env_variables)
				{
					combined->data.push_back(var);
				}

				return combined;
			}

			void add_declared_variable(std::string_view name_view, std::string_view /*scope*/)
			{
				std::string name(name_view);
				auto itor = std::find(m_declared_variable_names.begin(), m_declared_variable_names.end(), name);
				if (itor == m_declared_variable_names.end())
				{
					m_declared_variable_names.push_back(name);
					m_declared_variable_values.emplace_back(new t_atom);
				}
			}

			void add_user_variable(const variable* var)
			{
				m_env_variables.push_back(*var);
			}

			int add_referenced_variable(const variable* var)
			{
				auto itor = index_map.find(var->address);
				if (itor == index_map.end())
				{
					int idx = index_counter++;
					name_map.insert(std::make_pair(var->address, std::string(var->name)));
					index_map.insert(std::make_pair(var->address, idx));
					return idx;
				}
				return itor->second;
			}

			std::vector<std::string> get_binding_table()
			{
				std::vector<std::string> t;
				t.resize(index_counter);
				for (auto index_map_itor : index_map)
				{
					auto name_map_itor		 = name_map.find(index_map_itor.first);
					t[index_map_itor.second] = name_map_itor->second;
				}

				return t;
			}

			std::vector<const void*> get_address_table()
			{
				std::vector<const void*> t;
				t.resize(index_counter);
				for (auto index_map_itor : index_map)
				{
					t[index_map_itor.second] = index_map_itor.first;
				}

				return t;
			}
		};

		struct expr_portable_expression_build_bindings
		{
			std::vector<const void*> index_to_address; // this contains the native function/value address as originally compiled
			std::vector<std::string> index_to_name;
			std::vector<const char*> index_to_name_c_str;
		};

		struct compiled_expr : ::tp::compiled_expr
		{
			expr_portable_expression_build_bindings m_bindings;
			std::unique_ptr<unsigned char>			m_build_buffer;
			size_t									m_build_buffer_size;

			virtual size_t get_binding_array_size() const
			{
				return m_bindings.index_to_address.size();
			}

			virtual const void* const* get_binding_addresses() const
			{
				return (m_bindings.index_to_address.size() > 0) ? &(*m_bindings.index_to_address.cbegin()) : nullptr;
			}

			virtual const char* const* get_binding_names() const
			{
				return &(*m_bindings.index_to_name_c_str.cbegin());
			}

			virtual size_t get_data_size() const
			{
				return m_build_buffer_size;
			}

			virtual const unsigned char* get_data() const
			{
				return m_build_buffer.get();
			}
		};

		static size_t export_estimate(const expr_native* n, size_t& export_size, const variable_lookup* lookup, name_map& name_map, index_map& index_map, int& index_counter)
		{
			if (!n)
				return export_size;

			export_size += sizeof(expr_native);

			auto eval_arg = [&](int e) {
				export_estimate((const expr_native*)n->parameters[e], export_size, lookup, name_map, index_map, index_counter);
			};

			auto handle_addr = [&](const variable* var) -> bool {
				if (var)
				{
					auto itor = name_map.find(var->address);
					if (itor == name_map.end())
					{
						name_map.emplace(std::make_pair(var->address, std::string(var->name)));
						index_map.insert(std::make_pair(var->address, index_counter++));
					}

					if (var->type >= CLOSURE0 && var->type < CLOSURE_MAX)
					{
						auto context_itor = name_map.find(var->context);
						if (context_itor == name_map.end())
						{
							name_map.emplace(std::make_pair(var->context, std::string(var->name) + "_closure"));
							index_map.insert(std::make_pair(var->context, index_counter++));
						}
					}

					return true;
				}
				return false;
			};

			return eval_details::eval_generic(
				n->type, [&]() { return export_size; },
				[&]() {
					auto res = handle_addr(t_traits::find_by_addr(n->bound, lookup));
					assert(res);
					((void)res);
					return export_size;
				},
				[&](int a) {
					auto res = handle_addr(t_traits::find_by_addr(n->function, lookup));
					assert(res);
					((void)res);
					export_size += sizeof(n->parameters[0]) * a;

					for (int i = 0; i < a; ++i)
					{
						eval_arg(i);
					}
					return export_size;
				},
				[&](int a) {
					auto res = handle_addr(t_traits::find_by_addr(n->function, lookup));
					assert(res);
					((void)res);
					export_size += sizeof(n->parameters[0]) * a;

					for (int i = 0; i < a; ++i)
					{
						eval_arg(i);
					}
					return export_size;
				},
				[&]() { return export_size; });
		}

		template<typename T_REGISTER_FUNC>
		static size_t export_write(const expr_native* n, size_t& export_size, const variable_lookup* lookup, const unsigned char* out_buffer, T_REGISTER_FUNC register_func)
		{
			if (!n)
				return export_size;

			auto n_out = (expr_portable<t_traits>*)(out_buffer + export_size);

			export_size += sizeof(expr_native);
			n_out->type = n->type;

			auto eval_arg = [&](int e) {
				return export_write((const expr_native*)n->parameters[e], export_size, lookup, out_buffer, register_func);
			};

			return eval_details::eval_generic(
				n->type,
				[&]() {
					n_out->value = n->value;
					return export_size;
				},
				[&]() {
					register_func(n->bound, n_out, t_traits::find_by_addr(n->bound, lookup));
					return export_size;
				},
				[&](int a) {
					register_func(n->function, n_out, t_traits::find_by_addr(n->function, lookup));

					export_size += sizeof(n->parameters[0]) * eval_details::arity(n->type);

					for (int i = 0; i < eval_details::arity(n->type); ++i)
					{
						n_out->parameters[i] = export_size;
						eval_arg(i);
					}
					return export_size;
				},
				[&](int a) {
					register_func(n->function, n_out, t_traits::find_by_addr(n->function, lookup));

					export_size += sizeof(n->parameters[0]) * eval_details::arity(n->type);

					for (int i = 0; i < eval_details::arity(n->type); ++i)
					{
						n_out->parameters[i] = export_size;
						eval_arg(i);
					}
					return export_size;
				},
				[&]() { return export_size; });
		}

		static t_vector eval_compare(const expr_native* n, const expr_portable<t_traits>* n_portable, const unsigned char* expr_buffer, const void* const expr_context[])
		{
			if (!n)
				return t_traits::nan();

			assert(n->type == n_portable->type);

			auto eval_arg = [&](int e) {
				return eval_compare((const expr_native*)n->parameters[e], (const expr_portable*)&expr_buffer[n_portable->parameters[e]], expr_buffer, expr_context);
			};

			return eval_details::eval_generic(
				n->type, [&]() { return n_portable->value; },
				[&]() {
					assert(n->bound == expr_context[n_portable->bound]);
					return t_traits::load_atom(*((const t_atom*)(expr_context[n_portable->bound])));
				},
				[&](int a) {
					assert(n->function == expr_context[n_portable->function]);
					return eval_details::eval_function<t_vector>(a, expr_context[n_portable->function], t_traits::nan(), eval_arg);
				},
				[&](int a) {
					assert(n->function == expr_context[n_portable->function]);
					assert(n->parameters[arity(n->type)] == expr_context[n_portable->parameters[arity(n->type)]]);

					return eval_details::eval_closure<t_vector>(
						a, expr_context[n_portable->function], (void*)expr_context[n_portable->parameters[arity(n->type)]], t_traits::nan(), eval_arg);
				},
				[&]() { return t_traits::nan(); });
		}

		struct portable_compiled_program : compiled_program
		{
			std::vector<statement>		   program_statements;
			std::vector<std::string>	   binding_table;
			std::vector<const char*>	   binding_table_cstr;
			std::vector<const void*>	   address_table;
			std::unique_ptr<unsigned char> program_expression_buffer;
			size_t						   program_expression_buffer_size = 0;

			virtual size_t get_binding_array_size() const
			{
				return address_table.size();
			}

			virtual const void* const* get_binding_addresses() const
			{
				return &address_table[0];
			}

			virtual const char* const* get_binding_names() const
			{
				return &binding_table_cstr[0];
			}

			virtual size_t get_data_size() const
			{
				return program_expression_buffer_size;
			}

			virtual const unsigned char* get_data() const
			{
				return program_expression_buffer.get();
			}

			virtual size_t get_statement_array_size() const
			{
				return (int)program_statements.size();
			}

			virtual const statement* get_statements() const
			{
				return &program_statements[0];
			}
		};
	};

	namespace expr_details
	{
		template<typename T_TRAITS>
		compiled_expr* compile_using_indexer(typename portable<T_TRAITS>::expr_portable_expression_build_indexer& indexer, const char* expression, int* error)
		{
			auto			var_array = indexer.get_variable_array();
			auto variables = var_array->get_lookup();

			typename native<T_TRAITS>::expr_native* native_expr = native<T_TRAITS>::compile_native(expression, &variables, error);

			if (native_expr)
			{
				auto expr = new typename portable<T_TRAITS>::compiled_expr;

				size_t export_size = 0;
				portable<T_TRAITS>::export_estimate(native_expr, export_size, &variables, indexer.name_map, indexer.index_map, indexer.index_counter);

				expr->m_bindings.index_to_address.resize(indexer.index_counter);
				for (const auto& itor : indexer.index_map)
				{
					expr->m_bindings.index_to_address[itor.second] = itor.first;
				}

				expr->m_bindings.index_to_name.resize(indexer.index_counter);
				expr->m_bindings.index_to_name_c_str.resize(indexer.index_counter);
				for (int i = 0; i < indexer.index_counter; ++i)
				{
					auto itor = indexer.name_map.find(expr->m_bindings.index_to_address[i]);
					assert(itor != indexer.name_map.end());
					expr->m_bindings.index_to_name[i]		= itor->second;
					expr->m_bindings.index_to_name_c_str[i] = expr->m_bindings.index_to_name[i].c_str();
				}

				expr->m_build_buffer.reset(new uint8_t[export_size]);
				::memset(expr->m_build_buffer.get(), 0x0, export_size);
				expr->m_build_buffer_size = export_size;

				size_t actual_export_size = 0;
				portable<T_TRAITS>::export_write(
					native_expr, actual_export_size, &variables, expr->m_build_buffer.get(), [&](const void* addr, expr_portable<T_TRAITS>* out, const variable* v) -> void {
						assert(v != nullptr);
						auto itor = indexer.index_map.find(addr);
						assert(itor != indexer.index_map.end());
						out->function = itor->second;

						if (v->type >= CLOSURE0 && v->type < CLOSURE_MAX)
						{
							auto itor2 = indexer.index_map.find(v->context);
							assert(itor2 != indexer.index_map.end());
							out->parameters[eval_details::arity(v->type)] = itor2->second;
						}
					});

				native<T_TRAITS>::free_native(native_expr);
				return expr;
			}
			return nullptr;
		}

		template<typename T_TRAITS>
		compiled_expr* compile(const char* expression, const variable* variables, int var_count, int* error)
		{
			typename portable<T_TRAITS>::expr_portable_expression_build_indexer indexer;
			for (int v = 0; v < var_count; ++v)
			{
				indexer.add_user_variable(variables + v);
			}
			return compile_using_indexer<T_TRAITS>(indexer, expression, error);
		}

	} // namespace expr_details

	namespace program_details
	{
		struct parser
		{
			static inline std::string_view trim_leading_space(std::string_view v) noexcept
			{
				if (v.length() > 0)
				{
					for (size_t i = 0; i < v.length(); ++i)
					{
						if (!::isspace((unsigned char)v[i]))
						{
							return std::string_view(&v[i], v.length() - i);
						}
					}

					return std::string_view(&v[v.length()], 0);
				}
				return v;
			}

			static inline std::string_view trim_trailing_space(std::string_view v) noexcept
			{
				if (v.length() > 0)
				{
					for (size_t i = v.length(); i > 0; --i)
					{
						if (!::isspace((unsigned char)v[i - 1]))
						{
							return std::string_view(&v[0], i);
						}
					}

					if (!::isspace((unsigned char)v[0]))
					{
						std::string_view(&v[0], 1);
					}

					return std::string_view(&v[0], 0);
				}
				return v;
			}

			static inline std::string_view trim_all_space(std::string_view v) noexcept
			{
				return trim_leading_space(trim_trailing_space(v));
			}

			static inline std::tuple<std::string_view, std::string_view> split_at_index(std::string_view s, size_t index)
			{
				if (index < s.length())
				{
					return {trim_all_space(std::string_view{&s[0], index}), trim_all_space(std::string_view{&s[index], s.length() - index})};
				}
				return {s, std::string_view{}};
			}

			static inline std::tuple<std::string_view, std::string_view> split_at_index_excl(std::string_view s, size_t index)
			{
				auto [l, r] = split_at_index(s, index);
				if (r.length() > 1)
				{
					return {trim_all_space(l), trim_all_space(std::string_view{&r[1], r.length() - 1})};
				}
				return {l, std::string_view{}};
			}

			static inline std::tuple<std::string_view, std::string_view> split_at_char(std::string_view program, char c)
			{
				for (size_t i = 0; i < program.length(); ++i)
				{
					if (program[i] == c)
					{
						return split_at_index(program, i);
					}
				}
				return {program, std::string_view{}};
			}

			static inline std::tuple<std::string_view, std::string_view> split_at_char_excl(std::string_view program, char c)
			{
				for (size_t i = 0; i < program.length(); ++i)
				{
					if (program[i] == c)
					{
						return split_at_index_excl(program, i);
					}
				}
				return {program, std::string_view{}};
			}

			////

			static inline const auto keyword_return = std::string_view("return");
			static inline const auto keyword_jump	= std::string_view("jump");
			static inline const auto keyword_label	= std::string_view("label");
			static inline const auto keyword_var	= std::string_view("var");

			template<
				typename T_ADD_VARIABLE, typename T_ADD_LABEL, typename T_ADD_JUMP, typename T_ADD_JUMP_IF, typename T_ADD_RETURN_VALUE, typename T_ADD_ASSIGN, typename T_ADD_CALL>
			static inline void parse_statement(
				std::string_view statement, T_ADD_VARIABLE add_variable, T_ADD_LABEL add_label, T_ADD_JUMP add_jump, T_ADD_JUMP_IF add_jump_if, T_ADD_RETURN_VALUE add_return_value,
				T_ADD_ASSIGN add_assign, T_ADD_CALL add_call)
			{
				auto [operation, expression] = split_at_char_excl(statement, ':');

				if (expression.length() == 0)
				{
					add_call(operation);
				}
				else
				{
					if (operation == keyword_var)
					{
						auto [var_name, var_scope] = split_at_char_excl(expression, '?');
						add_variable(var_name, var_scope);
					}
					else if (operation == keyword_label)
					{
						add_label(expression);
					}
					else if (operation == keyword_jump)
					{
						auto [jump_label, jump_condition] = split_at_char_excl(expression, '?');

						if (jump_condition.length() > 0)
						{
							add_jump_if(jump_label, jump_condition);
						}
						else
						{
							add_jump(jump_label);
						}
					}
					else if (operation == keyword_return)
					{
						add_return_value(expression);
					}
					else
					{
						add_assign(operation, expression);
					}
				}
			}
		};

		struct label_manager
		{
			using handle								  = int;
			static inline constexpr int placeholder_index = -1;

			std::vector<int>							 m_label_statement_indexes;
			std::unordered_map<std::string_view, handle> m_label_handle_map;

			handle add_label(std::string_view label, int statement_index)
			{
				auto itor = m_label_handle_map.find(label);
				if (itor != m_label_handle_map.end())
				{
					handle label_handle = (*itor).second;

					// Placeholder was filled, fill in the program counter
					if (m_label_statement_indexes[label_handle] != placeholder_index)
					{
						// error, label repeated?
					}
					else
					{
						// The target of the jump label is the next statement of program
						m_label_statement_indexes[label_handle] = statement_index;
					}

					return label_handle;
				}
				else
				{
					// The target of the jump label is the next statement of program
					auto label_handle = (handle)m_label_statement_indexes.size();
					m_label_handle_map.insert(std::make_pair(label, label_handle));
					m_label_statement_indexes.push_back(statement_index);
					return label_handle;
				}
			}

			handle find_label(std::string_view label)
			{
				auto itor = m_label_handle_map.find(label);
				if (itor != m_label_handle_map.end())
				{
					// label exists, use its index
					return (handle)(*itor).second;
				}
				else
				{
					// add a temp label, which will be replaced then when the actual label is encountered
					auto new_handle = (handle)m_label_statement_indexes.size();
					m_label_handle_map.insert(std::make_pair(label, new_handle));
					m_label_statement_indexes.push_back(placeholder_index);
					return new_handle;
				}
			}

			int get_label_statement_index(handle label)
			{
				return m_label_statement_indexes[label];
			}
		};

		struct variable_manager
		{
			int										  m_variable_count = 0;
			std::unordered_map<std::string_view, int> m_variable_map;

			int find_label(std::string_view name)
			{
				auto itor = m_variable_map.find(name);
				if (itor == m_variable_map.end())
				{
					auto idx = m_variable_count++;
					m_variable_map.insert(std::make_pair(name, idx));
					return idx;
				}
				return itor->second;
			}
		};

		struct expression_manager
		{
			std::vector<std::string_view> m_expressions;

			int add_expression(std::string_view src)
			{
				auto idx = (int)m_expressions.size();
				m_expressions.push_back(src);
				return idx;
			}
		};

		struct jump_statement
		{
			label_manager::handle m_target_handle; // Pass 1: index isn't known isn't known until whole program is parsed
			int					  m_expression_index;

			int m_target_index; // Pass 2: set from handle to index (of statement immediately following the label)
			int m_expression_offset;
		};

		struct return_value_statement
		{
			int m_expression_index;

			int m_expression_offset;
		};

		struct assign_statement
		{
			int m_variable_build_index;
			int m_expression_index;

			int m_variable_final_index;
			int m_expression_offset;
		};

		struct call_statement
		{
			int m_expression_index;

			int m_expression_offset;
		};

		using any_statement = std::variant<jump_statement, return_value_statement, assign_statement, call_statement>;

		template<typename T_TRAITS>
		using t_indexer = typename portable<T_TRAITS>::expr_portable_expression_build_indexer;

		template<typename T_TRAITS>
		auto compile_using_indexer(const char* text, int* error, typename t_indexer<T_TRAITS>& indexer) -> typename portable<T_TRAITS>::portable_compiled_program*
		{
			auto program_src	   = parser::trim_all_space(std::string_view{text, strlen(text)});
			auto program_remaining = program_src;

			std::vector<any_statement> program_statements;
			using program_impl		   = typename portable<T_TRAITS>::portable_compiled_program;
			auto			   program = new program_impl();
			label_manager	   lm;
			variable_manager   vm;
			expression_manager em;

			while (program_remaining.length() > 0)
			{
				auto [statement, remaining] = parser::split_at_char_excl(program_remaining, ';');

				parser::parse_statement(
					statement,

					// variable
					[&](std::string_view name, std::string_view scope) { indexer.add_declared_variable(name, scope); },

					// label
					[&](std::string_view label) { lm.add_label(label, (int)program_statements.size()); },

					// jump
					[&](std::string_view destination_label) {
						any_statement s = jump_statement{lm.find_label(destination_label), -1};
						program_statements.push_back(s);
					},

					// jump_if
					[&](std::string_view destination_label, std::string_view condition) {
						any_statement s = jump_statement{lm.find_label(destination_label), em.add_expression(condition)};
						program_statements.push_back(s);
					},

					// return_value
					[&](std::string_view expression) {
						any_statement s = return_value_statement{em.add_expression(expression)};
						program_statements.push_back(s);
					},

					// assign
					[&](std::string_view destination, std::string_view expression) {
						any_statement s = assign_statement{vm.find_label(destination), em.add_expression(expression)};
						program_statements.push_back(s);
					},

					// call
					[&](std::string_view expression) {
						any_statement s = call_statement{em.add_expression(expression)};
						program_statements.push_back(s);
					});

				program_remaining = remaining;
			}

			// Fixup jump indices
			for (auto& s : program_statements)
			{
				if (std::holds_alternative<jump_statement>(s))
				{
					std::get<jump_statement>(s).m_target_index = lm.get_label_statement_index(std::get<jump_statement>(s).m_target_handle);
				}
			}

			// Add referenced variables to the lookup dict
			{
				auto			var_array_tmp = indexer.get_variable_array();
				auto var_lookup = var_array_tmp->get_lookup();
				for (auto itor : vm.m_variable_map)
				{
					auto name  = itor.first;
					auto index = itor.second;

					int final_index = -1;

					for (int var_idx = 0; var_idx < var_lookup.lookup_len; ++var_idx)
					{
						const auto& var = var_lookup.lookup[var_idx];

						if (name == std::string_view(var.name))
						{
							final_index = indexer.add_referenced_variable(&var);
							break;
						}
					}

					if (final_index == -1)
					{
						*error = -1; // TODO: variable not found
						// assert(!"Variable not found.");
						return nullptr;
					}

					// Remap all indexes to the indexer value
					for (auto& s : program_statements)
					{
						if (std::holds_alternative<assign_statement>(s))
						{
							if (std::get<assign_statement>(s).m_variable_build_index == index)
							{
								std::get<assign_statement>(s).m_variable_final_index = final_index;
							}
						}
					}
				}
			}

			// Compile all the expressions, redirect the statement expression indexes to the compiled buffer offset
			for (int expr_idx = 0; expr_idx < em.m_expressions.size(); ++expr_idx)
			{
				auto expr		   = em.m_expressions[expr_idx];
				auto compiled_expr = expr_details::compile_using_indexer<T_TRAITS>(indexer, expr.data(), error);

				if (compiled_expr)
				{
					const int current_expr_offset = (int)program->program_expression_buffer_size;
					auto	  compiled_size		  = (int)compiled_expr->get_data_size();
					const int new_expr_offset	  = (int)program->program_expression_buffer_size + compiled_size;

					auto new_expr_buffer = new unsigned char[new_expr_offset];
					::memcpy(new_expr_buffer, program->program_expression_buffer.get(), program->program_expression_buffer_size);
					::memcpy(new_expr_buffer + program->program_expression_buffer_size, compiled_expr->get_data(), compiled_size);

					program->program_expression_buffer_size = new_expr_offset;

					program->program_expression_buffer.reset(new_expr_buffer);

					// Fixup any expression indexes
					for (auto& s : program_statements)
					{
						if (std::holds_alternative<call_statement>(s))
						{
							if (std::get<call_statement>(s).m_expression_index == expr_idx)
							{
								std::get<call_statement>(s).m_expression_offset = current_expr_offset;
							}
						}

						if (std::holds_alternative<assign_statement>(s))
						{
							if (std::get<assign_statement>(s).m_expression_index == expr_idx)
							{
								std::get<assign_statement>(s).m_expression_offset = current_expr_offset;
							}
						}

						if (std::holds_alternative<return_value_statement>(s))
						{
							if (std::get<return_value_statement>(s).m_expression_index == expr_idx)
							{
								std::get<return_value_statement>(s).m_expression_offset = current_expr_offset;
							}
						}

						if (std::holds_alternative<jump_statement>(s))
						{
							if (std::get<jump_statement>(s).m_expression_index == expr_idx)
							{
								std::get<jump_statement>(s).m_expression_offset = current_expr_offset;
							}
						}
					}
				}
				else
				{
					*error = -1; // TODO: handle error
					break;
				}
			}

			for (auto s_in : program_statements)
			{
				statement s_out;

				if (std::holds_alternative<call_statement>(s_in))
				{
					s_out.type	= statement_type::call;
					s_out.arg_a = std::get<call_statement>(s_in).m_expression_offset;
					s_out.arg_b = -1;
				}
				else if (std::holds_alternative<assign_statement>(s_in))
				{
					s_out.type	= statement_type::assign;
					s_out.arg_a = std::get<assign_statement>(s_in).m_variable_final_index;
					s_out.arg_b = std::get<assign_statement>(s_in).m_expression_offset;
				}
				else if (std::holds_alternative<return_value_statement>(s_in))
				{
					s_out.type	= statement_type::return_value;
					s_out.arg_a = std::get<return_value_statement>(s_in).m_expression_offset;
					s_out.arg_b = -1;
				}
				else if (std::holds_alternative<jump_statement>(s_in))
				{
					s_out.type	= statement_type::jump;
					s_out.arg_a = std::get<jump_statement>(s_in).m_target_index;
					s_out.arg_b = std::get<jump_statement>(s_in).m_expression_offset;
				}

				program->program_statements.push_back(s_out);
			}

			program->binding_table = indexer.get_binding_table();
			for (const auto& n : program->binding_table)
			{
				program->binding_table_cstr.push_back(n.c_str());
			}

			program->address_table = indexer.get_address_table();

			return program;
		}

		template<typename T_TRAITS>
		auto compile(const char* text, const variable* variables, int var_count, int* error) -> typename portable<T_TRAITS>::portable_compiled_program*
		{
			t_indexer<T_TRAITS> indexer;
			for (int v = 0; v < var_count; ++v)
			{
				indexer.add_user_variable(variables + v);
			}
			return compile_using_indexer<T_TRAITS>(text, error, indexer);
		}

	} // namespace program_details
} // namespace tp
#endif // #if (TP_COMPILER_ENABLED)

namespace tp
{
	namespace details
	{
#if TP_MODERN_CPP
		template<typename T_VAL>
		struct variable_helper
		{
			static inline constexpr variable readonly_var(const char* name, const T_VAL* v) noexcept
			{
				return {name, v}; // TODO: make distinction between rw and ro
			}

			static inline constexpr variable readwrite_var(const char* name, T_VAL* v) noexcept
			{
				return {name, v}; // TODO: make distinction between rw and ro
			}

			template<typename T_FUNC>
			static inline constexpr variable function0(const char* name, T_FUNC func)
			{
				typedef T_VAL (*zeroargfn)();
				return {name, zeroargfn{func}, tp::FUNCTION0 | tp::FLAG_PURE, 0};
			}

			template<typename T_FUNC>
			static inline constexpr variable function1(const char* name, T_FUNC func)
			{
				typedef T_VAL (*oneargfn)(T_VAL);
				return {name, oneargfn{func}, tp::FUNCTION1 | tp::FLAG_PURE, 0};
			}

			template<typename T_FUNC>
			static inline constexpr variable function2(const char* name, T_FUNC func)
			{
				typedef T_VAL (*twoargfn)(T_VAL, T_VAL);
				return {name, twoargfn{func}, tp::FUNCTION2 | tp::FLAG_PURE, 0};
			}

			template<typename T_FUNC>
			static inline constexpr variable function3(const char* name, T_FUNC func)
			{
				typedef T_VAL (*threeargfn)(T_VAL, T_VAL, T_VAL);
				return {name, threeargfn{func}, tp::FUNCTION3 | tp::FLAG_PURE, 0};
			}

			template<typename T_FUNC>
			static inline constexpr variable function4(const char* name, T_FUNC func)
			{
				typedef T_VAL (*fourargfn)(T_VAL, T_VAL, T_VAL, T_VAL);
				return {name, fourargfn{func}, tp::FUNCTION4 | tp::FLAG_PURE, 0};
			}

			template<typename T_FUNC>
			static inline constexpr variable function5(const char* name, T_FUNC func)
			{
				typedef T_VAL (*fiveargfn)(T_VAL, T_VAL, T_VAL, T_VAL, T_VAL);
				return {name, fiveargfn{func}, tp::FUNCTION5 | tp::FLAG_PURE, 0};
			}

			template<typename T_FUNC>
			static inline constexpr variable function6(const char* name, T_FUNC func)
			{
				typedef T_VAL (*sixargfn)(T_VAL, T_VAL, T_VAL, T_VAL, T_VAL, T_VAL);
				return {name, sixargfn{func}, tp::FUNCTION6 | tp::FLAG_PURE, 0};
			}

			template<typename T_FUNC>
			static inline constexpr variable function7(const char* name, T_FUNC func)
			{
				typedef T_VAL (*sevenargfn)(T_VAL, T_VAL, T_VAL, T_VAL, T_VAL, T_VAL, T_VAL);
				return {name, sevenargfn{func}, tp::FUNCTION7 | tp::FLAG_PURE, 0};
			}
		};
#endif

		struct serialized_program
		{
#pragma pack(push, 1)
			struct header_chunk
			{
				uint16_t magic;
				uint16_t version;
				uint16_t num_binding_names;
				uint16_t num_subprograms;
			};

			struct chunk_header
			{
				uint16_t size;
				uint16_t padding;
			};

			struct chunk : chunk_header
			{
				using header = chunk_header;
				char data[1];
			};

			using program_chunk	  = chunk;
			using statement_chunk = chunk;
			using data_chunk	  = chunk;
			using string_chunk	  = chunk;
			using user_var_chunk  = chunk;
#pragma pack(pop)

			template<typename T>
			static inline constexpr T round_up_to_multiple(T value, T multiple) noexcept
			{
				return ((value + multiple - 1) / multiple) * multiple;
			}

			static inline uint16_t alignment()
			{
				return 4;
			}

			struct subprogram
			{
				const statement_chunk* statements{nullptr};
				const data_chunk*	   data{nullptr};
			};

			const string_chunk*	   strings{nullptr};
			const user_var_chunk*  user_vars{nullptr};
			void*				   raw_data{nullptr};
			size_t				   raw_data_size{0};
			const header_chunk*	   header{nullptr};
			const statement_chunk* first_subprogram{nullptr};

#if (TP_COMPILER_ENABLED)
			serialized_program(const compiled_program* const* programs, int num_programs, std::vector<std::string>& user_vars_in)
			{
				std::vector<subprogram> subprograms;
				subprograms.resize(num_programs);

				auto user_var_count = user_vars_in.size();

				auto binding_name_count = programs[0]->get_binding_array_size();
				auto binding_names		= programs[0]->get_binding_names();
				auto most_bindings_idx	= 0;
				for (int subprogram_idx = 1; subprogram_idx < num_programs; ++subprogram_idx)
				{
					auto next_binding_name_count = programs[subprogram_idx]->get_binding_array_size();
					auto next_binding_names		 = programs[subprogram_idx]->get_binding_names();

					for (auto i = std::min(binding_name_count, next_binding_name_count); i > 0; --i)
					{
						if (strcmp(binding_names[i - 1], next_binding_names[i - 1]) != 0)
						{
							assert(false);
						}
					}

					if (binding_name_count < next_binding_name_count)
					{
						most_bindings_idx  = subprogram_idx;
						binding_name_count = next_binding_name_count;
						binding_names	   = next_binding_names;
					}
				}

				struct program_state
				{
					statement_chunk statement_data;
					data_chunk		expression_data;
					size_t			statement_data_data_size;
					size_t			expression_data_data_size;
				};

				auto [total_program_size, out_header, program_states, strs, user_var_indexes, user_var_data, user_var_data_data_size] =
					[&]() -> std::tuple<size_t, header_chunk, std::vector<program_state>, std::vector<string_chunk>, std::vector<int>, user_var_chunk, size_t> {
					std::vector<program_state> program_states;
					program_states.resize(num_programs);

					size_t total_program_size = 0;

					header_chunk out_header;
					out_header.magic			 = uint16_t(0x1010);
					out_header.version			 = uint16_t(0x0001);
					out_header.num_binding_names = uint16_t(binding_name_count);
					out_header.num_subprograms	 = uint16_t(num_programs);

					total_program_size += sizeof(header_chunk);

					for (int subprogram_idx = 0; subprogram_idx < num_programs; ++subprogram_idx)
					{
						auto& prog_state	  = program_states[subprogram_idx];
						auto  prog			  = programs[subprogram_idx];
						auto  expression_size = prog->get_data_size();
						auto  num_statements  = prog->get_statement_array_size();
						auto  statement_src	  = prog->get_statements();

						prog_state.statement_data.size = uint16_t(num_statements * sizeof(statement_src[0]));
						total_program_size += sizeof(chunk_header);
						prog_state.statement_data_data_size = round_up_to_multiple(prog_state.statement_data.size, alignment());
						total_program_size += prog_state.statement_data_data_size;

						prog_state.expression_data.size = uint16_t(expression_size);
						total_program_size += sizeof(chunk_header);
						prog_state.expression_data_data_size = round_up_to_multiple(prog_state.expression_data.size, alignment());
						total_program_size += prog_state.expression_data_data_size;
					}

					std::vector<string_chunk> strs;
					for (size_t i = 0; i < binding_name_count; ++i)
					{
						string_chunk chonk;
						chonk.size = uint16_t(::strlen(binding_names[i]) + 1);
						total_program_size += sizeof(chunk_header);
						total_program_size += round_up_to_multiple(chonk.size, uint16_t(alignment()));
						strs.push_back(chonk);
					}

					std::vector<int> user_var_indexes;
					user_var_indexes.resize(user_var_count);
					std::fill(std::begin(user_var_indexes), std::end(user_var_indexes), -1);
					for (size_t i = 0; i < binding_name_count; ++i)
					{
						for (size_t j = 0; j < user_var_count; ++j)
						{
							if (user_var_indexes[j] == -1)
							{
								if (_stricmp(binding_names[i], user_vars_in[j].c_str()) == 0)
								{
									user_var_indexes[j] = int(i);
									break;
								}
							}
						}
					}

					user_var_chunk user_var_data;
					user_var_data.size = uint16_t(sizeof(int) * user_var_count);
					total_program_size += sizeof(chunk_header);
					const auto user_var_data_data_size = round_up_to_multiple(user_var_data.size, alignment());
					total_program_size += user_var_data_data_size;

					return {total_program_size, out_header, program_states, strs, user_var_indexes, user_var_data, user_var_data_data_size};
				}();

				//

				if (total_program_size > sizeof(out_header))
				{
					assert(total_program_size > sizeof(header_chunk) + user_var_data_data_size + sizeof(chunk_header));
					char* const serialized_program = (char*)::malloc(total_program_size);
					char*		p				   = serialized_program;
					if (p != nullptr)
					{
						//

						this->header = (header_chunk*)p;
						::memcpy(p, &out_header, sizeof(out_header));
						p += sizeof(header_chunk);

						first_subprogram = (statement_chunk*)p;

						for (int subprogram_idx = 0; subprogram_idx < num_programs; ++subprogram_idx)
						{
							auto& prog_state = program_states[subprogram_idx];
							auto& subprogram = subprograms[subprogram_idx];

							auto prog		   = programs[subprogram_idx];
							auto statement_src = prog->get_statements();

							auto expression_src = prog->get_data();

							subprogram.statements = (statement_chunk*)p;
							::memcpy(p, &prog_state.statement_data, sizeof(chunk_header));
							p += sizeof(chunk_header);
							::memcpy(p, &statement_src[0], prog_state.statement_data.size);
							p += prog_state.statement_data_data_size;

							subprogram.data = (data_chunk*)p;
							::memcpy(p, &prog_state.expression_data, sizeof(chunk_header));
							p += sizeof(chunk_header);
							::memcpy(p, &expression_src[0], prog_state.expression_data.size);
							p += prog_state.expression_data_data_size;
						}

						this->strings = (string_chunk*)p;

						for (size_t i = 0; i < binding_name_count; ++i)
						{
							::memcpy(p, &strs[i], sizeof(chunk_header));
							p += sizeof(chunk_header);

							::memcpy(p, binding_names[i], strs[i].size - 1);
							p[strs[i].size - 1] = '\0';
							p += round_up_to_multiple(strs[i].size, alignment());
						}

						this->user_vars = (user_var_chunk*)p;
						::memcpy(p, &user_var_data, sizeof(chunk_header));
						p += sizeof(chunk_header);
						::memcpy(p, &user_var_indexes[0], user_var_data.size);
						p += user_var_data_data_size;

						//

						this->raw_data		= serialized_program;
						this->raw_data_size = total_program_size;
					}
				}
			}
#endif // #if (TP_COMPILER_ENABLED)

			serialized_program(const void* data, size_t data_size)
			{
				this->raw_data		= nullptr;
				this->raw_data_size = data_size;

				const char* p = (const char*)data;

				this->header = (header_chunk*)p;
				p += sizeof(header_chunk);

				first_subprogram = (statement_chunk*)p;

				for (int subprogram_idx = 0; subprogram_idx < this->header->num_subprograms; ++subprogram_idx)
				{
					auto subprogram_statements = (statement_chunk*)p;
					p += sizeof(statement_chunk::header);
					p += round_up_to_multiple(subprogram_statements->size, alignment());

					auto subprogram_data = (data_chunk*)p;
					p += sizeof(data_chunk::header);
					p += round_up_to_multiple(subprogram_data->size, alignment());
				}

				this->strings = (string_chunk*)p;

				for (size_t i = 0; i < header->num_binding_names; ++i)
				{
					auto chonk = (const string_chunk*)p;
					p += sizeof(string_chunk::header);
					p += round_up_to_multiple(chonk->size, alignment());
				}

				this->user_vars = (user_var_chunk*)p;
				p += sizeof(user_var_chunk::header);
				p += round_up_to_multiple(this->user_vars->size, alignment());
			}

			serialized_program(std::tuple<const void*, size_t> args) : serialized_program(std::get<0>(args), std::get<1>(args)) {}

			static inline serialized_program* create_using_buffer(void* data, size_t data_size)
			{
				auto prog	   = new serialized_program(data, data_size);
				prog->raw_data = data;
				return prog;
			}

			~serialized_program()
			{
				if (raw_data)
				{
					::free(raw_data);
				}
			}

			const std::tuple<statement_chunk*, data_chunk*> get_subprogram_data(int subprogram_index) const noexcept
			{
				const char* p = (const char*)first_subprogram;

				for (int i = 0; i < this->header->num_subprograms; ++i)
				{
					auto statements = (statement_chunk*)p;
					p += sizeof(statement_chunk::header);
					p += round_up_to_multiple(statements->size, alignment());

					auto subprogram_data = (data_chunk*)p;
					p += sizeof(data_chunk::header);
					p += round_up_to_multiple(subprogram_data->size, alignment());

					if (i == subprogram_index)
					{
						return std::tuple<statement_chunk*, data_chunk*>(statements, subprogram_data);
					}
				}

				return std::tuple<statement_chunk*, data_chunk*>(nullptr, nullptr);
			}

			const statement* get_statements_array(int subprogram_index) const noexcept
			{
				auto  tup		 = get_subprogram_data(subprogram_index);
				auto& statements = std::get<0>(tup);
				return reinterpret_cast<const statement*>(&statements->data[0]);
			}

			size_t get_statements_array_size(int subprogram_index) const noexcept
			{
				auto  tup		 = get_subprogram_data(subprogram_index);
				auto& statements = std::get<0>(tup);
				return statements->size / sizeof(statement);
			}

			const void* get_expression_data(int subprogram_index) const noexcept
			{
				auto  tup			  = get_subprogram_data(subprogram_index);
				auto& subprogram_data = std::get<1>(tup);
				return &subprogram_data->data[0];
			}

			size_t get_expression_size(int subprogram_index) const noexcept
			{
				auto  tup			  = get_subprogram_data(subprogram_index);
				auto& subprogram_data = std::get<1>(tup);
				return subprogram_data->size;
			}

			size_t get_num_bindings() const noexcept
			{
				return this->header->num_binding_names;
			}

			const char* get_binding_string(uint16_t index) const noexcept
			{
				if (this->header->num_binding_names > index)
				{
					auto p = (const char*)this->strings;
					for (size_t i = 0; i < this->header->num_binding_names; ++i)
					{
						auto chonk = (const string_chunk*)p;
						if (i == index)
						{
							return &chonk->data[0];
						}

						p += sizeof(string_chunk::header);
						p += round_up_to_multiple(chonk->size, alignment());
					}
				}
				return nullptr;
			}

			size_t get_raw_data_size() const noexcept
			{
				return raw_data_size;
			}

			const void* get_raw_data() const noexcept
			{
				return raw_data;
			}

			size_t get_num_user_vars() const noexcept
			{
				return (user_vars != nullptr) ? user_vars->size / sizeof(int) : 0;
			}

			const int* get_user_vars() const noexcept
			{
				return (user_vars != nullptr) ? (int*)&user_vars->data[0] : nullptr;
			}

			uint16_t get_num_subprograms() const noexcept
			{
				return header->num_subprograms;
			}
		};
	} // namespace details

	template<typename T_TRAITS>
	struct impl
	{
		using env_traits	  = T_TRAITS;
		using variable		  = ::tp::variable;
		using variable_lookup = ::tp::variable_lookup;
		using t_atom		  = typename env_traits::t_atom;
		using t_vector		  = typename env_traits::t_vector;
#if TP_MODERN_CPP
		using variable_factory = details::variable_helper<t_vector>;
#endif // #if TP_MODERN_CPP
		using serialized_program = details::serialized_program;
#if (TP_COMPILER_ENABLED)
		using t_indexer = program_details::t_indexer<T_TRAITS>;
#endif // #if (TP_COMPILER_ENABLED)

		static inline t_vector eval(const void* expr_buffer, const void* const expr_context[]) noexcept
		{
			return eval_details::eval_portable_impl<env_traits, t_atom, t_vector>((const expr_portable<env_traits>*)expr_buffer, (const unsigned char*)expr_buffer, expr_context);
		}

		static inline t_vector eval_program(const statement* statement_array, int statement_array_size, const void* expr_buffer, const void* const expr_context[])
		{
			for (int statement_index = 0; statement_index < statement_array_size;)
			{
				auto& statement = statement_array[statement_index];
				
				if (statement.type == statement_type::jump)
			    {
					if (statement.arg_b == -1 ||
						(0.0f != eval(((const char*)expr_buffer) + statement.arg_b, expr_context))) // TODO: traits function like nan for zero, or compare function?
					{
						statement_index = statement.arg_a;
						continue;
					}
					else
					{
						++statement_index;
					}
				}
				else if (statement.type == statement_type::return_value)
				{
					return eval(((const char*)expr_buffer) + statement.arg_a, expr_context);
				}
				else if (statement.type == statement_type::assign)
				{
					auto dest = (t_vector*)expr_context[statement.arg_a];
					*dest	  = eval(((const char*)expr_buffer) + statement.arg_b, expr_context);
					++statement_index;
				}
				else if (statement.type == statement_type::call)
				{
					eval(((const char*)expr_buffer) + statement.arg_a, expr_context);
					++statement_index;
				}
				else
				{
					// fatal, unknown statement
					assert(0);
					return env_traits::nan();
				}
			}
			
			// TODO: should probably make this a std::optional or something to indicate success or faillure
			return env_traits::nan();
		}

		static inline t_vector eval_program(serialized_program& prog, int subprogram, const void* const* binding_addrs)
		{
			return eval_program(prog.get_statements_array(subprogram), (int)prog.get_statements_array_size(subprogram), prog.get_expression_data(subprogram), binding_addrs);
		}

#if (TP_COMPILER_ENABLED)
		static compiled_expr* compile(const char* expression, const variable* variables, int var_count, int* error)
		{
			return expr_details::compile<env_traits>(expression, variables, var_count, error);
		}

		static compiled_program* compile_program(const char* program, const variable* variables, int var_count, int* error)
		{
			return (compiled_program*)program_details::compile<env_traits>(program, variables, var_count, error);
		}

		static compiled_program* compile_program_using_indexer(const char* program, int* error, program_details::t_indexer<env_traits>& indexer)
		{
			return (compiled_program*)program_details::compile_using_indexer<env_traits>(program, error, indexer);
		}

		static inline t_vector eval(const compiled_expr* n)
		{
			return eval(n->get_data(), n->get_binding_addresses());
		}

		static inline t_vector interp(const char* expression, int* error)
		{
			compiled_expr* n = compile(expression, 0, 0, error);
			t_vector	   ret;
			if (n)
			{
				ret = eval(n);
				delete n;
			}
			else
			{
				ret = env_traits::nan();
			}
			return ret;
		}

		static inline t_vector eval_program(compiled_program* prog)
		{
			auto array_size		= prog->get_binding_array_size();
			auto binding_addrs	= prog->get_binding_addresses();
			auto binding_names	= prog->get_binding_names();
			auto data_size		= prog->get_data_size();
			auto data			= prog->get_data();
			auto num_statements = prog->get_statement_array_size();
			auto statements		= prog->get_statements();

			return eval_program(statements, (int)num_statements, data, binding_addrs);
		}
#endif // #if (TP_COMPILER_ENABLED)
	};
} // namespace tp

#if TP_STANDARD_LIBRARY
namespace tp_stdlib
{
	template<typename T_ATOM>
	struct native_builtins_impl;

	template<>
	struct native_builtins_impl<double>
	{
		static double pi(void)
		{
			return 3.14159265358979323846;
		}

		static double e(void)
		{
			return 2.71828182845904523536;
		}

		static double fac(double a)
		{ /* simplest version of fac */
			if (a < 0.0)
				return nan();
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

		static double ncr(double n, double r)
		{
			if (n < 0.0 || r < 0.0 || n < r)
				return nan();
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

		static double npr(double n, double r)
		{
			return ncr(n, r) * fac(r);
		}

		static double fabs(double n)
		{
			return ::fabs(n);
		}

		static double acos(double n)
		{
			return ::acos(n);
		}

		static double cosh(double n)
		{
			return ::cosh(n);
		}

		static double cos(double n)
		{
			return ::cos(n);
		}

		static double exp(double n)
		{
			return ::exp(n);
		}

		static double asin(double n)
		{
			return ::asin(n);
		}

		static double sinh(double n)
		{
			return ::sinh(n);
		}

		static double sin(double n)
		{
			return ::sin(n);
		}

		static double sqrt(double n)
		{
			return ::sqrt(n);
		}

		static double log(double n)
		{
			return ::log(n);
		}

		static double log10(double n)
		{
			return ::log10(n);
		}

		static double atan(double n)
		{
			return ::atan(n);
		}

		static double tanh(double n)
		{
			return ::tanh(n);
		}

		static double fmod(double n, double m)
		{
			return ::fmod(n, m);
		}

		static double tan(double n)
		{
			return ::tan(n);
		}

		static double atan2(double n, double m)
		{
			return ::atan2(n, m);
		}

		static double pow(double n, double m)
		{
			return ::pow(n, m);
		}

		static double floor(double d)
		{
			return ::floor(d);
		}

		static double ceil(double d)
		{
			return ::ceil(d);
		}

		static double add(double a, double b)
		{
			return a + b;
		}

		static double sub(double a, double b)
		{
			return a - b;
		}

		static double mul(double a, double b)
		{
			return a * b;
		}

		static double divide(double a, double b)
		{
			return a / b;
		}

		static double negate(double a)
		{
			return -a;
		}

		static double comma(double a, double b)
		{
			(void)a;
			return b;
		}

		static double greater(double a, double b)
		{
			return a > b;
		}

		static double greater_eq(double a, double b)
		{
			return a >= b;
		}

		static double lower(double a, double b)
		{
			return a < b;
		}

		static double lower_eq(double a, double b)
		{
			return a <= b;
		}

		static double equal(double a, double b)
		{
			return a == b;
		}

		static double not_equal(double a, double b)
		{
			return a != b;
		}

		static double logical_and(double a, double b)
		{
			return a != 0.0 && b != 0.0;
		}

		static double logical_or(double a, double b)
		{
			return a != 0.0 || b != 0.0;
		}

		static double logical_not(double a)
		{
			return a == 0.0;
		}

		static double logical_notnot(double a)
		{
			return a != 0.0;
		}

		static double negate_logical_not(double a)
		{
			return -(a == 0.0);
		}

		static double negate_logical_notnot(double a)
		{
			return -(a != 0.0);
		}

		static double nul()
		{
			return 0.0f;
		}

		static double nan()
		{
			return std::numeric_limits<double>::quiet_NaN();
		}
	};

	template<>
	struct native_builtins_impl<float>
	{
		static float pi(void)
		{
			return 3.14159265358979323846f;
		}

		static float e(void)
		{
			return 2.71828182845904523536f;
		}

		static float fac(float a)
		{ /* simplest version of fac */
			if (a < 0.0f)
				return nan();
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
			return (float)result;
		}

		static float ncr(float n, float r)
		{
			if (n < 0.0f || r < 0.0f || n < r)
				return nan();
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
			return (float)result;
		}

		static float npr(float n, float r)
		{
			return ncr(n, r) * fac(r);
		}

		static float fabs(float n)
		{
			return ::fabsf(n);
		}

		static float acos(float n)
		{
			return ::acosf(n);
		}

		static float cosh(float n)
		{
			return ::coshf(n);
		}

		static float cos(float n)
		{
			return ::cosf(n);
		}

		static float exp(float n)
		{
			return ::expf(n);
		}

		static float asin(float n)
		{
			return ::asinf(n);
		}

		static float sinh(float n)
		{
			return ::sinhf(n);
		}

		static float sin(float n)
		{
			return ::sinf(n);
		}

		static float sqrt(float n)
		{
			return ::sqrtf(n);
		}

		static float log(float n)
		{
			return ::logf(n);
		}

		static float log10(float n)
		{
			return ::log10f(n);
		}

		static float atan(float n)
		{
			return ::atanf(n);
		}

		static float tanh(float n)
		{
			return ::tanhf(n);
		}

		static float fmod(float n, float m)
		{
			return ::fmodf(n, m);
		}

		static float tan(float n)
		{
			return ::tanf(n);
		}

		static float atan2(float n, float m)
		{
			return ::atan2f(n, m);
		}

		static float pow(float n, float m)
		{
			return ::powf(n, m);
		}

		static float floor(float d)
		{
			return ::floorf(d);
		}

		static float ceil(float d)
		{
			return ::ceilf(d);
		}

		static float add(float a, float b)
		{
			return a + b;
		}

		static float sub(float a, float b)
		{
			return a - b;
		}

		static float mul(float a, float b)
		{
			return a * b;
		}

		static float divide(float a, float b)
		{
			return a / b;
		}

		static float negate(float a)
		{
			return -a;
		}

		static float comma(float a, float b)
		{
			(void)a;
			return b;
		}

		static float greater(float a, float b)
		{
			return a > b;
		}

		static float greater_eq(float a, float b)
		{
			return a >= b;
		}

		static float lower(float a, float b)
		{
			return a < b;
		}

		static float lower_eq(float a, float b)
		{
			return a <= b;
		}

		static float equal(float a, float b)
		{
			return a == b;
		}

		static float not_equal(float a, float b)
		{
			return a != b;
		}

		static float logical_and(float a, float b)
		{
			return a != 0.0f && b != 0.0f;
		}

		static float logical_or(float a, float b)
		{
			return a != 0.0f || b != 0.0f;
		}

		static float logical_not(float a)
		{
			return a == 0.0f;
		}

		static float logical_notnot(float a)
		{
			return a != 0.0f;
		}

		static float negate_logical_not(float a)
		{
			return (float)-(a == 0.0f);
		}

		static float negate_logical_notnot(float a)
		{
			return (float)-(a != 0.0f);
		}

		static float nul()
		{
			return 0.0f;
		}

		static float nan()
		{
			return std::numeric_limits<float>::quiet_NaN();
		}
	};

	template<typename T_ATOM>
	struct native_builtins : native_builtins_impl<T_ATOM>
	{
		using t_impl = native_builtins_impl<T_ATOM>;

		static /*inline*/ constexpr tp::variable functions[] = {/* must be in alphabetical order */
																{"abs", t_impl::fabs, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"acos", t_impl::acos, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"asin", t_impl::asin, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"atan", t_impl::atan, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"atan2", t_impl::atan2, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"ceil", t_impl::ceil, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"cos", t_impl::cos, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"cosh", t_impl::cosh, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"e", t_impl::e, tp::FUNCTION0 | tp::FLAG_PURE, 0},
																{"exp", t_impl::exp, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"fac", t_impl::fac, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"floor", t_impl::floor, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"ln", t_impl::log, tp::FUNCTION1 | tp::FLAG_PURE, 0},
#ifdef TP_NAT_LOG
																{"log", t_impl::log, tp::FUNCTION1 | tp::FLAG_PURE, 0},
#else
																{"log", t_impl::log10, tp::FUNCTION1 | tp::FLAG_PURE, 0},
#endif
																{"log10", t_impl::log10, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"ncr", t_impl::ncr, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"npr", t_impl::npr, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"pi", t_impl::pi, tp::FUNCTION0 | tp::FLAG_PURE, 0},
																{"pow", t_impl::pow, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"sin", t_impl::sin, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"sinh", t_impl::sinh, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"sqrt", t_impl::sqrt, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"tan", t_impl::tan, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"tanh", t_impl::tanh, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{0, 0, 0, 0}};

		static /*inline*/ constexpr tp::variable operators[] = {/* must be in alphabetical order */
																{"add", t_impl::add, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"comma", t_impl::comma, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"divide", t_impl::divide, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"equal", t_impl::equal, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"fmod", t_impl::fmod, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"greater", t_impl::greater, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"greater_eq", t_impl::greater_eq, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"logical_and", t_impl::logical_and, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"logical_not", t_impl::logical_not, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"logical_notnot", t_impl::logical_notnot, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"logical_or", t_impl::logical_or, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"lower", t_impl::lower, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"lower_eq", t_impl::lower_eq, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"mul", t_impl::mul, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"negate", t_impl::negate, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"negate_logical_not", t_impl::negate_logical_not, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"negate_logical_notnot", t_impl::negate_logical_notnot, tp::FUNCTION1 | tp::FLAG_PURE, 0},
																{"not_equal", t_impl::not_equal, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"pow", t_impl::pow, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{"sub", t_impl::sub, tp::FUNCTION2 | tp::FLAG_PURE, 0},
																{0, 0, 0, 0}};
	};

	template<typename T_NATIVE>
	struct compiler_builtins : T_NATIVE
	{
		using t_base = T_NATIVE;

		static inline int name_compare(const char* a, const char* b, size_t n)
		{
			while (n && *a && (*a == *b))
			{
				++a;
				++b;
				--n;
			}
			if (n == 0)
			{
				return 0;
			}
			else
			{
				return (*(unsigned char*)a - *(unsigned char*)b);
			}
		}

		static inline const ::tp::variable* find_in_sorted_array(const char* name, int len, const ::tp::variable* vars, int vars_len)
		{
			int imin = 0;
			int imax = vars_len - 2;

			/*Binary search.*/
			while (imax >= imin)
			{
				const int i = (imin + ((imax - imin) / 2));
				int		  c = name_compare(name, vars[i].name, len);
				if (!c)
					c = '\0' - vars[i].name[len];
				if (c == 0)
				{
					return vars + i;
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

		static inline const ::tp::variable* find_by_name(const char* name, int len, const ::tp::variable_lookup* lookup)
		{
			if (lookup)
			{
				const ::tp::variable* var	= lookup->lookup;
				int					  iters = lookup->lookup_len;
				for (; iters; ++var, --iters)
				{
					if (name_compare(name, var->name, len) == 0 && var->name[len] == '\0')
					{
						return var;
					}
				}
			}

			auto res = find_in_sorted_array(name, len, t_base::functions, int(sizeof(t_base::functions) / sizeof(::tp::variable)));
			if (!res)
			{
				res = find_in_sorted_array(name, len, t_base::operators, int(sizeof(t_base::operators) / sizeof(::tp::variable)));
			}
			return res;
		}

		static const ::tp::variable* find_by_addr(const void* addr, const ::tp::variable_lookup* lookup)
		{
			if (lookup)
			{
				for (int i = 0; i < lookup->lookup_len; ++i)
				{
					if (lookup->lookup[i].address == addr)
					{
						return &lookup->lookup[i];
					}
				}
			}

			for (auto var = &t_base::functions[0]; var->name != 0; ++var)
			{
				if (var->address == addr)
				{
					return var;
				}
			}

			for (auto var = &t_base::operators[0]; var->name != 0; ++var)
			{
				if (var->address == addr)
				{
					return var;
				}
			}

			return nullptr;
		}
	};

	struct env_traits_f32
	{
		using t_atom	   = float;
		using t_vector	   = float;
		using t_vector_int = int;

#if TP_COMPILER_ENABLED
		using t_vector_builtins = compiler_builtins<native_builtins<t_vector>>;
#endif

		static inline t_vector load_atom(t_atom a) noexcept
		{
			return a;
		}

		static inline t_vector as_truth(t_vector a) noexcept
		{
			return (a != 0.0f) ? 1.0f : 0.0f;
		}

		static inline t_vector explicit_load_atom(double a) noexcept
		{
			return (t_vector)a;
		}

		static inline t_vector explicit_load_atom(int a) noexcept
		{
			return (t_vector)a;
		}

		static inline double explicit_store_double(t_vector a)
		{
			return (double)a;
		}

		static inline int explicit_store_int(t_vector a)
		{
			return (int)a;
		}

		static inline t_vector nan()
		{
			return std::numeric_limits<float>::quiet_NaN();
		}

#if TP_COMPILER_ENABLED
		static inline const ::tp::variable* find_by_name(const char* name, int len, const ::tp::variable_lookup* lookup)
		{
			return t_vector_builtins::find_by_name(name, len, lookup);
		}

		static const ::tp::variable* find_by_addr(const void* addr, const ::tp::variable_lookup* lookup)
		{
			return t_vector_builtins::find_by_addr(addr, lookup);
		}
#endif // #if TP_COMPILER_ENABLED
	};

	struct env_traits_d64
	{
		using t_atom	   = double;
		using t_vector	   = double;
		using t_vector_int = int;

#if TP_COMPILER_ENABLED
		using t_vector_builtins = compiler_builtins<native_builtins<t_vector>>;
#endif

		static inline t_vector load_atom(t_atom a) noexcept
		{
			return a;
		}

		static inline t_vector as_truth(t_vector a) noexcept
		{
			return (a != 0.0f) ? 1.0f : 0.0f;
		}

		static inline t_vector explicit_load_atom(double a) noexcept
		{
			return (t_vector)a;
		}

		static inline t_vector explicit_load_atom(int a) noexcept
		{
			return (t_vector)a;
		}

		static inline double explicit_store_double(t_vector a)
		{
			return (double)a;
		}

		static inline int explicit_store_int(t_vector a)
		{
			return (int)a;
		}

		static inline t_vector nan()
		{
			return std::numeric_limits<double>::quiet_NaN();
		}

#if TP_COMPILER_ENABLED
		static inline const ::tp::variable* find_by_name(const char* name, int len, const ::tp::variable_lookup* lookup)
		{
			return t_vector_builtins::find_by_name(name, len, lookup);
		}

		static const ::tp::variable* find_by_addr(const void* addr, const ::tp::variable_lookup* lookup)
		{
			return t_vector_builtins::find_by_addr(addr, lookup);
		}
#endif // #if TP_COMPILER_ENABLED
	};
} // namespace tp_stdlib
#endif // #if TP_STANDARD_LIBRARY

#if TP_TESTING
#if !TP_STANDARD_LIBRARY
#error TP_STANDARD_LIBRARY should be defined for testing
#endif // #if !TP_STANDARD_LIBRARY
using te = tp::impl<tp_stdlib::env_traits_f32>;
#endif // #if TP_TESTING

#endif /*__TINYPROG_H__*/
