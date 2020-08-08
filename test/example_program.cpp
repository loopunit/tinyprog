#define TP_TESTING 1
#include "tinyprog.h"
#include <stdio.h>

bool serialize_program_to_disk(const char* file_name, te::serialized_program* sp)
{
	FILE* f;
	if (!::fopen_s(&f, file_name, "wb"))
	{
		::fwrite(sp->get_raw_data(), 1, sp->get_raw_data_size(), f);
		::fflush(f);
		::fclose(f);
		f = nullptr;

		return true;
	}

	return false;
}

te::serialized_program* create_program(const char* prog_texts[], size_t num_prog_texts, te::variable* vars, size_t num_vars)
{
	using builtins = tp::compiler_builtins<te::env_traits::t_vector_builtins>;

	te::t_indexer indexer;
	for (int i = 0; i < num_vars; ++i)
	{
		indexer.add_user_variable(vars + i);
	}
	
	std::vector<tp::compiled_program*> subprograms;
	for (int i = 0; i < num_prog_texts; ++i)
	{
		subprograms.push_back([i, prog_texts, &indexer]() {
			int	 err1 = 0;
			auto res  = te::compile_program_using_indexer(prog_texts[i], &err1, indexer);
			assert(res);
			return res;
		}());
	}

	return new te::serialized_program(&subprograms[0], int(num_prog_texts), indexer.m_declared_variable_names);
}

te::serialized_program* serialize_from_disk(const char* file_name)
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
			return te::serialized_program::create_using_buffer(mem, s);
		}
	}
	return nullptr;
}

int main(int argc, char* argv[])
{
	using builtins = tp::compiler_builtins<te::env_traits::t_vector_builtins>;

	te::env_traits::t_atom x = 0.0f, y = 0.0f;
	te::variable		   vars[] = {{"xx", &x}, {"y", &y}};
	static constexpr size_t vars_count = sizeof(vars) / sizeof(vars[0]);
	
	// compile & save to disk
	{
		const char* p1 =
			"var: x;"
			"x: sqrt(5^2+7^2+11^2+(8-2)^2);"
			"jump: is_negative ? x < 0;"
			"return: x;"
			"label: is_negative;"
			"return: -1 * x;";
	
		const char* p2 =
			"y: sqrt(5^2+7^2+11^2+(8-2)^2);"
			"jump: is_negative ? y < 0;"
			"return: y;"
			"label: is_negative;"
			"return: -1 * y;";
	
		const char* progs[] { p1, p2 };
	
		auto prog = create_program(progs, 2, vars, vars_count);
		assert(prog);
		if (!serialize_program_to_disk("progs.tpp", prog))
		{
			assert(0);
		}
		delete prog;
	}

	// Load from disk, execute
	{
		te::serialized_program* prog = serialize_from_disk("progs.tpp");;
		assert(prog);

		std::vector<const void*> binding_array;
		binding_array.resize(prog->get_num_bindings());
		std::fill_n(std::begin(binding_array), prog->get_num_bindings(), nullptr);

		std::vector<te::env_traits::t_vector> user_var_array;
		user_var_array.resize(prog->get_num_user_vars());
		std::fill(std::begin(user_var_array), std::end(user_var_array), 0.0f);

		for (uint16_t i = 0; i < (uint16_t)prog->get_num_user_vars(); ++i)
		{
			auto binding_idx = prog->get_user_vars()[i];
			binding_array[binding_idx] = &user_var_array[i];
		}

		for (uint16_t i = 0; i < (uint16_t)prog->get_num_bindings(); ++i)
		{
			if (!binding_array[i])
			{
				auto name = prog->get_binding_string(i);
				binding_array[i] = builtins::find_builtin_address(name);
				
				if (!binding_array[i])
				{
					for (uint16_t j = 0; j < vars_count; ++j)
					{
						if (_stricmp(vars[j].name, name) == 0)
						{
							binding_array[i] = vars[j].address;
						}
					}
				}
			}

			assert(binding_array[i] != nullptr);
		}
		
		auto a = te::eval_program(*prog, 0, &binding_array[0]) ;
		auto b = te::eval_program(*prog, 1, &binding_array[0]) ;
		assert(a == b);
		
		delete prog;
	}

	return 0;
}
