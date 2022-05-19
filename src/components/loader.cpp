#include "std_include.hpp"

namespace components
{
	std::vector<component*> loader::components_;
	utils::memory::allocator loader::mem_allocator_;

	void loader::initialize()
	{
		loader::mem_allocator_.clear();

		loader::_register(new gui());
		loader::_register(new command());
		loader::_register(new d3d9ex());
		loader::_register(new d3dbsp());
		loader::_register(new renderer());
		loader::_register(new gameview());
		loader::_register(new remote_net());
		loader::_register(new config());
		loader::_register(new effects());
		loader::_register(new effects_editor());
		loader::_register(new main_module());
		loader::_register(new discord());
		loader::_register(new pmesh());

		if(const auto	con = GET_GUI(ggui::console_dialog);
						con)
		{
			// print to external and internal console
			printf("[Modules] ---------------------\n");
			con->addline_no_format("[Modules] ---------------------\n");

			for (const auto& str : game::glob::loadedModules)
			{
				printf(str.c_str());
				con->addline_no_format(str.c_str());
			}

			// print to external and internal console
			printf("\n");
			con->addline_no_format("\n");
		}
	}

	void loader::uninitialize()
	{
		std::reverse(loader::components_.begin(), loader::components_.end());
		for (auto component : loader::components_)
		{
			delete component;
		}

		loader::components_.clear();
		loader::mem_allocator_.clear();
		fflush(stdout);
		fflush(stderr);
	}

	void loader::_register(component* component)
	{
		if (component)
		{
			game::glob::loadedModules.push_back("component registered: "s + component->get_name() + "\n");
			loader::components_.push_back(component);
		}
	}

	utils::memory::allocator* loader::get_alloctor()
	{
		return &loader::mem_allocator_;
	}
}
