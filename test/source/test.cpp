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

#include <doctest/doctest.h>

#define TP_TESTING 1
#include "tinyprog.h"

#include <stdio.h>
#include "minctest.h"

typedef struct
{
	const char*				 expr;
	te::env_traits::t_vector answer;
} test_case;

typedef struct
{
	const char* expr1;
	const char* expr2;
} test_equ;

void test_results()
{
	test_case cases[] = {
		{"1", te::env_traits::explicit_load_atom(1)},
		{"1 ", te::env_traits::explicit_load_atom(1)},
		{"(1)", te::env_traits::explicit_load_atom(1)},

		{"pi", te::env_traits::explicit_load_atom(3.14159)},
		{"atan(1)*4 - pi", te::env_traits::explicit_load_atom(0)},
		{"e", te::env_traits::explicit_load_atom(2.71828)},

		{"2+1", te::env_traits::explicit_load_atom(2 + 1)},
		{"(((2+(1))))", te::env_traits::explicit_load_atom(2 + 1)},
		{"3+2", te::env_traits::explicit_load_atom(3 + 2)},

		{"3+2+4", te::env_traits::explicit_load_atom(3 + 2 + 4)},
		{"(3+2)+4", te::env_traits::explicit_load_atom(3 + 2 + 4)},
		{"3+(2+4)", te::env_traits::explicit_load_atom(3 + 2 + 4)},
		{"(3+2+4)", te::env_traits::explicit_load_atom(3 + 2 + 4)},

		{"3*2*4", te::env_traits::explicit_load_atom(3 * 2 * 4)},
		{"(3*2)*4", te::env_traits::explicit_load_atom(3 * 2 * 4)},
		{"3*(2*4)", te::env_traits::explicit_load_atom(3 * 2 * 4)},
		{"(3*2*4)", te::env_traits::explicit_load_atom(3 * 2 * 4)},

		{"3-2-4", te::env_traits::explicit_load_atom(3 - 2 - 4)},
		{"(3-2)-4", te::env_traits::explicit_load_atom((3 - 2) - 4)},
		{"3-(2-4)", te::env_traits::explicit_load_atom(3 - (2 - 4))},
		{"(3-2-4)", te::env_traits::explicit_load_atom(3 - 2 - 4)},

		{"3/2/4", te::env_traits::explicit_load_atom(3.0 / 2.0 / 4.0)},
		{"(3/2)/4", te::env_traits::explicit_load_atom((3.0 / 2.0) / 4.0)},
		{"3/(2/4)", te::env_traits::explicit_load_atom(3.0 / (2.0 / 4.0))},
		{"(3/2/4)", te::env_traits::explicit_load_atom(3.0 / 2.0 / 4.0)},

		{"(3*2/4)", te::env_traits::explicit_load_atom(3.0 * 2.0 / 4.0)},
		{"(3/2*4)", te::env_traits::explicit_load_atom(3.0 / 2.0 * 4.0)},
		{"3*(2/4)", te::env_traits::explicit_load_atom(3.0 * (2.0 / 4.0))},

		{"asin sin .5", te::env_traits::explicit_load_atom(0.5)},
		{"sin asin .5", te::env_traits::explicit_load_atom(0.5)},
		{"ln exp .5", te::env_traits::explicit_load_atom(0.5)},
		{"exp ln .5", te::env_traits::explicit_load_atom(0.5)},

		{"asin sin-.5", te::env_traits::explicit_load_atom(-0.5)},
		{"asin sin-0.5", te::env_traits::explicit_load_atom(-0.5)},
		{"asin sin -0.5", te::env_traits::explicit_load_atom(-0.5)},
		{"asin (sin -0.5)", te::env_traits::explicit_load_atom(-0.5)},
		{"asin (sin (-0.5))", te::env_traits::explicit_load_atom(-0.5)},
		{"asin sin (-0.5)", te::env_traits::explicit_load_atom(-0.5)},
		{"(asin sin (-0.5))", te::env_traits::explicit_load_atom(-0.5)},

		{"log10 1000", te::env_traits::explicit_load_atom(3)},
		{"log10 1e3", te::env_traits::explicit_load_atom(3)},
		{"log10 1000", te::env_traits::explicit_load_atom(3)},
		{"log10 1e3", te::env_traits::explicit_load_atom(3)},
		{"log10(1000)", te::env_traits::explicit_load_atom(3)},
		{"log10(1e3)", te::env_traits::explicit_load_atom(3)},
		{"log10 1.0e3", te::env_traits::explicit_load_atom(3)},
		{"10^5*5e-5", te::env_traits::explicit_load_atom(5)},

#ifdef TE_NAT_LOG
		{"log 1000", te::env_traits::explicit_load_atom(6.9078)},
		{"log e", te::env_traits::explicit_load_atom(1)},
		{"log (e^10)", te::env_traits::explicit_load_atom(10)},
#else
		{"log 1000", te::env_traits::explicit_load_atom(3)},
#endif

		{"ln (e^10)", te::env_traits::explicit_load_atom(10)},
		{"100^.5+1", te::env_traits::explicit_load_atom(11)},
		{"100 ^.5+1", te::env_traits::explicit_load_atom(11)},
		{"100^+.5+1", te::env_traits::explicit_load_atom(11)},
		{"100^--.5+1", te::env_traits::explicit_load_atom(11)},
		{"100^---+-++---++-+-+-.5+1", te::env_traits::explicit_load_atom(11)},

		{"100^-.5+1", te::env_traits::explicit_load_atom(1.1)},
		{"100^---.5+1", te::env_traits::explicit_load_atom(1.1)},
		{"100^+---.5+1", te::env_traits::explicit_load_atom(1.1)},
		{"1e2^+---.5e0+1e0", te::env_traits::explicit_load_atom(1.1)},
		{"--(1e2^(+(-(-(-.5e0))))+1e0)", te::env_traits::explicit_load_atom(1.1)},

		{"sqrt 100 + 7", te::env_traits::explicit_load_atom(17)},
		{"sqrt 100 * 7", te::env_traits::explicit_load_atom(70)},
		{"sqrt (100 * 100)", te::env_traits::explicit_load_atom(100)},

		{"1,2", te::env_traits::explicit_load_atom(2)},
		{"1,2+1", te::env_traits::explicit_load_atom(3)},
		{"1+1,2+2,2+1", te::env_traits::explicit_load_atom(3)},
		{"1,2,3", te::env_traits::explicit_load_atom(3)},
		{"(1,2),3", te::env_traits::explicit_load_atom(3)},
		{"1,(2,3)", te::env_traits::explicit_load_atom(3)},
		{"-(1,(2,3))", te::env_traits::explicit_load_atom(-3)},

		{"2^2", te::env_traits::explicit_load_atom(4)},
		{"pow(2,2)", te::env_traits::explicit_load_atom(4)},

		{"atan2(1,1)", te::env_traits::explicit_load_atom(0.7854)},
		{"atan2(1,2)", te::env_traits::explicit_load_atom(0.4636)},
		{"atan2(2,1)", te::env_traits::explicit_load_atom(1.1071)},
		{"atan2(3,4)", te::env_traits::explicit_load_atom(0.6435)},
		{"atan2(3+3,4*2)", te::env_traits::explicit_load_atom(0.6435)},
		{"atan2(3+3,(4*2))", te::env_traits::explicit_load_atom(0.6435)},
		{"atan2((3+3),4*2)", te::env_traits::explicit_load_atom(0.6435)},
		{"atan2((3+3),(4*2))", te::env_traits::explicit_load_atom(0.6435)},

	};

	int i;
	for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i)
	{
		const char*				   expr	  = cases[i].expr;
		const te::env_traits::t_vector answer = cases[i].answer;

		int						   err;
		const te::env_traits::t_vector ev = te::interp(expr, &err);
		lok(!err);
		lfequal(ev, answer);

		if (err)
		{
			printf("FAILED: %s (%d)\n", expr, err);
		}
	}
}

