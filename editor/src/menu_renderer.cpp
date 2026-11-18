#include "std_include.hpp"

// windows.h macros
#undef min
#undef max

#include "host.hpp"
#include "ui.hpp"
#include "ext.hpp"
#include "menu_ext.hpp"

#include <codol_zone/zone.hpp>
#include <codol_zone/assets.hpp>
#include <d3dcommon.h>

// The Renderer menu: what is drawn and how.
//
//   through the renderer ( codol_view frame_options )
//     wireframe, fullbright ( the game's own "unlit" technique ), draw water, texture filter, texture resolution
//     ( mip bias ), specular and normal maps off ( the game's stand-in textures ), cubic clipping, sun shadows
//   a post process over the picture of the renderer ( one pixel shader, compiled at run time with the system's
//   d3dcompiler_47.dll; without it these modes say so and the view stays as it is )
//     fake lighting by normal / by view, the depth of the frame, the light preview of light entities and the
//     film values of a .vision file. Positions and normals come from the depth pre-pass of the game's shaders.
//   lines in the 3D view
//     patches as wires, the angles of entities, the origins of models, a tint over what is selected
//   the 2D views
//     names, grid blocks, coordinates, the cross under the mouse

namespace editor::render_modes
{
	namespace
	{
		namespace cz = codol::zone;

		constexpr int MAX_LIGHTS = 8;

		state g_state;

		struct film_values
		{
			bool enable = true;
			float contrast = 1.4f;
			float brightness = 0.0f;
			float desaturation = 0.2f;
			float desaturation_dark = 0.2f;		// of the dark parts of the picture ( newer vision files; else as desaturation )
			bool invert = false;
			float light_tint[3] = { 1.1f, 1.05f, 0.85f };
			float medium_tint[3] = { 1.0f, 1.0f, 1.0f };
			float dark_tint[3] = { 0.7f, 0.85f, 1.0f };
		};

		film_values g_film;
		bool g_show_film = false;
		std::string g_vision_filter;

		// vision files of the game: name -> text, read from the zones once ( a thread; the list fills when it is done )
		std::map<std::string, std::string> g_visions;
		std::atomic_bool g_visions_busy = false;
		bool g_visions_read = false;
		std::string g_visions_for;

		// light entities that stay in the preview after they are deselected
		std::vector<object_id> g_preview_lights;

		// ---- settings <-> state --------------------------------------------------------------------------------
		struct binding
		{
			const char* name;
			int kind;				// 0 bool, 1 int, 2 float ( saved as text ), 3 text
			void* value;
			const char* description;
			int lo = 0, hi = 1, initial = 0;
			bool started = false;
			std::string seen_setting, seen_live;
		};

		std::vector<binding> g_bindings;

		std::string live_text(const binding& b)
		{
			char text[64];

			switch (b.kind)
			{
			case 0: return *static_cast<bool*>(b.value) ? "1" : "0";
			case 1: return std::to_string(*static_cast<int*>(b.value));
			case 2: std::snprintf(text, sizeof(text), "%g", *static_cast<float*>(b.value)); return text;
			default: return *static_cast<std::string*>(b.value);
			}
		}

		std::string setting_text(const binding& b)
		{
			auto* dvar = dvars::find(b.name);

			switch (dvar->type)
			{
			case game::dvar_type::boolean: return dvar->current.enabled ? "1" : "0";
			case game::dvar_type::integer: return std::to_string(dvar->current.integer);
			default: return dvar->current.string ? dvar->current.string : "";
			}
		}

		void to_live(binding& b, const std::string& text)
		{
			switch (b.kind)
			{
			case 0: *static_cast<bool*>(b.value) = text == "1"; break;
			case 1: *static_cast<int*>(b.value) = std::clamp(std::atoi(text.c_str()), b.lo, b.hi); break;
			case 2: *static_cast<float*>(b.value) = static_cast<float>(std::atof(text.c_str())); break;
			default: *static_cast<std::string*>(b.value) = text; break;
			}
		}

		void to_setting(const binding& b, const std::string& text)
		{
			auto* dvar = dvars::find(b.name);

			switch (dvar->type)
			{
			case game::dvar_type::boolean: dvars::set_bool(dvar, text == "1"); break;
			case game::dvar_type::integer: dvars::set_int(dvar, std::atoi(text.c_str())); break;
			default: dvars::set_string(dvar, text.c_str()); break;
			}
		}

		void bind(const char* name, bool& value, const char* description)
		{
			dvars::register_bool(name, value ? 1 : 0, game::dvar_flags::saved, description);
			g_bindings.push_back({ name, 0, &value, description });
		}

		template <typename T>
		void bind_int(const char* name, T& value, const int lo, const int hi, const char* description)
		{
			static_assert(sizeof(T) == sizeof(int));
			dvars::register_int(name, static_cast<int>(value), lo, hi, game::dvar_flags::saved, description);
			g_bindings.push_back({ name, 1, &value, description, lo, hi });
		}

		void bind(const char* name, float& value, const char* description)
		{
			char text[32];
			std::snprintf(text, sizeof(text), "%g", value);
			dvars::register_string(name, text, game::dvar_flags::saved, description);
			g_bindings.push_back({ name, 2, &value, description });
		}

		void bind(const char* name, std::string& value, const char* description)
		{
			dvars::register_string(name, value.c_str(), game::dvar_flags::saved, description);
			g_bindings.push_back({ name, 3, &value, description });
		}

		// whichever of the two changed since the last frame is copied to the other
		void sync_settings()
		{
			for (auto& b : g_bindings)
			{
				const std::string setting = setting_text(b);

				if (!b.started || setting != b.seen_setting)
				{
					b.started = true;
					to_live(b, setting);
				}
				else if (const std::string live = live_text(b); live != b.seen_live)
				{
					to_setting(b, live);
				}

				b.seen_setting = setting_text(b);
				b.seen_live = live_text(b);
			}
		}

		// ---- vision files --------------------------------------------------------------------------------------

		// The film values of a vision file. The zones of 1.1.1.13 hold the files with the number of each setting in
		// place of its name ( MEASURED on the shipped files: mp_crash.vision has the values of CoD4's mp_crash under
		// these numbers ):
		//   520 film enable   521 brightness   522 contrast   523 desaturation   524 desaturation of the dark parts
		//   525 invert        526 light tint   527 medium tint   528 dark tint
		// ( 511 .. 519 are the glow values, which are not shown ). A file written by hand has the names of the
		// settings: r_filmContrast "1.4" and the like.
		bool apply_vision(const std::string& text)
		{
			std::istringstream in(text);
			std::string line;
			film_values film;
			bool any = false, dark = false;
			for (float& v : film.medium_tint) { v = 1.0f; }

			while (std::getline(in, line))
			{
				const auto open = line.find('"');
				const auto close = open == std::string::npos ? open : line.find('"', open + 1);
				if (close == std::string::npos) { continue; }

				std::string key = line.substr(0, open);
				for (char& c : key) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
				while (!key.empty() && (key.back() == ' ' || key.back() == '\t')) { key.pop_back(); }
				while (!key.empty() && (key.front() == ' ' || key.front() == '\t')) { key.erase(key.begin()); }
				const std::string value = line.substr(open + 1, close - open - 1);

				float v[3] = { 1.0f, 1.0f, 1.0f };
				const int n = std::sscanf(value.c_str(), "%f %f %f", &v[0], &v[1], &v[2]);
				if (n < 1 || key.empty()) { continue; }

				const bool numbered = std::isdigit(static_cast<unsigned char>(key[0])) != 0;
				const int id = numbered ? std::atoi(key.c_str()) : 0;
				const auto named = [&](const char* word) { return !numbered && key.find("film") != std::string::npos && key.find(word) != std::string::npos; };

				if (id == 520 || named("enable")) { film.enable = v[0] != 0.0f; any = true; }
				else if (id == 521 || named("brightness")) { film.brightness = v[0]; any = true; }
				else if (id == 522 || named("contrast")) { film.contrast = v[0]; any = true; }
				else if (id == 524 || named("desaturationdark")) { film.desaturation_dark = v[0]; dark = true; any = true; }
				else if (id == 523 || named("desaturation")) { film.desaturation = v[0]; any = true; }
				else if (id == 525 || named("invert")) { film.invert = v[0] != 0.0f; any = true; }
				else if ((id == 526 || named("lighttint")) && n == 3) { std::copy(v, v + 3, film.light_tint); any = true; }
				else if ((id == 527 || named("mediumtint")) && n == 3) { std::copy(v, v + 3, film.medium_tint); any = true; }
				else if ((id == 528 || named("darktint")) && n == 3) { std::copy(v, v + 3, film.dark_tint); any = true; }
			}

			if (!dark) { film.desaturation_dark = film.desaturation; }
			if (any) { g_film = film; }
			return any;
		}

