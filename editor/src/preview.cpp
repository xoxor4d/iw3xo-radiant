#include "std_include.hpp"

// windows.h macros
#undef min
#undef max

#include "host.hpp"
#include "ui.hpp"
#include "assets.hpp"
#include "preview.hpp"

#include <codol_assets/anim.hpp>
#include <codol_assets/model.hpp>
#include <codol_assets/sound.hpp>
#include <codol_assets/bitmap.hpp>
#include <codol_assets/fx_preview.hpp>

#include <mmsystem.h>

namespace editor::preview
{
	namespace fs = std::filesystem;
	namespace ca = codol::assets;
	namespace cz = codol::zone;

	namespace
	{
		constexpr float PI = 3.14159265f;
		constexpr ImVec4 red = ImVec4(0.96f, 0.45f, 0.40f, 1.0f);
		constexpr ImVec4 yellow = ImVec4(0.95f, 0.80f, 0.35f, 1.0f);
		constexpr ImVec4 dim = ImVec4(0.62f, 0.64f, 0.68f, 1.0f);

		// where the previewed objects stand: far below any map, so that nothing of the map is near them
		constexpr float3 SPOT{ 0.0f, 0.0f, -20000.0f };

		enum class kind : int
		{
			none,
			model,
			material,
			image,
			anim,
			sound,
			fx,
		};

		// ---- math: a bone is an offset and three axes ( rows ), a point goes p * R + o ------------------------------
		struct pose
		{
			float o[3] = {};
			float r[3][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
		};

		void rotate(const float r[3][3], const float in[3], float out[3])
		{
			for (int k = 0; k < 3; k++)
			{
				out[k] = in[0] * r[0][k] + in[1] * r[1][k] + in[2] * r[2][k];
			}
		}

		void rotate_back(const float r[3][3], const float in[3], float out[3])
		{
			for (int k = 0; k < 3; k++)
			{
				out[k] = in[0] * r[k][0] + in[1] * r[k][1] + in[2] * r[k][2];
			}
		}

		// child given relative to its parent -> model space
		pose compose(const pose& local, const pose& parent)
		{
			pose out;
			for (int i = 0; i < 3; i++)
			{
				rotate(parent.r, local.r[i], out.r[i]);
			}

			rotate(parent.r, local.o, out.o);
			for (int k = 0; k < 3; k++)
			{
				out.o[k] += parent.o[k];
			}

			return out;
		}

		// model space -> relative to the parent
		pose relative(const pose& child, const pose& parent)
		{
			pose out;
			for (int i = 0; i < 3; i++)
			{
				rotate_back(parent.r, child.r[i], out.r[i]);
			}

			const float d[3] = { child.o[0] - parent.o[0], child.o[1] - parent.o[1], child.o[2] - parent.o[2] };
			rotate_back(parent.r, d, out.o);
			return out;
		}

		pose blend(const ca::ExportPose& a, const ca::ExportPose& b, const float t)
		{
			pose out;
			ca::Quat qa = ca::quat_from_axis(a.axis), qb = ca::quat_from_axis(b.axis);
			const float dot = qa[0] * qb[0] + qa[1] * qb[1] + qa[2] * qb[2] + qa[3] * qb[3];
			float len = 0.0f;

			for (int k = 0; k < 4; k++)
			{
				qa[static_cast<std::size_t>(k)] = qa[static_cast<std::size_t>(k)] * (1.0f - t) + (dot < 0.0f ? -qb[static_cast<std::size_t>(k)] : qb[static_cast<std::size_t>(k)]) * t;
				len += qa[static_cast<std::size_t>(k)] * qa[static_cast<std::size_t>(k)];
			}

			len = std::sqrt(std::max(len, 1e-12f));
			for (float& part : qa)
			{
				part /= len;
			}

			ca::axis_from_quat(qa, out.r);
			for (int k = 0; k < 3; k++)
			{
				out.o[k] = a.offset[k] * (1.0f - t) + b.offset[k] * t;
			}

			return out;
		}

		std::string lower(std::string text)
		{
			for (char& c : text)
			{
				c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			}

			return text;
		}

		// ---- state --------------------------------------------------------------------------------------------
		struct sound_row
		{
			std::string alias, file;
			bool streamed = false, looping = false, found = false;
			fs::path path;
		};

		struct state_t
		{
			kind what = kind::none;
			std::string name;			// model, material, animation
			fs::path file;				// image, csv
			std::string note;			// under the picture: where it comes from
			std::string error;
			bool dirty = false;			// the renderer objects have to be made ( again )

			// renderer objects ( of the map that was loaded when they were made )
			int model_handle = 0;
			std::vector<int> meshes;
			std::vector<std::string> mesh_materials;
			float3 centre{};			// what the camera turns around, relative to SPOT
			float radius = 64.0f;

			// camera
			float yaw = 215.0f, pitch = 18.0f, zoom = 1.0f;

			// render target
			IDirect3DTexture9* target = nullptr;
			IDirect3DSurface9* depth = nullptr;
			int target_w = 0, target_h = 0;

			// material / image
			IDirect3DTexture9* picture = nullptr;		// owned by the renderer ( material ) ..
			IDirect3DTexture9* own_picture = nullptr;	// .. or made here ( image )
			int picture_w = 0, picture_h = 0;
			view::material_info material;
			bool material_known = false;
			std::string image_info;

			// animation
			ca::ExportModel model;
			ca::ExportAnim anim;
			std::vector<int> part_bone;					// part -> bone, -1: the model has no such bone
			std::vector<pose> bind;
			std::string anim_model;						// name of the model
			std::map<std::string, std::string> materials;	// export material -> material ( source files )
			bool compiled = false;						// read back from the zone
			float frame = 0.0f;
			bool playing = true;
			bool loop = true;
			float speed = 1.0f;
			float shown_frame = -1.0f;
			std::vector<std::vector<view::mesh_vertex>> mesh_vertices;	// per material
			std::vector<std::vector<std::array<int, 2>>> mesh_corners;	// per material: face, corner

			// sound
			std::vector<sound_row> sounds;
			std::string sound_message;
			std::string zone_alias, zone_name;

			// effect
			ca::EditorEffect fx;
			std::string fx_zone;						// asked for: a zone of the game or a .ff; empty: the project's .efx, else zones
			float fx_time = 0.0f, fx_duration = 1.0f;
			bool fx_loop = true;
			std::vector<std::string> fx_skipped;
			std::size_t fx_count = 0, fx_missing = 0;	// particles drawn last frame; of them without a picture
			std::map<std::string, IDirect3DTexture9*> fx_pictures;	// owned by the renderer
		};

		state_t g;
		std::vector<std::uint8_t> g_wave;		// what PlaySound plays ( it reads the memory while it plays )
		bool g_mci_open = false;

		void release_target()
		{
			if (g.depth) { g.depth->Release(); g.depth = nullptr; }
			if (g.target) { g.target->Release(); g.target = nullptr; }
			g.target_w = g.target_h = 0;
		}

		void drop_objects(const bool renderer_alive)
		{
			auto& s = get_scene();

			if (renderer_alive && s.renderer.loaded())
			{
				if (g.model_handle) { s.renderer.remove_model(g.model_handle); }
				for (const int handle : g.meshes) { s.renderer.destroy_mesh(handle); }
			}

			g.model_handle = 0;
			g.meshes.clear();
			g.mesh_materials.clear();
			g.picture = nullptr;
			g.fx_pictures.clear();
		}

		void clear_subject(const bool renderer_alive = true)
		{
			drop_objects(renderer_alive);

			if (g.own_picture) { g.own_picture->Release(); g.own_picture = nullptr; }

			g.what = kind::none;
			g.name.clear();
			g.file.clear();
			g.note.clear();
			g.error.clear();
			g.model = {};
			g.anim = {};
			g.part_bone.clear();
			g.bind.clear();
			g.materials.clear();
			g.mesh_vertices.clear();
			g.mesh_corners.clear();
			g.sounds.clear();
			g.sound_message.clear();
			g.material_known = false;
			g.image_info.clear();
			g.shown_frame = -1.0f;
			g.frame = 0.0f;
			g.fx = {};
			g.fx_zone.clear();
			g.fx_time = 0.0f;
			g.fx_skipped.clear();
			g.fx_pictures.clear();
			g.fx_count = g.fx_missing = 0;
		}

		void begin(const kind what)
		{
			clear_subject();
			g.what = what;
			g.dirty = true;
			g.yaw = 215.0f;
			g.pitch = 18.0f;
			g.zoom = 1.0f;
			ui::get().show_preview = true;
		}

		// ---- model ------------------------------------------------------------------------------------------
		void make_model()
		{
			auto& s = get_scene();
			float3 mins, maxs;
			std::string error;

			view::transform at;
			at.origin = SPOT;
			g.model_handle = s.renderer.add_model(g.name, at, &error);

			if (!g.model_handle)
			{
				g.error = "the model cannot be shown: " + error;
				return;
			}

			s.renderer.set_hidden({ view::object_kind::placed_model, g.model_handle }, true);

			if (s.renderer.model_bounds(g.name, mins, maxs))
			{
				g.centre = { (mins.x + maxs.x) * 0.5f, (mins.y + maxs.y) * 0.5f, (mins.z + maxs.z) * 0.5f };
				const float dx = maxs.x - mins.x, dy = maxs.y - mins.y, dz = maxs.z - mins.z;
				g.radius = std::max(4.0f, 0.5f * std::sqrt(dx * dx + dy * dy + dz * dz));

				char text[160];
				std::snprintf(text, sizeof(text), "%.0f x %.0f x %.0f units", dx, dy, dz);
				g.note = text;
			}
			else
			{
				g.centre = {};
				g.radius = 64.0f;
			}
		}

