#pragma once

namespace components
{
	class generate_previews : public component
	{
	public:
		generate_previews();
		~generate_previews();
		const char* get_name() override { return "generate_previews"; };

		static void generate_prefab_previews(const std::filesystem::path& path);
	};
}
