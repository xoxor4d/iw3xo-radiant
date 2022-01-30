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

	void Material_GetInfo(game::Material* handle, game::MaterialInfo* matInfo)
	{
		if (!handle || !matInfo)
		{
			Assert();
		}
		
		memcpy(matInfo, Material_FromHandle(handle), sizeof(game::MaterialInfo));
	}

}