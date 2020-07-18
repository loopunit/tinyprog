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

#ifndef __TINYEXPR_H__
#define __TINYEXPR_H__

#include <limits>

#ifndef TE_COMPILER_ENABLED
#	define TE_COMPILER_ENABLED 1
#endif // TE_COMPILER_ENABLED

namespace te
{
	enum
	{
		TE_VARIABLE = 0,

		TE_CONSTANT = 1,

		TE_FUNCTION0 = 8,
		TE_FUNCTION1,
		TE_FUNCTION2,
		TE_FUNCTION3,
		TE_FUNCTION4,
		TE_FUNCTION5,
		TE_FUNCTION6,
		TE_FUNCTION7,

		TE_CLOSURE0 = 16,
		TE_CLOSURE1,
		TE_CLOSURE2,
		TE_CLOSURE3,
		TE_CLOSURE4,
		TE_CLOSURE5,
		TE_CLOSURE6,
		TE_CLOSURE7,

		TE_FLAG_PURE = 32
	};

	struct variable
	{
		const char* name;
		const void* address;
		int			type;
		void*		context;
	};

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

		static float nan()
		{
			return std::numeric_limits<float>::quiet_NaN();
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

		static inline constexpr variable functions[] = {/* must be in alphabetical order */
			{"abs", t_impl::fabs, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"acos", t_impl::acos, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"asin", t_impl::asin, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"atan", t_impl::atan, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"atan2", t_impl::atan2, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"ceil", t_impl::ceil, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"cos", t_impl::cos, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"cosh", t_impl::cosh, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"e", t_impl::e, TE_FUNCTION0 | TE_FLAG_PURE, 0},
			{"exp", t_impl::exp, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"fac", t_impl::fac, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"floor", t_impl::floor, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"ln", t_impl::log, TE_FUNCTION1 | TE_FLAG_PURE, 0},
#ifdef TE_NAT_LOG
			{"log", t_impl::log, TE_FUNCTION1 | TE_FLAG_PURE, 0},
#else
			{"log", t_impl::log10, TE_FUNCTION1 | TE_FLAG_PURE, 0},
#endif
			{"log10", t_impl::log10, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"ncr", t_impl::ncr, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"npr", t_impl::npr, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"pi", t_impl::pi, TE_FUNCTION0 | TE_FLAG_PURE, 0},
			{"pow", t_impl::pow, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"sin", t_impl::sin, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"sinh", t_impl::sinh, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"sqrt", t_impl::sqrt, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"tan", t_impl::tan, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"tanh", t_impl::tanh, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{0, 0, 0, 0}};

		static inline constexpr variable operators[] = {/* must be in alphabetical order */
			{"add", t_impl::add, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"comma", t_impl::comma, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"divide", t_impl::divide, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"equal", t_impl::equal, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"fmod", t_impl::fmod, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"greater", t_impl::greater, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"greater_eq", t_impl::greater_eq, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"logical_and", t_impl::logical_and, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"logical_not", t_impl::logical_not, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"logical_notnot", t_impl::logical_notnot, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"logical_or", t_impl::logical_or, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"lower", t_impl::lower, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"lower_eq", t_impl::lower_eq, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"mul", t_impl::mul, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"negate", t_impl::negate, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"negate_logical_not", t_impl::negate_logical_not, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"negate_logical_notnot", t_impl::negate_logical_notnot, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"not_equal", t_impl::not_equal, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"pow", t_impl::pow, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"sub", t_impl::sub, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{0, 0, 0, 0}};

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

		static inline const variable* find_builtin_function(const char* name, int len)
		{
			int imin = 0;
			int imax = sizeof(functions) / sizeof(variable) - 2;

			/*Binary search.*/
			while (imax >= imin)
			{
				const int i = (imin + ((imax - imin) / 2));
				int		  c = name_compare(name, functions[i].name, len);
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

		static inline const variable* find_builtin_operator(const char* name, int len)
		{
			int imin = 0;
			int imax = sizeof(operators) / sizeof(variable) - 2;

			/*Binary search.*/
			while (imax >= imin)
			{
				const int i = (imin + ((imax - imin) / 2));
				int		  c = name_compare(name, operators[i].name, len);

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

		static inline const variable* find_builtin(const char* name, int len)
		{
			auto res = find_builtin_function(name, len);
			if (!res)
			{
				res = find_builtin_operator(name, len);
			}
			return res;
		}

		static inline const variable* find_builtin(const char* name)
		{
			return find_builtin(name, static_cast<int>(::strlen(name)));
		}

		static inline const void* find_builtin_address(const char* name)
		{
			auto b = find_builtin(name, static_cast<int>(::strlen(name)));
			if (b)
			{
				return b->address;
			}
			return nullptr;
		}
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
			return (((t) & (TE_FUNCTION0 | TE_CLOSURE0)) ? ((t)&0x00000007) : 0);
		}

		template<typename T_TRAITS, typename T_ATOM, typename T_VECTOR>
		static inline auto eval_portable_impl(const expr_portable<T_TRAITS>* n_portable,
			const unsigned char*											 expr_buffer,
			const void* const expr_context[]) noexcept -> typename T_VECTOR
		{
			using t_atom	 = T_ATOM;
			using t_vector	 = T_VECTOR;
			using t_traits	 = T_TRAITS;
			using t_builtins = typename T_TRAITS::t_vector_builtins;

#define TE_FUN(...) ((t_vector(*)(__VA_ARGS__))expr_context[n_portable->function])

#define M(e)                                                                                                           \
	eval_portable_impl<T_TRAITS, T_ATOM, T_VECTOR>(                                                                    \
		(const expr_portable<t_traits>*)&expr_buffer[n_portable->parameters[e]], expr_buffer, expr_context)

			switch (type_mask(n_portable->type))
			{
			case TE_CONSTANT:
				return t_traits::load_atom(n_portable->value);

			case TE_VARIABLE:
				return t_traits::load_atom((expr_context != nullptr)
											   ? *((const t_vector*)(expr_context[n_portable->bound]))
											   : t_builtins::nan());

			case TE_FUNCTION0:
			case TE_FUNCTION1:
			case TE_FUNCTION2:
			case TE_FUNCTION3:
			case TE_FUNCTION4:
			case TE_FUNCTION5:
			case TE_FUNCTION6:
			case TE_FUNCTION7:
			{
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
					return t_builtins::nan();
				}
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
				auto arity_params = (void*)expr_context[n_portable->parameters[arity(n_portable->type)]];

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
					return t_builtins::nan();
				}
			}

			default:
				return t_builtins::nan();
			}
#undef TE_FUN
#undef M
		}
	} // namespace eval_details

	struct env_traits
	{
		using t_atom			= float;
		using t_vector			= float;
		using t_vector_int		= int;
		using t_atom_builtins	= native_builtins<t_atom>;
		using t_vector_builtins = native_builtins<t_vector>;

		static inline t_vector load_atom(t_atom a) noexcept
		{
			return a;
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
	};

	inline env_traits::t_vector eval(const void* expr_buffer, const void* const expr_context[]) noexcept
	{
		return eval_details::eval_portable_impl<env_traits, env_traits::t_atom, env_traits::t_vector>(
			(const expr_portable<env_traits>*)expr_buffer, (const unsigned char*)expr_buffer, expr_context);
	}

#if (TE_COMPILER_ENABLED)
	struct compiled_expr
	{
		virtual ~compiled_expr() = default;

		size_t				 get_binding_array_size() const;
		const void* const*	 get_binding_addresses() const;
		const char* const*	 get_binding_names() const;
		size_t				 get_expr_data_size() const;
		const unsigned char* get_expr_data() const;
	};

	compiled_expr* compile(const char* expression, const variable* variables, int var_count, int* error);

	inline env_traits::t_vector eval(const compiled_expr* n)
	{
		return eval(n->get_expr_data(), n->get_binding_addresses());
	}

	inline env_traits::t_vector interp(const char* expression, int* error)
	{
		compiled_expr*		 n = compile(expression, 0, 0, error);
		env_traits::t_vector ret;
		if (n)
		{
			ret = eval(n);
			delete n;
		}
		else
		{
			ret = env_traits::t_vector_builtins::nan();
		}
		return ret;
	}

	// struct program
	//{
	//	virtual void bind_variables(const variable* variables, int var_count) = 0;
	//	virtual bool compile_statement(const char* statement, int* error)		 = 0;
	//
	//	virtual const void* const*	 get_binding_addresses()				 = 0;
	//	virtual const char* const*	 get_binding_names()					 = 0;
	//	virtual size_t				 get_data_size(const compiled_expr n) = 0;
	//	virtual const unsigned char* get_data(const compiled_expr n)		 = 0;
	//};

#endif // #if (TE_COMPILER_ENABLED)
} // namespace te

#define te_free(X) delete X

#endif /*__TINYEXPR_H__*/
