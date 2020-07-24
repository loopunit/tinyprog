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

#include <stdio.h>
#include <time.h>
#include <math.h>

#define TP_TESTING 1
#include "tinyprog.h"

#define loops 10000

typedef te::env_traits::t_atom (*function1)(te::env_traits::t_atom);

void bench(const char* expr, function1 func)
{
	int				i, j;
	volatile te::env_traits::t_atom d;
	te::env_traits::t_atom			tmp;
	clock_t			start;

	te::variable lk = {"a", &tmp};

	printf("Expression: %s\n", expr);

	printf("native ");
	start = clock();
	d	  = 0;
	for (j = 0; j < loops; ++j)
		for (i = 0; i < loops; ++i)
		{
			tmp = (te::env_traits::t_atom)i;
			d += func(tmp);
		}
	const int nelapsed = (clock() - start) * 1000 / CLOCKS_PER_SEC;

	/*Million floats per second input.*/
	printf(" %.5g", d);
	if (nelapsed)
		printf("\t%5dms\t%5dmfps\n", nelapsed, loops * loops / nelapsed / 1000);
	else
		printf("\tinf\n");

	printf("interp ");
	auto n	= te::compile(expr, &lk, 1, 0);
	start	= clock();
	d		= 0;
	for (j = 0; j < loops; ++j)
		for (i = 0; i < loops; ++i)
		{
			tmp = (te::env_traits::t_atom)i;
			d += te::eval(n);
		}
	const int eelapsed = (clock() - start) * 1000 / CLOCKS_PER_SEC;
	delete n;

	/*Million floats per second input.*/
	printf(" %.5g", d);
	if (eelapsed)
		printf("\t%5dms\t%5dmfps\n", eelapsed, loops * loops / eelapsed / 1000);
	else
		printf("\tinf\n");

	printf("%.2f%% longer\n", (((te::env_traits::t_atom)eelapsed / nelapsed) - 1.0) * 100.0);

	printf("\n");
}

te::env_traits::t_atom a5(te::env_traits::t_atom a)
{
	return a + 5;
}

te::env_traits::t_atom a52(te::env_traits::t_atom a)
{
	return (a + 5) * 2;
}

te::env_traits::t_atom a10(te::env_traits::t_atom a)
{
	return a + (5 * 2);
}

te::env_traits::t_atom as(te::env_traits::t_atom a)
{
	return te::env_traits::t_vector_builtins::sqrt(
		te::env_traits::t_vector_builtins::pow(a, 1.5) + te::env_traits::t_vector_builtins::pow(a, 2.5));
}

te::env_traits::t_atom al(te::env_traits::t_atom a)
{
	return (1 / (a + 1) + 2 / (a + 2) + 3 / (a + 3));
}

int main(int argc, char* argv[])
{
	bench("sqrt(a^1.5+a^2.5)", as);
	bench("a+5", a5);
	bench("a+(5*2)", a10);
	bench("(a+5)*2", a52);
	bench("(1/(a+1)+2/(a+2)+3/(a+3))", al);

	return 0;
}