		void read_visions()
		{
			if (g_visions_busy.exchange(true))
			{
				return;
			}

			const auto install = std::filesystem::u8path(components::codol_game::install_path());
			std::vector<std::string> zones;
			if (const auto& map = get_scene().map_name; !map.empty()) { zones.push_back(map); }
			zones.insert(zones.end(), { "common_mp", "common_gametype_mp", "codol_common", "code_post_gfx_mp" });
			g_visions_for = get_scene().map_name;

			std::thread([install, zones]
			{
				std::map<std::string, std::string> found;
				std::string note;
				cz::ClientKey key;

				if (const auto status = key.load_from_exe(install / "codoMP_client_shipRetail.exe"); !status)
				{
					note = status.message;
				}
				else
				{
					for (const auto& name : zones)
					{
						std::error_code ec;
						std::filesystem::path file;

						for (const auto& dir : { install / "client_shipRetail" / "patch", install / "client_shipRetail" / "chinese" })
						{
							if (std::filesystem::is_regular_file(dir / (name + ".ff"), ec)) { file = dir / (name + ".ff"); break; }
						}

						cz::Zone zone;
						if (file.empty() || !zone.open(file, key)) { continue; }

						for (const auto& entry : zone.assets())
						{
							if (entry.type != codol::c1::ASSET_TYPE_RAWFILE) { continue; }

							std::string asset(zone.asset_name(entry.type, entry.header));
							for (char& c : asset) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
							if (asset.size() < 8 || asset.substr(asset.size() - 7) != ".vision") { continue; }

							cz::RawFileView view;
							std::vector<std::uint8_t> bytes;

							if (cz::read_rawfile(zone, entry.header, view) && cz::rawfile_contents(view, bytes))
							{
								const auto slash = asset.find_last_of('/');
								found.emplace(asset.substr(slash == std::string::npos ? 0 : slash + 1, asset.size() - 7 - (slash == std::string::npos ? 0 : slash + 1)), std::string(bytes.begin(), bytes.end()));
							}
						}
					}
				}

				components::codol_game::post_to_gui([found = std::move(found), note]() mutable
				{
					g_visions = std::move(found);
					g_visions_read = true;
					g_visions_busy = false;
					game::printf_to_console("[renderer] %d vision files in the zones%s%s\n", static_cast<int>(g_visions.size()), note.empty() ? "" : ": ", note.c_str());

					// the vision that was picked before the list was there
					if (const auto it = g_visions.find(g_state.vision); it != g_visions.end()) { apply_vision(it->second); }
				});
			}).detach();
		}

		void pick_vision(const std::string& name)
		{
			if (const auto it = g_visions.find(name); it != g_visions.end())
			{
				if (!apply_vision(it->second))
				{
					game::printf_to_console("^1[renderer] vision %s has no film values\n", name.c_str());
					return;
				}

				g_state.vision = name;
				g_state.filmtweaks = true;
				game::printf_to_console("[renderer] vision %s: contrast %g brightness %g desaturation %g  light %g %g %g  dark %g %g %g%s\n", name.c_str(), g_film.contrast, g_film.brightness,
					g_film.desaturation, g_film.light_tint[0], g_film.light_tint[1], g_film.light_tint[2], g_film.dark_tint[0], g_film.dark_tint[1], g_film.dark_tint[2], g_film.enable ? "" : "  ( the file turns film off )");
			}
			else if (!name.empty())
			{
				// the list is not there yet: the vision is applied when it is
				if (!g_visions_read) { g_state.vision = name; read_visions(); }
				else { game::printf_to_console("^1[renderer] no vision file %s in the zones\n", name.c_str()); }
			}
		}

		// ---- the post process ----------------------------------------------------------------------------------

		const char* const POST_SHADER = R"(
sampler2D frame_s : register(s0);
sampler2D depth_s : register(s1);

float4 mode : register(c0);			// x: 1 fake light by normal, 2 by view, 3 depth; y: number of lights; z: film on; w: film invert
float4 view_tan : register(c1);		// x, y: tangent of half the field of view; z, w: 1 / width, 1 / height
float4 cam_pos : register(c2);
float4 cam_fwd : register(c3);
float4 cam_right : register(c4);
float4 cam_up : register(c5);
float4 film : register(c6);			// contrast, brightness, desaturation, desaturation of the dark parts
float4 film_light : register(c7);
float4 film_dark : register(c8);
float4 light_pos[8] : register(c9);		// xyz, radius
float4 light_col[8] : register(c17);	// rgb, 1 for a spot
float4 light_dir[8] : register(c25);	// xyz the way a spot looks, cosine of half the outer cone
float4 light_cone[8] : register(c33);	// cosine of half the inner cone
float4 film_medium : register(c41);

float depth_at(float2 uv)
{
	return tex2Dlod(depth_s, float4(uv, 0.0, 0.0)).r;
}

float3 world_at(float2 uv, float depth)
{
	float2 ndc = float2(uv.x * 2.0 - 1.0, 1.0 - uv.y * 2.0);
	return cam_pos.xyz + cam_right.xyz * (ndc.x * view_tan.x * depth) + cam_up.xyz * (ndc.y * view_tan.y * depth) + cam_fwd.xyz * depth;
}

float4 main(float2 uv : TEXCOORD0) : COLOR0
{
	float4 color = tex2Dlod(frame_s, float4(uv, 0.0, 0.0));
	float depth = depth_at(uv);
	float3 result = color.rgb;

	// the surface under the pixel, and its normal from the pixels next to it: of the two neighbours on a line the
	// one closer in depth is on the same surface
	float2 right = float2(view_tan.z, 0.0), down = float2(0.0, view_tan.w);
	float d_r = depth_at(uv + right), d_l = depth_at(uv - right), d_d = depth_at(uv + down), d_u = depth_at(uv - down);
	float3 p = world_at(uv, depth);
	float3 dx = abs(d_r - depth) < abs(d_l - depth) ? world_at(uv + right, d_r) - p : p - world_at(uv - right, d_l);
	float3 dy = abs(d_d - depth) < abs(d_u - depth) ? world_at(uv + down, d_d) - p : p - world_at(uv - down, d_u);
	float3 n = normalize(cross(dx, dy) + float3(0.0, 0.0, 0.000001));
	float3 to_eye = normalize(cam_pos.xyz - p);
	n = dot(n, to_eye) < 0.0 ? -n : n;

	float covered = depth > 0.001 ? 1.0 : 0.0;
	float3 shaded = result;

	if (mode.x > 2.5)
	{
		shaded = float3(saturate(depth / 4096.0), frac(depth / 256.0), 1.0 - saturate(depth / 16384.0));
	}
	else if (mode.x > 1.5)
	{
		shaded = color.rgb * (0.25 + 0.75 * saturate(dot(n, to_eye)));
	}
	else if (mode.x > 0.5)
	{
		// a light from above and a little from two sides: every orientation has a shade of its own
		shaded = color.rgb * saturate(0.56 + 0.30 * n.z + 0.12 * n.x + 0.07 * n.y) * 1.25;
	}

	float3 lit = float3(0.0, 0.0, 0.0);
	for (int i = 0; i < 8; i++)
	{
		float3 to_light = light_pos[i].xyz - p;
		float distance = length(to_light);
		float3 l = to_light / max(distance, 0.001);
		float reach = saturate(1.0 - distance / max(light_pos[i].w, 1.0));
		float cone = light_col[i].w > 0.5 ? smoothstep(light_dir[i].w, max(light_cone[i].x, light_dir[i].w + 0.001), dot(-l, light_dir[i].xyz)) : 1.0;
		// falls off in a line to the radius, as the game's point lights do
		lit += (i < mode.y ? 1.0 : 0.0) * light_col[i].rgb * (reach * cone * (0.2 + 0.8 * saturate(dot(n, l))));
	}

	// the picture is what the light falls on: brighter where it is bright already, never nothing where it is dark
	shaded += (color.rgb * 2.0 + 0.25) * lit;
	result = lerp(result, shaded, covered);

	if (mode.z > 0.5)
	{
		float luminance = dot(result, float3(0.299, 0.587, 0.114));
		float tone = saturate(luminance);
		result = lerp(result, float3(luminance, luminance, luminance), lerp(film.w, film.z, tone));
		// dark, medium and light tint over the tones of the picture
		result *= tone < 0.5 ? lerp(film_dark.rgb, film_medium.rgb, tone * 2.0) : lerp(film_medium.rgb, film_light.rgb, tone * 2.0 - 1.0);
		result = (result - 0.5) * film.x + 0.5 + film.y;
		result = mode.w > 0.5 ? 1.0 - result : result;
	}

	return float4(saturate(result), 1.0);
}
)";

		using compile_fn = HRESULT(WINAPI*)(LPCVOID, SIZE_T, LPCSTR, const void*, void*, LPCSTR, LPCSTR, UINT, UINT, ID3DBlob**, ID3DBlob**);

