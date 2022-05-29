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
		static void _register(component* component);

		static utils::memory::allocator* get_alloctor();

	private:
		static std::vector<component*> components_;
		static utils::memory::allocator mem_allocator_;
	};
}

#include "modules/command.hpp"
#include "modules/config.hpp"
#include "modules/d3d9ex.hpp"
#include "modules/d3dbsp.hpp"
#include "modules/discord.hpp"
#include "modules/effects.hpp"
#include "modules/effects_editor.hpp"
#include "modules/exec.hpp"
#include "modules/gameview.hpp"
#include "modules/gui.hpp"
#include "modules/main_module.hpp"
#include "modules/pmesh.hpp"
#include "modules/process.hpp"
#include "modules/reflectionprobes.hpp"
#include "modules/renderer.hpp"
#include "modules/remote_net.hpp"
