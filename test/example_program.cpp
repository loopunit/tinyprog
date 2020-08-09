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

float test_closure(void* context, float arg) 
{
	printf("Closure called: %llu, %f\n", (size_t)context, arg);
	return arg;
}

int main(int argc, char* argv[])
{
	using builtins = tp::compiler_builtins<te::env_traits::t_vector_builtins>;

	te::env_traits::t_atom	x = 0.0f, y = -1.0f;
	te::variable			vars[]	   = {{"xx", &x}, {"y", &y}, {"test_closure", test_closure, tp::CLOSURE1, (void*)0xf33db33ff33db33f}};
	static constexpr size_t vars_count = sizeof(vars) / sizeof(vars[0]);

	// compile & save to disk
	{
		const char* constructor =
			"var: x;"
			"x: 255.0;"
			"var: y;"
			"y: 255.0;"
			"xx: 255.0;";

		const char* p1 =
			"x: sqrt(5^2+7^2+11^2+(8-2)^2);"
			"test_closure(x);"
			"jump: is_negative ? x < 0;"
			"return: x;"
			"label: is_negative;"
			"return: -1 * x;";

		const char* p2 =
			"y: sqrt(5^2+7^2+11^2+(8-2)^2);"
			"test_closure(y);"
			"jump: is_negative ? y < 0;"
			"return: y;"
			"label: is_negative;"
			"return: -1 * y;";

		const char*	 progs[]{constructor, p1, p2};
		const size_t num_progs = sizeof(progs) / sizeof(progs[0]);

		auto prog = create_program(progs, num_progs, vars, vars_count);
		assert(prog);
		if (!serialize_program_to_disk("progs.tpp", prog))
		{
			assert(0);
		}
		delete prog;
	}

	// Load from disk, setup bindings, execute
	{
		te::serialized_program* prog = serialize_from_disk("progs.tpp");
		assert(prog);

		std::vector<const void*> binding_array;
		binding_array.resize(prog->get_num_bindings());
		std::fill_n(std::begin(binding_array), prog->get_num_bindings(), nullptr);

		std::vector<te::env_traits::t_vector> user_var_array;
		user_var_array.resize(prog->get_num_user_vars());
		std::fill(std::begin(user_var_array), std::end(user_var_array), 0.0f);

		// Binding precendence will be: declared->user->builtin

		// Declared vars come first
		for (uint16_t i = 0; i < (uint16_t)prog->get_num_user_vars(); ++i)
		{
			auto binding_idx		   = prog->get_user_vars()[i];
			binding_array[binding_idx] = &user_var_array[i];
		}

		std::unordered_map<std::string, void*> closure_contexts;

		for (uint16_t i = 0; i < (uint16_t)prog->get_num_bindings(); ++i)
		{
			// If the binding was not already set as a declared var
			if (!binding_array[i])
			{
				auto name = prog->get_binding_string(i);

				// If not a builtin, see if we have a user binding
				for (uint16_t j = 0; j < vars_count; ++j)
				{
					if (_stricmp(vars[j].name, name) == 0)
					{
						binding_array[i] = vars[j].address;

						if (vars[j].type >= tp::CLOSURE0 && vars[j].type < tp::CLOSURE_MAX)
						{
							closure_contexts.insert({std::string(vars[j].name) + "_closure", vars[j].context});
						}
					}
				}

				if (!binding_array[i])
				{
					// See if the binding is a builtin
					binding_array[i] = builtins::find_builtin_address(name);
				}

				if (!binding_array[i])
				{
					// look in the closure contexts
					auto itor = closure_contexts.find(name);
					if (itor != std::end(closure_contexts))
					{
						binding_array[i] = itor->second;
					}
				}
			}

			// All bindings must be valid, otherwise the runtime will crash.
			assert(binding_array[i] != nullptr);
		}

		float* results	   = new float[prog->get_num_subprograms()];
		float  last_result = 0;
		for (int i = 0; i < prog->get_num_subprograms(); ++i)
		{
			results[i]	= te::eval_program(*prog, i, &binding_array[0]);
			last_result = results[i];
		}

		for (int i = 1; i < prog->get_num_subprograms(); ++i)
		{
			assert(results[i] == last_result);
		}

		assert(x == 255.0f); // x should have been initialized to 255 in the constructor
		assert(y == -1.0f);	 // y should have been overridden by the declared var

		delete prog;
	}

	return 0;
}
