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

#include "tinyexpr.h"
#include <stdio.h>
#include "minctest.h"

typedef struct
{
	const char*			expr;
	te_traits::t_vector answer;
} test_case;

typedef struct
{
	const char* expr1;
	const char* expr2;
} test_equ;

void test_results()
{
	test_case cases[] = {
		{"1", te_traits::load_atom(1)},
		{"1 ", te_traits::load_atom(1)},
		{"(1)", te_traits::load_atom(1)},

		{"pi", te_traits::load_atom(3.14159)},
		{"atan(1)*4 - pi", te_traits::load_atom(0)},
		{"e", te_traits::load_atom(2.71828)},

		{"2+1", te_traits::load_atom(2 + 1)},
		{"(((2+(1))))", te_traits::load_atom(2 + 1)},
		{"3+2", te_traits::load_atom(3 + 2)},

		{"3+2+4", te_traits::load_atom(3 + 2 + 4)},
		{"(3+2)+4", te_traits::load_atom(3 + 2 + 4)},
		{"3+(2+4)", te_traits::load_atom(3 + 2 + 4)},
		{"(3+2+4)", te_traits::load_atom(3 + 2 + 4)},

		{"3*2*4", te_traits::load_atom(3 * 2 * 4)},
		{"(3*2)*4", te_traits::load_atom(3 * 2 * 4)},
		{"3*(2*4)", te_traits::load_atom(3 * 2 * 4)},
		{"(3*2*4)", te_traits::load_atom(3 * 2 * 4)},

		{"3-2-4", te_traits::load_atom(3 - 2 - 4)},
		{"(3-2)-4", te_traits::load_atom((3 - 2) - 4)},
		{"3-(2-4)", te_traits::load_atom(3 - (2 - 4))},
		{"(3-2-4)", te_traits::load_atom(3 - 2 - 4)},

		{"3/2/4", te_traits::load_atom(3.0 / 2.0 / 4.0)},
		{"(3/2)/4", te_traits::load_atom((3.0 / 2.0) / 4.0)},
		{"3/(2/4)", te_traits::load_atom(3.0 / (2.0 / 4.0))},
		{"(3/2/4)", te_traits::load_atom(3.0 / 2.0 / 4.0)},

		{"(3*2/4)", te_traits::load_atom(3.0 * 2.0 / 4.0)},
		{"(3/2*4)", te_traits::load_atom(3.0 / 2.0 * 4.0)},
		{"3*(2/4)", te_traits::load_atom(3.0 * (2.0 / 4.0))},

		{"asin sin .5", te_traits::load_atom(0.5)},
		{"sin asin .5", te_traits::load_atom(0.5)},
		{"ln exp .5", te_traits::load_atom(0.5)},
		{"exp ln .5", te_traits::load_atom(0.5)},

		{"asin sin-.5", te_traits::load_atom(-0.5)},
		{"asin sin-0.5", te_traits::load_atom(-0.5)},
		{"asin sin -0.5", te_traits::load_atom(-0.5)},
		{"asin (sin -0.5)", te_traits::load_atom(-0.5)},
		{"asin (sin (-0.5))", te_traits::load_atom(-0.5)},
		{"asin sin (-0.5)", te_traits::load_atom(-0.5)},
		{"(asin sin (-0.5))", te_traits::load_atom(-0.5)},

		{"log10 1000", te_traits::load_atom(3)},
		{"log10 1e3", te_traits::load_atom(3)},
		{"log10 1000", te_traits::load_atom(3)},
		{"log10 1e3", te_traits::load_atom(3)},
		{"log10(1000)", te_traits::load_atom(3)},
		{"log10(1e3)", te_traits::load_atom(3)},
		{"log10 1.0e3", te_traits::load_atom(3)},
		{"10^5*5e-5", te_traits::load_atom(5)},

#ifdef TE_NAT_LOG
		{"log 1000", te_traits::load_atom(6.9078)},
		{"log e", te_traits::load_atom(1)},
		{"log (e^10)", te_traits::load_atom(10)},
#else
		{"log 1000", te_traits::load_atom(3)},
#endif

		{"ln (e^10)", te_traits::load_atom(10)},
		{"100^.5+1", te_traits::load_atom(11)},
		{"100 ^.5+1", te_traits::load_atom(11)},
		{"100^+.5+1", te_traits::load_atom(11)},
		{"100^--.5+1", te_traits::load_atom(11)},
		{"100^---+-++---++-+-+-.5+1", 11},

		{"100^-.5+1", te_traits::load_atom(1.1)},
		{"100^---.5+1", te_traits::load_atom(1.1)},
		{"100^+---.5+1", te_traits::load_atom(1.1)},
		{"1e2^+---.5e0+1e0", te_traits::load_atom(1.1)},
		{"--(1e2^(+(-(-(-.5e0))))+1e0)", te_traits::load_atom(1.1)},

		{"sqrt 100 + 7", te_traits::load_atom(17)},
		{"sqrt 100 * 7", te_traits::load_atom(70)},
		{"sqrt (100 * 100)", te_traits::load_atom(100)},

		{"1,2", te_traits::load_atom(2)},
		{"1,2+1", te_traits::load_atom(3)},
		{"1+1,2+2,2+1", te_traits::load_atom(3)},
		{"1,2,3", te_traits::load_atom(3)},
		{"(1,2),3", te_traits::load_atom(3)},
		{"1,(2,3)", te_traits::load_atom(3)},
		{"-(1,(2,3))", te_traits::load_atom(-3)},

		{"2^2", te_traits::load_atom(4)},
		{"pow(2,2)", te_traits::load_atom(4)},

		{"atan2(1,1)", te_traits::load_atom(0.7854)},
		{"atan2(1,2)", te_traits::load_atom(0.4636)},
		{"atan2(2,1)", te_traits::load_atom(1.1071)},
		{"atan2(3,4)", te_traits::load_atom(0.6435)},
		{"atan2(3+3,4*2)", te_traits::load_atom(0.6435)},
		{"atan2(3+3,(4*2))", te_traits::load_atom(0.6435)},
		{"atan2((3+3),4*2)", te_traits::load_atom(0.6435)},
		{"atan2((3+3),(4*2))", te_traits::load_atom(0.6435)},

	};

	int i;
	for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i)
	{
		const char*				  expr	 = cases[i].expr;
		const te_traits::t_vector answer = cases[i].answer;

		int						  err;
		const te_traits::t_vector ev = te_interp(expr, &err);
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
		{"", te_traits::load_atom(1)},
		{"1+", te_traits::load_atom(2)},
		{"1)", te_traits::load_atom(2)},
		{"(1", te_traits::load_atom(2)},
		{"1**1", te_traits::load_atom(3)},
		{"1*2(+4", te_traits::load_atom(4)},
		{"1*2(1+4", te_traits::load_atom(4)},
		{"a+5", te_traits::load_atom(1)},
		{"A+5", te_traits::load_atom(1)},
		{"Aa+5", te_traits::load_atom(1)},
		{"1^^5", te_traits::load_atom(3)},
		{"1**5", te_traits::load_atom(3)},
		{"sin(cos5", te_traits::load_atom(8)},
	};

	int i;
	for (i = 0; i < sizeof(errors) / sizeof(test_case); ++i)
	{
		const char* expr = errors[i].expr;
		const auto	e	 = errors[i].answer;

		int						  err;
		const te_traits::t_vector r = te_interp(expr, &err);
		lequal(err, e);
		lok(r != r);

		auto n = te_compile(expr, 0, 0, &err);
		lequal(err, e);
		lok(!n);

		if (err != e)
		{
			printf("FAILED: %s\n", expr);
		}

		const te_traits::t_vector k = te_interp(expr, 0);
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

		int						  err;
		const te_traits::t_vector r = te_interp(expr, &err);
		lequal(err, 0);
		lok(r != r);

		auto n = te_compile(expr, 0, 0, &err);
		lok(n);
		lequal(err, 0);
		const te_traits::t_vector c = te_eval(n);
		lok(c != c);
		te_free(n);
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

		int						  err;
		const te_traits::t_vector r = te_interp(expr, &err);
		lequal(err, 0);
		lok(r == r + 1);

		auto n = te_compile(expr, 0, 0, &err);
		lok(n);
		lequal(err, 0);
		const te_traits::t_vector c = te_eval(n);
		lok(c == c + 1);
		te_free(n);
	}
}

