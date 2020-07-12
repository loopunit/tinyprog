#include "tinyexpr.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
	tinyexpr	te_instance;
	const char* c = "sqrt(5^2+7^2+11^2+(8-2)^2)";
	double		r = te_instance.te_interp(c, 0);
	printf("The expression:\n\t%s\nevaluates to:\n\t%f\n", c, r);
	return 0;
}