		// ---- material ---------------------------------------------------------------------------------------
		void add_quad(std::vector<view::mesh_vertex>& out, const float3& a, const float3& b, const float3& c, const float3& d, const float3& n, const float repeat)
		{
			const float3 corners[4] = { a, b, c, d };
			const float uv[4][2] = { { 0, repeat }, { repeat, repeat }, { repeat, 0 }, { 0, 0 } };

			for (const int i : { 0, 1, 2, 0, 2, 3 })
			{
				view::mesh_vertex v;
				v.position = { SPOT.x + corners[i].x, SPOT.y + corners[i].y, SPOT.z + corners[i].z };
				v.normal = n;
				v.uv[0] = uv[i][0];
				v.uv[1] = uv[i][1];
				out.push_back(v);
			}
		}

		void make_material()
		{
			auto& s = get_scene();
			g.material_known = s.renderer.material_details(g.name, g.material);

			if (!g.material_known)
			{
				g.error = "there is no material " + g.name + " in the zones of the open map";
				return;
			}

			g.picture = s.renderer.material_thumbnail(g.name, 512);
			g.picture_w = g.material.width;
			g.picture_h = g.material.height;

			// a cube of 64 units: the material as its own shaders draw it, lit by the sun of the map
			constexpr float h = 32.0f;
			std::vector<view::mesh_vertex> cube;
			add_quad(cube, { -h, -h, -h }, { h, -h, -h }, { h, -h, h }, { -h, -h, h }, { 0, -1, 0 }, 1.0f);
			add_quad(cube, { h, -h, -h }, { h, h, -h }, { h, h, h }, { h, -h, h }, { 1, 0, 0 }, 1.0f);
			add_quad(cube, { h, h, -h }, { -h, h, -h }, { -h, h, h }, { h, h, h }, { 0, 1, 0 }, 1.0f);
			add_quad(cube, { -h, h, -h }, { -h, -h, -h }, { -h, -h, h }, { -h, h, h }, { -1, 0, 0 }, 1.0f);
			add_quad(cube, { -h, -h, h }, { h, -h, h }, { h, h, h }, { -h, h, h }, { 0, 0, 1 }, 1.0f);
			add_quad(cube, { -h, h, -h }, { h, h, -h }, { h, -h, -h }, { -h, -h, -h }, { 0, 0, -1 }, 1.0f);

			view::mesh_data data;
			data.vertices = cube.data();
			data.vertex_count = cube.size();
			data.material = g.name;

			std::string error;
			if (const int handle = s.renderer.create_mesh(data, &error))
			{
				s.renderer.set_mesh_visible(handle, false);
				g.meshes.push_back(handle);

				const auto shading = s.renderer.mesh_shading_of(handle);
				g.note = shading == view::mesh_shading::game_material ? "the cube: drawn with the material's technique set"
					: shading == view::mesh_shading::color_map ? "the cube: its colour map only ( the technique set is not one for geometry )" : "the cube: no colour map to draw";
			}

			g.centre = {};
			g.radius = 56.0f;
		}

		// ---- image ------------------------------------------------------------------------------------------
		void make_image()
		{
			ca::SourceImage image;

			if (const auto status = ca::load_image(g.file, image); !status || image.faces.empty())
			{
				g.error = "the image cannot be read: " + status.message;
				return;
			}

			const auto& face = image.faces[0];
			auto* device = game::glob::d3d9_device;

			if (!device || face.width <= 0 || face.height <= 0)
			{
				g.error = "no picture";
				return;
			}

			if (FAILED(device->CreateTexture(static_cast<UINT>(face.width), static_cast<UINT>(face.height), 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &g.own_picture, nullptr)))
			{
				g.error = "the picture is too large for a texture of this device";
				return;
			}

			D3DLOCKED_RECT rect{};
			if (SUCCEEDED(g.own_picture->LockRect(0, &rect, nullptr, 0)))
			{
				for (int y = 0; y < face.height; y++)
				{
					auto* row = static_cast<std::uint8_t*>(rect.pBits) + static_cast<std::size_t>(y) * static_cast<std::size_t>(rect.Pitch);
					const auto* from = face.at(0, y);

					for (int x = 0; x < face.width; x++)
					{
						row[x * 4 + 0] = from[x * 4 + 2];
						row[x * 4 + 1] = from[x * 4 + 1];
						row[x * 4 + 2] = from[x * 4 + 0];
						row[x * 4 + 3] = from[x * 4 + 3];
					}
				}

				g.own_picture->UnlockRect(0);
			}

			g.picture_w = face.width;
			g.picture_h = face.height;

			const auto pow2 = [](const int v) { return v > 0 && (v & (v - 1)) == 0; };
			char text[200];
			std::snprintf(text, sizeof(text), "%d x %d%s%s%s", face.width, face.height, image.is_cube() ? ", cube map ( the +x face )" : "", image.hasAlpha ? ", alpha channel" : "",
				pow2(face.width) && pow2(face.height) ? "" : " - not powers of two: the compiler will refuse it");
			g.image_info = text;
		}

		// ---- animation --------------------------------------------------------------------------------------
		// the compiled animation and its model, read back from the project's zone
		bool read_compiled(const std::string& anim_name, const std::string& model_name, std::string& why)
		{
			std::error_code ec;
			const auto zone_file = assets::zone_file();

			if (model_name.empty() || !fs::is_regular_file(zone_file, ec))
			{
				why = model_name.empty() ? "the animation names no model of the project" : "the project has no zone yet";
				return false;
			}

			cz::ClientKey key;
			const auto install = fs::u8path(components::codol_game::install_path());

			if (const auto status = key.load_from_exe(install / "codoMP_client_shipRetail.exe"); !status)
			{
				why = status.message;
				return false;
			}

			cz::Zone zone;
			if (const auto status = zone.open(zone_file, key); !status)
			{
				why = status.message;
				return false;
			}

			const auto anim_header = zone.find_asset(codol::c1::ASSET_TYPE_XANIMPARTS, anim_name);
			const auto model_header = zone.find_asset(codol::c1::ASSET_TYPE_XMODEL, model_name);

			if (!anim_header || !model_header)
			{
				why = !anim_header ? "the zone has no animation " + anim_name + " ( Build )" : "the zone has no model " + model_name + " ( Build )";
				return false;
			}

			cz::ModelView view;
			ca::ExportModel model;

			if (!cz::read_model(zone, model_header, view, true) || view.isReference || !ca::extract_model_lod(zone, view, 0, model))
			{
				why = "the model " + model_name + " of the zone cannot be read back";
				return false;
			}

			ca::RawAnim raw;
			ca::DecodedAnim decoded;
			std::string error;

			if (!ca::read_raw_anim(zone, anim_header, raw, &error))
			{
				why = "the animation of the zone cannot be read: " + error;
				return false;
			}

			if (const auto status = ca::decode_anim(raw, decoded); !status)
			{
				why = "the animation of the zone cannot be decoded: " + status.message;
				return false;
			}

			ca::ExportAnim anim;
			ca::extract_anim(decoded, &model, anim);

			if (anim.frames.empty())
			{
				why = "the animation of the zone has no frames";
				return false;
			}

			g.model = std::move(model);
			g.anim = std::move(anim);
			g.materials.clear();
			return true;
		}

		bool read_sources(const assets::preview_files& files, std::string& why)
		{
			if (files.mesh.empty())
			{
				why = "the animation has no model: give it one ( the model line of its .anim, or the Model field here )";
				return false;
			}

			ca::ExportModel model;
			ca::ExportAnim anim;

			if (const auto status = ca::load_xmodel_export(files.mesh, model); !status)
			{
				why = to_utf8(files.mesh.filename()) + ": " + status.message;
				return false;
			}

			if (const auto status = ca::load_xanim_export(files.xanim, anim); !status)
			{
				why = to_utf8(files.xanim.filename()) + ": " + status.message;
				return false;
			}

			if (anim.frames.empty())
			{
				why = "the animation has no frames";
				return false;
			}

			g.model = std::move(model);
			g.anim = std::move(anim);
			g.materials = files.materials;
			return true;
		}

		// model space pose of every bone at a frame
		void pose_at(const float frame, std::vector<pose>& out)
		{
			const auto& anim = g.anim;
			const auto& bones = g.model.bones;
			const int last = static_cast<int>(anim.frames.size()) - 1;
			const float clamped = std::clamp(frame, 0.0f, static_cast<float>(last));
			const int f0 = std::min(static_cast<int>(clamped), last), f1 = std::min(f0 + 1, last);
			const float t = clamped - static_cast<float>(f0);

			std::vector<int> part_of(bones.size(), -1);
			for (std::size_t p = 0; p < g.part_bone.size(); p++)
			{
				if (g.part_bone[p] >= 0)
				{
					part_of[static_cast<std::size_t>(g.part_bone[p])] = static_cast<int>(p);
				}
			}

			out.assign(bones.size(), pose{});
			std::vector<char> done(bones.size(), 0);

			// parents first, whatever the order of the bone list is
			for (std::size_t pass = 0; pass < bones.size(); pass++)
			{
				bool progress = false;

				for (std::size_t i = 0; i < bones.size(); i++)
				{
					const int parent = bones[i].parent;

					if (done[i] || (parent >= 0 && static_cast<std::size_t>(parent) < bones.size() && !done[static_cast<std::size_t>(parent)]))
					{
						continue;
					}

					const bool has_parent = parent >= 0 && static_cast<std::size_t>(parent) < bones.size();
					const int part = part_of[i];

					if (part >= 0)
					{
						pose p = blend(anim.frames[static_cast<std::size_t>(f0)][static_cast<std::size_t>(part)], anim.frames[static_cast<std::size_t>(f1)][static_cast<std::size_t>(part)], t);

						if (anim.local)
						{
							// the stored form: relative to the parent; a part without a translation track keeps the model's own
							const pose local_bind = has_parent ? relative(g.bind[i], g.bind[static_cast<std::size_t>(parent)]) : g.bind[i];

							if (std::find(anim.noTranslation.begin(), anim.noTranslation.end(), part) != anim.noTranslation.end())
							{
								std::copy(local_bind.o, local_bind.o + 3, p.o);
							}

							out[i] = has_parent ? compose(p, out[static_cast<std::size_t>(parent)]) : p;
						}
						else
						{
							out[i] = p;
						}
					}
					else
					{
						// a bone the animation does not move follows its parent
						out[i] = has_parent ? compose(relative(g.bind[i], g.bind[static_cast<std::size_t>(parent)]), out[static_cast<std::size_t>(parent)]) : g.bind[i];
					}

					done[i] = 1;
					progress = true;
				}

				if (!progress)
				{
					break;
				}
			}
		}

