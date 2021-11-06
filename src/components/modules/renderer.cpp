#include "std_include.hpp"

namespace components
{
	void renderer::R_ConvertColorToBytes(float* from, game::GfxColor* gfx_col)
	{
		if (from)
		{
			return game::Byte4PackPixelColor(from, gfx_col);
		}

		gfx_col->packed = 0; //-1;
	}
	
	void renderer::R_AddCmdDrawTextAtPosition(const char* text, game::Font_s* font, float* origin, float* pixel_step_x, float* pixel_step_y, float* color)
	{
		if (text && *text)
		{
			const size_t t_size = strlen(text);
			auto cmd = reinterpret_cast<game::GfxCmdDrawText3D*>(game::R_RenderBufferCmdCheck((t_size + 0x34) & 0xFFFFFFFC, 16));
			if (cmd)
			{
				cmd->org[0] = origin[0];
				cmd->org[1] = origin[1];
				cmd->org[2] = origin[2];
				cmd->font = font;
				cmd->xPixelStep[0] = pixel_step_x[0];
				cmd->xPixelStep[1] = pixel_step_x[1];
				cmd->xPixelStep[2] = pixel_step_x[2];
				cmd->yPixelStep[0] = pixel_step_y[0];
				cmd->yPixelStep[1] = pixel_step_y[1];
				cmd->yPixelStep[2] = pixel_step_y[2];

				R_ConvertColorToBytes(color, &cmd->color);
				memcpy(cmd->text, text, t_size);
				cmd->text[t_size] = 0;
			}
		}
	}

	void renderer::copy_scene_to_texture(ggui::e_gfxwindow wnd, IDirect3DTexture9*& dest)
	{
		// get the backbuffer surface
		IDirect3DSurface9* surf_backbuffer = nullptr;
		game::dx->windows[wnd].swapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &surf_backbuffer);

		// write surface to file (test)
		//D3DXSaveSurfaceToFileA("surface_to_file.png", D3DXIFF_PNG, surf_backbuffer, NULL, NULL);

		// check if window size was changed -> release and recreate the texture surface
		if (dest)
		{
			D3DSURFACE_DESC desc;
			dest->GetLevelDesc(0, &desc);

			if (desc.Width != static_cast<unsigned int>(game::dx->windows[wnd].width) || desc.Height != static_cast<unsigned int>(game::dx->windows[wnd].height))
			{
				dest->Release();
				dest = nullptr;
			}
		}

		// create or re-create ^ the texture surface
		if (!dest)
		{
			D3DXCreateTexture(game::dx->device, game::dx->windows[wnd].width, game::dx->windows[wnd].height, D3DX_DEFAULT, D3DUSAGE_RENDERTARGET, D3DFMT_R8G8B8, D3DPOOL_DEFAULT, &dest);
		}

		// "bind" texture to surface
		IDirect3DSurface9* surf_texture;
		dest->GetSurfaceLevel(0, &surf_texture);

		// "copy" backbuffer to our texture surface
		game::dx->device->StretchRect(surf_backbuffer, NULL, surf_texture, NULL, D3DTEXF_NONE);

		// release the backbuffer surface or we'll leak memory
		surf_backbuffer->Release();