void test_syntax()
{
	test_case errors[] = {
		{"", te::env_traits::explicit_load_atom(1)},
		{"1+", te::env_traits::explicit_load_atom(2)},
		{"1)", te::env_traits::explicit_load_atom(2)},
		{"(1", te::env_traits::explicit_load_atom(2)},
		{"1**1", te::env_traits::explicit_load_atom(3)},
		{"1*2(+4", te::env_traits::explicit_load_atom(4)},
		{"1*2(1+4", te::env_traits::explicit_load_atom(4)},
		{"a+5", te::env_traits::explicit_load_atom(1)},
		{"A+5", te::env_traits::explicit_load_atom(1)},
		{"Aa+5", te::env_traits::explicit_load_atom(1)},
		{"1^^5", te::env_traits::explicit_load_atom(3)},
		{"1**5", te::env_traits::explicit_load_atom(3)},
		{"sin(cos5", te::env_traits::explicit_load_atom(8)},
	};

	int i;
	for (i = 0; i < sizeof(errors) / sizeof(test_case); ++i)
	{
		const char* expr = errors[i].expr;
		const auto	e	 = errors[i].answer;

		int						   err;
		const te::env_traits::t_vector r = te::interp(expr, &err);
		lequal(err, e);
		lok(r != r);

		auto n = te::compile(expr, 0, 0, &err);
		lequal(err, e);
		lok(!n);

		if (err != e)
		{
			printf("FAILED: %s\n", expr);
		}

		const te::env_traits::t_vector k = te::interp(expr, 0);
		lok(k != k);
	}
}