void test_variables()
{
	te_traits::t_vector x, y, test;
	te_variable			lookup[] = {{"x", &x}, {"y", &y}, {"te_st", &test}};

	int err;

	auto expr1 = te_compile("cos x + sin y", lookup, 2, &err);
	lok(expr1);
	lok(!err);

	auto expr2 = te_compile("x+x+x-y", lookup, 2, &err);
	lok(expr2);
	lok(!err);

	auto expr3 = te_compile("x*y^3", lookup, 2, &err);
	lok(expr3);
	lok(!err);

	auto expr4 = te_compile("te_st+5", lookup, 3, &err);
	lok(expr4);
	lok(!err);

	for (y = 2; y < 3; ++y)
	{
		for (x = 0; x < 5; ++x)
		{
			te_traits::t_vector ev;

			ev = te_eval(expr1);
			lfequal(ev, cos(x) + sin(y));

			ev = te_eval(expr2);
			lfequal(ev, x + x + x - y);

			ev = te_eval(expr3);
			lfequal(ev, x * y * y * y);

			test = x;
			ev	 = te_eval(expr4);
			lfequal(ev, x + 5);
		}
	}

	te_free(expr1);
	te_free(expr2);
	te_free(expr3);
	te_free(expr4);

	auto expr5 = te_compile("xx*y^3", lookup, 2, &err);
	lok(!expr5);
	lok(err);

	auto expr6 = te_compile("tes", lookup, 3, &err);
	lok(!expr6);
	lok(err);

	auto expr7 = te_compile("sinn x", lookup, 2, &err);
	lok(!expr7);
	lok(err);

	auto expr8 = te_compile("si x", lookup, 2, &err);
	lok(!expr8);
	lok(err);
}

