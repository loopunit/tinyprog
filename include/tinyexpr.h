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

#define TE_COMPILER_ENABLED 1

#if TE_COMPILER_ENABLED
#	include <unordered_map>
#	include <vector>
#	include <memory>

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
	std::vector<const void*> index_to_address; // this contains the native function/value address as originally compiled
	std::vector<std::string> index_to_name;
};

using te_expr_portable_expression_context = const void*[];

struct te_expr_portable
{
	int type;
	union
	{
		double value;
		size_t bound;
		size_t function;
	};
	size_t parameters[1];
};

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

struct te_expr
{
	te_expr_portable_expression_build_indexer  m_indexer;
	te_expr_portable_expression_build_bindings m_bindings;
	std::unique_ptr<std::uint8_t>			   m_build_buffer;
};

/* Parses the input expression, evaluates it, and frees it. */
/* Returns NaN on error. */
double te_interp(const char* expression, int* error);

/* Parses the input expression and binds variables. */
/* Returns NULL on error. */
te_expr* te_compile(const char* expression, const te_variable* variables, int var_count, int* error);

/* Evaluates the expression. */
double te_eval(const te_expr* n);

/* Frees the expression. */
/* This is safe to call on NULL pointers. */
void te_free(te_expr* n);

#endif // #if TE_COMPILER_ENABLED

#endif /*__TINYEXPR_H__*/