void test_nans()
{
	const char* nans[] = {
		"0/0",
		"1%0",
		"1%(1%0)",
		"(1%0)%1",
		"fac(-1)",
		"ncr(2, 4)",
		"ncr(-2, 4)",
		"ncr(2, -4)",
		"npr(2, 4)",
		"npr(-2, 4)",
		"npr(2, -4)",
	};

	int i;
	for (i = 0; i < sizeof(nans) / sizeof(const char*); ++i)
	{
		const char* expr = nans[i];

		int						   err;
		const te::env_traits::t_vector r = te::interp(expr, &err);
		lequal(err, 0);
		lok(r != r);

		auto n = te::compile(expr, 0, 0, &err);
		lok(n);
		lequal(err, 0);
		const te::env_traits::t_vector c = te::eval(n);
		lok(c != c);
		delete n;
	}
}

void test_infs()
{
	const char* infs[] = {
		"1/0",
		"log(0)",
		"pow(2,10000000)",
		"fac(300)",
		"ncr(300,100)",
		"ncr(300000,100)",
		"ncr(300000,100)*8",
		"npr(3,2)*ncr(300000,100)",
		"npr(100,90)",
		"npr(30,25)",
	};

	int i;
	for (i = 0; i < sizeof(infs) / sizeof(const char*); ++i)
	{
		const char* expr = infs[i];

		int						   err;
		const te::env_traits::t_vector r = te::interp(expr, &err);
		lequal(err, 0);
		lok(r == r + 1);

		auto n = te::compile(expr, 0, 0, &err);
		lok(n);
		lequal(err, 0);
		const te::env_traits::t_vector c = te::eval(n);
		lok(c == c + 1);
		delete n;
	}
}

void test_variables()
{
	te::env_traits::t_vector x, y, test;
	te::variable			 lookup[] = {{"x", &x}, {"y", &y}, {"te_st", &test}};

	int err;

	auto expr1 = te::compile("cos x + sin y", lookup, 2, &err);
	lok(expr1);
	lok(!err);

	auto expr2 = te::compile("x+x+x-y", lookup, 2, &err);
	lok(expr2);
	lok(!err);

	auto expr3 = te::compile("x*y^3", lookup, 2, &err);
	lok(expr3);
	lok(!err);

	auto expr4 = te::compile("te_st+5", lookup, 3, &err);
	lok(expr4);
	lok(!err);

	for (y = 2; y < 3; ++y)
	{
		for (x = 0; x < 5; ++x)
		{
			te::env_traits::t_vector ev;

			ev = te::eval(expr1);
			lfequal(ev, cos(x) + sin(y));

			ev = te::eval(expr2);
			lfequal(ev, x + x + x - y);

			ev = te::eval(expr3);
			lfequal(ev, x * y * y * y);

			test = x;
			ev	 = te::eval(expr4);
			lfequal(ev, x + 5);
		}
	}

	delete expr1;
	delete expr2;
	delete expr3;
	delete expr4;

	auto expr5 = te::compile("xx*y^3", lookup, 2, &err);
	lok(!expr5);
	lok(err);

	auto expr6 = te::compile("tes", lookup, 3, &err);
	lok(!expr6);
	lok(err);

	auto expr7 = te::compile("sinn x", lookup, 2, &err);
	lok(!expr7);
	lok(err);

	auto expr8 = te::compile("si x", lookup, 2, &err);
	lok(!expr8);
	lok(err);
}

