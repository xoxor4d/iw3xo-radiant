#pragma once

namespace Components
{
	class Component
	{
	public:
		Component() {};
		virtual ~Component() {};
		virtual const char* getName() { return "Unknown"; };
	};

	class Loader
	{
	public:
		static void Initialize();
		static void Uninitialize();
		static void Register(Component* component);

		static Utils::Memory::Allocator* GetAlloctor();

	private:
		static std::vector<Component*> Components;
		static Utils::Memory::Allocator MemAllocator;
	};
}

#include "Modules/QuickPatch.hpp"
#include "Modules/RemoteNet.hpp"
#include "Modules/Config.hpp"