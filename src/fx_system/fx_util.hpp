#pragma once
#include "fx_structs.hpp"

namespace fx_system
{
	float				Com_ParseFloat(const char** buf_p);
	int					Com_ParseInt(const char** buf_p);

	game::Material*		Material_FromHandle(game::Material* handle);
	void				Material_GetInfo(game::Material* handle, game::MaterialInfo* matInfo);
}