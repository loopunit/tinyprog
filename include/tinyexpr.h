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

#ifndef TE_IMPLEMENT
#	define TE_IMPLEMENT 0
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
		TE_FUNCTION_MAX,

		TE_CLOSURE0 = 16,
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

		template<typename T_HANDLE_CONSTANT,
			typename T_HANDLE_VARIABLE,
			typename T_HANDLE_FUNCTION,
			typename T_HANDLE_CLOSURE,
			typename T_HANDLE_ERROR>
		static inline auto eval_generic(int type,
			T_HANDLE_CONSTANT				handle_constant,
			T_HANDLE_VARIABLE				handle_variable,
			T_HANDLE_FUNCTION				handle_function,
			T_HANDLE_CLOSURE				handle_closure,
			T_HANDLE_ERROR					handle_error)
		{
			const auto t = type_mask(type);
			if (t == TE_CONSTANT)
			{
				return handle_constant();
			}
			else if (t == TE_VARIABLE)
			{
				return handle_variable();
			}
			else if (t >= TE_FUNCTION0)
			{
				if (t < TE_FUNCTION_MAX)
				{
					return handle_function(t - TE_FUNCTION0);
				}
				if (t >= TE_CLOSURE0)
				{
					if (t < TE_CLOSURE_MAX)
					{
						return handle_closure(t - TE_CLOSURE0);
					}
				}
			}

			return handle_error();
		}

		template<typename T_VECTOR, typename T_RET, typename T_EVAL_ARG>
		auto eval_function(int a, const void* fn, T_RET error_val, T_EVAL_ARG eval_arg) -> T_RET
		{
#define TE_FUN(...) ((T_RET(*)(__VA_ARGS__))fn)
			switch (a)
			{
			case 0:
				return TE_FUN(void)();
			case 1:
				return TE_FUN(T_VECTOR)(eval_arg(0));
			case 2:
				return TE_FUN(T_VECTOR, T_VECTOR)(eval_arg(0), eval_arg(1));
			case 3:
				return TE_FUN(T_VECTOR, T_VECTOR, T_VECTOR)(eval_arg(0), eval_arg(1), eval_arg(2));
			case 4:
				return TE_FUN(T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR)(
					eval_arg(0), eval_arg(1), eval_arg(2), eval_arg(3));
			case 5:
				return TE_FUN(T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR)(
					eval_arg(0), eval_arg(1), eval_arg(2), eval_arg(3), eval_arg(4));
			case 6:
				return TE_FUN(T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR)(
					eval_arg(0), eval_arg(1), eval_arg(2), eval_arg(3), eval_arg(4), eval_arg(5));
			case 7:
				return TE_FUN(T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR)(
					eval_arg(0), eval_arg(1), eval_arg(2), eval_arg(3), eval_arg(4), eval_arg(5), eval_arg(6));
			}
#undef TE_FUN
			return error_val;
		}

		template<typename T_VECTOR, typename T_RET, typename T_EVAL_ARG>
		auto eval_closure(int a, const void* fn, const void* arity_params, T_RET error_val, T_EVAL_ARG eval_arg)
			-> T_RET
		{
#define TE_FUN(...) ((T_RET(*)(__VA_ARGS__))fn)
			switch (a)
			{
			case 0:
				return TE_FUN(const void*)(arity_params);
			case 1:
				return TE_FUN(const void*, T_VECTOR)(arity_params, eval_arg(0));
			case 2:
				return TE_FUN(const void*, T_VECTOR, T_VECTOR)(arity_params, eval_arg(0), eval_arg(1));
			case 3:
				return TE_FUN(const void*, T_VECTOR, T_VECTOR, T_VECTOR)(
					arity_params, eval_arg(0), eval_arg(1), eval_arg(2));
			case 4:
				return TE_FUN(const void*, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR)(
					arity_params, eval_arg(0), eval_arg(1), eval_arg(2), eval_arg(3));
			case 5:
				return TE_FUN(const void*, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR)(
					arity_params, eval_arg(0), eval_arg(1), eval_arg(2), eval_arg(3), eval_arg(4));
			case 6:
				return TE_FUN(const void*, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR)(
					arity_params, eval_arg(0), eval_arg(1), eval_arg(2), eval_arg(3), eval_arg(4), eval_arg(5));
			case 7:
				return TE_FUN(const void*, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR, T_VECTOR)(
					arity_params,
					eval_arg(0),
					eval_arg(1),
					eval_arg(2),
					eval_arg(3),
					eval_arg(4),
					eval_arg(5),
					eval_arg(6));
			}
#undef TE_FUN
			return error_val;
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

			auto eval_arg = [&](int e) {
				return eval_portable_impl<T_TRAITS, T_ATOM, T_VECTOR>(
					(const expr_portable<t_traits>*)&expr_buffer[n_portable->parameters[e]], expr_buffer, expr_context);
			};

			return eval_generic(
				n_portable->type,
				[&]() { return t_traits::load_atom(n_portable->value); },
				[&]() {
					return t_traits::load_atom((expr_context != nullptr)
												   ? *((const t_vector*)(expr_context[n_portable->bound]))
												   : t_builtins::nan());
				},
				[&](int a) {
					return eval_function<t_vector>(a, expr_context[n_portable->function], t_builtins::nan(), eval_arg);
				},
				[&](int a) {
					return eval_closure<t_vector>(a,
						expr_context[n_portable->function],
						(void*)expr_context[n_portable->parameters[a]],
						t_builtins::nan(),
						eval_arg);
				},
				[&]() { return t_builtins::nan(); });
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
		size_t				 get_data_size() const;
		const unsigned char* get_data() const;
	};

	compiled_expr* compile(const char* expression, const variable* variables, int var_count, int* error);

	inline env_traits::t_vector eval(const compiled_expr* n)
	{
		return eval(n->get_data(), n->get_binding_addresses());
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

	struct compiled_program
	{
		virtual ~compiled_program() = default;

		void bind_variables(const variable* variables, int var_count);
		bool compile_statement(const char* statement, int* error);

		size_t				 get_expression_offset_array_size() const;
		const size_t*		 get_expression_offsets() const;
		size_t				 get_binding_array_size() const;
		const void* const*	 get_binding_addresses() const;
		const char* const*	 get_binding_names() const;
		size_t				 get_data_size() const;
		const unsigned char* get_data() const;
	};
#endif // #if (TE_COMPILER_ENABLED)
} // namespace te