		struct post_state
		{
			bool tried = false;
			std::string problem;				// why there is no post process
			IDirect3DPixelShader9* shader = nullptr;
			IDirect3DTexture9* target = nullptr;
			IDirect3DSurface9* surface = nullptr;
			IDirect3DStateBlock9* saved = nullptr;
			int width = 0, height = 0;
			bool said = false;
		};

		post_state g_post;

		void release_post_targets()
		{
			if (g_post.surface) { g_post.surface->Release(); g_post.surface = nullptr; }
			if (g_post.target) { g_post.target->Release(); g_post.target = nullptr; }
			if (g_post.saved) { g_post.saved->Release(); g_post.saved = nullptr; }
			g_post.width = g_post.height = 0;
		}

		bool ensure_shader(IDirect3DDevice9* device)
		{
			if (g_post.tried)
			{
				return g_post.shader != nullptr;
			}

			g_post.tried = true;

			// the compiler of the system ( Windows 8.1 and later have it ); nothing is linked against it
			HMODULE module_ = LoadLibraryW(L"d3dcompiler_47.dll");
			const auto compile = module_ ? reinterpret_cast<compile_fn>(GetProcAddress(module_, "D3DCompile")) : nullptr;

			if (!compile)
			{
				g_post.problem = "d3dcompiler_47.dll is not on this system: fake lighting, the light preview and film values are not available";
				return false;
			}

			ID3DBlob* code = nullptr;
			ID3DBlob* errors = nullptr;
			const HRESULT hr = compile(POST_SHADER, std::strlen(POST_SHADER), "codol_editor_post", nullptr, nullptr, "main", "ps_3_0", 0, 0, &code, &errors);

			if (FAILED(hr) || !code)
			{
				g_post.problem = "the post process shader does not compile: " + (errors ? std::string(static_cast<const char*>(errors->GetBufferPointer()), errors->GetBufferSize()) : std::string("no message"));
				if (errors) { errors->Release(); }
				return false;
			}

			if (errors) { errors->Release(); }

			if (FAILED(device->CreatePixelShader(static_cast<const DWORD*>(code->GetBufferPointer()), &g_post.shader)))
			{
				g_post.problem = "the device does not take the post process shader ( ps_3_0 )";
				g_post.shader = nullptr;
			}

			code->Release();
			return g_post.shader != nullptr;
		}

		// the lights of the preview: selected light entities and the ones of the list, the nearest first
		struct preview_light
		{
			ui::light_info info;
			float distance = 0.0f;
		};

		std::vector<preview_light> collect_lights()
		{
			std::vector<preview_light> out;
			if (!g_state.light_preview) { return out; }

			auto& s = get_scene();
			const auto& doc = s.doc;
			std::vector<object_id> ids = g_preview_lights;

			for (const object_id id : doc.selection())
			{
				if (std::find(ids.begin(), ids.end(), id) == ids.end()) { ids.push_back(id); }
			}

			for (const object_id id : ids)
			{
				const auto* e = doc.find_entity(id);
				if (!e || !ui::is_light_class(e->classname()) || !doc.is_visible(id)) { continue; }

				preview_light light;
				light.info = ui::read_light(doc, *e);
				const float dx = light.info.origin.x - s.cam.origin.x, dy = light.info.origin.y - s.cam.origin.y, dz = light.info.origin.z - s.cam.origin.z;
				light.distance = std::sqrt(dx * dx + dy * dy + dz * dz);
				out.push_back(light);
			}

			std::sort(out.begin(), out.end(), [](const preview_light& a, const preview_light& b) { return a.distance < b.distance; });
			if (out.size() > MAX_LIGHTS) { out.resize(MAX_LIGHTS); }
			return out;
		}

		// ---- lines of the 3D view ------------------------------------------------------------------------------
		std::vector<line_vertex> g_lines, g_tint;
		std::uint64_t g_lines_statics = ~0ull, g_lines_selected = ~0ull, g_lines_key = ~0ull;

		void rebuild_lines()
		{
			auto& s = get_scene();
			auto& doc = s.doc;
			g_lines.clear();
			g_tint.clear();

			// patches as wires: every edge of the control mesh
			if (g_state.show_patches == patches::textured_wire)
			{
				const DWORD color = colors::d3d(colors::patch);

				for (const auto& object : s.statics.objects)
				{
					if (!object.patch || doc.is_selected(object.id)) { continue; }

					for (std::uint32_t i = 0; i < object.count; i++)
					{
						const auto& p = s.statics.points[object.first + i];
						g_lines.push_back({ p.x, p.y, p.z, color });
					}
				}
			}

			const auto arrow = [&](const cached_entity& e, const DWORD color)
			{
				// where the entity looks ( its yaw ), from its origin
				const float yaw = e.yaw * 0.0174532925f, c = std::cos(yaw), sn = std::sin(yaw);
				const float reach = std::clamp(std::max(e.maxs[0] - e.mins[0], e.maxs[1] - e.mins[1]) * 0.9f, 20.0f, 64.0f);
				const float o[3] = { e.origin[0], e.origin[1], e.origin[2] };
				const float tip[3] = { o[0] + c * reach, o[1] + sn * reach, o[2] };
				g_lines.push_back({ o[0], o[1], o[2], color });
				g_lines.push_back({ tip[0], tip[1], tip[2], color });

				for (const float side : { -1.0f, 1.0f })
				{
					g_lines.push_back({ tip[0], tip[1], tip[2], color });
					g_lines.push_back({ tip[0] - c * reach * 0.25f - sn * side * reach * 0.15f, tip[1] - sn * reach * 0.25f + c * side * reach * 0.15f, tip[2], color });
				}
			};

			const auto cross = [&](const cached_entity& e)
			{
				constexpr float reach = 10.0f;
				const float o[3] = { e.origin[0], e.origin[1], e.origin[2] };
				g_lines.push_back({ o[0] - reach, o[1], o[2], 0xFFFF4040 });
				g_lines.push_back({ o[0] + reach, o[1], o[2], 0xFFFF4040 });
				g_lines.push_back({ o[0], o[1] - reach, o[2], 0xFF40FF40 });
				g_lines.push_back({ o[0], o[1] + reach, o[2], 0xFF40FF40 });
				g_lines.push_back({ o[0], o[1], o[2] - reach, 0xFF4080FF });
				g_lines.push_back({ o[0], o[1], o[2] + reach, 0xFF4080FF });
			};

			for (const auto* list : { &s.statics.entities, &s.selected.entities })
			{
				for (const auto& e : *list)
				{
					if (list == &s.statics.entities && doc.is_selected(e.id)) { continue; }
					if (g_state.show_angles) { arrow(e, to_d3d_color(e.color)); }
					if (g_state.model_origins && e.has_model) { cross(e); }
				}
			}

			// a tint over what is selected: the faces of brushes, the triangles of models
			if (g_state.tint_selected)
			{
				const DWORD color = (colors::d3d(colors::selection) & 0x00FFFFFFu) | 0x48000000u;
				std::size_t budget = 3 * 60000;

				for (const object_id id : doc.selection())
				{
					if (const auto* geo = doc.geometry(id))
					{
						for (const auto& loop : geo->face_vertices)
						{
							for (std::size_t i = 1; i + 1 < loop.size() && budget >= 3; i++)
							{
								for (const std::size_t k : { static_cast<std::size_t>(0), i, i + 1 })
								{
									const auto& p = geo->vertices[static_cast<std::size_t>(loop[k])];
									g_tint.push_back({ static_cast<float>(p.x), static_cast<float>(p.y), static_cast<float>(p.z), color });
								}

								budget -= 3;
							}
						}
					}
					else if (doc.kind(id) == edit::object_kind::patch)
					{
						edit::mesh mesh;
						if (doc.build_mesh(id, mesh, 2))
						{
							for (std::size_t i = 0; i + 2 < mesh.indices.size() && budget >= 3; i += 3)
							{
								for (int k = 0; k < 3; k++)
								{
									const auto& v = mesh.vertices[mesh.indices[i + static_cast<std::size_t>(k)]];
									g_tint.push_back({ v.position[0], v.position[1], v.position[2], color });
								}

								budget -= 3;
							}
						}
					}
					else
					{
						std::vector<view::object_ref> refs;
						std::vector<float3> triangles;
						s.refs_of(id, refs);

						for (const auto& ref : refs)
						{
							triangles.clear();
							if (s.renderer.object_triangles(ref, triangles) && triangles.size() <= budget)
							{
								budget -= triangles.size();
								for (const auto& p : triangles) { g_tint.push_back({ p.x, p.y, p.z, color }); }
							}
						}
					}
				}
			}
		}

