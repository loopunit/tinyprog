#define TP_COMPILER_ENABLED 1
#include "tinyprog.h"
#include <stdio.h>

/* An example of calling a C function. */
te::env_traits::t_atom my_sum(te::env_traits::t_atom a, te::env_traits::t_atom b)
{
	printf("Called C function with %f and %f.\n", a, b);
	return a + b;
}

int main(int argc, char* argv[])
{
	te::variable vars[] = {{"mysum", my_sum, tp::FUNCTION2}};

	const char* expression = "mysum(5, 6)";
	printf("Evaluating:\n\t%s\n", expression);

	int	 err;
	auto n = te::compile(expression, vars, 1, &err);

	if (n)
	{
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