#define cross_check(a, b)                                                                                              \
	do                                                                                                                 \
	{                                                                                                                  \
		if ((b) != (b))                                                                                                \
			break;                                                                                                     \
		auto expr = te::compile((a), lookup, 2, &err);                                                                     \
		lfequal(te::eval(expr), (b));                                                                                      \
		lok(!err);                                                                                                     \
		delete expr;                                                                                                 \
	} while (0)

void test_functions()
{
	te::env_traits::t_atom x, y;
	te::variable		   lookup[] = {{"x", &x}, {"y", &y}};

	int err;

	for (x = te::env_traits::t_atom(-5); x < te::env_traits::t_atom(5); x += te::env_traits::t_atom(.2))
	{
		cross_check("abs x", te::env_traits::explicit_load_atom(fabs(x)));
		cross_check("acos x", te::env_traits::explicit_load_atom(acos(x)));
		cross_check("asin x", te::env_traits::explicit_load_atom(asin(x)));
		cross_check("atan x", te::env_traits::explicit_load_atom(atan(x)));
		cross_check("ceil x", te::env_traits::explicit_load_atom(ceil(x)));
		cross_check("cos x", te::env_traits::explicit_load_atom(cos(x)));
		cross_check("cosh x", te::env_traits::explicit_load_atom(cosh(x)));
		cross_check("exp x", te::env_traits::explicit_load_atom(exp(x)));
		cross_check("floor x", te::env_traits::explicit_load_atom(floor(x)));
		cross_check("ln x", te::env_traits::explicit_load_atom(log(x)));
		cross_check("log10 x", te::env_traits::explicit_load_atom(log10(x)));
		cross_check("sin x", te::env_traits::explicit_load_atom(sin(x)));
		cross_check("sinh x", te::env_traits::explicit_load_atom(sinh(x)));
		cross_check("sqrt x", te::env_traits::explicit_load_atom(sqrt(x)));
		cross_check("tan x", te::env_traits::explicit_load_atom(tan(x)));
		cross_check("tanh x", te::env_traits::explicit_load_atom(tanh(x)));

		for (y = te::env_traits::t_atom(-2); y < te::env_traits::t_atom(2); y += te::env_traits::t_atom(.2))
		{
			if (fabs(x) < 0.01)
				break;
			cross_check("atan2(x,y)", te::env_traits::explicit_load_atom(atan2(x, y)));
			cross_check("pow(x,y)", te::env_traits::explicit_load_atom(pow(x, y)));
		}
	}
}

te::env_traits::t_vector sum0()
{
	return 6;
}
te::env_traits::t_vector sum1(te::env_traits::t_vector a)
{
	return a * 2;
}
te::env_traits::t_vector sum2(te::env_traits::t_vector a, te::env_traits::t_vector b)
{
	return a + b;
}
te::env_traits::t_vector sum3(te::env_traits::t_vector a, te::env_traits::t_vector b, te::env_traits::t_vector c)
{
	return a + b + c;
}
te::env_traits::t_vector sum4(
	te::env_traits::t_vector a, te::env_traits::t_vector b, te::env_traits::t_vector c, te::env_traits::t_vector d)
{
	return a + b + c + d;
}
te::env_traits::t_vector sum5(te::env_traits::t_vector a,
	te::env_traits::t_vector						   b,
	te::env_traits::t_vector						   c,
	te::env_traits::t_vector						   d,
	te::env_traits::t_vector						   e)
{
	return a + b + c + d + e;
}
te::env_traits::t_vector sum6(te::env_traits::t_vector a,
	te::env_traits::t_vector						   b,
	te::env_traits::t_vector						   c,
	te::env_traits::t_vector						   d,
	te::env_traits::t_vector						   e,
	te::env_traits::t_vector						   f)
{
	return a + b + c + d + e + f;
}
te::env_traits::t_vector sum7(te::env_traits::t_vector a,
	te::env_traits::t_vector						   c,
	te::env_traits::t_vector						   d,
	te::env_traits::t_vector						   b,
	te::env_traits::t_vector						   e,
	te::env_traits::t_vector						   f,
	te::env_traits::t_vector						   g)
{
	return a + b + c + d + e + f + g;
}