		void skin(const float frame)
		{
			std::vector<pose> now;
			pose_at(frame, now);

			const auto& model = g.model;
			std::vector<std::array<float, 3>> positions(model.vertices.size());
			std::vector<int> main_bone(model.vertices.size(), -1);

			for (std::size_t v = 0; v < model.vertices.size(); v++)
			{
				const auto& vertex = model.vertices[v];
				float sum[3] = {}, total = 0.0f, best = -1.0f;

				for (const auto& [bone, weight] : vertex.weights)
				{
					if (bone < 0 || static_cast<std::size_t>(bone) >= now.size())
					{
						continue;
					}

					const auto& b = g.bind[static_cast<std::size_t>(bone)];
					const auto& a = now[static_cast<std::size_t>(bone)];
					const float d[3] = { vertex.position[0] - b.o[0], vertex.position[1] - b.o[1], vertex.position[2] - b.o[2] };
					float local[3], moved[3];
					rotate_back(b.r, d, local);
					rotate(a.r, local, moved);

					for (int k = 0; k < 3; k++)
					{
						sum[k] += (moved[k] + a.o[k]) * weight;
					}

					total += weight;

					if (weight > best)
					{
						best = weight;
						main_bone[v] = bone;
					}
				}

				if (total > 1e-6f)
				{
					positions[v] = { sum[0] / total, sum[1] / total, sum[2] / total };
				}
				else
				{
					positions[v] = { vertex.position[0], vertex.position[1], vertex.position[2] };
				}
			}

			for (std::size_t m = 0; m < g.mesh_corners.size(); m++)
			{
				auto& vertices = g.mesh_vertices[m];
				const auto& corners = g.mesh_corners[m];
				vertices.resize(corners.size());

				for (std::size_t i = 0; i < corners.size(); i++)
				{
					const auto& corner = model.faces[static_cast<std::size_t>(corners[i][0])].corners[corners[i][1]];
					const auto vertex = static_cast<std::size_t>(std::clamp(corner.vertex, 0, static_cast<int>(model.vertices.size()) - 1));
					auto& out = vertices[i];
					out.position = { SPOT.x + positions[vertex][0], SPOT.y + positions[vertex][1], SPOT.z + positions[vertex][2] };

					float normal[3] = { corner.normal[0], corner.normal[1], corner.normal[2] };

					if (const int bone = main_bone[vertex]; bone >= 0)
					{
						float local[3];
						rotate_back(g.bind[static_cast<std::size_t>(bone)].r, corner.normal, local);
						rotate(now[static_cast<std::size_t>(bone)].r, local, normal);
					}

					out.normal = { normal[0], normal[1], normal[2] };
					out.uv[0] = corner.uv[0];
					out.uv[1] = corner.uv[1];
					out.color = 0xFFFFFFFF;
				}
			}
		}

		void upload_meshes()
		{
			auto& s = get_scene();

			for (std::size_t m = 0; m < g.mesh_vertices.size(); m++)
			{
				if (g.mesh_vertices[m].empty())
				{
					continue;
				}

				view::mesh_data data;
				data.vertices = g.mesh_vertices[m].data();
				data.vertex_count = g.mesh_vertices[m].size();
				data.material = g.mesh_materials[m];
				data.two_sided = true;

				if (m < g.meshes.size() && g.meshes[m])
				{
					s.renderer.update_mesh(g.meshes[m], data);
				}
				else
				{
					if (g.meshes.size() <= m) { g.meshes.resize(m + 1, 0); }
					g.meshes[m] = s.renderer.create_mesh(data);

					if (g.meshes[m])
					{
						s.renderer.set_mesh_visible(g.meshes[m], false);
					}
				}
			}
		}

		void make_anim()
		{
			assets::preview_files files;
			std::string why_compiled, why_sources;

			if (!assets::anim_files(g.name, files))
			{
				g.error = "the project has no animation " + g.name;
				return;
			}

			// another model than the animation's own: a model of the project
			if (!g.anim_model.empty() && g.anim_model != files.model)
			{
				assets::preview_files other;

				if (assets::model_files(g.anim_model, other))
				{
					files.model = g.anim_model;
					files.mesh = other.mesh;
					files.materials = other.materials;
				}
			}
			else
			{
				g.anim_model = files.model;
			}

			g.compiled = read_compiled(g.name, files.model, why_compiled);

			if (!g.compiled && !read_sources(files, why_sources))
			{
				g.error = why_sources;
				return;
			}

			g.note = g.compiled ? "the compiled animation and model, read back from " + assets::zone_name() + ".ff"
				: "from the source files ( " + why_compiled + " )";

			const auto& model = g.model;
			g.bind.resize(model.bones.size());

			for (std::size_t i = 0; i < model.bones.size(); i++)
			{
				std::copy(model.bones[i].offset, model.bones[i].offset + 3, g.bind[i].o);
				std::memcpy(g.bind[i].r, model.bones[i].axis, sizeof(g.bind[i].r));
			}

			std::unordered_map<std::string, int> bone_by_name;
			for (std::size_t i = 0; i < model.bones.size(); i++)
			{
				bone_by_name.emplace(lower(model.bones[i].name), static_cast<int>(i));
			}

			g.part_bone.assign(g.anim.parts.size(), -1);
			int missing = 0;

			for (std::size_t p = 0; p < g.anim.parts.size(); p++)
			{
				if (const auto it = bone_by_name.find(lower(g.anim.parts[p])); it != bone_by_name.end())
				{
					g.part_bone[p] = it->second;
				}
				else
				{
					missing++;
				}
			}

			if (missing)
			{
				g.note += "; " + std::to_string(missing) + " part( s ) of the animation are no bones of the model";
			}

			// one mesh per material
			g.mesh_materials.clear();
			g.mesh_corners.assign(model.materials.size(), {});

			for (const auto& name : model.materials)
			{
				const auto mapped = g.materials.find(name);
				g.mesh_materials.push_back(g.compiled ? name : mapped != g.materials.end() ? mapped->second : assets::default_material_for(name));
			}

			float mins[3] = { 1e30f, 1e30f, 1e30f }, maxs[3] = { -1e30f, -1e30f, -1e30f };

			for (std::size_t f = 0; f < model.faces.size(); f++)
			{
				const auto& face = model.faces[f];
				if (face.material < 0 || static_cast<std::size_t>(face.material) >= g.mesh_corners.size())
				{
					continue;
				}

				for (int c = 0; c < 3; c++)
				{
					g.mesh_corners[static_cast<std::size_t>(face.material)].push_back({ static_cast<int>(f), c });
				}
			}

			for (const auto& vertex : model.vertices)
			{
				for (int k = 0; k < 3; k++)
				{
					mins[k] = std::min(mins[k], vertex.position[k]);
					maxs[k] = std::max(maxs[k], vertex.position[k]);
				}
			}

			if (model.vertices.empty() || model.faces.empty())
			{
				g.error = "the model has no triangles";
				return;
			}

			g.centre = { (mins[0] + maxs[0]) * 0.5f, (mins[1] + maxs[1]) * 0.5f, (mins[2] + maxs[2]) * 0.5f };
			const float dx = maxs[0] - mins[0], dy = maxs[1] - mins[1], dz = maxs[2] - mins[2];
			g.radius = std::max(4.0f, 0.6f * std::sqrt(dx * dx + dy * dy + dz * dz));

			g.mesh_vertices.assign(g.mesh_corners.size(), {});

			// what the animation reaches: a few frames across it
			{
				const int count = static_cast<int>(g.anim.frames.size());

				for (int f = 0; f < count; f += std::max(1, count / 12))
				{
					skin(static_cast<float>(f));

					for (const auto& vertices : g.mesh_vertices)
					{
						for (const auto& v : vertices)
						{
							const float p[3] = { v.position.x - SPOT.x, v.position.y - SPOT.y, v.position.z - SPOT.z };
							for (int k = 0; k < 3; k++)
							{
								mins[k] = std::min(mins[k], p[k]);
								maxs[k] = std::max(maxs[k], p[k]);
							}
						}
					}
				}

				g.centre = { (mins[0] + maxs[0]) * 0.5f, (mins[1] + maxs[1]) * 0.5f, (mins[2] + maxs[2]) * 0.5f };
				const float ex = maxs[0] - mins[0], ey = maxs[1] - mins[1], ez = maxs[2] - mins[2];
				g.radius = std::max(4.0f, 0.6f * std::sqrt(ex * ex + ey * ey + ez * ez));
			}

			g.loop = files.loop;
			g.frame = 0.0f;
			g.shown_frame = -1.0f;
			g.playing = true;
		}

		// ---- sound ------------------------------------------------------------------------------------------
		void stop_playing()
		{
			PlaySoundW(nullptr, nullptr, 0);

			if (g_mci_open)
			{
				mciSendStringW(L"close codol_preview", nullptr, 0, nullptr);
				g_mci_open = false;
			}
		}

