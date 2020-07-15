#include "tinyexpr.h"
#include <stdio.h>

/* An example of calling a C function. */
double my_sum(double a, double b)
{
	printf("Called C function with %f and %f.\n", a, b);
	return a + b;
}

int main(int argc, char* argv[])
{
	const char* expression = "mysum(5, 6)";
	printf("Evaluating:\n\t%s\n", expression);

	int		 err;
	tinyexpr te_instance;

	te_instance.register_variables({{"mysum", my_sum, tinyexpr::TE_FUNCTION2}});
	auto n = te_instance.te_compile(expression, &err);

	if (n)
	{
		const double r = te_instance.te_eval(n);

		tinyexpr::te_export_dict export_dict;
		auto					 n_portable = te_instance.te_export_portable(n, export_dict);

		const double r_portable = te_instance.te_eval_portable(n_portable, &export_dict.binding_table_data[0]);

		printf("Result:\n\t%f::%f\n", r, r_portable);
		te_instance.te_free_portable(n_portable);
		te_instance.te_free(n);
	}
	else
	{
		/* Show the user where the error is at. */
		printf("\t%*s^\nError near here", err - 1, "");
	}

	return 0;
}
