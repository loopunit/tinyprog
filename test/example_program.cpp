#define TP_COMPILER_ENABLED 1
#include "tinyprog.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
	const char* p =
		"x: sqrt(5^2+7^2+11^2+(8-2)^2);"
		"jump: is_negative ? x < 0;"
		"return: x;"
		"label: is_negative;"
		"return: -1 * x;";

	te::env_traits::t_atom x = 0.0f, y = 0.0f;
	te::variable		   vars[] = {{"x", &x}, {"y", &y}};

	int	 err  = 0;
	auto prog = te::compile_program(p, vars, 2, &err);
	auto result = te::eval_program(prog);
	delete prog;
	return 0;
}