		bool play_wave(const ca::WaveData& source, std::string& message)
		{
			// the device plays PCM: IMA ADPCM ( the form loaded sounds are stored in ) is decoded here
			ca::WaveData pcm;

			if (source.format == ca::WAVE_IMA_ADPCM)
			{
				if (const auto status = ca::ima_adpcm_decode(source, pcm); !status)
				{
					message = "IMA ADPCM: " + status.message;
					return false;
				}
			}
			else
			{
				pcm = source;
			}

			stop_playing();
			g_wave = ca::write_wav(pcm);

			if (!PlaySoundW(reinterpret_cast<LPCWSTR>(g_wave.data()), nullptr, SND_MEMORY | SND_ASYNC | SND_NODEFAULT))
			{
				message = "Windows did not play it ( no audio device? )";
				return false;
			}

			char text[160];
			std::snprintf(text, sizeof(text), "%.2f s, %u Hz, %s%s", pcm.rate ? static_cast<double>(pcm.frames()) / pcm.rate : 0.0, pcm.rate, pcm.channels == 2 ? "stereo" : "mono",
				source.format == ca::WAVE_IMA_ADPCM ? ", IMA ADPCM decoded" : "");
			message = text;
			return true;
		}

		bool play_file(const fs::path& file, std::string& message)
		{
			const std::string ext = lower(to_utf8(file.extension()));

			if (ext == ".wav")
			{
				ca::WaveData wave;

				if (const auto status = ca::load_wav(file, wave); !status)
				{
					message = to_utf8(file.filename()) + ": " + status.message;
					return false;
				}

				return play_wave(wave, message);
			}

			// .mp3 ( streamed sounds ): the system's player
			stop_playing();
			const std::wstring open = L"open \"" + file.wstring() + L"\" type mpegvideo alias codol_preview";

			if (const MCIERROR code = mciSendStringW(open.c_str(), nullptr, 0, nullptr))
			{
				wchar_t text[256] = {};
				mciGetErrorStringW(code, text, 255);
				message = to_utf8(file.filename()) + ": " + to_utf8(fs::path(text));
				return false;
			}

			g_mci_open = true;

			if (const MCIERROR code = mciSendStringW(L"play codol_preview", nullptr, 0, nullptr))
			{
				wchar_t text[256] = {};
				mciGetErrorStringW(code, text, 255);
				message = to_utf8(file.filename()) + ": " + to_utf8(fs::path(text));
				stop_playing();
				return false;
			}

			message = "playing " + to_utf8(file.filename());
			return true;
		}

		void read_csv(const fs::path& csv, std::vector<sound_row>& rows, std::string& error)
		{
			rows.clear();
			std::ifstream in(csv, std::ios::binary);
			const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
			std::vector<ca::SoundAliasDef> defs;

			if (const auto status = ca::parse_sound_aliases(text, defs); !status)
			{
				error = to_utf8(csv.filename()) + ": " + status.message;
				return;
			}

			const fs::path root = ca::sound_source_root(csv);
			std::error_code ec;

			for (const auto& def : defs)
			{
				sound_row row;
				row.alias = def.name;
				row.file = def.file;
				row.streamed = def.streamed;
				row.looping = def.looping || def.rlooping;

				std::string relative = def.file;
				std::replace(relative.begin(), relative.end(), '\\', '/');
				if (lower(relative).rfind("sound/", 0) == 0) { relative.erase(0, 6); }

				row.path = root / "sound" / fs::u8path(relative);
				row.found = !def.file.empty() && fs::is_regular_file(row.path, ec);
				rows.push_back(std::move(row));
			}
		}

		// an alias of a zone: its first variant with a loaded sound
		bool play_zone_alias(const std::string& alias, const std::string& zone, std::string& message)
		{
			const auto install = fs::u8path(components::codol_game::install_path());
			std::error_code ec;
			fs::path file;

			if (zone.empty())
			{
				file = assets::zone_file();
			}
			else if (lower(zone).size() > 3 && lower(zone).substr(zone.size() - 3) == ".ff")
			{
				file = fs::u8path(zone);
			}
			else
			{
				for (const auto& dir : { install / "client_shipRetail" / "patch", install / "client_shipRetail" / "chinese" })
				{
					if (fs::is_regular_file(dir / (zone + ".ff"), ec))
					{
						file = dir / (zone + ".ff");
						break;
					}
				}
			}

			if (file.empty() || !fs::is_regular_file(file, ec))
			{
				message = zone.empty() ? "the project has no zone yet ( Build )" : "there is no zone " + zone;
				return false;
			}

			cz::ClientKey key;
			if (const auto status = key.load_from_exe(install / "codoMP_client_shipRetail.exe"); !status)
			{
				message = status.message;
				return false;
			}

			// the wave data of a loaded sound is in the temporary block: only the editor's record of the stream has it
			cz::ZoneEditor editor;
			if (const auto status = editor.open(file, key); !status)
			{
				message = to_utf8(file.filename()) + ": " + status.message;
				return false;
			}

			const auto& loaded = editor.zone();
			const auto header = loaded.find_asset(codol::c1::ASSET_TYPE_SOUND, lower(alias));
			ca::CompiledAliasList list;

			if (!header || !ca::extract_sound_alias_list(loaded, header, list))
			{
				message = to_utf8(file.filename()) + " has no sound alias " + alias;
				return false;
			}

			for (const auto& variant : list.aliases)
			{
				if (!variant.hasFile)
				{
					continue;
				}

				if (variant.streamed)
				{
					message = alias + " is streamed ( " + variant.streamDir + "/" + variant.streamName + " ): the file is in the game's archives or the project's sound folder, not in the zone";
					continue;
				}

				const auto sound = loaded.find_asset(codol::c1::ASSET_TYPE_LOADED_SOUND, variant.loadedName);
				ca::CompiledLoadedSound compiled;

				if (!sound || !ca::extract_loaded_sound(editor, sound, compiled))
				{
					message = "the loaded sound " + variant.loadedName + " of " + alias + " is not in " + to_utf8(file.filename()) + " ( a reference to another zone )";
					continue;
				}

				std::string played;
				if (!play_wave(ca::loaded_sound_wave(compiled), played))
				{
					message = played;
					return false;
				}

				message = alias + ": " + variant.loadedName + " from " + to_utf8(file.filename()) + " ( " + played + " )";
				return true;
			}

			if (message.empty())
			{
				message = alias + " has no sound file";
			}

			return false;
		}

		// ---- drawing ------------------------------------------------------------------------------------------
		bool ensure_target(const int w, const int h)
		{
			auto* device = game::glob::d3d9_device;

			if (g.target && g.target_w == w && g.target_h == h)
			{
				return true;
			}

			release_target();

			if (!device || w < 8 || h < 8)
			{
				return false;
			}

			if (FAILED(device->CreateTexture(static_cast<UINT>(w), static_cast<UINT>(h), 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g.target, nullptr)))
			{
				g.target = nullptr;
				return false;
			}

			if (FAILED(device->CreateDepthStencilSurface(static_cast<UINT>(w), static_cast<UINT>(h), D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, TRUE, &g.depth, nullptr)))
			{
				release_target();
				return false;
			}

			g.target_w = w;
			g.target_h = h;
			return true;
		}

		void set_hidden(const bool hidden)
		{
			auto& s = get_scene();

			if (g.model_handle)
			{
				s.renderer.set_hidden({ view::object_kind::placed_model, g.model_handle }, hidden);
			}

			for (const int handle : g.meshes)
			{
				if (handle) { s.renderer.set_mesh_visible(handle, !hidden); }
			}
		}

		// the 3D picture: the objects of the preview alone, seen by a camera that turns around them
		void draw_scene(const int w, const int h)
		{
			auto& s = get_scene();

			if (!ensure_target(w, h))
			{
				ImGui::Dummy(ImVec2(static_cast<float>(w), static_cast<float>(h)));
				return;
			}

			view::camera cam;
			cam.fov_x = 50.0f;
			cam.pitch = g.pitch;
			cam.yaw = g.yaw;
			const auto axes = cam.axes();
			const float tan_half = std::tan(cam.fov_x * 0.5f * PI / 180.0f) * std::min(1.0f, static_cast<float>(h) / static_cast<float>(w));
			const float distance = g.radius / std::max(tan_half, 0.05f) * 1.15f * g.zoom;
			cam.origin = { SPOT.x + g.centre.x - axes.forward.x * distance, SPOT.y + g.centre.y - axes.forward.y * distance, SPOT.z + g.centre.z - axes.forward.z * distance };
			cam.z_near = std::max(0.5f, distance * 0.02f);

			view::frame_options frame;
			frame.game_time = static_cast<float>(ImGui::GetTime());
			frame.draw_world = false;
			frame.draw_static_models = false;
			frame.draw_entity_models = false;
			frame.draw_brush_models = false;
			frame.cell_culling = false;
			frame.sun_shadows = false;
			frame.float_z = false;
			frame.clear_color = 0xFF2A2C30;

			IDirect3DSurface9* surface = nullptr;
			if (FAILED(g.target->GetSurfaceLevel(0, &surface)))
			{
				return;
			}

			frame.target = surface;
			frame.depth_stencil = g.depth;

			set_hidden(false);
			s.renderer.render(cam, w, h, frame);
			set_hidden(true);
			surface->Release();

			auto* dl = ImGui::GetWindowDrawList();
			const ImVec2 at = ImGui::GetCursorScreenPos();

			dl->AddCallback([](const ImDrawList*, const ImDrawCmd*)
			{
				if (game::glob::d3d9_device)
				{
					game::glob::d3d9_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				}
			}, nullptr);

			ImGui::Image(g.target, ImVec2(static_cast<float>(w), static_cast<float>(h)));
			dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

			// turn with the left button, zoom with the wheel
			ImGui::SetCursorScreenPos(at);
			ImGui::InvisibleButton("##preview_orbit", ImVec2(static_cast<float>(w), static_cast<float>(h)));

			if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f))
			{
				const auto& io = ImGui::GetIO();
				g.yaw -= io.MouseDelta.x * 0.5f;
				g.pitch = std::clamp(g.pitch + io.MouseDelta.y * 0.5f, -5.0f, 89.0f);
			}

