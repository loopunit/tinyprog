#define TP_COMPILER_ENABLED 1
#include "tinyprog.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    const char *c = "sqrt(5^2+7^2+11^2+(8-2)^2)";
	te::env_traits::t_atom r = te::interp(c, 0);
    printf("The expression:\n\t%s\nevaluates to:\n\t%f\n", c, r);
    return 0;
}
