#pragma once

namespace components
{
	class reflectionprobes : public component
	{
	
	public:
		reflectionprobes();
		~reflectionprobes();
		const char* get_name() override { return "reflectionprobes"; };

		static std::uint8_t* cubemapshot_image_ptr[6];

		static void generate_reflections_for_bsp();

	private:
		static void generate_reflections(game::GfxReflectionProbe* probes, const unsigned int probe_count);
	};
}