		const char* const MODE_NAMES[] = { "game", "fullbright", "fake_normal", "fake_view", "depth", "case" };
		const char* const FILTER_NAMES[] = { "As the materials say", "Point ( no mip maps )", "Bilinear ( no mip maps )", "Trilinear", "Anisotropic" };
		const char* const RESOLUTION_NAMES[] = { "Maximum", "High ( half )", "Normal ( quarter )", "Low ( eighth )" };
		const char* const PATCH_NAMES[] = { "Textured", "Wireframe", "Textured + wireframe" };
		// by entities:: value; the menu lists them in CoD4 radiant's order ( ENTITY_MENU_ORDER )
		const char* const ENTITY_NAMES[] = { "Skinned", "Bounding Box", "Skinned And Boxed", "Wireframe", "Selected Wireframe", "Selected Skinned" };
		const char* const ENTITY_WORDS[] = { "models", "boxes", "both", "wireframe", "selected_wireframe", "selected_models" };
		const char* const ENTITY_TIPS[] = {
			"Models drawn with their materials.",
			"Every entity is its box; no model is drawn.",
			"Models drawn, and the box of every entity around them.",
			"Models drawn as the triangles of their mesh, front and back, in the colours of their materials.",
			"The models of the selected entities as wireframes; every other entity is its box.",
			"The models of the selected entities drawn; every other entity is its box." };
		constexpr int ENTITY_MENU_ORDER[] = { 1, 3, 4, 5, 0, 2 };

		void set_entities(const entities mode)
		{
			g_state.show_entities = mode;
			get_scene().selected_serial++;
			game::printf_to_console("[renderer] entities: %s\n", ENTITY_NAMES[static_cast<int>(mode)]);
		}

		void set_mode(const shading mode)
		{
			g_state.mode = mode;
			game::printf_to_console("[renderer] %s\n", MODE_NAMES[static_cast<int>(mode)]);
		}

		void refresh_view_lines()
		{
			get_scene().selected_serial++;
			g_lines_key = ~0ull;
		}

