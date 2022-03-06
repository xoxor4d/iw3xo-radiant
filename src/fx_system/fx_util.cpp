#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) __debugbreak();	else {	\
					game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__); }

namespace fx_system
{
	float Com_ParseFloat(const char** buf_p)
	{
		return static_cast<float>(atof(game::Com_Parse(buf_p)));
	}

	int Com_ParseInt(const char** buf_p)
	{
		return atoi(game::Com_Parse(buf_p));
	}

	game::Material* Material_FromHandle(game::Material* handle)
	{
		if (!handle || !handle->info.name || !*handle->info.name)
		{
			Assert();
		}
		
		return handle;
	}

	const char* Material_GetName(game::Material* handle)
	{
		if (!handle)
		{
			Assert();
		}

		const char* name = Material_FromHandle(handle)->info.name;
		char sym = *name;

		for (const char* itr = name; sym; ++itr)
		{
			if (sym == '/')
			{
				name = itr + 1;
			}

			sym = itr[1];
		}

		return name;
	}

	void Material_GetInfo(game::Material* handle, game::MaterialInfo* matInfo)
	{
		if (!handle || !matInfo)
		{
			Assert();
		}
		
		memcpy(matInfo, Material_FromHandle(handle), sizeof(game::MaterialInfo));
	}

	void alloc_assign_string(const char** visuals, const char* in)
	{
		if (!visuals || !in)
		{
			Assert();
		}

		const auto len = strlen(in);
		if (*visuals)
		{
			free((void*)*visuals);
		}

		*visuals = static_cast<const char*>(malloc(len + 1));
		if(*visuals)
		{
			strcpy((char*)*visuals, in);
		}
		else
		{
			Assert();
		}
	}

	// #ENV_DEPENDENT
	FxEditorEffectDef* get_editor_effect()
	{
#ifdef FXEDITOR
		return reinterpret_cast<FxEditorEffectDef*>(0x6579BC);
#else
		return &fx_system::ed_editor_effect;
#endif
	}


#ifdef FXEDITOR
	// #ENV_DEPENDENT
	const char* get_loaded_effect_string()
	{
		return reinterpret_cast<const char*>(0x63787C);
	}

#else

	std::string get_loaded_effect_string()
	{
		if (const auto& fs_basepath = game::Dvar_FindVar("fs_basepath");
						fs_basepath && ed_editor_effect.name)
		{
			std::string filepath = fs_basepath->current.string;
			filepath += "\\raw\\fx\\"s;
			filepath += ed_editor_effect.name + ".efx"s;

			return filepath;
		}

		return "";
	}
#endif
}