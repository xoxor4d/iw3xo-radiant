#include "std_include.hpp"

namespace components
{
	std::vector<component*> loader::components_;
	utils::memory::allocator loader::mem_allocator_;

	void loader::initialize()
	{
		loader::mem_allocator_.clear();

		loader::Register(new command());
		loader::Register(new d3d9ex());
		loader::Register(new gui());
		loader::Register(new quick_patch());
		loader::Register(new remote_net());
		loader::Register(new config());
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

	void loader::Register(component* component)
	{
		if (component)
		{
			game::glob::loadedModules.append(utils::va("component registered: %s\n", component->get_name()));
			loader::components_.push_back(component);
		}
	}

	utils::memory::allocator* loader::GetAlloctor()
	{
		return &loader::mem_allocator_;
	}
}
