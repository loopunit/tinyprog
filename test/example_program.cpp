#define TP_TESTING 1
#include "tinyprog.h"
#include <stdio.h>

bool serialize_program_to_disk(const char* file_name, const char* prog_text, te::t_indexer& indexer)
{
	using builtins = tp::compiler_builtins<te::env_traits::t_vector_builtins>;

	auto prog = [prog_text, &indexer]() {
		int	 err1 = 0;
		auto res  = te::compile_program_using_indexer(prog_text, &err1, indexer);
		assert(res);
		return res;
	}();

	te::serialized_program sp(prog, indexer.m_declared_variable_names);

	FILE* f;
	if (!::fopen_s(&f, file_name, "wb"))
	{
		::fwrite(sp.get_raw_data(), 1, sp.get_raw_data_size(), f);
		::fflush(f);
		::fclose(f);
		f = nullptr;

		return true;
	}

	return false;
}

std::tuple<void*, size_t> serialize_from_disk(const char* file_name)
{
	FILE* f;
	if (!::fopen_s(&f, file_name, "rb"))
	{
		::fseek(f, 0, SEEK_END);
		auto s = ::ftell(f);
		::fseek(f, 0, SEEK_SET);
		auto mem = ::malloc(s);
		if (mem)
		{
			::fread(mem, 1, s, f);
			return {mem, s};
		}
	}
	return {nullptr, 0};
}

int main(int argc, char* argv[])
{
	using builtins = tp::compiler_builtins<te::env_traits::t_vector_builtins>;

	//te::env_traits::t_atom x = 0.0f, y = 0.0f;
	//te::variable		   vars[] = {{"x", &x}, {"y", &y}};
	
	//{
	//	te::t_indexer indexer;
	//	//indexer.add_user_variable(vars + 0);
	//	//indexer.add_user_variable(vars + 1);
	//
	//	const char* p1 =
	//		"var: x;"
	//		"x: sqrt(5^2+7^2+11^2+(8-2)^2);"
	//		"jump: is_negative ? x < 0;"
	//		"return: x;"
	//		"label: is_negative;"
	//		"return: -1 * x;";
	//
	//	serialize_program_to_disk("prog1.tpp", p1, indexer);
	//
	//	const char* p2 =
	//		"var: y;"
	//		"y: sqrt(5^2+7^2+11^2+(8-2)^2);"
	//		"jump: is_negative ? y < 0;"
	//		"return: y;"
	//		"label: is_negative;"
	//		"return: -1 * y;";
	//
	//	serialize_program_to_disk("prog2.tpp", p2, indexer);
	//}

	{
		auto [buffer, buffer_size] = serialize_from_disk("prog1.tpp");
		te::serialized_program prog(buffer, buffer_size);

		std::vector<const void*> binding_array;
		binding_array.resize(prog.get_num_bindings());
		std::fill_n(std::begin(binding_array), prog.get_num_bindings(), nullptr);
	
		std::vector<te::env_traits::t_vector> user_var_array;
		user_var_array.resize(prog.get_num_user_vars());
		
		for (uint16_t i = 0; i < (uint16_t)prog.get_num_user_vars(); ++i)
		{
			auto binding_idx = prog.get_user_vars()[i];
			binding_array[binding_idx] = &user_var_array[i];
		}

		for (uint16_t i = 0; i < (uint16_t)prog.get_num_bindings(); ++i)
		{
			if (!binding_array[i])
			{
				auto name = prog.get_binding_string(i);
				binding_array[i] = builtins::find_builtin_address(name);
			}
			assert(binding_array[i] != nullptr);
		}
		
		auto result = te::eval_program(prog, &binding_array[0]);
		::free(buffer);
	}

	//
	// auto prog1 = [&indexer]() {
	//	int	 err1 = 0;
	//	auto res  = te::compile_program_using_indexer(p1, &err1, indexer);
	//	assert(res);
	//	return res;
	//}();
	// auto result1 = te::eval_program(prog1);
	//
	// auto prog2 = [&indexer]() -> auto
	//{
	//	int	 err2 = 0;
	//	auto res  = te::compile_program_using_indexer(p2, &err2, indexer);
	//	assert(res);
	//	assert(err2 == 0);
	//	return res;
	//}
	//();
	// auto result2 = te::eval_program(prog2);
	//
	// assert(result1 == result2);
	// assert(prog1->get_binding_array_size() <= prog2->get_binding_array_size());
	//
	// auto ba1 = prog1->get_binding_addresses();
	// auto ba2 = prog2->get_binding_addresses();
	//
	// for (int i = 0; i < prog1->get_binding_array_size(); ++i)
	//{
	//	assert(ba1[i] == ba2[i]);
	//}
	//
	// using builtins = tp::compiler_builtins<te::env_traits::t_vector_builtins>;
	//
	//{
	//	std::vector<const void*> binding_array;
	//	te::serialized_program	 sp1(prog1);
	//
	//	for (uint16_t i = 0; i < (uint16_t)sp1.get_num_bindings(); ++i)
	//	{
	//		auto name = sp1.get_binding_string(i);
	//		if (_stricmp(name, "x") == 0)
	//		{
	//			binding_array.push_back(&x);
	//		}
	//		else if (_stricmp(name, "y") == 0)
	//		{
	//			binding_array.push_back(&y);
	//		}
	//		else
	//		{
	//			binding_array.push_back(builtins::find_builtin_address(name));
	//		}
	//	}
	//	auto spresult1 = te::eval_program(sp1, &binding_array[0]);
	//}
	//
	//{
	//	std::vector<const void*> binding_array;
	//	te::serialized_program	 sp2(prog2);
	//
	//	for (uint16_t i = 0; i < sp2.get_num_bindings(); ++i)
	//	{
	//		auto name = sp2.get_binding_string(i);
	//		if (_stricmp(name, "x") == 0)
	//		{
	//			binding_array.push_back(&x);
	//		}
	//		else if (_stricmp(name, "y") == 0)
	//		{
	//			binding_array.push_back(&y);
	//		}
	//		else
	//		{
	//			binding_array.push_back(builtins::find_builtin_address(name));
	//		}
	//	}
	//	auto spresult2 = te::eval_program(sp2, &binding_array[0]);
	//}
	//
	// delete prog1;
	// delete prog2;

	return 0;
}

#if 0
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
#endif