void test_dynamic()
{
	te::env_traits::t_vector x, f;
	te::variable			 lookup[] = {
		{"x", &x},
		{"f", &f},
		{"sum0", sum0, tp::FUNCTION0},
		{"sum1", sum1, tp::FUNCTION1},
		{"sum2", sum2, tp::FUNCTION2},
		{"sum3", sum3, tp::FUNCTION3},
		{"sum4", sum4, tp::FUNCTION4},
		{"sum5", sum5, tp::FUNCTION5},
		{"sum6", sum6, tp::FUNCTION6},
		{"sum7", sum7, tp::FUNCTION7},
	};

	test_case cases[] = {
		{"x", 2},
		{"f+x", 7},
		{"x+x", 4},
		{"x+f", 7},
		{"f+f", 10},
		{"f+sum0", 11},
		{"sum0+sum0", 12},
		{"sum0()+sum0", 12},
		{"sum0+sum0()", 12},
		{"sum0()+(0)+sum0()", 12},
		{"sum1 sum0", 12},
		{"sum1(sum0)", 12},
		{"sum1 f", 10},
		{"sum1 x", 4},
		{"sum2 (sum0, x)", 8},
		{"sum3 (sum0, x, 2)", 10},
		{"sum2(2,3)", 5},
		{"sum3(2,3,4)", 9},
		{"sum4(2,3,4,5)", 14},
		{"sum5(2,3,4,5,6)", 20},
		{"sum6(2,3,4,5,6,7)", 27},
		{"sum7(2,3,4,5,6,7,8)", 35},
	};

	x = 2;
	f = 5;

	int i;
	for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i)
	{
		const char*				   expr	  = cases[i].expr;
		const te::env_traits::t_vector answer = cases[i].answer;

		int	 err;
		auto ex = te::compile(expr, lookup, sizeof(lookup) / sizeof(te::variable), &err);
		lok(ex);
		lfequal(te::eval(ex), answer);
		delete ex;
	}
}

te::env_traits::t_vector clo0(void* context)
{
	if (context)
		return *((te::env_traits::t_vector*)context) + 6;
	return 6;
}
te::env_traits::t_vector clo1(void* context, te::env_traits::t_vector a)
{
	if (context)
		return *((te::env_traits::t_vector*)context) + a * 2;
	return a * 2;
}
te::env_traits::t_vector clo2(void* context, te::env_traits::t_vector a, te::env_traits::t_vector b)
{
	if (context)
		return *((te::env_traits::t_vector*)context) + a + b;
	return a + b;
}

te::env_traits::t_vector cell(void* context, te::env_traits::t_vector a)
{
	te::env_traits::t_vector* c = (te::env_traits::t_vector*)context;
	return c[(int)a];
}

