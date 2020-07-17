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

struct te_traits
{
	using t_atom   = double;
	using t_vector = double;

	static inline t_vector load_atom(t_atom a) noexcept
	{
		return a;
	}

	static inline const t_vector nan() noexcept
	{
		return load_atom(std::numeric_limits<t_atom>::quiet_NaN());
	}
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
		using t_atom   = T_ATOM;
		using t_vector = T_VECTOR;
		using t_traits = T_TRAITS;

#define TE_FUN(...) ((t_vector(*)(__VA_ARGS__))expr_context[n_portable->function])

#define M(e)                                                                                                           \
	te_eval_portable_impl<T_TRAITS, T_ATOM, T_VECTOR>(                                                                 \
		(const te_expr_portable<t_traits>*)&expr_buffer[n_portable->parameters[e]], expr_buffer, expr_context)

		switch (te_type_mask(n_portable->type))
		{
		case TE_CONSTANT:
			return t_traits::load_atom(n_portable->value);

		case TE_VARIABLE:
			return t_traits::load_atom(
				(expr_context != nullptr) ? *((const t_vector*)(expr_context[n_portable->bound])) : t_traits::nan());

		case TE_FUNCTION0:
		case TE_FUNCTION1:
		case TE_FUNCTION2:
		case TE_FUNCTION3:
		case TE_FUNCTION4:
		case TE_FUNCTION5:
		case TE_FUNCTION6:
		case TE_FUNCTION7:

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
				return t_traits::nan();
			}
		}

		default:
			return t_traits::nan();
		}
#undef TE_FUN
#undef M
	}
} // namespace details

inline te_traits::t_atom te_eval(const void* expr_buffer, const void* const expr_context[]) noexcept
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

inline te_traits::t_atom te_eval(const te_compiled_expr n)
{
	return te_eval(te_get_expr_data(n), te_get_binding_addresses(n));
}

inline te_traits::t_atom te_interp(const char* expression, int* error)
{
	te_compiled_expr  n = te_compile(expression, 0, 0, error);
	te_traits::t_atom ret;
	if (n)
	{
		ret = te_eval(n);
		te_free(n);
	}
	else
	{
		ret = te_traits::nan();
	}
	return ret;
}

#endif // #if (TE_COMPILER_ENABLED)

#endif /*__TINYEXPR_H__*/
