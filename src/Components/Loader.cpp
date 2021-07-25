#include "STDInclude.hpp"

namespace Components
{
	std::vector<Component*> Loader::Components;
	Utils::Memory::Allocator Loader::MemAllocator;

	void Loader::Initialize()
	{
		Loader::MemAllocator.clear();

		Loader::Register(new Command());
		Loader::Register(new D3D9Ex());
		Loader::Register(new Gui());
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
			Game::Globals::loadedModules.append(Utils::VA("Component registered: %s\n", component->getName()));
			Loader::Components.push_back(component);
		}
	}

	Utils::Memory::Allocator* Loader::GetAlloctor()
	{
		return &Loader::MemAllocator;
	}
}