#define cross_check(a, b)                                                                                              \
	do                                                                                                                 \
	{                                                                                                                  \
		if ((b) != (b))                                                                                                \
			break;                                                                                                     \
		auto expr = te_compile((a), lookup, 2, &err);                                                                  \
		lfequal(te_eval(expr), (b));                                                                                   \
		lok(!err);                                                                                                     \
		te_free(expr);                                                                                                 \
	} while (0)

void test_functions()
{
	te_traits::t_atom x, y;
	te_variable		  lookup[] = {{"x", &x}, {"y", &y}};

	int err;

	for (x = te_traits::t_atom(-5); x < te_traits::t_atom(5); x += te_traits::t_atom(.2))
	{
		cross_check("abs x", te_traits::load_atom(fabs(x)));
		cross_check("acos x", te_traits::load_atom(acos(x)));
		cross_check("asin x", te_traits::load_atom(asin(x)));
		cross_check("atan x", te_traits::load_atom(atan(x)));
		cross_check("ceil x", te_traits::load_atom(ceil(x)));
		cross_check("cos x", te_traits::load_atom(cos(x)));
		cross_check("cosh x", te_traits::load_atom(cosh(x)));
		cross_check("exp x", te_traits::load_atom(exp(x)));
		cross_check("floor x", te_traits::load_atom(floor(x)));
		cross_check("ln x", te_traits::load_atom(log(x)));
		cross_check("log10 x", te_traits::load_atom(log10(x)));
		cross_check("sin x", te_traits::load_atom(sin(x)));
		cross_check("sinh x", te_traits::load_atom(sinh(x)));
		cross_check("sqrt x", te_traits::load_atom(sqrt(x)));
		cross_check("tan x", te_traits::load_atom(tan(x)));
		cross_check("tanh x", te_traits::load_atom(tanh(x)));

		for (y = te_traits::t_atom(-2); y < te_traits::t_atom(2); y += te_traits::t_atom(.2))
		{
			if (fabs(x) < 0.01)
				break;
			cross_check("atan2(x,y)", te_traits::load_atom(atan2(x, y)));
			cross_check("pow(x,y)", te_traits::load_atom(pow(x, y)));
		}
	}
}

