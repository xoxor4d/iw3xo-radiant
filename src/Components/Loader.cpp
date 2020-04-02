#include "STDInclude.hpp"

namespace Components
{
	std::vector<Component*> Loader::Components;
	Utils::Memory::Allocator Loader::MemAllocator;

	void Loader::Initialize()
	{
		Loader::MemAllocator.clear();

		Loader::Register(new QuickPatch());
		Loader::Register(new RemNet());
		Loader::Register(new Config());
	}

	void Loader::Uninitialize()
	{
		std::reverse(Loader::Components.begin(), Loader::Components.end());
		for (auto component : Loader::Components)
		{
			delete component;
		}

		Loader::Components.clear();
		Loader::MemAllocator.clear();
		fflush(stdout);
		fflush(stderr);
	}

	void Loader::Register(Component* component)
	{
		if (component)
		{
			Loader::Components.push_back(component);
		}
	}

	Utils::Memory::Allocator* Loader::GetAlloctor()
	{
		return &Loader::MemAllocator;
	}
}
