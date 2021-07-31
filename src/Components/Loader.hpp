#pragma once

namespace components
{
	class component
	{
	public:
		component() {};
		virtual ~component() {};
		virtual const char* get_name() { return "Unknown"; };
	};

	class loader
	{
	public:
		static void initialize();
		static void uninitialize();
		static void Register(component* component);

		static utils::memory::allocator* GetAlloctor();

	private:
		static std::vector<component*> components_;
		static utils::memory::allocator mem_allocator_;
	};
}

#include "modules/command.hpp"
#include "modules/d3d9ex.hpp"
#include "modules/gui.hpp"
#include "modules/quick_patch.hpp"
#include "modules/remote_net.hpp"
#include "modules/config.hpp"
