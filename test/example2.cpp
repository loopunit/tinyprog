#define TP_COMPILER_ENABLED 1
#include "tinyprog.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Usage: example2 \"expression\"\n");
		return 0;
	}

	const char* expression = argv[1];
	printf("Evaluating:\n\t%s\n", expression);

	/* This shows an example where the variables
	 * x and y are bound at eval-time. */
	te::env_traits::t_atom x, y;
	te::variable		   vars[] = {{"x", &x}, {"y", &y}};

	/* This will compile the expression and check for errors. */
	int	 err;
	auto n = te::compile(expression, vars, 2, &err);

	if (n)
	{
		/* The variables can be changed here, and eval can be called as many
		 * times as you like. This is fairly efficient because the parsing has
		 * already been done. */
		x			   = 3;
		y			   = 4;
		const te::env_traits::t_atom r = te::eval(n);
		printf("Result:\n\t%f\n", r);

		delete n;
	}
	else
	{
		/* Show the user where the error is at. */
		printf("\t%*s^\nError near here", err - 1, "");
	}

	return 0;
}