		// write texture to file (test)
		//D3DXSaveTextureToFileA("texture_to_file.png", D3DXIFF_PNG, dest, NULL);
	}

	struct gameview_helper_s
	{
		bool enabled;
		bool s_model_origin;
		int  s_xyflags;

		// geo
		bool s_filter_mantle;
		bool s_filter_nodraw;
		bool s_filter_shadowcaster;
		bool s_filter_traverse;
		bool s_filter_foliage;
		bool s_filter_weaponclip;
		bool s_filter_goal;
		bool s_filter_lightgrid;
		bool s_filter_caulk;
		bool s_filter_nodrawdecal;
		bool s_filter_clip;
		bool s_filter_hint;
		bool s_filter_portals;

		// ents
		bool s_filter_lights;
		bool s_filter_reflectionprobes;
		bool s_filter_aispawners;
		bool s_filter_pathnodes;
		bool s_filter_actionnodes;
		bool s_filter_info;
		bool s_filter_mpentities;
		bool s_filter_script_origins;

		// tiggers
		bool s_filter_trigger;
		bool s_filter_bcs;
		bool s_filter_escort;
		bool s_filter_unlock;
		bool s_filter_vehicle;
		bool s_filter_friendly_respawn;
		bool s_filter_flag;
		bool s_filter_fog;
		bool s_filter_auto_adjust;
		bool s_filter_spawner;
		bool s_filter_stopspawner;
		bool s_filter_friendlychain;
		bool s_filter_ambient;
		bool s_filter_damage;
		bool s_filter_trigger_hint;

		// other (optional if installed custom filters)
		bool s_filter_opt_ladder;
	};

	gameview_helper_s _gameview = {};

	void renderer::game_view(bool state)
	{
		// toggle off
		if(!state)
		{
			dvars::set_bool(dvars::radiant_gameview, false);
			
			if(!_gameview.enabled)
			{
				return;
			}

			if((_gameview.s_xyflags & 2) == 0) {
				game::g_qeglobals->d_savedinfo.d_xyShowFlags &= ~2;
			}

			if ((_gameview.s_xyflags & 4) == 0) {
				game::g_qeglobals->d_savedinfo.d_xyShowFlags &= ~4;
			}

			dvars::set_bool(dvars::r_draw_model_origin, _gameview.s_model_origin);

			// geometry filters
			{
				if (_gameview.s_filter_mantle)				ggui::filter::toggle_by_name("mantle", ggui::filter::GEOMETRY, true);
				if (_gameview.s_filter_nodraw)				ggui::filter::toggle_by_name("nodraw_notsolid", ggui::filter::GEOMETRY, true);
				if (_gameview.s_filter_shadowcaster)		ggui::filter::toggle_by_name("shadowcaster", ggui::filter::GEOMETRY, true);
				if (_gameview.s_filter_traverse)			ggui::filter::toggle_by_name("traverse", ggui::filter::GEOMETRY, true);
				if (_gameview.s_filter_foliage)				ggui::filter::toggle_by_name("foliage", ggui::filter::GEOMETRY, true);
				if (_gameview.s_filter_weaponclip)			ggui::filter::toggle_by_name("weaponclip", ggui::filter::GEOMETRY, true);
				if (_gameview.s_filter_goal)				ggui::filter::toggle_by_name("goalvolumes", ggui::filter::GEOMETRY, true);
				if (_gameview.s_filter_lightgrid)			ggui::filter::toggle_by_name("lightgrid", ggui::filter::GEOMETRY, true);
				if (_gameview.s_filter_caulk)				ggui::filter::toggle_by_name("caulk", ggui::filter::GEOMETRY, true);
				if (_gameview.s_filter_nodrawdecal)			ggui::filter::toggle_by_name("nodrawdecal", ggui::filter::GEOMETRY, true);
				if (_gameview.s_filter_clip)				ggui::filter::toggle_by_name("clip", ggui::filter::GEOMETRY, true);
				if (_gameview.s_filter_hint)				ggui::filter::toggle_by_name("hintskip", ggui::filter::GEOMETRY, true);
				if (_gameview.s_filter_portals)				ggui::filter::toggle_by_name("portals", ggui::filter::GEOMETRY, true);
			}

			// entity filters
			{
				if (_gameview.s_filter_lights)				ggui::filter::toggle_by_name("lights", ggui::filter::ENTITY, true);
				if (_gameview.s_filter_reflectionprobes)	ggui::filter::toggle_by_name("reflectionprobes", ggui::filter::ENTITY, true);
				if (_gameview.s_filter_aispawners)			ggui::filter::toggle_by_name("aispawners", ggui::filter::ENTITY, true);
				if (_gameview.s_filter_pathnodes)			ggui::filter::toggle_by_name("pathnodes", ggui::filter::ENTITY, true);
				if (_gameview.s_filter_actionnodes)			ggui::filter::toggle_by_name("actionnodes", ggui::filter::ENTITY, true);
				if (_gameview.s_filter_info)				ggui::filter::toggle_by_name("info", ggui::filter::ENTITY, true);
				if (_gameview.s_filter_mpentities)			ggui::filter::toggle_by_name("mp_entities", ggui::filter::ENTITY, true);
				if (_gameview.s_filter_script_origins)		ggui::filter::toggle_by_name("script_origins", ggui::filter::ENTITY, true);
			}
			
			// trigger filters
			{
				if (_gameview.s_filter_trigger)				ggui::filter::toggle_by_name("triggers", ggui::filter::TRIGGER, true);
				if (_gameview.s_filter_bcs)					ggui::filter::toggle_by_name("bcs", ggui::filter::TRIGGER, true);
				if (_gameview.s_filter_escort)				ggui::filter::toggle_by_name("escort", ggui::filter::TRIGGER, true);
				if (_gameview.s_filter_unlock)				ggui::filter::toggle_by_name("unlock", ggui::filter::TRIGGER, true);
				if (_gameview.s_filter_vehicle)				ggui::filter::toggle_by_name("vehicle", ggui::filter::TRIGGER, true);
				if (_gameview.s_filter_friendly_respawn)	ggui::filter::toggle_by_name("friendly_respawn", ggui::filter::TRIGGER, true);
				if (_gameview.s_filter_flag)				ggui::filter::toggle_by_name("flag", ggui::filter::TRIGGER, true);
				if (_gameview.s_filter_fog)					ggui::filter::toggle_by_name("fog", ggui::filter::TRIGGER, true);
				if (_gameview.s_filter_auto_adjust)			ggui::filter::toggle_by_name("auto_adjust", ggui::filter::TRIGGER, true);
				if (_gameview.s_filter_spawner)				ggui::filter::toggle_by_name("spawner", ggui::filter::TRIGGER, true);
				if (_gameview.s_filter_stopspawner)			ggui::filter::toggle_by_name("stopspawner", ggui::filter::TRIGGER, true);
				if (_gameview.s_filter_friendlychain)		ggui::filter::toggle_by_name("friendlychain", ggui::filter::TRIGGER, true);
				if (_gameview.s_filter_ambient)				ggui::filter::toggle_by_name("ambient", ggui::filter::TRIGGER, true);
				if (_gameview.s_filter_damage)				ggui::filter::toggle_by_name("damage", ggui::filter::TRIGGER, true);
				if (_gameview.s_filter_trigger_hint)		ggui::filter::toggle_by_name("hint", ggui::filter::TRIGGER, true);
			}

			// other filters
			{
				if (_gameview.s_filter_opt_ladder)		ggui::filter::toggle_by_name("ladder", ggui::filter::OTHER, true);
			}

			memset(&_gameview, 0, sizeof(gameview_helper_s));
		}
		// toggle on
		else
		{
			dvars::set_bool(dvars::radiant_gameview, true);
			
			if (!_gameview.enabled)
			{
				_gameview.s_xyflags = game::g_qeglobals->d_savedinfo.d_xyShowFlags;
				_gameview.s_model_origin = dvars::r_draw_model_origin->current.enabled;
				_gameview.enabled = true;
			}

			game::g_qeglobals->d_savedinfo.d_xyShowFlags |= (2|4); // add angles and connection lines flags to disable rendering
			dvars::set_bool(dvars::r_draw_model_origin, false);

			// geometry filters
			{
				_gameview.s_filter_mantle			= ggui::filter::toggle_by_name("mantle", ggui::filter::GEOMETRY, false);
				_gameview.s_filter_nodraw			= ggui::filter::toggle_by_name("nodraw_notsolid", ggui::filter::GEOMETRY, false);
				_gameview.s_filter_shadowcaster		= ggui::filter::toggle_by_name("shadowcaster", ggui::filter::GEOMETRY, false);
				_gameview.s_filter_traverse			= ggui::filter::toggle_by_name("traverse", ggui::filter::GEOMETRY, false);
				_gameview.s_filter_foliage			= ggui::filter::toggle_by_name("foliage", ggui::filter::GEOMETRY, false);
				_gameview.s_filter_weaponclip		= ggui::filter::toggle_by_name("weaponclip", ggui::filter::GEOMETRY, false);
				_gameview.s_filter_goal				= ggui::filter::toggle_by_name("goalvolumes", ggui::filter::GEOMETRY, false);
				_gameview.s_filter_lightgrid		= ggui::filter::toggle_by_name("lightgrid", ggui::filter::GEOMETRY, false);
				_gameview.s_filter_caulk			= ggui::filter::toggle_by_name("caulk", ggui::filter::GEOMETRY, false);
				_gameview.s_filter_nodrawdecal		= ggui::filter::toggle_by_name("nodrawdecal", ggui::filter::GEOMETRY, false);
				_gameview.s_filter_clip				= ggui::filter::toggle_by_name("clip", ggui::filter::GEOMETRY, false);
				_gameview.s_filter_hint				= ggui::filter::toggle_by_name("hintskip", ggui::filter::GEOMETRY, false);
				_gameview.s_filter_portals			= ggui::filter::toggle_by_name("portals", ggui::filter::GEOMETRY, false);
			}
			
			// entity filters
			{
				_gameview.s_filter_lights			= ggui::filter::toggle_by_name("lights", ggui::filter::ENTITY, false);
				_gameview.s_filter_reflectionprobes = ggui::filter::toggle_by_name("reflectionprobes", ggui::filter::ENTITY, false);
				_gameview.s_filter_aispawners		= ggui::filter::toggle_by_name("aispawners", ggui::filter::ENTITY, false);
				_gameview.s_filter_pathnodes		= ggui::filter::toggle_by_name("pathnodes", ggui::filter::ENTITY, false);
				_gameview.s_filter_actionnodes		= ggui::filter::toggle_by_name("actionnodes", ggui::filter::ENTITY, false);
				_gameview.s_filter_info				= ggui::filter::toggle_by_name("info", ggui::filter::ENTITY, false);
				_gameview.s_filter_mpentities		= ggui::filter::toggle_by_name("mp_entities", ggui::filter::ENTITY, false);
				_gameview.s_filter_script_origins	= ggui::filter::toggle_by_name("script_origins", ggui::filter::ENTITY, false);
			}
			
			// trigger filters
			{
				_gameview.s_filter_trigger			= ggui::filter::toggle_by_name("triggers", ggui::filter::TRIGGER, false);
				_gameview.s_filter_bcs				= ggui::filter::toggle_by_name("bcs", ggui::filter::TRIGGER, false);
				_gameview.s_filter_escort			= ggui::filter::toggle_by_name("escort", ggui::filter::TRIGGER, false);
				_gameview.s_filter_unlock			= ggui::filter::toggle_by_name("unlock", ggui::filter::TRIGGER, false);
				_gameview.s_filter_vehicle			= ggui::filter::toggle_by_name("vehicle", ggui::filter::TRIGGER, false);
				_gameview.s_filter_friendly_respawn = ggui::filter::toggle_by_name("friendly_respawn", ggui::filter::TRIGGER, false);
				_gameview.s_filter_flag				= ggui::filter::toggle_by_name("flag", ggui::filter::TRIGGER, false);
				_gameview.s_filter_fog				= ggui::filter::toggle_by_name("fog", ggui::filter::TRIGGER, false);
				_gameview.s_filter_auto_adjust		= ggui::filter::toggle_by_name("auto_adjust", ggui::filter::TRIGGER, false);
				_gameview.s_filter_spawner			= ggui::filter::toggle_by_name("spawner", ggui::filter::TRIGGER, false);
				_gameview.s_filter_stopspawner		= ggui::filter::toggle_by_name("stopspawner", ggui::filter::TRIGGER, false);
				_gameview.s_filter_friendlychain	= ggui::filter::toggle_by_name("friendlychain", ggui::filter::TRIGGER, false);
				_gameview.s_filter_ambient			= ggui::filter::toggle_by_name("ambient", ggui::filter::TRIGGER, false);
				_gameview.s_filter_damage			= ggui::filter::toggle_by_name("damage", ggui::filter::TRIGGER, false);
				_gameview.s_filter_trigger_hint		= ggui::filter::toggle_by_name("hint", ggui::filter::TRIGGER, false);
			}

			// other filters
			{
				_gameview.s_filter_opt_ladder = ggui::filter::toggle_by_name("ladder", ggui::filter::OTHER, false);
			}
		}
	}

	bool should_render_origin()
	{
		if (dvars::r_draw_model_origin && !dvars::r_draw_model_origin->current.enabled)
		{
			return false;
		}

		return true;
	}

	void __declspec(naked) render_origin_boxes_stub()
	{
		//const static uint32_t draw_origin_func = 0x478E30;
		const static uint32_t continue_pt = 0x478E38;
		__asm
		{
			pushad;
			call    should_render_origin;
			test    al, al;
			popad;
			
			je      SKIP;
			mov     eax, 0x10B0;
			jmp		continue_pt;

		SKIP:
			pop     ebp;
			retn;
		}
	}

	void renderer::register_dvars()
	{
		dvars::r_draw_model_origin = dvars::register_bool(
			/* name		*/ "r_draw_model_origin",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "render model origins");

		dvars::radiant_gameview = dvars::register_bool(
			/* name		*/ "radiant_gameview",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none,
			/* desc		*/ "hides everything thats not visible in-game");
	}
	
	renderer::renderer()
	{
		// set default value for r_vsync to false
		utils::hook::set<BYTE>(0x51FB1A + 1, 0x0);

		utils::hook(0x478E33, render_origin_boxes_stub, HOOK_JUMP).install()->quick();

		command::register_command_with_hotkey("xo_gameview"s, [](auto)
			{
				components::renderer::game_view(!dvars::radiant_gameview->current.enabled);
			});
	}

	renderer::~renderer()
	{ }
}
