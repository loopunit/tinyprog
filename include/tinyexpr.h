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

struct te_variable
{
	const char* name;
	const void* address;
	int			type;
	void*		context;
};

template<typename T_TRAITS>
struct te_expr_portable
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

template<typename T>
inline T te_type_mask(const T t) noexcept
{
	return ((t)&0x0000001F);
}

template<typename T>
inline T te_arity(const T t) noexcept
{
	return (((t) & (TE_FUNCTION0 | TE_CLOSURE0)) ? ((t)&0x00000007) : 0);
}

namespace details
{
	template<typename T_TRAITS, typename T_ATOM, typename T_VECTOR>
	static inline auto te_eval_portable_impl(const te_expr_portable<T_TRAITS>* n_portable,
		const unsigned char*												   expr_buffer,
		const void* const expr_context[]) noexcept -> typename T_VECTOR
	{
		using t_atom	 = T_ATOM;
		using t_vector	 = T_VECTOR;
		using t_traits	 = T_TRAITS;
		using t_builtins = typename T_TRAITS::t_vector_builtins;

#define TE_FUN(...) ((t_vector(*)(__VA_ARGS__))expr_context[n_portable->function])

#define M(e)                                                                                                           \
	te_eval_portable_impl<T_TRAITS, T_ATOM, T_VECTOR>(                                                                 \
		(const te_expr_portable<t_traits>*)&expr_buffer[n_portable->parameters[e]], expr_buffer, expr_context)

		switch (te_type_mask(n_portable->type))
		{
		case TE_CONSTANT:
			return t_traits::load_atom(n_portable->value);

		case TE_VARIABLE:
			return t_traits::load_atom((expr_context != nullptr) ? *((const t_vector*)(expr_context[n_portable->bound]))
																 : t_builtins::te_nan());

		case TE_FUNCTION0:
		case TE_FUNCTION1:
		case TE_FUNCTION2:
		case TE_FUNCTION3:
		case TE_FUNCTION4:
		case TE_FUNCTION5:
		case TE_FUNCTION6:
		case TE_FUNCTION7:
		{
			switch (te_arity(n_portable->type))
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
				return t_builtins::te_nan();
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
			auto arity_params = (void*)expr_context[n_portable->parameters[te_arity(n_portable->type)]];

			switch (te_arity(n_portable->type))
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
				return t_builtins::te_nan();
			}
		}

		default:
			return t_builtins::te_nan();
		}
#undef TE_FUN
#undef M
	}

	template<typename T_ATOM>
	struct te_native_builtins_impl;

	template<>
	struct te_native_builtins_impl<double>
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
				return te_nan();
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
				return te_nan();
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

		static float te_nan()
		{
			return std::numeric_limits<double>::quiet_NaN();
		}
	};

	template<>
	struct te_native_builtins_impl<float>
	{
		static float te_pi(void)
		{
			return 3.14159265358979323846f;
		}

		static float te_e(void)
		{
			return 2.71828182845904523536f;
		}

		static float te_fac(float a)
		{ /* simplest version of fac */
			if (a < 0.0f)
				return te_nan();
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

		static float te_ncr(float n, float r)
		{
			if (n < 0.0f || r < 0.0f || n < r)
				return te_nan();
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

		static float te_npr(float n, float r)
		{
			return te_ncr(n, r) * te_fac(r);
		}

		static float te_fabs(float n)
		{
			return ::fabsf(n);
		}

		static float te_acos(float n)
		{
			return ::acosf(n);
		}

		static float te_cosh(float n)
		{
			return ::coshf(n);
		}

		static float te_cos(float n)
		{
			return ::cosf(n);
		}

		static float te_exp(float n)
		{
			return ::expf(n);
		}

		static float te_asin(float n)
		{
			return ::asinf(n);
		}

		static float te_sinh(float n)
		{
			return ::sinhf(n);
		}

		static float te_sin(float n)
		{
			return ::sinf(n);
		}

		static float te_sqrt(float n)
		{
			return ::sqrtf(n);
		}

		static float te_log(float n)
		{
			return ::logf(n);
		}

		static float te_log10(float n)
		{
			return ::log10f(n);
		}

		static float te_atan(float n)
		{
			return ::atanf(n);
		}

		static float te_tanh(float n)
		{
			return ::tanhf(n);
		}

		static float te_fmod(float n, float m)
		{
			return ::fmodf(n, m);
		}

		static float te_tan(float n)
		{
			return ::tanf(n);
		}

		static float te_atan2(float n, float m)
		{
			return ::atan2f(n, m);
		}

		static float te_pow(float n, float m)
		{
			return ::powf(n, m);
		}

		static float te_floor(float d)
		{
			return ::floorf(d);
		}

		static float te_ceil(float d)
		{
			return ::ceilf(d);
		}

		static float te_add(float a, float b)
		{
			return a + b;
		}

		static float te_sub(float a, float b)
		{
			return a - b;
		}

		static float te_mul(float a, float b)
		{
			return a * b;
		}

		static float te_divide(float a, float b)
		{
			return a / b;
		}

		static float te_negate(float a)
		{
			return -a;
		}

		static float te_comma(float a, float b)
		{
			(void)a;
			return b;
		}

		static float te_greater(float a, float b)
		{
			return a > b;
		}

		static float te_greater_eq(float a, float b)
		{
			return a >= b;
		}

		static float te_lower(float a, float b)
		{
			return a < b;
		}

		static float te_lower_eq(float a, float b)
		{
			return a <= b;
		}

		static float te_equal(float a, float b)
		{
			return a == b;
		}

		static float te_not_equal(float a, float b)
		{
			return a != b;
		}

		static float te_logical_and(float a, float b)
		{
			return a != 0.0f && b != 0.0f;
		}

		static float te_logical_or(float a, float b)
		{
			return a != 0.0f || b != 0.0f;
		}

		static float te_logical_not(float a)
		{
			return a == 0.0f;
		}

		static float te_logical_notnot(float a)
		{
			return a != 0.0f;
		}

		static float te_negate_logical_not(float a)
		{
			return -(a == 0.0f);
		}

		static float te_negate_logical_notnot(float a)
		{
			return -(a != 0.0f);
		}

		static float te_nul()
		{
			return 0.0f;
		}

		static float te_nan()
		{
			return std::numeric_limits<float>::quiet_NaN();
		}
	};

	template<typename T_IMPL>
	struct te_native_builtins : T_IMPL
	{
		using t_impl = T_IMPL;

		static inline constexpr te_variable functions[] = {/* must be in alphabetical order */
			{"abs", t_impl::te_fabs, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"acos", t_impl::te_acos, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"asin", t_impl::te_asin, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"atan", t_impl::te_atan, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"atan2", t_impl::te_atan2, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"ceil", t_impl::te_ceil, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"cos", t_impl::te_cos, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"cosh", t_impl::te_cosh, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"e", t_impl::te_e, TE_FUNCTION0 | TE_FLAG_PURE, 0},
			{"exp", t_impl::te_exp, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"fac", t_impl::te_fac, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"floor", t_impl::te_floor, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"ln", t_impl::te_log, TE_FUNCTION1 | TE_FLAG_PURE, 0},
#ifdef TE_NAT_LOG
			{"log", t_impl::te_log, TE_FUNCTION1 | TE_FLAG_PURE, 0},
#else
			{"log", t_impl::te_log10, TE_FUNCTION1 | TE_FLAG_PURE, 0},
#endif
			{"log10", t_impl::te_log10, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"ncr", t_impl::te_ncr, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"npr", t_impl::te_npr, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"pi", t_impl::te_pi, TE_FUNCTION0 | TE_FLAG_PURE, 0},
			{"pow", t_impl::te_pow, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"sin", t_impl::te_sin, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"sinh", t_impl::te_sinh, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"sqrt", t_impl::te_sqrt, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"tan", t_impl::te_tan, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"tanh", t_impl::te_tanh, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{0, 0, 0, 0}};

		static inline constexpr te_variable operators[] = {/* must be in alphabetical order */
			{"add", t_impl::te_add, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"comma", t_impl::te_comma, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"divide", t_impl::te_divide, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"equal", t_impl::te_equal, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"fmod", t_impl::te_fmod, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"greater", t_impl::te_greater, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"greater_eq", t_impl::te_greater_eq, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"logical_and", t_impl::te_logical_and, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"logical_not", t_impl::te_logical_not, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"logical_notnot", t_impl::te_logical_notnot, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"logical_or", t_impl::te_logical_or, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"lower", t_impl::te_lower, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"lower_eq", t_impl::te_lower_eq, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"mul", t_impl::te_mul, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"negate", t_impl::te_negate, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"negate_logical_not", t_impl::te_negate_logical_not, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"negate_logical_notnot", t_impl::te_negate_logical_notnot, TE_FUNCTION1 | TE_FLAG_PURE, 0},
			{"not_equal", t_impl::te_not_equal, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"pow", t_impl::te_pow, TE_FUNCTION2 | TE_FLAG_PURE, 0},
			{"sub", t_impl::te_sub, TE_FUNCTION2 | TE_FLAG_PURE, 0},
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

		static inline const te_variable* find_builtin_function(const char* name, int len)
		{
			int imin = 0;
			int imax = sizeof(functions) / sizeof(te_variable) - 2;

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

		static inline const te_variable* find_builtin_operator(const char* name, int len)
		{
			int imin = 0;
			int imax = sizeof(operators) / sizeof(te_variable) - 2;

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

		static inline const te_variable* find_builtin(const char* name, int len)
		{
			auto res = find_builtin_function(name, len);
			if (!res)
			{
				res = find_builtin_operator(name, len);
			}
			return res;
		}

		static inline const te_variable* find_builtin(const char* name)
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
} // namespace details

struct te_traits
{
	using t_atom			= float;
	using t_vector			= float;
	using t_vector_int		= int;
	using t_atom_builtins	= details::te_native_builtins_impl<t_atom>;
	using t_vector_builtins = details::te_native_builtins_impl<t_vector>;

	static inline t_vector load_atom(t_atom a) noexcept
	{
		return a;
	}

	static inline t_vector load_atom(double a) noexcept
	{
		return (t_vector)a;
	}

	static inline t_vector load_atom(int a) noexcept
	{
		return (t_vector)a;
	}
};

inline te_traits::t_vector te_eval(const void* expr_buffer, const void* const expr_context[]) noexcept
{
	return details::te_eval_portable_impl<te_traits, te_traits::t_atom, te_traits::t_vector>(
		(const te_expr_portable<te_traits>*)expr_buffer, (const unsigned char*)expr_buffer, expr_context);
}

#if (TE_COMPILER_ENABLED)
using te_compiled_expr = void*;
te_compiled_expr	 te_compile(const char* expression, const te_variable* variables, int var_count, int* error);
size_t				 te_get_binding_array_size(const te_compiled_expr n);
const void* const*	 te_get_binding_addresses(const te_compiled_expr n);
const char* const*	 te_get_binding_names(const te_compiled_expr n);
size_t				 te_get_expr_data_size(const te_compiled_expr n);
const unsigned char* te_get_expr_data(const te_compiled_expr n);
void				 te_free(te_compiled_expr n);

inline te_traits::t_vector te_eval(const te_compiled_expr n)
{
	return te_eval(te_get_expr_data(n), te_get_binding_addresses(n));
}

inline te_traits::t_vector te_interp(const char* expression, int* error)
{
	te_compiled_expr	n = te_compile(expression, 0, 0, error);
	te_traits::t_vector ret;
	if (n)
	{
		ret = te_eval(n);
		te_free(n);
	}
	else
	{
		ret = te_traits::t_vector_builtins::te_nan();
	}
	return ret;
}

struct te_program
{
	virtual void bind_variables(const te_variable* variables, int var_count) = 0;
	virtual bool compile_statement(const char* statement, int* error)		 = 0;

	virtual const void* const*	 get_binding_addresses()				 = 0;
	virtual const char* const*	 get_binding_names()					 = 0;
	virtual size_t				 get_data_size(const te_compiled_expr n) = 0;
	virtual const unsigned char* get_data(const te_compiled_expr n)		 = 0;
};

#endif // #if (TE_COMPILER_ENABLED)

#endif /*__TINYEXPR_H__*/