void test_closure()
{
	te::env_traits::t_vector extra;
	te::env_traits::t_vector c[] = {5, 6, 7, 8, 9};

	te::variable lookup[] = {
		{"c0", clo0, tp::CLOSURE0, &extra},
		{"c1", clo1, tp::CLOSURE1, &extra},
		{"c2", clo2, tp::CLOSURE2, &extra},
		{"cell", cell, tp::CLOSURE1, c},
	};

	test_case cases[] = {
		{"c0", 6},
		{"c1 4", 8},
		{"c2 (10, 20)", 30},
	};

	int i;
	for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i)
	{
		const char*				   expr	  = cases[i].expr;
		const te::env_traits::t_vector answer = cases[i].answer;

		int	 err;
		auto ex = te::compile(expr, lookup, sizeof(lookup) / sizeof(te::variable), &err);
		lok(ex);

		extra = 0;
		lfequal(te::eval(ex), answer + extra);

		extra = 10;
		lfequal(te::eval(ex), answer + extra);

		delete ex;
	}

	test_case cases2[] = {
		{"cell 0", 5},
		{"cell 1", 6},
		{"cell 0 + cell 1", 11},
		{"cell 1 * cell 3 + cell 4", 57},
	};

	for (i = 0; i < sizeof(cases2) / sizeof(test_case); ++i)
	{
		const char*				   expr	  = cases2[i].expr;
		const te::env_traits::t_vector answer = cases2[i].answer;

		int	 err;
		auto ex = te::compile(expr, lookup, sizeof(lookup) / sizeof(te::variable), &err);
		lok(ex);
		lfequal(te::eval(ex), answer);
		delete ex;
	}
}

void test_optimize()
{
	test_case cases[] = {
		{"5+5", te::env_traits::explicit_load_atom(10)},
		{"pow(2,2)", te::env_traits::explicit_load_atom(4)},
		{"sqrt 100", te::env_traits::explicit_load_atom(10)},
		{"pi * 2", te::env_traits::explicit_load_atom(6.2832)},
	};

	int i;
	for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i)
	{
		const char*				   expr	  = cases[i].expr;
		const te::env_traits::t_vector answer = cases[i].answer;

		int	 err;
		auto ex = te::compile(expr, 0, 0, &err);
		lok(ex);

		/* The answer should be know without
		 * even running eval. */
		// lfequal(ex->value, answer); // TODO
		lfequal(te::eval(ex), answer);

		delete ex;
	}
}

void test_pow()
{
#ifdef TE_POW_FROM_RIGHT
	test_equ cases[] = {{"2^3^4", "2^(3^4)"},
		{"-2^2", "-(2^2)"},
		{"--2^2", "(2^2)"},
		{"---2^2", "-(2^2)"},
		{"-(2)^2", "-(2^2)"},
		{"-(2*1)^2", "-(2^2)"},
		{"-2^2", "-4"},
		{"2^1.1^1.2^1.3", "2^(1.1^(1.2^1.3))"},
		{"-a^b", "-(a^b)"},
		{"-a^-b", "-(a^-b)"}};
#else
	test_equ cases[] = {{"2^3^4", "(2^3)^4"},
		{"-2^2", "(-2)^2"},
		{"--2^2", "2^2"},
		{"---2^2", "(-2)^2"},
		{"-2^2", "4"},
		{"2^1.1^1.2^1.3", "((2^1.1)^1.2)^1.3"},
		{"-a^b", "(-a)^b"},
		{"-a^-b", "(-a)^(-b)"}};
#endif

	te::env_traits::t_vector a = 2, b = 3;

	te::variable lookup[] = {{"a", &a}, {"b", &b}};

	int i;
	for (i = 0; i < sizeof(cases) / sizeof(test_equ); ++i)
	{
		const char* expr1 = cases[i].expr1;
		const char* expr2 = cases[i].expr2;

		auto ex1 = te::compile(expr1, lookup, sizeof(lookup) / sizeof(te::variable), 0);
		auto ex2 = te::compile(expr2, lookup, sizeof(lookup) / sizeof(te::variable), 0);

		lok(ex1);
		lok(ex2);

		te::env_traits::t_vector r1 = te::eval(ex1);
		te::env_traits::t_vector r2 = te::eval(ex2);

		fflush(stdout);
		lfequal(r1, r2);

		delete ex1;
		delete ex2;
	}
}