			if (ImGui::IsItemHovered())
			{
				if (const float wheel = ImGui::GetIO().MouseWheel; wheel != 0.0f)
				{
					g.zoom = std::clamp(g.zoom * (wheel > 0.0f ? 0.88f : 1.14f), 0.2f, 6.0f);
				}
			}
		}

		void draw_picture(IDirect3DTexture9* texture, const int pw, const int ph, const float max_w, const float max_h)
		{
			if (!texture || pw <= 0 || ph <= 0)
			{
				return;
			}

			const float scale = std::min({ max_w / static_cast<float>(pw), max_h / static_cast<float>(ph), 4.0f });
			const ImVec2 size(std::max(1.0f, static_cast<float>(pw) * scale), std::max(1.0f, static_cast<float>(ph) * scale));
			const ImVec2 at = ImGui::GetCursorScreenPos();
			auto* dl = ImGui::GetWindowDrawList();

			// a checker shows through where the picture is transparent
			constexpr float cell = 12.0f;
			dl->PushClipRect(at, ImVec2(at.x + size.x, at.y + size.y), true);
			for (float y = 0.0f; y < size.y; y += cell)
			{
				for (float x = 0.0f; x < size.x; x += cell)
				{
					const bool odd = (static_cast<int>(x / cell) + static_cast<int>(y / cell)) & 1;
					dl->AddRectFilled(ImVec2(at.x + x, at.y + y), ImVec2(at.x + x + cell, at.y + y + cell), odd ? IM_COL32(70, 72, 78, 255) : IM_COL32(52, 54, 60, 255));
				}
			}

			dl->PopClipRect();
			ImGui::Image(texture, size);
		}

		// ---- effect ------------------------------------------------------------------------------------------
		// An effect of the project is its .efx; one of the game is read out of a zone ( FxEffectDef ) and turned back
		// into the editor's form ( codol_assets read_fx / extract_fx ), so both are simulated the same way.
		bool fx_from_zone(const fs::path& file, const std::string& name, std::string& why)
		{
			cz::ClientKey key;
			if (const auto status = key.load_from_exe(fs::u8path(components::codol_game::install_path()) / "codoMP_client_shipRetail.exe"); !status)
			{
				why = status.message;
				return false;
			}

			cz::Zone zone;
			if (const auto status = zone.open(file, key); !status)
			{
				why = to_utf8(file.filename()) + ": " + status.message;
				return false;
			}

			const auto header = zone.find_asset(codol::c1::ASSET_TYPE_FX, lower(name));
			ca::CompiledFx compiled;

			if (!header || !ca::read_fx(zone, header, compiled))
			{
				why = to_utf8(file.filename()) + " has no effect " + name;
				return false;
			}

			if (const auto status = ca::extract_fx(compiled, g.fx); !status)
			{
				why = name + ": " + status.message;
				return false;
			}

			return true;
		}

		void make_fx()
		{
			const auto install = fs::u8path(components::codol_game::install_path());
			std::error_code ec;
			std::string why;
			bool found = false;
			g.fx = {};

			const auto zone_path = [&](const std::string& zone) -> fs::path
			{
				if (lower(zone).size() > 3 && lower(zone).substr(zone.size() - 3) == ".ff") { return fs::u8path(zone); }

				for (const auto& dir : { install / "client_shipRetail" / "patch", install / "client_shipRetail" / "chinese" })
				{
					if (fs::is_regular_file(dir / (zone + ".ff"), ec)) { return dir / (zone + ".ff"); }
				}

				return {};
			};

			if (!g.fx_zone.empty())
			{
				const fs::path file = zone_path(g.fx_zone);
				found = !file.empty() && fx_from_zone(file, g.name, why);
				if (file.empty()) { why = "there is no zone " + g.fx_zone; }
				if (found) { g.note = "read out of " + to_utf8(file.filename()) + " ( the compiled effect, turned back into curves )"; }
			}
			else
			{
				// the project's source file, else the zones the game would have it in: the map's, the common ones
				const fs::path source = assets::project_dir() / "fx" / fs::u8path(g.name + ".efx");

				if (!assets::project().empty() && fs::is_regular_file(source, ec))
				{
					if (const auto status = ca::load_efx(source, g.fx); status)
					{
						found = true;
						g.note = "the project's fx\\" + g.name + ".efx";
					}
					else
					{
						why = to_utf8(source.filename()) + ": " + status.message;
					}
				}
				else
				{
					std::vector<std::string> zones;
					if (!get_scene().map_name.empty()) { zones.push_back(get_scene().map_name); }
					zones.insert(zones.end(), { "common_mp", "common_gametype_mp", "code_post_gfx_mp" });

					for (const auto& zone : zones)
					{
						const fs::path file = zone_path(zone);

						if (!file.empty() && fx_from_zone(file, g.name, why))
						{
							found = true;
							g.note = "read out of " + to_utf8(file.filename()) + " ( the compiled effect, turned back into curves )";
							break;
						}
					}

					if (!found) { why = "no effect " + g.name + ": neither fx\\" + g.name + ".efx of the project nor in the zone of the map or the common zones ( name a zone: asset_preview fx <name> <zone> )"; }
				}
			}

			if (!found)
			{
				g.error = why;
				return;
			}

			// An element that names no atlas size of its own draws a material with an atlas all the same: the effects
			// compiler takes columns and rows from the material ( the table of the install's materials, made with the
			// first effect build ), and so does the preview - else a cell would show the whole sheet.
			{
				ca::FxAtlasLibrary atlas;

				if (atlas.load(assets::project_dir().parent_path() / "codol_fxatlas.txt"))
				{
					for (auto& e : g.fx.elems)
					{
						int columns = 1, rows = 1;

						if (e.atlasColIndexBits == 0 && e.atlasRowIndexBits == 0 && !e.visuals.empty() && atlas.find(e.visuals[0], columns, rows))
						{
							while ((1 << e.atlasColIndexBits) < columns) { e.atlasColIndexBits++; }
							while ((1 << e.atlasRowIndexBits) < rows) { e.atlasRowIndexBits++; }
						}
					}
				}
			}

			g.fx_duration = ca::fx_preview_duration(g.fx);
			g.fx_time = 0.0f;

			// what the camera turns around: everything the effect reaches during one pass
			float mins[3] = { 1e9f, 1e9f, 1e9f }, maxs[3] = { -1e9f, -1e9f, -1e9f };
			ca::FxPreview moment;

			for (int k = 0; k <= 24; k++)
			{
				ca::simulate_fx(g.fx, g.fx_duration * static_cast<float>(k) / 24.0f, moment);

				for (const auto& p : moment.particles)
				{
					const float reach = std::max(p.width, p.height) * 0.5f;
					for (int a = 0; a < 3; a++)
					{
						// what falls below the ground the effect stands on is not what one wants to look at
						const float at = a == 2 ? std::max(p.position[a], -8.0f) : p.position[a];
						mins[a] = std::min(mins[a], at - reach);
						maxs[a] = std::max(maxs[a], at + reach);
					}
				}
			}

			ca::simulate_fx(g.fx, 0.0f, moment);
			g.fx_skipped = moment.skipped;

			if (mins[0] > maxs[0])
			{
				g.centre = {};
				g.radius = 64.0f;
			}
			else
			{
				g.centre = { (mins[0] + maxs[0]) * 0.5f, (mins[1] + maxs[1]) * 0.5f, (mins[2] + maxs[2]) * 0.5f };
				const float dx = maxs[0] - mins[0], dy = maxs[1] - mins[1], dz = maxs[2] - mins[2];
				g.radius = std::clamp(0.5f * std::sqrt(dx * dx + dy * dy + dz * dz), 8.0f, 4000.0f);
			}
		}

		// a material is drawn additively when its name says light: the editor does not read blend functions here
		bool fx_additive(const std::string_view material)
		{
			for (const char* word : { "spark", "fire", "flare", "glow", "flash", "light", "tracer", "_add", "ember", "muzzle" })
			{
				if (material.find(word) != std::string_view::npos) { return true; }
			}

			return false;
		}