#if TE_IMPLEMENT
#	include <unordered_map>
#	include <vector>
#	include <memory>
#	include <cassert>

namespace te
{
#	if (TE_COMPILER_ENABLED)
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

		typedef t_vector (*te_fun2)(t_vector, t_vector);

		enum
		{
			TOK_NULL = TE_CLOSURE_MAX,
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

		static inline bool is_pure(int t) noexcept
		{
			return (((t)&TE_FLAG_PURE) != 0);
		}

		static inline bool is_function(int t) noexcept
		{
			return (((t)&TE_FUNCTION0) != 0);
		}

		static inline bool is_closure(int t) noexcept
		{
			return (((t)&TE_CLOSURE0) != 0);
		}

#		define NEW_EXPR(type, ...)                                                                                    \
			[&]() {                                                                                                    \
				const expr_native* _args[] = {__VA_ARGS__};                                                            \
				return new_expr((type), _args);                                                                        \
			}()

		static expr_native* new_expr(const int type, const expr_native* parameters[])
		{
			const int	 arity = eval_details::arity(type);
			const int	 psize = sizeof(void*) * arity;
			const int	 size  = (sizeof(expr_native) - sizeof(void*)) + psize + (is_closure(type) ? sizeof(void*) : 0);
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
				if (!*s->next || *s->next == ';')
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
							const auto t = eval_details::type_mask(var->type);
							if (t == TE_VARIABLE)
							{
								s->type	 = TOK_VARIABLE;
								s->bound = (const t_atom*)var->address;
							}
							else if (t >= TE_FUNCTION0)
							{
								if (t < TE_FUNCTION_MAX)
								{
									s->type		= var->type;
									s->function = var->address;
								}
								else if (t >= TE_CLOSURE0)
								{
									if (t < TE_CLOSURE_MAX)
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

			const auto t = eval_details::type_mask(s->type);

			if (t == TOK_NUMBER)
			{
				ret		   = new_expr(TE_CONSTANT, 0);
				ret->value = s->value;
				next_token(s);
			}
			else if (t == TOK_VARIABLE)
			{
				ret		   = new_expr(TE_VARIABLE, 0);
				ret->bound = s->bound;
				next_token(s);
			}
			else if ((t >= TE_FUNCTION0 && t < TE_FUNCTION_MAX) || (t >= TE_CLOSURE0 && t < TE_CLOSURE_MAX))
			{
				const auto arity = eval_details::arity(s->type);
				if (arity == 0)
				{
					ret			  = new_expr(s->type, 0);
					ret->function = s->function;
					if (is_closure(s->type))
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
				}
			}
			else if (t == TOK_OPEN)
			{
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
			}
			else
			{
				ret		   = new_expr(0, 0);
				s->type	   = TOK_ERROR;
				ret->value = t_builtins::nan();
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

#		ifdef TE_POW_FROM_RIGHT
		static expr_native* factor(state* s)
		{
			/* <factor>    =    <power> {"^" <power>} */
			expr_native* ret = power(s);

			const void*	 left_function = NULL;
			expr_native* insertion	   = 0;

			if (ret->type == (TE_FUNCTION1 | TE_FLAG_PURE) &&
				(ret->function == t_builtins::find_builtin_address("negate") ||
					ret->function == t_builtins::find_builtin_address("logical_not") ||
					ret->function == t_builtins::find_builtin_address("logical_notnot") ||
					ret->function == t_builtins::find_builtin_address("negate_logical_not") ||
					ret->function == t_builtins::find_builtin_address("negate_logical_notnot")))
			{
				left_function	= ret->function;
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
					insert->function	= t;
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
#		else
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
#		endif

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
			if (!n)
				return t_builtins::nan();

			auto eval_arg = [&](int e) {
				return eval_native((const expr_native*)n->parameters[e]);
			};

			return eval_details::eval_generic(
				n->type,
				[&]() { return n->value; },
				[&]() { return *n->bound; },
				[&](int a) {
					return eval_details::eval_function<t_vector>(a, n->function, t_builtins::nan(), eval_arg);
				},
				[&](int a) {
					return eval_details::eval_closure<t_vector>(
						a, n->function, (void*)n->parameters[a], t_builtins::nan(), eval_arg);
				},
				[&]() { return t_builtins::nan(); });
		}

		static void optimize(expr_native* n)
		{
			/* Evaluates as much as possible. */
			if (n->type == TE_CONSTANT)
				return;
			if (n->type == TE_VARIABLE)
				return;

			/* Only optimize out functions flagged as pure. */
			if (is_pure(n->type))
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

		static expr_native* compile_native(const char* expression, const variable* variables, int var_count, int* error)
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

			if (t == TE_CONSTANT)
			{
				printf("%f\n", n->value);
			}
			else if (t == TE_VARIABLE)
			{
				printf("bound %p\n", n->bound);
			}
			else if ((t >= TE_FUNCTION0 && t < TE_FUNCTION_MAX) || (t >= TE_CLOSURE0 && t < TE_CLOSURE_MAX))
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
		};

		struct expr_portable_expression_build_bindings
		{
			std::vector<const void*>
									 index_to_address; // this contains the native function/value address as originally compiled
			std::vector<std::string> index_to_name;
			std::vector<const char*> index_to_name_c_str;
		};

		struct compiled_expr : ::te::compiled_expr
		{
			expr_portable_expression_build_indexer	m_indexer;
			expr_portable_expression_build_bindings m_bindings;
			std::unique_ptr<unsigned char>			m_build_buffer;
			size_t									m_build_buffer_size;
		};

		static size_t export_estimate(const expr_native* n,
			size_t&										 export_size,
			const variable*								 lookup,
			int											 lookup_len,
			name_map&									 name_map,
			index_map&									 index_map,
			int&										 index_counter)
		{
			if (!n)
				return export_size;

			export_size += sizeof(expr_native);

			auto eval_arg = [&](int e) {
				export_estimate((const expr_native*)n->parameters[e],
					export_size,
					lookup,
					lookup_len,
					name_map,
					index_map,
					index_counter);
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

					if (var->type >= TE_CLOSURE0 && var->type < TE_CLOSURE_MAX)
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

			return eval_details::eval_generic(
				n->type,
				[&]() { return export_size; },
				[&]() {
					auto res = handle_addr(native<t_traits>::find_bind_by_addr(n->bound, lookup, lookup_len));
					assert(res);
					return export_size;
				},
				[&](int a) {
					auto res = handle_addr(native<t_traits>::find_bind_or_any_by_addr(n->function, lookup, lookup_len));
					assert(res);
					export_size += sizeof(n->parameters[0]) * a;

					for (int i = 0; i < a; ++i)
					{
						eval_arg(i);
					}
					return export_size;
				},
				[&](int a) {
					auto res = handle_addr(native<t_traits>::find_bind_or_any_by_addr(n->function, lookup, lookup_len));
					assert(res);
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
		static size_t export_write(const expr_native* n,
			size_t&									  export_size,
			const variable*							  lookup,
			int										  lookup_len,
			const unsigned char*					  out_buffer,
			T_REGISTER_FUNC							  register_func)
		{
			if (!n)
				return export_size;

			auto n_out = (expr_portable<t_traits>*)(out_buffer + export_size);

			export_size += sizeof(expr_native);
			n_out->type = n->type;

			auto eval_arg = [&](int e) {
				return export_write(
					(const expr_native*)n->parameters[e], export_size, lookup, lookup_len, out_buffer, register_func);
			};

			return eval_details::eval_generic(
				n->type,
				[&]() {
					n_out->value = n->value;
					return export_size;
				},
				[&]() {
					register_func(n->bound, n_out, native<t_traits>::find_bind_by_addr(n->bound, lookup, lookup_len));
					return export_size;
				},
				[&](int a) {
					register_func(n->function,
						n_out,
						native<t_traits>::find_bind_or_any_by_addr(n->function, lookup, lookup_len));

					export_size += sizeof(n->parameters[0]) * eval_details::arity(n->type);

					for (int i = 0; i < eval_details::arity(n->type); ++i)
					{
						n_out->parameters[i] = export_size;
						eval_arg(i);
					}
					return export_size;
				},
				[&](int a) {
					register_func(n->function,
						n_out,
						native<t_traits>::find_bind_or_any_by_addr(n->function, lookup, lookup_len));

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

		static t_vector eval_compare(const expr_native* n,
			const expr_portable<t_traits>*				n_portable,
			const unsigned char*						expr_buffer,
			const void* const							expr_context[])
		{
			if (!n)
				return t_traits::nan();

			assert(n->type == n_portable->type);

			auto eval_arg = [&](int e) {
				return eval_compare((const expr_native*)n->parameters[e],
					(const expr_portable*)&expr_buffer[n_portable->parameters[e]],
					expr_buffer,
					expr_context);
			};

			return eval_details::eval_generic(
				n->type,
				[&]() { return n_portable->value; },
				[&]() {
					assert(n->bound == expr_context[n_portable->bound]);
					return t_traits::load_atom(*((const t_atom*)(expr_context[n_portable->bound])));
				},
				[&](int a) {
					assert(n->function == expr_context[n_portable->function]);
					return eval_details::eval_function<t_vector>(
						a, expr_context[n_portable->function], t_traits::nan(), eval_arg);
				},
				[&](int a) {
					assert(n->function == expr_context[n_portable->function]);
					assert(n->parameters[arity(n->type)] == expr_context[n_portable->parameters[arity(n->type)]]);

					return eval_details::eval_closure<t_vector>(a,
						expr_context[n_portable->function],
						(void*)expr_context[n_portable->parameters[arity(n->type)]],
						t_traits::nan(),
						eval_arg);
				},
				[&]() { return t_traits::nan(); });
		}

		struct compiled_program : ::te::compiled_program
		{
			expr_portable_expression_build_indexer	m_indexer;
			expr_portable_expression_build_bindings m_bindings;
			std::unique_ptr<unsigned char>			m_build_buffer;
			size_t									m_build_buffer_size;
		};
	};

	namespace expr_details
	{
		template<typename T_TRAITS>
		compiled_expr* compile(const char* expression, const variable* variables, int var_count, int* error)
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

						if (v->type >= TE_CLOSURE0 && v->type < TE_CLOSURE_MAX)
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
		size_t get_binding_array_size(const compiled_expr* n)
		{
			auto n_impl = (const portable<T_TRAITS>::compiled_expr*)n;
			if (n_impl)
			{
				return n_impl->m_bindings.index_to_address.size();
			}
			return 0;
		}

		template<typename T_TRAITS>
		const void* const* get_binding_addresses(const compiled_expr* n)
		{
			auto n_impl = (const portable<T_TRAITS>::compiled_expr*)n;
			if (n_impl && (n_impl->m_bindings.index_to_address.size() > 0))
			{
				return &(*n_impl->m_bindings.index_to_address.cbegin());
			}
			return nullptr;
		}

		template<typename T_TRAITS>
		const char* const* get_binding_names(const compiled_expr* n)
		{
			auto n_impl = (const portable<T_TRAITS>::compiled_expr*)n;
			if (n_impl)
			{
				return &(*n_impl->m_bindings.index_to_name_c_str.cbegin());
			}
			return nullptr;
		}

		template<typename T_TRAITS>
		size_t get_data_size(const compiled_expr* n)
		{
			auto n_impl = (const portable<T_TRAITS>::compiled_expr*)n;
			if (n_impl)
			{
				return n_impl->m_build_buffer_size;
			}
			return 0;
		}

		template<typename T_TRAITS>
		const unsigned char* get_data(const compiled_expr* n)
		{
			auto n_impl = (const portable<T_TRAITS>::compiled_expr*)n;
			if (n_impl)
			{
				return n_impl->m_build_buffer.get();
			}
			return nullptr;
		}
	} // namespace expr_details

	namespace program_details
	{
		template<typename T_TRAITS>
		void bind_variables(compiled_program* p, const variable* variables, int var_count)
		{
			auto p_impl = (const portable<T_TRAITS>::compiled_program*)p;
		}

		template<typename T_TRAITS>
		bool compile_statement(compiled_program* p, const char* statement, int* error)
		{
			auto p_impl = (const portable<T_TRAITS>::compiled_program*)p;
			return false;
		}

		template<typename T_TRAITS>
		size_t get_expression_offset_array_size(const compiled_program* p)
		{
			auto p_impl = (const portable<T_TRAITS>::compiled_program*)p;
			return 0;
		}

		template<typename T_TRAITS>
		const size_t* get_expression_offsets(const compiled_program* p)
		{
			auto p_impl = (const portable<T_TRAITS>::compiled_program*)p;
			return nullptr;
		}

		template<typename T_TRAITS>
		size_t get_binding_array_size(const compiled_program* p)
		{
			auto p_impl = (const portable<T_TRAITS>::compiled_program*)p;
			return 0;
		}

		template<typename T_TRAITS>
		const void* const* get_binding_addresses(const compiled_program* p)
		{
			auto p_impl = (const portable<T_TRAITS>::compiled_program*)p;
			return nullptr;
		}

		template<typename T_TRAITS>
		const char* const* get_binding_names(const compiled_program* p)
		{
			auto p_impl = (const portable<T_TRAITS>::compiled_program*)p;
			return nullptr;
		}

		template<typename T_TRAITS>
		size_t get_data_size(const compiled_program* p)
		{
			auto p_impl = (const portable<T_TRAITS>::compiled_program*)p;
			return 0;
		}

		template<typename T_TRAITS>
		const unsigned char* get_data(const compiled_program* p)
		{
			auto p_impl = (const portable<T_TRAITS>::compiled_program*)p;
			return nullptr;
		}
	
		template<typename T_TRAITS>
		compiled_program* create_program()
		{
			return nullptr;
		}
	} // namespace program_details

	compiled_expr* compile(const char* expression, const variable* variables, int var_count, int* error)
	{
		return expr_details::compile<env_traits>(expression, variables, var_count, error);
	}

	size_t compiled_expr::get_binding_array_size() const
	{
		return expr_details::get_binding_array_size<env_traits>(this);
	}

	const void* const* compiled_expr::get_binding_addresses() const
	{
		return expr_details::get_binding_addresses<env_traits>(this);
	}

	const char* const* compiled_expr::get_binding_names() const
	{
		return expr_details::get_binding_names<env_traits>(this);
	}

	size_t compiled_expr::get_data_size() const
	{
		return expr_details::get_data_size<env_traits>(this);
	}

	const unsigned char* compiled_expr::get_data() const
	{
		return expr_details::get_data<env_traits>(this);
	}

	////

	compiled_program* create_program()
	{
		return program_details::create_program<env_traits>();
	}

	void compiled_program::bind_variables(const variable* variables, int var_count)
	{
		return program_details::bind_variables<env_traits>(this, variables, var_count);
	}

	bool compiled_program::compile_statement(const char* statement, int* error)
	{
		return program_details::compile_statement<env_traits>(this, statement, error);
	}

	size_t compiled_program::get_expression_offset_array_size() const
	{
		return program_details::get_expression_offset_array_size<env_traits>(this);
	}

	const size_t* compiled_program::get_expression_offsets() const
	{
		return program_details::get_expression_offsets<env_traits>(this);
	}

	size_t compiled_program::get_binding_array_size() const
	{
		return program_details::get_binding_array_size<env_traits>(this);
	}

	const void* const* compiled_program::get_binding_addresses() const
	{
		return program_details::get_binding_addresses<env_traits>(this);
	}

	const char* const* compiled_program::get_binding_names() const
	{
		return program_details::get_binding_names<env_traits>(this);
	}

	size_t compiled_program::get_data_size() const
	{
		return program_details::get_data_size<env_traits>(this);
	}

	const unsigned char* compiled_program::get_data() const
	{
		return program_details::get_data<env_traits>(this);
	}
}

#	endif // #if (TE_COMPILER_ENABLED)
#endif

#endif /*__TINYEXPR_H__*/
