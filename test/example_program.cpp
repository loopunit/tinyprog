#define TP_TESTING 1
#include "tinyprog.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
	//te::env_traits::t_atom x = 0.0f, y = 0.0f;
	//te::variable		   vars[] = {{"x", &x}, {"y", &y}};
	te::t_indexer		   indexer;

	//indexer.add_user_variable(vars + 0);
	//indexer.add_user_variable(vars + 1);

	auto prog1 = [&indexer]() {
		const char* p1 =
			"var: x;"
			"x: sqrt(5^2+7^2+11^2+(8-2)^2);"
			"jump: is_negative ? x < 0;"
			"return: x;"
			"label: is_negative;"
			"return: -1 * x;";
		int	 err1 = 0;
		auto res  = te::compile_program_using_indexer(p1, &err1, indexer);
		assert(res);
		return res;
	}();
	auto result1 = te::eval_program(prog1);

	auto prog2 = [&indexer]() -> auto
	{
		const char* p2 =
			"var: y;"
			"y: sqrt(5^2+7^2+11^2+(8-2)^2);"
			"jump: is_negative ? y < 0;"
			"return: y;"
			"label: is_negative;"
			"return: -1 * y;";
		int	 err2 = 0;
		auto res  = te::compile_program_using_indexer(p2, &err2, indexer);
		assert(res);
		assert(err2 == 0);
		return res;
	}
	();
	auto result2 = te::eval_program(prog2);

	assert(result1 == result2);
	assert(prog1->get_binding_array_size() <= prog2->get_binding_array_size());

	auto ba1 = prog1->get_binding_addresses();
	auto ba2 = prog2->get_binding_addresses();

	for (int i = 0; i < prog1->get_binding_array_size(); ++i)
	{
		assert(ba1[i] == ba2[i]);
	}

	delete prog1;
	delete prog2;

	return 0;
}
