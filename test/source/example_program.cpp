#include <doctest/doctest.h>

#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include <vector>
#include <unordered_map>

#define TP_COMPILER_ENABLED 1
#define TP_STANDARD_LIBRARY 1
#include "tinyprog.h"

using te = tp::impl<tp_stdlib::env_traits_f32>;

#if TP_COMPILER_ENABLED
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
	using builtins = te::env_traits::t_vector_builtins;

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
#endif

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

TEST_CASE("example_program") 
{
	using builtins = tp_stdlib::compiler_builtins<tp_stdlib::native_builtins<float>>;

	te::env_traits::t_atom	x = 0.0f, y = -1.0f;
	te::variable			vars[]	   = {{"xx", &x}, {"y", &y}, {"test_closure", test_closure, tp::CLOSURE1, (void*)0xf33db33ff33db33f}};
	static constexpr size_t vars_count = sizeof(vars) / sizeof(vars[0]);

	// compile & save to disk
#if TP_COMPILER_ENABLED
	{
		const char* constructor =
			"var: x;"
			"x: 255.0;"
			"var: y;"
			"y: 255.0;"
			"xx: 255.0;";
	
		const char* p1 =
			"var: x_tmp ? local;"
			"x_tmp: sqrt(5^2+7^2+11^2+(8-2)^2);"
			"test_closure(x_tmp);"
			"jump: is_negative ? x_tmp < 0;"
			"return: x_tmp;"
			"label: is_negative;"
			"return: -1 * x_tmp;";
	
		const char* p2 =
			"var: y;"
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

		assert(prog->get_num_user_vars() == 3);

		delete prog;
	}
#endif // #if TP_COMPILER_ENABLED

	// Load from disk, setup bindings, execute
	{
		te::serialized_program* prog = serialize_from_disk("progs.tpp");
		assert(prog);

		// Reserve binding memory
		std::vector<const void*> binding_array;
		binding_array.resize(prog->get_num_bindings());
		std::fill_n(std::begin(binding_array), prog->get_num_bindings(), nullptr);

		// Reserve memory for user vars
		std::vector<te::env_traits::t_vector> user_var_array;
		user_var_array.resize(prog->get_num_user_vars());
		std::fill(std::begin(user_var_array), std::end(user_var_array), 0.0f);

		// Binding setup
		{
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

					// Closure contexts are always first
					if (!binding_array[i])
					{
						auto itor = closure_contexts.find(name);
						if (itor != std::end(closure_contexts))
						{
							binding_array[i] = itor->second;
						}
					}
				
					// User bindings are next priority
					for (uint16_t j = 0; j < vars_count; ++j)
					{
						if (_stricmp(vars[j].name, name) == 0)
						{
							binding_array[i] = vars[j].address;

							if (vars[j].type >= tp::CLOSURE0 && vars[j].type < tp::CLOSURE_MAX)
							{
								// Closures are always declared first, followed by a context, which is appended with "_closure".
								// Add this to the lookup so we find it later.
								closure_contexts.insert({std::string(vars[j].name) + "_closure", vars[j].context});
							}
						}
					}

					// Builtins come last
					if (!binding_array[i])
					{
						auto t = builtins::find_by_name(name, int(strlen(name)), nullptr);
						binding_array[i] = t ? t->address : nullptr;
					}

					// All bindings must be valid, otherwise the runtime will crash.
					assert(binding_array[i] != nullptr);
				}

			}
		}

		// Execute the test programs
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
}
