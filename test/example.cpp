#define TE_IMPLEMENT 1
#include "tinyexpr.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    using namespace te;

    const char *c = "sqrt(5^2+7^2+11^2+(8-2)^2)";
	env_traits::t_atom r = interp(c, 0);
    printf("The expression:\n\t%s\nevaluates to:\n\t%f\n", c, r);
    return 0;
}
