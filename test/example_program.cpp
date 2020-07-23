#define TE_IMPLEMENT 1
#include "tinyexpr.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
	using namespace te;

	const char* p =
		"x: sqrt(5^2+7^2+11^2+(8-2)^2);"
		"jump: is_negative ? x < 0;"
		"return: x;"
		"label: is_negative;"
		"return: -1 * x;";

	env_traits::t_atom x, y;
	variable		   vars[] = {{"x", &x}, {"y", &y}};

	int	 err  = 0;
	auto prog = compile_program(p, vars, 2, &err);
	
	auto array_size = prog->get_binding_array_size();
	auto binding_addrs = prog->get_binding_addresses();
	auto binding_names = prog->get_binding_names();
	auto data_size = prog->get_data_size();
	auto data = prog->get_data();
	auto num_statements = prog->get_statement_array_size();
	auto statements = prog->get_statements();

	delete prog;
	return 0;
}