		void set_patches(const patches mode)
		{
			const bool hidden_before = hide_patch_meshes();
			g_state.show_patches = mode;

			// the meshes of patches are made, or not, when everything is made again
			if (hidden_before != hide_patch_meshes() && get_scene().open) { get_scene().refresh_assets(); }
			refresh_view_lines();
			game::printf_to_console("[renderer] patches: %s\n", PATCH_NAMES[static_cast<int>(mode)]);
		}
	}

	state& get()
	{
		return g_state;
	}

	bool entity_boxes_always()
	{
		// the box stands for the model where the model is not drawn, and in "skinned and boxed"
		return g_state.show_entities != entities::models && g_state.show_entities != entities::wireframe;
	}

	bool hide_patch_meshes()
	{
		return g_state.show_patches == patches::wireframe;
	}

	void device_lost()
	{
		release_post_targets();
	}

	// ---- actions, settings, commands -------------------------------------------------------------------------------

	void register_actions()
	{
		auto& st = g_state;

		bind("ed_r_outline", st.outline_selected, "3D view: outline what is selected");
		bind("ed_r_tint", st.tint_selected, "3D view: tint what is selected");
		bind("ed_r_connections", get_scene().show_connections, "target lines of entities");
		bind("ed_r_angles", st.show_angles, "3D view: an arrow where an entity looks");
		bind("ed_r_origins", st.model_origins, "3D view: a cross at the origin of every model");
		bind("ed_r_grid_names", st.grid_names, "2D views: names of entities");
		bind("ed_r_grid_blocks", ui::ext::get().show_grid_blocks, "2D views: the blocks of 1024 units");
		bind("ed_r_grid_coordinates", st.grid_coordinates, "2D views: coordinates at the edges and of the blocks");
		bind("ed_r_grid_crosshair", st.grid_crosshair, "2D views: a cross under the mouse");
		bind_int("ed_r_mode", st.mode, 0, 5, "3D view: 0 as the game, 1 fullbright, 2 fake light by normal, 3 fake light by view, 4 depth, 5 case textures");
		bind("ed_r_wireframe", st.wireframe, "3D view: wireframe");
		bind("ed_r_water", st.draw_water, "3D view: draw water");
		bind_int("ed_r_filter", st.texture_filter, 0, 4, "texture filter: 0 as the materials say, 1 point, 2 bilinear, 3 trilinear, 4 anisotropic");
		bind_int("ed_r_resolution", st.texture_resolution, 0, 3, "texture resolution: 0 maximum, 1 half, 2 quarter, 3 eighth");
		bind("ed_r_specular", st.specular, "3D view: specular maps");
		bind("ed_r_normalmaps", st.normal_maps, "3D view: normal maps");
		bind_int("ed_r_patches", st.show_patches, 0, 2, "patches: 0 textured, 1 wireframe, 2 textured with wires");
		bind_int("ed_r_entities", st.show_entities, 0, static_cast<int>(entities::count) - 1, "entities: 0 skinned, 1 bounding box, 2 skinned and boxed, 3 wireframe, 4 selected wireframe, 5 selected skinned");
		bind("ed_r_cubic", st.cubic_clip, "3D view: cubic clipping");
		bind("ed_r_cubic_distance", st.cubic_distance, "half the edge of the clipping cube, units");
		bind("ed_r_sun_shadows", st.sun_shadows, "3D view: sun shadows");
		bind("ed_r_light_preview", st.light_preview, "3D view: the light of selected light entities");
		bind("ed_r_light_max", st.light_max_intensity, "light preview at full intensity whatever the entity says");
		bind("ed_r_film", st.filmtweaks, "3D view: the film values ( of a vision file, or typed )");
		bind("ed_r_vision", st.vision, "the vision file whose film values are shown");

		using namespace actions;
		const auto flip = [](bool& value, const char* what, const bool lines = false)
		{
			return [&value, what, lines]
			{
				value = !value;
				if (lines) { refresh_view_lines(); }
				game::printf_to_console("[renderer] %s: %s\n", what, value ? "on" : "off");
			};
		};

		add("render.outline", "Outline selected", "J", flip(st.outline_selected, "outline selected"), views, no_map).toggle([] { return g_state.outline_selected; });
		add("render.tint", "Tint selected", "Shift+J", flip(st.tint_selected, "tint selected", true), views, no_map).toggle([] { return g_state.tint_selected; });
		add("render.connections", "Show connections", "", [] { auto& s = get_scene(); s.show_connections = !s.show_connections; refresh_view_lines(); }, anywhere, no_map)
			.toggle([] { return get_scene().show_connections; });
		add("render.angles", "Show angles", "", flip(st.show_angles, "show angles", true), anywhere, no_map).toggle([] { return g_state.show_angles; });
		add("render.model_origins", "Draw model origins", "", flip(st.model_origins, "model origins", true), anywhere, no_map).toggle([] { return g_state.model_origins; });

		add("render.grid_names", "2D views: show names", "", flip(st.grid_names, "2D names"), anywhere, no_map).toggle([] { return g_state.grid_names; });
		add("render.grid_blocks", "2D views: show grid blocks", "", flip(ui::ext::get().show_grid_blocks, "2D grid blocks"), anywhere, no_map).toggle([] { return ui::ext::get().show_grid_blocks; });
		add("render.grid_coordinates", "2D views: edge and block coordinates", "", flip(st.grid_coordinates, "2D coordinates"), anywhere, no_map).toggle([] { return g_state.grid_coordinates; });
		add("render.grid_crosshair", "2D views: crosshair", "Shift+X", flip(st.grid_crosshair, "2D crosshair"), views, no_map).toggle([] { return g_state.grid_crosshair; });

		add("render.wireframe", "Wireframe", "", flip(st.wireframe, "wireframe"), anywhere, no_map).toggle([] { return g_state.wireframe; });
		add("render.fullbright", "Fullbright", "", [] { set_mode(g_state.mode == shading::fullbright ? shading::game : shading::fullbright); }, anywhere, no_map)
			.toggle([] { return g_state.mode == shading::fullbright; }).help("The colour maps alone: the game's own unlit techniques, no lightmap, sun or specular.");
		add("render.fake_normal", "Normal-based fake lighting", "", [] { set_mode(g_state.mode == shading::fake_normal ? shading::game : shading::fake_normal); }, anywhere, no_map)
			.toggle([] { return g_state.mode == shading::fake_normal; }).help("Fullbright, shaded by the way a surface faces.");
		add("render.fake_view", "View-based fake lighting", "", [] { set_mode(g_state.mode == shading::fake_view ? shading::game : shading::fake_view); }, anywhere, no_map)
			.toggle([] { return g_state.mode == shading::fake_view; }).help("Fullbright, shaded by the angle to the camera: a light on the camera.");
		add("render.depth", "Depth", "", [] { set_mode(g_state.mode == shading::depth ? shading::game : shading::depth); }, anywhere, no_map).toggle([] { return g_state.mode == shading::depth; });
		add("render.case_textures", "Case textures", "", [] { set_mode(g_state.mode == shading::case_textures ? shading::game : shading::case_textures); }, anywhere, no_map)
			.toggle([] { return g_state.mode == shading::case_textures; })
			.help("CoD4 radiant's case textures: every colour map is replaced by a grid of the same size in texels, coloured by that size\n"
				"( blue 64 and less, cyan 128, green 256, grey 512, orange 1024, red 2048 ): the resolution of the textures and how they are\n"
				"stretched over the surfaces. Unlit, as fullbright.");
		add("render.game_lighting", "Lighting of the game", "", [] { set_mode(shading::game); }, anywhere, no_map).toggle([] { return g_state.mode == shading::game; });

		add("render.water", "Draw water", "", flip(st.draw_water, "draw water"), anywhere, no_map).toggle([] { return g_state.draw_water; });
		add("render.specular", "Specular maps", "", flip(st.specular, "specular maps"), anywhere, no_map).toggle([] { return g_state.specular; });
		add("render.normal_maps", "Normal maps ( bump )", "", flip(st.normal_maps, "normal maps"), anywhere, no_map).toggle([] { return g_state.normal_maps; });
		add("render.sun_shadows", "Sun shadows", "", flip(st.sun_shadows, "sun shadows"), anywhere, no_map).toggle([] { return g_state.sun_shadows; });

		add("render.patches_cycle", "Show patches as: next", "Shift+W", [] { set_patches(static_cast<patches>((static_cast<int>(g_state.show_patches) + 1) % 3)); }, views, no_map)
			.toggle([] { return g_state.show_patches != patches::textured; });
		add("render.entities_cycle", "Show entities as: next", "", []
		{
			// in the order of the menu
			int at = 0;
			for (int i = 0; i < static_cast<int>(entities::count); i++) { if (ENTITY_MENU_ORDER[i] == static_cast<int>(g_state.show_entities)) { at = i; } }
			set_entities(static_cast<entities>(ENTITY_MENU_ORDER[(at + 1) % static_cast<int>(entities::count)]));
		}, anywhere, no_map).toggle([] { return g_state.show_entities != entities::models; });

		// one action per mode: a key, a toolbar button, "act render.entities_wireframe"
		for (int i = 0; i < static_cast<int>(entities::count); i++)
		{
			const std::string id = std::string("render.entities_") + ENTITY_WORDS[i];
			add(id, std::string("Show entities as: ") + ENTITY_NAMES[i], "", [i] { set_entities(static_cast<entities>(i)); }, anywhere, no_map)
				.toggle([i] { return static_cast<int>(g_state.show_entities) == i; }).help(ENTITY_TIPS[i]);
		}

		add("render.cubic_clip", "Cubic clipping", "Ctrl+Backslash", flip(st.cubic_clip, "cubic clipping"), anywhere, no_map).toggle([] { return g_state.cubic_clip; });
		add("render.cubic_grow", "Cubic clipping: farther", "Ctrl+]", []
		{
			g_state.cubic_distance = std::min(g_state.cubic_distance * 1.25f, 65536.0f);
			game::printf_to_console("[renderer] cubic clipping: %.0f\n", g_state.cubic_distance);
		}, anywhere, no_map | repeat);
		add("render.cubic_shrink", "Cubic clipping: nearer", "Ctrl+[", []
		{
			g_state.cubic_distance = std::max(g_state.cubic_distance / 1.25f, 256.0f);
			game::printf_to_console("[renderer] cubic clipping: %.0f\n", g_state.cubic_distance);
		}, anywhere, no_map | repeat);

		add("render.light_preview", "Light preview", "", flip(st.light_preview, "light preview"), anywhere, no_map).toggle([] { return g_state.light_preview; })
			.help("The light of the selected light entities ( and of the preview list ) in the 3D view: radius, colour, cone. An approximation, not the bake.");
		add("render.light_preview_add", "Light preview: keep previewing the selected", "", []
		{
			int added = 0;
			for (const object_id id : get_scene().doc.selection())
			{
				const auto* e = get_scene().doc.find_entity(id);
				if (e && ui::is_light_class(e->classname()) && std::find(g_preview_lights.begin(), g_preview_lights.end(), id) == g_preview_lights.end()) { g_preview_lights.push_back(id); added++; }
			}

			game::printf_to_console("[renderer] light preview: %d added, %d in the list\n", added, static_cast<int>(g_preview_lights.size()));
		});
		add("render.light_preview_remove", "Light preview: stop previewing the selected", "", []
		{
			for (const object_id id : get_scene().doc.selection()) { std::erase(g_preview_lights, id); }
			game::printf_to_console("[renderer] light preview: %d in the list\n", static_cast<int>(g_preview_lights.size()));
		});
		add("render.light_preview_clear", "Light preview: clear the list", "", [] { g_preview_lights.clear(); }, anywhere, no_map);
		add("render.light_max", "Light preview at max intensity", "", flip(st.light_max_intensity, "light preview at max intensity"), anywhere, no_map).toggle([] { return g_state.light_max_intensity; });

		add("render.filmtweaks", "Filmtweaks", "", flip(st.filmtweaks, "filmtweaks"), anywhere, no_map).toggle([] { return g_state.filmtweaks; });
		add("render.film_settings", "Filmtweak settings / vision ..", "", [] { g_show_film = !g_show_film; if (g_show_film && !g_visions_read) { read_visions(); } }, anywhere, no_map)
			.toggle([] { return g_show_film; });

		// ---- console
		using args_t = const std::vector<std::string>&;

		// r_mode <game|fullbright|fake_normal|fake_view|depth>
		command::register_command("r_mode", [](args_t args)
		{
			for (int i = 0; i < 6 && args.size() > 1; i++)
			{
				if (args[1] == MODE_NAMES[i]) { set_mode(static_cast<shading>(i)); return; }
			}

			game::printf_to_console("usage: r_mode <game|fullbright|fake_normal|fake_view|depth|case>   now: %s%s%s\n", MODE_NAMES[static_cast<int>(g_state.mode)],
				g_post.problem.empty() ? "" : "   ", g_post.problem.c_str());
		});

		// r_patches <textured|wireframe|both>, r_entities <models|boxes|both>
		command::register_command("r_patches", [](args_t args)
		{
			if (args.size() > 1) { set_patches(args[1] == "wireframe" ? patches::wireframe : (args[1] == "both" ? patches::textured_wire : patches::textured)); }
		});

		command::register_command("r_entities", [](args_t args)
		{
			if (args.size() > 1)
			{
				for (int i = 0; i < static_cast<int>(entities::count); i++)
				{
					if (args[1] == ENTITY_WORDS[i] || args[1] == std::to_string(i)) { set_entities(static_cast<entities>(i)); return; }
				}
				game::printf_to_console("usage: r_entities <models|boxes|both|wireframe|selected_wireframe|selected_models>\n");
				return;
			}

			{
				game::printf_to_console("[renderer] entities: %s\n", ENTITY_NAMES[static_cast<int>(g_state.show_entities)]);
			}
		});

		// r_cubic <distance|off>
		command::register_command("r_cubic", [](args_t args)
		{
			if (args.size() > 1)
			{
				g_state.cubic_clip = args[1] != "off" && args[1] != "0";
				if (const float d = static_cast<float>(std::atof(args[1].c_str())); d >= 64.0f) { g_state.cubic_distance = d; }
			}

			game::printf_to_console("[renderer] cubic clipping: %s, %.0f\n", g_state.cubic_clip ? "on" : "off", g_state.cubic_distance);
		});

		// r_vision <name|off|list [text]|show <name>>: the film values of a vision file of the game
		command::register_command("r_vision", [](args_t args)
		{
			const std::string what = args.size() > 1 ? args[1] : "list";

			if (what == "off")
			{
				g_state.filmtweaks = false;
			}
			else if (what == "show")
			{
				// the text of a vision file
				const auto it = g_visions.find(args.size() > 2 ? args[2] : std::string());
				game::printf_to_console("%s\n", it != g_visions.end() ? it->second.c_str() : "^1no such vision file ( r_vision list )");
			}
			else if (what == "list")
			{
				if (!g_visions_read) { read_visions(); game::printf_to_console("[renderer] reading the vision files of the zones ..\n"); return; }

				int shown = 0;
				for (const auto& [name, text] : g_visions)
				{
					if (args.size() > 2 && name.find(args[2]) == std::string::npos) { continue; }
					if (++shown <= 60) { game::printf_to_console("  %s\n", name.c_str()); }
				}

				game::printf_to_console("%d vision files\n", shown);
			}
			else
			{
				pick_vision(what);
			}
		});

		// r_film [contrast brightness desaturation [lr lg lb dr dg db]]: the film values by hand
		command::register_command("r_film", [](args_t args)
		{
			const auto number = [&](const std::size_t i, const float fallback) { return i < args.size() ? static_cast<float>(std::atof(args[i].c_str())) : fallback; };

			if (args.size() > 1)
			{
				g_film.enable = true;
				g_film.contrast = number(1, g_film.contrast);
				g_film.brightness = number(2, g_film.brightness);
				g_film.desaturation = g_film.desaturation_dark = number(3, g_film.desaturation);
				for (int i = 0; i < 3; i++) { g_film.light_tint[i] = number(4 + static_cast<std::size_t>(i), g_film.light_tint[i]); }
				for (int i = 0; i < 3; i++) { g_film.dark_tint[i] = number(7 + static_cast<std::size_t>(i), g_film.dark_tint[i]); }
				g_state.filmtweaks = true;
			}

			game::printf_to_console("[renderer] film %s: contrast %g brightness %g desaturation %g  light %g %g %g  dark %g %g %g\n", g_state.filmtweaks ? "on" : "off", g_film.contrast, g_film.brightness,
				g_film.desaturation, g_film.light_tint[0], g_film.light_tint[1], g_film.light_tint[2], g_film.dark_tint[0], g_film.dark_tint[1], g_film.dark_tint[2]);
		});

		// r_status: what the 3D view does right now
		command::register_command("r_status", [](args_t)
		{
			const auto lights = collect_lights();
			game::printf_to_console("[renderer] mode %s%s, filter %d, resolution %d, specular %d, normal maps %d, water %d, cubic %s %.0f, patches %s, entities %s, lights previewed %d ( list %d ), film %s ( vision \"%s\" )\n",
				MODE_NAMES[static_cast<int>(g_state.mode)], g_state.wireframe ? " + wireframe" : "", g_state.texture_filter, g_state.texture_resolution, g_state.specular ? 1 : 0,
				g_state.normal_maps ? 1 : 0, g_state.draw_water ? 1 : 0, g_state.cubic_clip ? "on" : "off", g_state.cubic_distance, PATCH_NAMES[static_cast<int>(g_state.show_patches)],
				ENTITY_NAMES[static_cast<int>(g_state.show_entities)], static_cast<int>(lights.size()), static_cast<int>(g_preview_lights.size()), g_state.filmtweaks ? "on" : "off", g_state.vision.c_str());
			game::printf_to_console("[renderer] post process: %s\n", g_post.problem.empty() ? (g_post.shader ? "ready" : "not used yet") : g_post.problem.c_str());
		});
	}

	// ---- menu ----------------------------------------------------------------------------------------------------

	void draw_menu()
	{
		if (!editor::menu::begin("Renderer"))
		{
			return;
		}

		if (editor::menu::begin("General"))
		{
			actions::menu_item("render.outline");
			actions::menu_item("render.tint");
			actions::menu_item("render.connections");
			actions::menu_item("render.angles");
			actions::menu_item("render.model_origins");
			editor::menu::end();
		}

		if (editor::menu::begin("Grid Window"))
		{
			actions::menu_item("render.grid_names", "Show Names");
			actions::menu_item("render.grid_blocks", "Show Grid Blocks");
			actions::menu_item("render.grid_coordinates", "Draw Edge and Block Coordinates");
			actions::menu_item("render.grid_crosshair", "Draw Crosshair");
			editor::menu::end();
		}

		if (editor::menu::begin("Render Method / Features"))
		{
			actions::menu_item("render.game_lighting");
			actions::menu_item("render.fullbright");
			actions::menu_item("render.fake_normal");
			actions::menu_item("render.fake_view");
			actions::menu_item("render.depth", "Depth ( of the pre-pass )");
			ImGui::Separator();
			actions::menu_item("render.wireframe");
			actions::menu_item("render.case_textures", "Case Textures");
			ImGui::Separator();
			actions::menu_item("render.water");
			actions::menu_item("render.specular");
			actions::menu_item("render.normal_maps");
			ImGui::Separator();
			actions::menu_item("render.filmtweaks");
			actions::menu_item("render.film_settings");

			if (!g_post.problem.empty())
			{
				ImGui::Separator();
				ImGui::TextDisabled("%s", g_post.problem.c_str());
			}

			editor::menu::end();
		}

		if (editor::menu::begin("Texture Filter"))
		{
			for (int i = 0; i < 5; i++)
			{
				if (editor::menu::item(FILTER_NAMES[i], nullptr, g_state.texture_filter == i)) { g_state.texture_filter = i; }
			}

			editor::menu::end();
		}

		if (editor::menu::begin("Texture Resolution"))
		{
			for (int i = 0; i < 4; i++)
			{
				if (editor::menu::item(RESOLUTION_NAMES[i], nullptr, g_state.texture_resolution == i)) { g_state.texture_resolution = i; }
			}

			ImGui::Separator();
			ImGui::TextDisabled("One setting for colour, specular and normal maps:\nthe mip level every material texture is read at.");
			editor::menu::end();
		}

		ImGui::Separator();
		actions::menu_item("texture.reload", "Reload the project's images and materials");
		ImGui::Separator();

		if (editor::menu::begin("Show Patches As"))
		{
			for (int i = 0; i < 3; i++)
			{
				if (editor::menu::item(PATCH_NAMES[i], i == 0 ? editor::menu::key("render.patches_cycle") : nullptr, static_cast<int>(g_state.show_patches) == i)) { set_patches(static_cast<patches>(i)); }
			}

			editor::menu::end();
		}

		if (editor::menu::begin("Show Entities As"))
		{
			for (const int i : ENTITY_MENU_ORDER)
			{
				char id[48];
				std::snprintf(id, sizeof(id), "render.entities_%s", ENTITY_WORDS[i]);
				if (editor::menu::item(ENTITY_NAMES[i], editor::menu::key(id), static_cast<int>(g_state.show_entities) == i))
				{
					set_entities(static_cast<entities>(i));
				}

				if (ImGui::IsItemHovered()) { ImGui::SetTooltip("%s", ENTITY_TIPS[i]); }
			}
			editor::menu::end();
		}

		if (editor::menu::begin("Light / Sun Preview"))
		{
			actions::menu_item("render.light_preview", "Enable Light Preview");
			actions::menu_item("render.sun_shadows", "Enable Sun Shadows");

			const bool stage = get_scene().open && get_scene().stage;
			if (editor::menu::item("Sun Preview: rebuild the stage with the worldspawn's light", nullptr, false, stage)) { command::execute("ed_restage"); }
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			{
				ImGui::SetTooltip("A map of your own is drawn in a stage with the sun, sky and ambient light of its worldspawn keys.\nA map of the game is drawn with its own baked light: there is nothing to preview.");
			}

			ImGui::Separator();
			actions::menu_item("render.light_preview_add", "Start Previewing Selected");
			actions::menu_item("render.light_preview_remove", "Stop Previewing Selected");
			actions::menu_item("render.light_preview_clear", "Clear Preview List");
			ImGui::Separator();
			actions::menu_item("render.light_max", "Preview At Max Intensity");
			ImGui::TextDisabled("%d light( s ) in the list. Selected lights are always previewed;\nthe radius and cone of a selected light are drawn as lines.", static_cast<int>(g_preview_lights.size()));
			editor::menu::end();
		}

		ImGui::Separator();
		actions::menu_item("render.cubic_clip");
		actions::menu_item("render.cubic_grow");
		actions::menu_item("render.cubic_shrink");
		ImGui::SetNextItemWidth(220.0f);
		ImGui::SliderFloat("##cubic_distance", &g_state.cubic_distance, 256.0f, 65536.0f, "cube: %.0f units", ImGuiSliderFlags_Logarithmic);
		editor::menu::end();
	}

	// ---- windows ---------------------------------------------------------------------------------------------------

	void draw_windows()
	{
		sync_settings();

		// the vision of the last session
		static bool first = true;
		if (first && get_scene().open)
		{
			first = false;
			if (g_state.filmtweaks && !g_state.vision.empty()) { read_visions(); }
		}

		if (!g_show_film)
		{
			return;
		}

		// another map has other vision files
		if (g_visions_read && !g_visions_busy && g_visions_for != get_scene().map_name)
		{
			read_visions();
		}

		ImGui::SetNextWindowSize(ImVec2(520.0f, 560.0f), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Filmtweaks##window", &g_show_film, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return;
		}

		ImGui::Checkbox("Show the film values in the 3D view", &g_state.filmtweaks);
		ImGui::Separator();

		ImGui::Checkbox("enabled ( r_filmTweakEnable )", &g_film.enable);
		ImGui::SameLine();
		ImGui::Checkbox("invert", &g_film.invert);
		ImGui::SliderFloat("contrast", &g_film.contrast, 0.0f, 4.0f, "%.2f");
		ImGui::SliderFloat("brightness", &g_film.brightness, -1.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("desaturation", &g_film.desaturation, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("desaturation, dark", &g_film.desaturation_dark, 0.0f, 1.0f, "%.2f");
		ImGui::ColorEdit3("light tint", g_film.light_tint, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		ImGui::ColorEdit3("medium tint", g_film.medium_tint, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		ImGui::ColorEdit3("dark tint", g_film.dark_tint, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

		if (ImGui::Button("Neutral"))
		{
			g_film = {};
			g_film.contrast = 1.0f;
			g_film.desaturation = g_film.desaturation_dark = 0.0f;
			for (int i = 0; i < 3; i++) { g_film.light_tint[i] = g_film.dark_tint[i] = 1.0f; }
			g_state.vision.clear();
		}

		ImGui::SameLine();
		ImGui::TextDisabled(g_state.vision.empty() ? "typed values" : "vision: %s", g_state.vision.c_str());
		ImGui::Separator();

		ImGui::SetNextItemWidth(-140.0f);
		ImGui::InputTextWithHint("##vision_filter", "vision files of the zones: filter", &g_vision_filter);
		ImGui::SameLine();
		if (ImGui::Button("Read again", ImVec2(-1.0f, 0.0f))) { read_visions(); }

		if (g_visions_busy)
		{
			ImGui::TextDisabled("reading the zones ..");
		}
		else if (g_visions.empty())
		{
			ImGui::TextDisabled(g_visions_read ? "The zones of this map hold no vision files." : "");
		}

		if (ImGui::BeginChild("##vision_list", ImVec2(0.0f, 0.0f), true))
		{
			for (const auto& [name, text] : g_visions)
			{
				if (!g_vision_filter.empty() && name.find(g_vision_filter) == std::string::npos) { continue; }

				if (ImGui::Selectable(name.c_str(), name == g_state.vision))
				{
					pick_vision(name);
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("%s", text.substr(0, 900).c_str());
				}
			}
		}

		ImGui::EndChild();
		ImGui::End();
	}

	// ---- the frame ---------------------------------------------------------------------------------------------------

	void apply(view::frame_options& frame)
	{
		const auto& st = g_state;
		const bool post_modes = st.mode == shading::fake_normal || st.mode == shading::fake_view || st.mode == shading::depth;

		frame.wireframe = st.wireframe;
		frame.sun_shadows = st.sun_shadows;
		// the fake lights shade the colour maps
		frame.fullbright = st.mode == shading::fullbright || st.mode == shading::fake_normal || st.mode == shading::fake_view || st.mode == shading::case_textures;
		frame.case_textures = st.mode == shading::case_textures;
		frame.draw_water = st.draw_water;
		frame.texture_filter = st.texture_filter;
		frame.mip_bias = static_cast<float>(st.texture_resolution);
		frame.specular_maps = st.specular;
		frame.normal_maps = st.normal_maps;
		frame.far_clip = st.cubic_clip ? st.cubic_distance : 0.0f;
		frame.clear_color = colors::d3d(colors::view_background) | 0xFF000000u;

		// Show Entities As: models of entities drawn, as wireframes or not at all ( their box is drawn by the view )
		switch (st.show_entities)
		{
		case entities::boxes:
			frame.model_style = [](const view::object_ref& ref)
			{
				return get_scene().object_of(ref) != edit::no_object ? view::model_style::hidden : view::model_style::solid;
			};
			break;
		case entities::wireframe:
			frame.model_style = [](const view::object_ref& ref)
			{
				return get_scene().object_of(ref) != edit::no_object ? view::model_style::wireframe : view::model_style::solid;
			};
			break;
		case entities::selected_wireframe:
		case entities::selected_models:
		{
			const int shown = st.show_entities == entities::selected_wireframe ? view::model_style::wireframe : view::model_style::solid;
			frame.model_style = [shown](const view::object_ref& ref)
			{
				const auto& s = get_scene();
				const object_id id = s.object_of(ref);
				return id == edit::no_object ? view::model_style::solid : (s.doc.is_selected(id) ? shown : view::model_style::hidden);
			};
			break;
		}
		default:
			break;
		}

		// the post process reads the depth pre-pass
		if (post_modes || st.light_preview)
		{
			frame.float_z = true;
		}
	}

	void draw_view_legend(ImDrawList* dl, const ImVec2& at, const int width, const int height)
	{
		if (g_state.mode != shading::case_textures || !dl)
		{
			return;
		}

		auto sizes = get_scene().renderer.case_texture_sizes();
		std::sort(sizes.begin(), sizes.end(), [](const auto& a, const auto& b) { return a.first * a.second != b.first * b.second ? a.first * a.second > b.first * b.second : a.first > b.first; });

		const float line = ImGui::GetTextLineHeight() + 2.0f;
		const int rows = std::max(1, std::min(static_cast<int>(sizes.size()), static_cast<int>((height - 60) / line) - 2));
		const float w = 150.0f;
		const ImVec2 top(at.x + static_cast<float>(width) - w - 10.0f, at.y + 10.0f);
		dl->AddRectFilled(top, ImVec2(top.x + w, top.y + line * (static_cast<float>(std::min<int>(rows, static_cast<int>(sizes.size()))) + 1.5f)), IM_COL32(0, 0, 0, 170), 3.0f);
		dl->AddText(ImVec2(top.x + 6.0f, top.y + 3.0f), IM_COL32(230, 230, 230, 255), sizes.empty() ? "case textures: -" : "case textures");

		for (int i = 0; i < rows && i < static_cast<int>(sizes.size()); i++)
		{
			const auto [sw, sh] = sizes[static_cast<std::size_t>(i)];
			const std::uint32_t c = view::map_renderer::case_texture_color(sw, sh);	// D3DCOLOR: ARGB
			const ImU32 color = IM_COL32((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, 255);
			const float y = top.y + 3.0f + line * static_cast<float>(i + 1);
			dl->AddRectFilled(ImVec2(top.x + 6.0f, y + 2.0f), ImVec2(top.x + 20.0f, y + line - 4.0f), color);
			char text[48];
			if (i == rows - 1 && rows < static_cast<int>(sizes.size())) { std::snprintf(text, sizeof(text), ".. %d more", static_cast<int>(sizes.size()) - i); }
			else { std::snprintf(text, sizeof(text), "%d x %d", sw, sh); }
			dl->AddText(ImVec2(top.x + 26.0f, y), IM_COL32(230, 230, 230, 255), text);
		}
	}
	IDirect3DTexture9* post_process(IDirect3DTexture9* frame_texture, const int width, const int height)
	{
		auto& s = get_scene();
		auto* device = game::glob::d3d9_device;
		const auto& st = g_state;

		if (!frame_texture || !device || width < 2 || height < 2)
		{
			return frame_texture;
		}

		const auto lights = collect_lights();
		const bool film = st.filmtweaks && g_film.enable;
		const int mode = st.mode == shading::fake_normal ? 1 : (st.mode == shading::fake_view ? 2 : (st.mode == shading::depth ? 3 : 0));

		if (!mode && lights.empty() && !film)
		{
			return frame_texture;
		}

		if (!ensure_shader(device))
		{
			if (!g_post.said)
			{
				g_post.said = true;
				game::printf_to_console("^1[renderer] %s\n", g_post.problem.c_str());
			}

			return frame_texture;
		}

		if (!g_post.target || g_post.width != width || g_post.height != height)
		{
			release_post_targets();

			if (FAILED(device->CreateTexture(static_cast<UINT>(width), static_cast<UINT>(height), 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_post.target, nullptr))
				|| FAILED(g_post.target->GetSurfaceLevel(0, &g_post.surface)))
			{
				release_post_targets();
				return frame_texture;
			}

			g_post.width = width;
			g_post.height = height;
		}

		if (!g_post.saved && FAILED(device->CreateStateBlock(D3DSBT_ALL, &g_post.saved)))
		{
			return frame_texture;
		}

		IDirect3DTexture9* depth = s.renderer.depth_texture();

		// ---- the device as the host has it
		g_post.saved->Capture();
		IDirect3DSurface9* host_target = nullptr;
		IDirect3DSurface9* host_depth = nullptr;
		device->GetRenderTarget(0, &host_target);
		device->GetDepthStencilSurface(&host_depth);

		const bool scene = SUCCEEDED(device->BeginScene());
		device->SetRenderTarget(0, g_post.surface);
		device->SetDepthStencilSurface(nullptr);

		const D3DVIEWPORT9 viewport{ 0, 0, static_cast<DWORD>(width), static_cast<DWORD>(height), 0.0f, 1.0f };
		device->SetViewport(&viewport);

		device->SetVertexShader(nullptr);
		device->SetPixelShader(g_post.shader);
		device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
		device->SetRenderState(D3DRS_ZENABLE, FALSE);
		device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
		device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF);
		device->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);
		device->SetRenderState(D3DRS_FOGENABLE, FALSE);

		for (DWORD i = 0; i < 2; i++)
		{
			device->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			device->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			device->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
			device->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			device->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
			device->SetSamplerState(i, D3DSAMP_SRGBTEXTURE, FALSE);
			device->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, 0);
		}

		device->SetTexture(0, frame_texture);
		device->SetTexture(1, depth);

		// ---- constants, in the order of the shader ( c0 .. )
		const auto axes = s.cam.axes();
		const float tan_x = std::tan(s.cam.fov_x * 0.5f * 0.0174532925f);
		const float tan_y = tan_x * static_cast<float>(height) / static_cast<float>(width);

		float constants[10 + 4 * MAX_LIGHTS][4] = {};
		const auto set = [&](const int index, const float x, const float y, const float z, const float w)
		{
			constants[index][0] = x; constants[index][1] = y; constants[index][2] = z; constants[index][3] = w;
		};

		// without the depth of the frame only the film is left
		const bool have_depth = depth != nullptr;
		set(0, have_depth ? static_cast<float>(mode) : 0.0f, have_depth ? static_cast<float>(lights.size()) : 0.0f, film ? 1.0f : 0.0f, g_film.invert ? 1.0f : 0.0f);
		set(1, tan_x, tan_y, 1.0f / static_cast<float>(width), 1.0f / static_cast<float>(height));
		set(2, s.cam.origin.x, s.cam.origin.y, s.cam.origin.z, 0.0f);
		set(3, axes.forward.x, axes.forward.y, axes.forward.z, 0.0f);
		set(4, -axes.left.x, -axes.left.y, -axes.left.z, 0.0f);
		set(5, axes.up.x, axes.up.y, axes.up.z, 0.0f);
		set(6, g_film.contrast, g_film.brightness, g_film.desaturation, g_film.desaturation_dark);
				set(9 + 4 * MAX_LIGHTS, g_film.medium_tint[0], g_film.medium_tint[1], g_film.medium_tint[2], 0.0f);
		set(7, g_film.light_tint[0], g_film.light_tint[1], g_film.light_tint[2], 0.0f);
		set(8, g_film.dark_tint[0], g_film.dark_tint[1], g_film.dark_tint[2], 0.0f);

		for (std::size_t i = 0; i < lights.size(); i++)
		{
			const auto& l = lights[i].info;
			const float intensity = st.light_max_intensity ? 1.0f : std::clamp(l.intensity, 0.0f, 4.0f);
			const float outer = l.spot ? std::cos(std::clamp(l.outer, 1.0f, 179.0f) * 0.5f * 0.0174532925f) : -1.0f;
			const float inner = l.spot ? std::cos(std::clamp(l.inner > 0.0f ? l.inner : l.outer * 0.8f, 0.5f, 179.0f) * 0.5f * 0.0174532925f) : 1.0f;
			set(9 + static_cast<int>(i), l.origin.x, l.origin.y, l.origin.z, std::max(l.radius, 1.0f));
			set(9 + MAX_LIGHTS + static_cast<int>(i), l.color[0] * intensity, l.color[1] * intensity, l.color[2] * intensity, l.spot ? 1.0f : 0.0f);
			set(9 + 2 * MAX_LIGHTS + static_cast<int>(i), l.forward.x, l.forward.y, l.forward.z, outer);
			set(9 + 3 * MAX_LIGHTS + static_cast<int>(i), inner, 0.0f, 0.0f, 0.0f);
		}

		device->SetPixelShaderConstantF(0, &constants[0][0], static_cast<UINT>(std::size(constants)));

		// ---- one triangle pair over the whole target ( half a pixel up and left: texel centres on pixel centres )
		struct vertex { float x, y, z, rhw, u, v; };
		const float w = static_cast<float>(width), h = static_cast<float>(height);
		const vertex quad[4] = {
			{ -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f }, { w - 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f },
			{ -0.5f, h - 0.5f, 0.0f, 1.0f, 0.0f, 1.0f }, { w - 0.5f, h - 0.5f, 0.0f, 1.0f, 1.0f, 1.0f } };
		device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(vertex));

		if (scene) { device->EndScene(); }

		// ---- back to the host's device
		device->SetTexture(0, nullptr);
		device->SetTexture(1, nullptr);

		if (host_target) { device->SetRenderTarget(0, host_target); host_target->Release(); }
		device->SetDepthStencilSurface(host_depth);
		if (host_depth) { host_depth->Release(); }
		g_post.saved->Apply();

		return g_post.target;
	}

	void draw_overlay(const view::overlay_context& ctx)
	{
		auto& s = get_scene();
		const auto& st = g_state;
		auto* device = ctx.device;

		const std::uint64_t key = (static_cast<std::uint64_t>(st.show_patches) << 8) | (st.show_angles ? 1u : 0u) | (st.model_origins ? 2u : 0u) | (st.tint_selected ? 4u : 0u) | (colors::serial() << 16);

		if (g_lines_statics != s.statics_serial || g_lines_selected != s.selected_serial || g_lines_key != key)
		{
			g_lines_statics = s.statics_serial;
			g_lines_selected = s.selected_serial;
			g_lines_key = key;
			rebuild_lines();
		}

		if (g_lines.empty() && g_tint.empty())
		{
			return;
		}

		device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		device->SetRenderState(D3DRS_ZENABLE, TRUE);
		device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

		const float bias = -0.00003f;
		device->SetRenderState(D3DRS_DEPTHBIAS, *reinterpret_cast<const DWORD*>(&bias));

		constexpr std::size_t chunk = 20000;

		for (std::size_t first = 0; first < g_lines.size(); first += chunk * 2)
		{
			const auto count = std::min(chunk, (g_lines.size() - first) / 2);
			device->DrawPrimitiveUP(D3DPT_LINELIST, static_cast<UINT>(count), g_lines.data() + first, sizeof(line_vertex));
		}

		for (std::size_t first = 0; first < g_tint.size(); first += chunk * 3)
		{
			const auto count = std::min(chunk, (g_tint.size() - first) / 3);
			device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, static_cast<UINT>(count), g_tint.data() + first, sizeof(line_vertex));
		}
	}

	// ---- the 2D views ------------------------------------------------------------------------------------------------

	void draw_grid_extras(const grid_context& ctx)
	{
		const auto& st = g_state;
		auto* dl = ctx.dl;
		if (!dl) { return; }

		const int h = ctx.axis == 0 ? 1 : 0, v = ctx.axis == 2 ? 1 : 2;
		static const char* const letters = "xyz";

		const auto to_screen = [&](const double wh, const double wv)
		{
			return ImVec2(ctx.pos.x + ctx.size.x * 0.5f + static_cast<float>((wh - ctx.center[0]) * ctx.scale), ctx.pos.y + ctx.size.y * 0.5f - static_cast<float>((wv - ctx.center[1]) * ctx.scale));
		};

		const double half_w = ctx.size.x * 0.5 / ctx.scale, half_h = ctx.size.y * 0.5 / ctx.scale;
		const double w0 = ctx.center[0] - half_w, w1 = ctx.center[0] + half_w, v0 = ctx.center[1] - half_h, v1 = ctx.center[1] + half_h;
		auto* font = ImGui::GetIO().Fonts->Fonts.Size > 2 ? ImGui::GetIO().Fonts->Fonts[2] : ImGui::GetFont();

		if (ui::ext::get().show_grid_blocks)
		{
			// the lines of the blocks are drawn with the lines of the Grid menu ( menu_selection.cpp ): here their coordinates
			const double block = 1024.0;
			const ImU32 color = colors::get(colors::grid_block);

			if (st.grid_coordinates && block * ctx.scale >= 96.0)
			{
				char text[48];

				for (double x = std::floor(w0 / block) * block; x <= w1; x += block)
				{
					for (double y = std::floor(v0 / block) * block; y <= v1; y += block)
					{
						std::snprintf(text, sizeof(text), "%c %g  %c %g", letters[h], x, letters[v], y);
						const ImVec2 at = to_screen(x, y + block);
						dl->AddText(font, font->FontSize, ImVec2(at.x + 4.0f, at.y + 3.0f), color, text);
					}
				}
			}
		}

		if (st.grid_crosshair && ctx.hovered)
		{
			const ImVec2 mouse = ImGui::GetIO().MousePos;
			const ImU32 color = (colors::get(colors::grid_axis) & 0x00FFFFFFu) | 0xB0000000u;
			dl->AddLine(ImVec2(mouse.x, ctx.pos.y), ImVec2(mouse.x, ctx.pos.y + ctx.size.y), color);
			dl->AddLine(ImVec2(ctx.pos.x, mouse.y), ImVec2(ctx.pos.x + ctx.size.x, mouse.y), color);

			if (st.grid_coordinates)
			{
				char text[64];
				const double wh = ctx.center[0] + (mouse.x - ctx.pos.x - ctx.size.x * 0.5f) / ctx.scale, wv = ctx.center[1] - (mouse.y - ctx.pos.y - ctx.size.y * 0.5f) / ctx.scale;
				std::snprintf(text, sizeof(text), "%c %.0f  %c %.0f", letters[h], wh, letters[v], wv);
				dl->AddText(font, font->FontSize, ImVec2(mouse.x + 8.0f, mouse.y + 6.0f), colors::get(colors::grid_text), text);
			}
		}
	}
}