		// The effect at g.fx_time: every particle as a textured quad, far ones first, in the window's draw list. The
		// camera turns around the effect like the one of the 3D pictures.
		void draw_fx(const int w, const int h)
		{
			auto& s = get_scene();
			auto* dl = ImGui::GetWindowDrawList();
			const ImVec2 at = ImGui::GetCursorScreenPos();
			const ImVec2 end(at.x + static_cast<float>(w), at.y + static_cast<float>(h));
			dl->AddRectFilled(at, end, IM_COL32(42, 44, 48, 255));
			dl->PushClipRect(at, end, true);

			view::camera cam;
			cam.fov_x = 50.0f;
			cam.pitch = g.pitch;
			cam.yaw = g.yaw;
			const auto axes = cam.axes();
			const float tan_half = std::tan(cam.fov_x * 0.5f * PI / 180.0f);
			const float distance = g.radius / std::max(tan_half * std::min(1.0f, static_cast<float>(h) / static_cast<float>(w)), 0.05f) * 1.25f * g.zoom;
			const float eye[3] = { g.centre.x - axes.forward.x * distance, g.centre.y - axes.forward.y * distance, g.centre.z - axes.forward.z * distance };
			const float fwd[3] = { axes.forward.x, axes.forward.y, axes.forward.z }, right[3] = { -axes.left.x, -axes.left.y, -axes.left.z }, up[3] = { axes.up.x, axes.up.y, axes.up.z };
			const float focal = static_cast<float>(w) * 0.5f / tan_half;
			const float cx = at.x + static_cast<float>(w) * 0.5f, cy = at.y + static_cast<float>(h) * 0.5f;

			const auto project = [&](const float p[3], ImVec2& out) -> float
			{
				const float d[3] = { p[0] - eye[0], p[1] - eye[1], p[2] - eye[2] };
				const float z = d[0] * fwd[0] + d[1] * fwd[1] + d[2] * fwd[2];
				if (z < 0.5f) { return z; }
				out = ImVec2(cx + (d[0] * right[0] + d[1] * right[1] + d[2] * right[2]) / z * focal, cy - (d[0] * up[0] + d[1] * up[1] + d[2] * up[2]) / z * focal);
				return z;
			};

			// the ground the effect stands on: a grid of 32 units through its origin
			{
				// at most a dozen lines each way: a tall plume does not get a carpet
				const float step = 32.0f * std::max(1.0f, std::ceil(g.radius / 32.0f / 6.0f));
				const float reach = std::max(2.0f * step, std::ceil(g.radius / step) * step);
				for (float v = -reach; v <= reach + 0.5f; v += step)
				{
					const float a[3] = { v, -reach, 0 }, b[3] = { v, reach, 0 }, c[3] = { -reach, v, 0 }, d[3] = { reach, v, 0 };
					ImVec2 pa, pb, pc, pd;
					if (project(a, pa) >= 0.5f && project(b, pb) >= 0.5f) { dl->AddLine(pa, pb, v == 0.0f ? IM_COL32(110, 114, 122, 255) : IM_COL32(64, 67, 73, 255)); }
					if (project(c, pc) >= 0.5f && project(d, pd) >= 0.5f) { dl->AddLine(pc, pd, v == 0.0f ? IM_COL32(110, 114, 122, 255) : IM_COL32(64, 67, 73, 255)); }
				}
			}

			static ca::FxPreview moment;
			ca::simulate_fx(g.fx, g.fx_time, moment);

			struct drawn
			{
				const ca::FxParticle* p;
				float depth;
			};

			std::vector<drawn> order;
			order.reserve(moment.particles.size());

			for (const auto& p : moment.particles)
			{
				const float d[3] = { p.position[0] - eye[0], p.position[1] - eye[1], p.position[2] - eye[2] };
				order.push_back({ &p, d[0] * fwd[0] + d[1] * fwd[1] + d[2] * fwd[2] });
			}

			std::sort(order.begin(), order.end(), [](const drawn& a, const drawn& b) { return a.depth > b.depth; });

			g.fx_count = order.size();
			g.fx_missing = 0;
			int mode = 0;		// 0: as ImGui blends, 1: additive

			for (const auto& item : order)
			{
				const auto& p = *item.p;
				if (item.depth < 1.0f || p.width <= 0.0f || p.color[3] <= 0.003f) { continue; }

				// the picture of its material ( the colour map, whole: the cell is cut out by the texture coordinates )
				const std::string material(p.material);
				auto found = g.fx_pictures.find(material);

				if (found == g.fx_pictures.end())
				{
					found = g.fx_pictures.emplace(material, s.renderer.loaded() ? s.renderer.material_thumbnail(material, 512) : nullptr).first;
				}

				// two edges of the quad, half lengths
				float e1[3], e2[3], centre[3] = { p.position[0], p.position[1], p.position[2] };
				const float turn = p.rotation * PI / 180.0f, c = std::cos(turn), sn = std::sin(turn);

				if (p.shape == ca::FxShape::tail || p.shape == ca::FxShape::line)
				{
					float along[3] = { p.velocity[0], p.velocity[1], p.velocity[2] };
					float len = std::sqrt(along[0] * along[0] + along[1] * along[1] + along[2] * along[2]);
					if (len < 1e-4f) { along[0] = up[0]; along[1] = up[1]; along[2] = up[2]; len = 1.0f; }
					for (float& v : along) { v /= len; }
					// across: perpendicular to the way it flies and to the line of sight
					float across[3] = { along[1] * fwd[2] - along[2] * fwd[1], along[2] * fwd[0] - along[0] * fwd[2], along[0] * fwd[1] - along[1] * fwd[0] };
					float al = std::sqrt(across[0] * across[0] + across[1] * across[1] + across[2] * across[2]);
					if (al < 1e-4f) { across[0] = right[0]; across[1] = right[1]; across[2] = right[2]; al = 1.0f; }
					for (int k = 0; k < 3; k++)
					{
						e1[k] = across[k] / al * p.width * 0.5f;
						e2[k] = along[k] * p.height * 0.5f;
						centre[k] -= along[k] * p.height * 0.5f;
					}
				}
				else if (p.shape == ca::FxShape::oriented)
				{
					// in the plane across the effect's forward axis
					const float n[3] = { p.normal[0], p.normal[1], p.normal[2] };
					float a[3] = { n[1], -n[0], 0.0f };
					if (std::fabs(a[0]) + std::fabs(a[1]) < 1e-4f) { a[0] = 1.0f; a[1] = 0.0f; }
					const float al = std::sqrt(a[0] * a[0] + a[1] * a[1]);
					a[0] /= al; a[1] /= al;
					const float b[3] = { n[1] * a[2] - n[2] * a[1], n[2] * a[0] - n[0] * a[2], n[0] * a[1] - n[1] * a[0] };
					for (int k = 0; k < 3; k++)
					{
						e1[k] = (a[k] * c + b[k] * sn) * p.width * 0.5f;
						e2[k] = (b[k] * c - a[k] * sn) * p.height * 0.5f;
					}
				}
				else
				{
					for (int k = 0; k < 3; k++)
					{
						e1[k] = (right[k] * c + up[k] * sn) * p.width * 0.5f;
						e2[k] = (up[k] * c - right[k] * sn) * p.height * 0.5f;
					}
				}

				ImVec2 q[4];
				bool visible = true;
				const float sign[4][2] = { { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 } };		// top left, top right, bottom right, bottom left

				for (int k = 0; k < 4 && visible; k++)
				{
					const float corner[3] = { centre[0] + e1[0] * sign[k][0] + e2[0] * sign[k][1], centre[1] + e1[1] * sign[k][0] + e2[1] * sign[k][1], centre[2] + e1[2] * sign[k][0] + e2[2] * sign[k][1] };
					visible = project(corner, q[k]) >= 0.5f;
				}

				if (!visible) { continue; }

				const ImU32 colour = IM_COL32(static_cast<int>(p.color[0] * 255.0f), static_cast<int>(p.color[1] * 255.0f), static_cast<int>(p.color[2] * 255.0f), static_cast<int>(p.color[3] * 255.0f));
				const int wanted = fx_additive(p.material) ? 1 : 0;

				if (wanted != mode)
				{
					if (wanted)
					{
						dl->AddCallback([](const ImDrawList*, const ImDrawCmd*)
						{
							if (game::glob::d3d9_device)
							{
								game::glob::d3d9_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
								game::glob::d3d9_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
							}
						}, nullptr);
					}
					else
					{
						dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
					}

					mode = wanted;
				}

				if (found->second)
				{
					dl->AddImageQuad(found->second, q[0], q[1], q[2], q[3], ImVec2(p.u0, p.v0), ImVec2(p.u1, p.v0), ImVec2(p.u1, p.v1), ImVec2(p.u0, p.v1), colour);
				}
				else
				{
					// no picture ( the material is not in the zones of the open map ): a soft disc of its size and colour
					g.fx_missing++;
					const float radius = 0.5f * std::max(std::fabs(q[1].x - q[0].x) + std::fabs(q[1].y - q[0].y), std::fabs(q[3].x - q[0].x) + std::fabs(q[3].y - q[0].y)) * 0.6f;
					const ImVec2 mid((q[0].x + q[2].x) * 0.5f, (q[0].y + q[2].y) * 0.5f);
					dl->AddCircleFilled(mid, std::max(radius, 1.0f), (colour & 0x00FFFFFF) | (static_cast<ImU32>(p.color[3] * 110.0f) << 24), 20);
					dl->AddCircleFilled(mid, std::max(radius * 0.55f, 1.0f), (colour & 0x00FFFFFF) | (static_cast<ImU32>(p.color[3] * 150.0f) << 24), 16);
				}
			}

			if (mode)
			{
				dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
			}

			dl->PopClipRect();

			// turn with the left button, zoom with the wheel
			ImGui::InvisibleButton("##preview_fx_orbit", ImVec2(static_cast<float>(w), static_cast<float>(h)));

			if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f))
			{
				const auto& io = ImGui::GetIO();
				g.yaw -= io.MouseDelta.x * 0.5f;
				g.pitch = std::clamp(g.pitch + io.MouseDelta.y * 0.5f, -5.0f, 89.0f);
			}

			if (ImGui::IsItemHovered())
			{
				if (const float wheel = ImGui::GetIO().MouseWheel; wheel != 0.0f)
				{
					g.zoom = std::clamp(g.zoom * (wheel > 0.0f ? 0.88f : 1.14f), 0.2f, 6.0f);
				}
			}
		}

		void draw_fx_controls()
		{
			if (g.playing)
			{
				g.fx_time += ImGui::GetIO().DeltaTime * g.speed;

				if (g.fx_time > g.fx_duration)
				{
					if (g.fx_loop) { g.fx_time = std::fmod(g.fx_time, std::max(g.fx_duration, 0.05f)); }
					else { g.fx_time = g.fx_duration; g.playing = false; }
				}
			}

			if (ImGui::Button(g.playing ? "Pause" : "Play", ImVec2(64.0f, 0.0f)))
			{
				g.playing = !g.playing;
				if (g.playing && g.fx_time >= g.fx_duration) { g.fx_time = 0.0f; }
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(std::max(80.0f, ImGui::GetContentRegionAvail().x - 250.0f));
			if (ImGui::SliderFloat("##fx_time", &g.fx_time, 0.0f, g.fx_duration, "%.2f s")) { g.playing = false; }
			ImGui::SameLine();
			ImGui::SetNextItemWidth(70.0f);
			ImGui::DragFloat("##fx_speed", &g.speed, 0.01f, 0.05f, 4.0f, "x %.2f");
			ImGui::SameLine();
			ImGui::Checkbox("Loop", &g.fx_loop);
			ImGui::SameLine();
			ImGui::TextColored(dim, "%d particles", static_cast<int>(g.fx_count));

			ImGui::PushTextWrapPos(0.0f);
			ImGui::TextColored(dim, "%s. %d element( s ), one pass %.2f s. A preview by the editor, not the game's effect system: sprites, tails and lines as flat pictures of their material's "
				"colour map, blended ( added where the material's name says light ), no depth, no lighting.", g.note.c_str(), static_cast<int>(g.fx.elems.size()), g.fx_duration);

			if (g.fx_missing)
			{
				ImGui::TextColored(yellow, "%d particle( s ) as plain discs: their material is not in the zones of the open map.", static_cast<int>(g.fx_missing));
			}

			for (const auto& line : g.fx_skipped)
			{
				ImGui::TextColored(yellow, "%s", line.c_str());
			}

			ImGui::PopTextWrapPos();
		}

		void make_objects()
		{
			g.dirty = false;
			g.error.clear();
			auto& s = get_scene();

			if (g.what == kind::fx)
			{
				make_fx();
				return;
			}

			if (g.what == kind::image)
			{
				make_image();
				return;
			}

			if (g.what == kind::sound)
			{
				read_csv(g.file, g.sounds, g.error);
				return;
			}

			if (!s.renderer.loaded())
			{
				g.error = "Open a map: the preview is drawn by the renderer of the open map.";
				return;
			}

			if (g.what == kind::model) { make_model(); }
			else if (g.what == kind::material) { make_material(); }
			else if (g.what == kind::anim) { make_anim(); }
		}

		void draw_anim_controls()
		{
			const float last = static_cast<float>(std::max<std::size_t>(g.anim.frames.size(), 1) - 1);
			const float rate = g.anim.framerate > 0.0f ? g.anim.framerate : 30.0f;

			if (g.playing && last > 0.0f)
			{
				g.frame += ImGui::GetIO().DeltaTime * rate * g.speed;

				if (g.frame > last)
				{
					if (g.loop)
					{
						g.frame = std::fmod(g.frame, last);
					}
					else
					{
						g.frame = last;
						g.playing = false;
					}
				}
			}

			if (ImGui::Button(g.playing ? "Pause" : "Play", ImVec2(64.0f, 0.0f)))
			{
				if (!g.playing && g.frame >= last) { g.frame = 0.0f; }
				g.playing = !g.playing;
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(-1.0f);

			char label[64];
			std::snprintf(label, sizeof(label), "frame %%.1f / %.0f", last);
			if (ImGui::SliderFloat("##timeline", &g.frame, 0.0f, std::max(last, 0.001f), label, ImGuiSliderFlags_AlwaysClamp))
			{
				g.playing = false;
			}

			// the note tracks, on the timeline
			if (!g.anim.notes.empty() && last > 0.0f)
			{
				const ImVec2 lo = ImGui::GetItemRectMin(), hi = ImGui::GetItemRectMax();
				auto* dl = ImGui::GetWindowDrawList();

				for (const auto& note : g.anim.notes)
				{
					const float x = lo.x + (hi.x - lo.x) * std::clamp((note.frame - static_cast<float>(g.anim.firstFrame)) / last, 0.0f, 1.0f);
					dl->AddLine(ImVec2(x, hi.y - 5.0f), ImVec2(x, hi.y), IM_COL32(255, 200, 80, 255), 2.0f);
				}
			}

			ImGui::Checkbox("Loop", &g.loop);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(110.0f);
			ImGui::SliderFloat("##speed", &g.speed, 0.1f, 2.0f, "speed %.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::SameLine();
			ImGui::TextColored(dim, "%.2f s at %.0f fps, %d parts", last / rate, rate, static_cast<int>(g.anim.parts.size()));

			// the model: any model of the project can stand in
			const auto models = assets::project_models();

			if (!models.empty())
			{
				ImGui::SetNextItemWidth(220.0f);

				if (ImGui::BeginCombo("Model", g.anim_model.empty() ? "( a file )" : g.anim_model.c_str()))
				{
					for (const auto& name : models)
					{
						if (ImGui::Selectable(name.c_str(), name == g.anim_model) && name != g.anim_model)
						{
							const std::string anim = g.name;
							show_anim(anim, name);
						}
					}

					ImGui::EndCombo();
				}
			}

			if (!g.anim.notes.empty())
			{
				std::string notes = "notes:";
				for (const auto& note : g.anim.notes)
				{
					char text[96];
					std::snprintf(text, sizeof(text), "  %s @ %.0f", note.name.c_str(), note.frame - static_cast<float>(g.anim.firstFrame));
					notes += text;
				}

				ImGui::PushTextWrapPos(0.0f);
				ImGui::TextColored(dim, "%s", notes.c_str());
				ImGui::PopTextWrapPos();
			}
		}

		void draw_sounds()
		{
			ImGui::TextColored(dim, "%s: %d rows", to_utf8(g.file.filename()).c_str(), static_cast<int>(g.sounds.size()));
			ImGui::SameLine();

			if (ImGui::SmallButton("Stop"))
			{
				stop_playing();
				g.sound_message.clear();
			}

			if (ImGui::BeginChild("##sound_rows", ImVec2(0.0f, -ImGui::GetFrameHeightWithSpacing() * 4.4f), true))
			{
				for (std::size_t i = 0; i < g.sounds.size(); i++)
				{
					const auto& row = g.sounds[i];
					ImGui::PushID(static_cast<int>(i));
					ImGui::BeginDisabled(!row.found);

					if (ImGui::SmallButton("Play"))
					{
						if (!play_file(row.path, g.sound_message))
						{
							game::printf_to_console("^1[preview] %s\n", g.sound_message.c_str());
						}
					}

					ImGui::EndDisabled();
					ImGui::SameLine();
					ImGui::TextUnformatted(row.alias.c_str());
					ImGui::SameLine();
					ImGui::TextColored(row.found ? dim : yellow, "%s%s%s%s", row.file.c_str(), row.streamed ? "  streamed" : "", row.looping ? "  looping" : "",
						row.found ? "" : row.file.empty() ? "  no file" : "  not in the project's sound folder: a sound of the game");
					ImGui::PopID();
				}
			}

			ImGui::EndChild();

			// an alias as the game has it: from the project's zone, or from a zone of the game
			const float half = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
			ImGui::SetNextItemWidth(half);
			ImGui::InputTextWithHint("##zone_alias", "alias", &g.zone_alias);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(half);
			ImGui::InputTextWithHint("##zone_name", "zone ( empty: the project's )", &g.zone_name);

			if (ImGui::Button("Play from zone") && !g.zone_alias.empty())
			{
				play_zone_alias(g.zone_alias, g.zone_name, g.sound_message);
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Reads the alias and its loaded sound out of a fastfile - the project's zone as it was built, or a zone of the game\n( common_mp, mp_rust .. ) - and plays it. Loaded sounds only; streamed ones are files of the game's archives.");
			}

			if (!g.sound_message.empty())
			{
				ImGui::PushTextWrapPos(0.0f);
				ImGui::TextUnformatted(g.sound_message.c_str());
				ImGui::PopTextWrapPos();
			}
		}
	}

	// ---- subjects ----------------------------------------------------------------------------------------------

	void show_model(const std::string& name)
	{
		if (g.what == kind::model && g.name == name)
		{
			ui::get().show_preview = true;
			return;
		}

		begin(kind::model);
		g.name = name;
	}

	void show_material(const std::string& name)
	{
		if (g.what == kind::material && g.name == name)
		{
			ui::get().show_preview = true;
			return;
		}

		begin(kind::material);
		g.name = name;
	}

	void show_image(const fs::path& file)
	{
		begin(kind::image);
		g.file = file;
		g.name = to_utf8(file.filename());
	}

	void show_anim(const std::string& name, const std::string& model)
	{
		begin(kind::anim);
		g.name = name;
		g.anim_model = model;
	}

	void show_sounds(const fs::path& csv)
	{
		begin(kind::sound);
		g.file = csv;
		g.name = to_utf8(csv.filename());
	}

	void show_fx(const std::string& name, const std::string& zone)
	{
		begin(kind::fx);
		g.name = name;
		g.fx_zone = zone;
		g.playing = true;
		g.speed = 1.0f;
		g.pitch = 12.0f;
		g.zoom = 0.75f;
	}

	bool play_alias(const std::string& alias, const std::string& zone, std::string& message)
	{
		// a row of one of the project's csv files first: its source file
		if (zone.empty())
		{
			for (const auto& entry : assets::items())
			{
				if (entry.type != assets::kind::sound)
				{
					continue;
				}

				std::vector<sound_row> rows;
				std::string error;
				read_csv(entry.file, rows, error);

				for (const auto& row : rows)
				{
					if (lower(row.alias) == lower(alias) && row.found)
					{
						return play_file(row.path, message);
					}
				}
			}
		}

		return play_zone_alias(alias, zone, message);
	}

	void stop_sound()
	{
		stop_playing();
	}

	void close()
	{
		stop_playing();
		clear_subject();
		ui::get().show_preview = false;
	}

	void assets_changed()
	{
		if (g.what == kind::model || g.what == kind::material || g.what == kind::anim || g.what == kind::fx)
		{
			drop_objects(true);
			g.dirty = true;
			g.shown_frame = -1.0f;
		}
	}

	void renderer_changed()
	{
		// the handles belonged to the map that was loaded
		g.model_handle = 0;
		g.meshes.clear();
		g.picture = nullptr;
		g.fx_pictures.clear();

		if (g.what == kind::model || g.what == kind::material || g.what == kind::anim)
		{
			g.dirty = true;
			g.shown_frame = -1.0f;
		}
	}

	void device_lost()
	{
		release_target();
	}

	void frame()
	{
		auto& st = ui::get();

		if (!st.show_preview)
		{
			// nothing is kept in the renderer for a closed window
			if (g.what != kind::none)
			{
				stop_playing();
				clear_subject();
			}

			return;
		}

		// over the views, not over the lists that are clicked to fill it
		ImGui::SetNextWindowSize(ImVec2(460.0f, 560.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.55f));

		if (!ImGui::Begin("Preview##window", &st.show_preview, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return;
		}

		if (g.what == kind::none)
		{
			ImGui::PushTextWrapPos(0.0f);
			ImGui::TextDisabled("Click a model in the Models window, a material in the Materials window or a line of the Assets window: it is shown here before you use it.");
			ImGui::PopTextWrapPos();
			ImGui::End();
			return;
		}

		if (g.dirty)
		{
			make_objects();
		}

		static const char* const words[] = { "", "model", "material", "image", "animation", "sounds", "effect" };
		ImGui::TextColored(dim, "%s", words[static_cast<int>(g.what)]);
		ImGui::SameLine();
		ImGui::TextUnformatted(g.name.c_str());

		if (!g.error.empty())
		{
			ImGui::PushTextWrapPos(0.0f);
			ImGui::TextColored(red, "%s", g.error.c_str());
			ImGui::PopTextWrapPos();
			ImGui::End();
			return;
		}

		const ImVec2 avail = ImGui::GetContentRegionAvail();
		const float line = ImGui::GetFrameHeightWithSpacing();
		auto& s = get_scene();

		switch (g.what)
		{
		case kind::model:
		{
			const int w = std::max(64, static_cast<int>(avail.x)), h = std::max(64, static_cast<int>(avail.y - line * 2.4f));
			draw_scene(w, h);
			ImGui::TextColored(dim, "%s   drag: turn, wheel: zoom", g.note.c_str());

			if (ImGui::Button("Place in the map"))
			{
				const auto target = edit::snap_point(s.ray_target(s.cam.origin, s.cam.axes().forward), s.snap ? s.grid : 0.0);
				if (!s.create_model_entity(g.name, target, 0.0))
				{
					ImGui::Toast(ImGuiToastType_Error, "Preview", s.doc.last_error().c_str());
				}
			}

			ImGui::SameLine();
			ImGui::TextColored(dim, "a %s where the camera of the 3D view looks", s.model_class.c_str());
			break;
		}

		case kind::material:
		{
			const float half = std::max(64.0f, (avail.y - line * 4.6f) * 0.5f);
			draw_scene(std::max(64, static_cast<int>(avail.x)), static_cast<int>(half));
			draw_picture(g.picture, g.picture_w > 0 ? g.picture_w : 256, g.picture_h > 0 ? g.picture_h : 256, avail.x, half);

			ImGui::PushTextWrapPos(0.0f);
			ImGui::TextColored(dim, "%s, colour map %s ( %d x %d )%s%s%s   zone %s", g.material.technique_set.c_str(), g.material.color_map.empty() ? "none" : g.material.color_map.c_str(),
				g.material.width, g.material.height, g.material.world_material ? ", world material" : "", g.material.blends ? ", blended" : "", g.material.alpha_tested ? ", alpha tested" : "",
				g.material.zone.c_str());
			ImGui::TextColored(dim, "%s", g.note.c_str());
			ImGui::PopTextWrapPos();

			if (ImGui::Button("Use for new brushes"))
			{
				s.doc.set_default_material(g.name);
			}

			ImGui::SameLine();
			ImGui::BeginDisabled(s.doc.selection().empty() && s.doc.selected_faces().empty());

			if (ImGui::Button("Apply to the selection"))
			{
				s.doc.set_default_material(g.name);
				ui::report(s.doc.set_material(g.name), "Set material");
			}

			ImGui::EndDisabled();
			break;
		}

		case kind::image:
			draw_picture(g.own_picture, g.picture_w, g.picture_h, avail.x, avail.y - line * 2.4f);
			ImGui::TextColored(dim, "%s", g.image_info.c_str());

			if (ImGui::Button("New material from it .."))
			{
				assets::open_material_dialog({ g.file });
			}

			break;

		case kind::anim:
		{
			const int w = std::max(64, static_cast<int>(avail.x)), h = std::max(64, static_cast<int>(avail.y - line * 5.6f));

			if (g.frame != g.shown_frame)
			{
				skin(g.frame);
				upload_meshes();
				g.shown_frame = g.frame;
			}

			draw_scene(w, h);
			draw_anim_controls();
			ImGui::PushTextWrapPos(0.0f);
			ImGui::TextColored(dim, "%s", g.note.c_str());
			ImGui::PopTextWrapPos();
			break;
		}

		case kind::sound:
			draw_sounds();
			break;

		case kind::fx:
		{
			const float below = line * (3.6f + static_cast<float>(std::min<std::size_t>(g.fx_skipped.size(), 6)));
			draw_fx(std::max(64, static_cast<int>(avail.x)), std::max(64, static_cast<int>(avail.y - below)));
			draw_fx_controls();
			break;
		}

		default:
			break;
		}

		ImGui::End();
	}

	void register_commands()
	{
		using args_t = const std::vector<std::string>&;

		// asset_preview model <xmodel> | material <name> | image <file> | anim <name> [model] | sounds <csv of the project> |
		// frame <n> | play | pause | orbit <yaw> <pitch> [zoom] | close
		command::register_command("asset_preview", [](args_t args)
		{
			const std::string what = args.size() > 1 ? args[1] : "";
			const std::string name = args.size() > 2 ? args[2] : "";

			if (what == "model" && !name.empty()) { show_model(name); }
			else if (what == "material" && !name.empty()) { show_material(name); }
			else if (what == "image" && !name.empty())
			{
				fs::path file = fs::u8path(name);
				if (!file.is_absolute()) { file = assets::project_dir() / "images" / file; }
				show_image(file);
			}
			else if (what == "anim" && !name.empty()) { show_anim(name, args.size() > 3 ? args[3] : std::string()); }
			else if (what == "sounds" && !name.empty())
			{
				fs::path file = fs::u8path(name);
				if (!file.is_absolute()) { file = assets::project_dir() / "soundaliases" / file; }
				show_sounds(file);
			}
			else if (what == "fx" && !name.empty()) { show_fx(name, args.size() > 3 ? args[3] : std::string()); }
			else if (what == "time" && !name.empty()) { g.fx_time = static_cast<float>(std::atof(name.c_str())); g.playing = false; }
			else if (what == "frame" && !name.empty()) { g.frame = static_cast<float>(std::atof(name.c_str())); g.playing = false; }
			else if (what == "play") { g.playing = true; }
			else if (what == "pause") { g.playing = false; }
			else if (what == "orbit" && args.size() > 3)
			{
				g.yaw = static_cast<float>(std::atof(args[2].c_str()));
				g.pitch = static_cast<float>(std::atof(args[3].c_str()));
				if (args.size() > 4) { g.zoom = static_cast<float>(std::atof(args[4].c_str())); }
			}
			else if (what == "close") { close(); }
			else if (what == "status")
			{
				game::printf_to_console("[preview] %s %s: %s%s  frame %.1f of %d, %s, %d meshes, model handle %d\n", what.c_str(), g.name.c_str(), g.error.empty() ? "ok" : g.error.c_str(),
					g.note.empty() ? "" : ( "  " + g.note ).c_str(), g.frame, static_cast<int>(g.anim.frames.size()), g.playing ? "playing" : "paused", static_cast<int>(g.meshes.size()), g.model_handle);

				if (g.what == kind::fx)
				{
					game::printf_to_console("[preview] effect: %d elements, one pass %.2f s, at %.2f s: %d particles ( %d without a picture ), %d elements not drawn\n", static_cast<int>(g.fx.elems.size()),
						g.fx_duration, g.fx_time, static_cast<int>(g.fx_count), static_cast<int>(g.fx_missing), static_cast<int>(g.fx_skipped.size()));
					for (const auto& skipped : g.fx_skipped) { game::printf_to_console("[preview]   %s\n", skipped.c_str()); }

					// the first particles, to check a picture against numbers
					ca::FxPreview moment;
					ca::simulate_fx(g.fx, g.fx_time, moment);
					for (std::size_t k = 0; k < moment.particles.size() && k < 4; k++)
					{
						const auto& p = moment.particles[k];
						game::printf_to_console("[preview]   particle %d of element %d: %s at %.1f %.1f %.1f, %.1f x %.1f, colour %.2f %.2f %.2f alpha %.2f, age %.2f of %.2f s\n", p.index, p.element,
							std::string(p.material).c_str(), p.position[0], p.position[1], p.position[2], p.width, p.height, p.color[0], p.color[1], p.color[2], p.color[3], p.age, p.life);
					}
					game::printf_to_console("[preview]   camera around %.1f %.1f %.1f, radius %.1f\n", g.centre.x, g.centre.y, g.centre.z, g.radius);
				}
			}
			else
			{
				game::printf_to_console("usage: asset_preview model <xmodel> | material <name> | image <file> | anim <name> [model] | sounds <csv> | fx <name> [zone | file.ff] | time <seconds> | frame <n> | play | pause | orbit <yaw> <pitch> [zoom] | status | close\n");
			}
		});

		// asset_play <alias> [zone | file.ff]: the source file of an alias of the project, else its loaded sound out of a zone
		command::register_command("asset_play", [](args_t args)
		{
			if (args.size() < 2)
			{
				game::printf_to_console("usage: asset_play <alias> [zone of the game | file.ff]   ( asset_play stop )\n");
				return;
			}

			if (args[1] == "stop")
			{
				stop_playing();
				return;
			}

			std::string message;
			const bool ok = play_alias(args[1], args.size() > 2 ? args[2] : std::string(), message);
			game::printf_to_console("%s[preview] %s\n", ok ? "" : "^1", message.c_str());
		});
	}
}