te_traits::t_vector sum0()
{
	return 6;
}
te_traits::t_vector sum1(te_traits::t_vector a)
{
	return a * 2;
}
te_traits::t_vector sum2(te_traits::t_vector a, te_traits::t_vector b)
{
	return a + b;
}
te_traits::t_vector sum3(te_traits::t_vector a, te_traits::t_vector b, te_traits::t_vector c)
{
	return a + b + c;
}
te_traits::t_vector sum4(te_traits::t_vector a, te_traits::t_vector b, te_traits::t_vector c, te_traits::t_vector d)
{
	return a + b + c + d;
}
te_traits::t_vector sum5(
	te_traits::t_vector a, te_traits::t_vector b, te_traits::t_vector c, te_traits::t_vector d, te_traits::t_vector e)
{
	return a + b + c + d + e;
}
te_traits::t_vector sum6(te_traits::t_vector a,
	te_traits::t_vector						 b,
	te_traits::t_vector						 c,
	te_traits::t_vector						 d,
	te_traits::t_vector						 e,
	te_traits::t_vector						 f)
{
	return a + b + c + d + e + f;
}
te_traits::t_vector sum7(te_traits::t_vector a,
	te_traits::t_vector						 b,
	te_traits::t_vector						 c,
	te_traits::t_vector						 d,
	te_traits::t_vector						 e,
	te_traits::t_vector						 f,
	te_traits::t_vector						 g)
{
	return a + b + c + d + e + f + g;
}

void test_dynamic()
{
	te_traits::t_vector x, f;
	te_variable			lookup[] = {
		{"x", &x},
		{"f", &f},
		{"sum0", sum0, TE_FUNCTION0},
		{"sum1", sum1, TE_FUNCTION1},
		{"sum2", sum2, TE_FUNCTION2},
		{"sum3", sum3, TE_FUNCTION3},
		{"sum4", sum4, TE_FUNCTION4},
		{"sum5", sum5, TE_FUNCTION5},
		{"sum6", sum6, TE_FUNCTION6},
		{"sum7", sum7, TE_FUNCTION7},
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
		const char*				  expr	 = cases[i].expr;
		const te_traits::t_vector answer = cases[i].answer;

		int	 err;
		auto ex = te_compile(expr, lookup, sizeof(lookup) / sizeof(te_variable), &err);
		lok(ex);
		lfequal(te_eval(ex), answer);
		te_free(ex);
	}
}

te_traits::t_vector clo0(void* context)
{
	if (context)
		return *((te_traits::t_vector*)context) + 6;
	return 6;
}
te_traits::t_vector clo1(void* context, te_traits::t_vector a)
{
	if (context)
		return *((te_traits::t_vector*)context) + a * 2;
	return a * 2;
}
te_traits::t_vector clo2(void* context, te_traits::t_vector a, te_traits::t_vector b)
{
	if (context)
		return *((te_traits::t_vector*)context) + a + b;
	return a + b;
}

te_traits::t_vector cell(void* context, te_traits::t_vector a)
{
	te_traits::t_vector* c = (te_traits::t_vector*)context;
	return c[(int)a];
}

