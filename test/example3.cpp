#include "tinyexpr.h"
#include <stdio.h>

/* An example of calling a C function. */
te::env_traits::t_atom my_sum(te::env_traits::t_atom a, te::env_traits::t_atom b)
{
	printf("Called C function with %f and %f.\n", a, b);
	return a + b;
}

int main(int argc, char* argv[])
{
	using namespace te;

	variable vars[] = {{"mysum", my_sum, TE_FUNCTION2}};

	const char* expression = "mysum(5, 6)";
	printf("Evaluating:\n\t%s\n", expression);

	int	 err;
	auto n = compile(expression, vars, 1, &err);

	if (n)
	{
		const env_traits::t_atom r = eval(n);
		printf("Result:\n\t%f\n", r);
		te_free(n);
	}
	else
	{
		/* Show the user where the error is at. */
		printf("\t%*s^\nError near here", err - 1, "");
	}

	return 0;
}