void test_combinatorics()
{
	test_case cases[] = {
		{"fac(0)", 1},
		{"fac(0.2)", 1},
		{"fac(1)", 1},
		{"fac(2)", 2},
		{"fac(3)", 6},
		{"fac(4.8)", 24},
		{"fac(10)", 3628800},

		{"ncr(0,0)", 1},
		{"ncr(10,1)", 10},
		{"ncr(10,0)", 1},
		{"ncr(10,10)", 1},
		{"ncr(16,7)", 11440},
		{"ncr(16,9)", 11440},
		{"ncr(100,95)", 75287520},

		{"npr(0,0)", 1},
		{"npr(10,1)", 10},
		{"npr(10,0)", 1},
		{"npr(10,10)", 3628800},
		{"npr(20,5)", 1860480},
		{"npr(100,4)", 94109400},
	};

	int i;
	for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i)
	{
		const char*				   expr	  = cases[i].expr;
		const te::env_traits::t_vector answer = cases[i].answer;

		int						   err;
		const te::env_traits::t_vector ev = te::interp(expr, &err);
		lok(!err);
		lfequal(ev, answer);

		if (err)
		{
			printf("FAILED: %s (%d)\n", expr, err);
		}
	}
}

void test_logic()
{
	test_case cases[] = {
		{"1 && 1", 1},
		{"1 && 0", 0},
		{"0 && 1", 0},
		{"0 && 0", 0},
		{"1 || 1", 1},
		{"1 || 0", 1},
		{"0 || 1", 1},
		{"0 || 0", 0},
		{"!0", 1},
		{"!1", 0},
		{"!2", 0},

		{"!-2", 0},
		{"-!2", 0},
		{"!!0", 0},
		{"!!1", 1},
		{"!!2", 1},
		{"!!-2", 1},
		{"!-!2", 1},
		{"-!!2", -1},
		{"--!!2", 1},

		{"1 < 2", 1},
		{"2 < 2", 0},
		{"2 <= 2", 1},
		{"2 > 1", 1},
		{"2 > 2", 0},
		{"2 >= 2", 1},
		{"2 > -2", 1},
		{"-2 < 2", 1},

		{"0 == 0", 1},
		{"0 != 0", 0},
		{"2 == 2", 1},
		{"2 != 2", 0},
		{"2 == 3", 0},
		{"2 != 3", 1},
		{"2 == 2.0001", 0},
		{"2 != 2.0001", 1},

		{"1 < 2 && 2 < 3", 1},
		{"1 < 2 && 3 < 2", 0},
		{"2 < 1 && 2 < 3", 0},
		{"2 < 1 && 3 < 2", 0},
		{"1 < 2 || 2 < 3", 1},
		{"1 < 2 || 3 < 2", 1},
		{"2 < 1 || 2 < 3", 1},
		{"2 < 1 || 3 < 2", 0},

		{"1 < 1+1", 1},
		{"1 < 1*2", 1},
		{"1 < 2/2", 0},
		{"1 < 2^2", 1},

		{"5+5 < 4+10", 1},
		{"5+(5 < 4)+10", 15},
		{"5+(5 < 4+10)", 6},
		{"(5+5 < 4)+10", 10},
		{"5+!(5 < 4)+10", 16},
		{"5+!(5 < 4+10)", 5},
		{"!(5+5 < 4)+10", 11},

#ifdef TE_POW_FROM_RIGHT
		{"!0^2", 1},
		{"!0^-1", 0},
		{"-!0^2", -1},
#else
		{"!0^2", 1},
		{"!0^-1", 1},
		{"-!0^2", 1},
#endif

	};

	int i;
	for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i)
	{
		const char*				   expr	  = cases[i].expr;
		const te::env_traits::t_vector answer = cases[i].answer;

		int						   err;
		const te::env_traits::t_vector ev = te::interp(expr, &err);
		lok(!err);
		lfequal(ev, answer);

		if (err)
		{
			printf("FAILED: %s (%d)\n", expr, err);
		}
	}
}

TEST_CASE("expression_test") 
{
	lrun("Results", test_results);
	lrun("Syntax", test_syntax);
	lrun("NaNs", test_nans);
	lrun("INFs", test_infs);
	lrun("Variables", test_variables);
	lrun("Functions", test_functions);
	lrun("Dynamic", test_dynamic);
	lrun("Closure", test_closure);
	lrun("Optimize", test_optimize);
	lrun("Pow", test_pow);
	lrun("Combinatorics", test_combinatorics);
	lrun("Logic", test_logic);
	lresults();
}