void test_closure()
{
	te_traits::t_vector extra;
	te_traits::t_vector c[] = {5, 6, 7, 8, 9};

	te_variable lookup[] = {
		{"c0", clo0, TE_CLOSURE0, &extra},
		{"c1", clo1, TE_CLOSURE1, &extra},
		{"c2", clo2, TE_CLOSURE2, &extra},
		{"cell", cell, TE_CLOSURE1, c},
	};

	test_case cases[] = {
		{"c0", 6},
		{"c1 4", 8},
		{"c2 (10, 20)", 30},
	};

	int i;
	for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i)
	{
		const char*				  expr	 = cases[i].expr;
		const te_traits::t_vector answer = cases[i].answer;

		int	 err;
		auto ex = te_compile(expr, lookup, sizeof(lookup) / sizeof(te_variable), &err);
		lok(ex);

		extra = 0;
		lfequal(te_eval(ex), answer + extra);

		extra = 10;
		lfequal(te_eval(ex), answer + extra);

		te_free(ex);
	}

	test_case cases2[] = {
		{"cell 0", 5},
		{"cell 1", 6},
		{"cell 0 + cell 1", 11},
		{"cell 1 * cell 3 + cell 4", 57},
	};

	for (i = 0; i < sizeof(cases2) / sizeof(test_case); ++i)
	{
		const char*				  expr	 = cases2[i].expr;
		const te_traits::t_vector answer = cases2[i].answer;

		int	 err;
		auto ex = te_compile(expr, lookup, sizeof(lookup) / sizeof(te_variable), &err);
		lok(ex);
		lfequal(te_eval(ex), answer);
		te_free(ex);
	}
}

void test_optimize()
{
	test_case cases[] = {
		{"5+5", te_traits::load_atom(10)},
		{"pow(2,2)", te_traits::load_atom(4)},
		{"sqrt 100", te_traits::load_atom(10)},
		{"pi * 2", te_traits::load_atom(6.2832)},
	};

	int i;
	for (i = 0; i < sizeof(cases) / sizeof(test_case); ++i)
	{
		const char*				  expr	 = cases[i].expr;
		const te_traits::t_vector answer = cases[i].answer;

		int	 err;
		auto ex = te_compile(expr, 0, 0, &err);
		lok(ex);

		/* The answer should be know without
		 * even running eval. */
		// lfequal(ex->value, answer); // TODO
		lfequal(te_eval(ex), answer);

		te_free(ex);
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

	te_traits::t_vector a = 2, b = 3;

	te_variable lookup[] = {{"a", &a}, {"b", &b}};

	int i;
	for (i = 0; i < sizeof(cases) / sizeof(test_equ); ++i)
	{
		const char* expr1 = cases[i].expr1;
		const char* expr2 = cases[i].expr2;

		auto ex1 = te_compile(expr1, lookup, sizeof(lookup) / sizeof(te_variable), 0);
		auto ex2 = te_compile(expr2, lookup, sizeof(lookup) / sizeof(te_variable), 0);

		lok(ex1);
		lok(ex2);

		te_traits::t_vector r1 = te_eval(ex1);
		te_traits::t_vector r2 = te_eval(ex2);

		fflush(stdout);
		lfequal(r1, r2);

		te_free(ex1);
		te_free(ex2);
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
		const char*				  expr	 = cases[i].expr;
		const te_traits::t_vector answer = cases[i].answer;

		int						  err;
		const te_traits::t_vector ev = te_interp(expr, &err);
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
		const char*				  expr	 = cases[i].expr;
		const te_traits::t_vector answer = cases[i].answer;

		int						  err;
		const te_traits::t_vector ev = te_interp(expr, &err);
		lok(!err);
		lfequal(ev, answer);

		if (err)
		{
			printf("FAILED: %s (%d)\n", expr, err);
		}
	}
}

int main(int argc, char* argv[])
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

	return lfails != 0;
}
