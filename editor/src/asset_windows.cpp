#include "std_include.hpp"

// windows.h macros
#undef min
#undef max

#include "host.hpp"
#include "ui.hpp"
#include "assets.hpp"
#include "preview.hpp"

#include "codol/path_picker.hpp"

// The Assets window and the windows that make assets: New material, Import model, Import animation, GDT materials.
// They only collect what the functions of assets.hpp need - the console commands call the same functions.

namespace editor::assets
{
	namespace fs = std::filesystem;

	namespace
	{
		constexpr ImVec4 red = ImVec4(0.96f, 0.45f, 0.40f, 1.0f);
		constexpr ImVec4 yellow = ImVec4(0.95f, 0.80f, 0.35f, 1.0f);
		constexpr ImVec4 green = ImVec4(0.48f, 0.85f, 0.60f, 1.0f);
		constexpr ImVec4 dim = ImVec4(0.62f, 0.64f, 0.68f, 1.0f);

		std::string lower(std::string text)
		{
			for (char& c : text)
			{
				c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			}

			return text;
		}

		// Explorer's "open file" window, started at the file of the field; `filter` as codol::ui::pick_path wants it
		bool browse(const char* filter, std::string& out)
		{
			return codol::ui::pick_path(codol::ui::path_kind::open_file, out, filter, "Select a file", main_window());
		}

		constexpr const char* image_filter = "Images ( *.png, *.tga, *.dds )|*.png;*.tga;*.dds|All files|*.*";
		constexpr const char* export_filter = "xmodel_export|*.xmodel_export|All files|*.*";
		constexpr const char* gdt_filter = "GDT ( *.gdt )|*.gdt|All files|*.*";
		constexpr const char* anim_filter = "xanim_export|*.xanim_export|All files|*.*";
		constexpr const char* skeleton_filter = "xmodel_export ( the model of the animation )|*.xmodel_export|All files|*.*";
		constexpr const char* csv_filter = "Sound aliases ( *.csv )|*.csv|All files|*.*";
		constexpr const char* efx_filter = "Effects ( *.efx )|*.efx|All files|*.*";

		// one line: label, path, ".."
		bool path_row(const char* label, const char* id, std::string& path, const char* filter, const float label_width = 110.0f)
		{
			bool changed = false;
			ImGui::PushID(id);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(label);
			ImGui::SameLine(label_width);
			ImGui::SetNextItemWidth(-44.0f);
			changed |= ImGui::InputText("##path", &path);
			ImGui::SameLine();

			if (ImGui::Button("..", ImVec2(36.0f, 0.0f)))
			{
				changed |= browse(filter, path);
			}

			ImGui::PopID();
			return changed;
		}

		bool ends_with(const std::string& text, const char* tail)
		{
			const std::size_t n = std::strlen(tail);
			return text.size() > n && text.compare(text.size() - n, n, tail) == 0;
		}

		// ---- New material ----------------------------------------------------------------------------------

		struct material_dialog
		{
			bool open = false;
			bool appearing = false;
			std::string name, color, normal, specular;
			int kind = 0;
			int surface = 0;
			bool overwrite = false;
			std::string error;
		};

		material_dialog g_material;

		void draw_material_dialog()
		{
			auto& d = g_material;
			if (!d.open)
			{
				return;
			}

			ImGui::SetNextWindowSize(ImVec2(640.0f, 0.0f), ImGuiCond_Appearing);
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			if (d.appearing)
			{
				ImGui::SetNextWindowFocus();
				d.appearing = false;
			}

			if (ImGui::Begin("New material##assets", &d.open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize))
			{
				const auto& kinds = material_kinds();
				const auto& surfaces = surface_types();

				ImGui::PushItemWidth(520.0f);
				path_row("Colour map", "color", d.color, image_filter);
				path_row("Normal map", "normal", d.normal, image_filter);
				path_row("Specular map", "specular", d.specular, image_filter);
				ImGui::PopItemWidth();
				ImGui::TextColored(dim, "PNG, TGA or DDS; sizes that are powers of two. Normal and specular map are optional.");
				ImGui::Spacing();

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Kind");
				ImGui::SameLine(110.0f);
				ImGui::SetNextItemWidth(330.0f);

				if (ImGui::BeginCombo("##kind", kinds[static_cast<std::size_t>(d.kind)].label))
				{
					for (int i = 0; i < static_cast<int>(kinds.size()); i++)
					{
						if (ImGui::Selectable(kinds[static_cast<std::size_t>(i)].label, i == d.kind)) { d.kind = i; }
					}

					ImGui::EndCombo();
				}

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Surface type");
				ImGui::SameLine(110.0f);
				ImGui::SetNextItemWidth(330.0f);

				if (ImGui::BeginCombo("##surface", d.surface ? surfaces[static_cast<std::size_t>(d.surface)].c_str() : "( none )"))
				{
					for (int i = 0; i < static_cast<int>(surfaces.size()); i++)
					{
						if (ImGui::Selectable(i ? surfaces[static_cast<std::size_t>(i)].c_str() : "( none )", i == d.surface)) { d.surface = i; }
					}

					ImGui::EndCombo();
				}

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Name");
				ImGui::SameLine(110.0f);
				ImGui::TextColored(dim, "%s", kinds[static_cast<std::size_t>(d.kind)].prefix);
				ImGui::SameLine(0.0f, 2.0f);
				ImGui::SetNextItemWidth(300.0f);
				ImGui::InputTextWithHint("##name", "name of the material", &d.name);
				ImGui::SameLine();
				ImGui::Checkbox("Overwrite", &d.overwrite);

				ImGui::Spacing();

				const bool can = !d.color.empty() && !d.name.empty() && !project().empty();
				if (!can) { ImGui::BeginDisabled(); }

				if (ImGui::Button("Create and build", ImVec2(170.0f, 0.0f)))
				{
					material_request request;
					request.name = d.name;
					request.kind = kinds[static_cast<std::size_t>(d.kind)].id;
					request.color = fs::u8path(d.color);
					request.normal = fs::u8path(d.normal);
					request.specular = fs::u8path(d.specular);
					request.surface_type = surfaces[static_cast<std::size_t>(d.surface)];
					request.overwrite = d.overwrite;

					std::string made;
					if (new_material(request, d.error, &made))
					{
						// the material of new brushes, as a click in the browser would make it
						get_scene().doc.set_default_material(made);
						ui::get().show_assets = true;
						d.open = false;
						d.error.clear();
					}
				}

				if (!can) { ImGui::EndDisabled(); }

				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(110.0f, 0.0f)))
				{
					d.open = false;
				}

				if (project().empty())
				{
					ImGui::TextColored(yellow, "Open a map first: the assets belong to the project of a map.");
				}

				if (!d.error.empty())
				{
					ImGui::PushTextWrapPos(600.0f);
					ImGui::TextColored(red, "%s", d.error.c_str());
					ImGui::PopTextWrapPos();
				}

				const std::string& prefix = kinds[static_cast<std::size_t>(d.kind)].prefix;
				if (prefix == "wc/")
				{
					ImGui::PushTextWrapPos(600.0f);
					ImGui::TextColored(dim, "A new world material is drawn in the editor. In a compiled map new faces with it collide but are not drawn: "
						"codol_mapc only draws materials the world of the base map already uses.");
					ImGui::PopTextWrapPos();
				}
			}

			ImGui::End();
		}

		// ---- Import model ----------------------------------------------------------------------------------

		struct model_dialog
		{
			bool open = false;
			bool appearing = false;
			std::string name;
			struct lod
			{
				std::string file;
				float distance = 0.0f;
			};
			std::vector<lod> lods;
			int collision = 0;			// 0: none, n: LOD n - 1
			std::vector<std::pair<std::string, std::string>> materials;		// export material -> material
			std::string materials_of;	// the LOD files the table was read from
			bool overwrite = false;
			std::string error;
		};

		model_dialog g_model;

		void reset_distances(model_dialog& d)
		{
			for (std::size_t i = 0; i < d.lods.size(); i++)
			{
				d.lods[i].distance = i + 1 >= d.lods.size() ? 1000000.0f : 600.0f * static_cast<float>(1u << i);
			}
		}

		// the materials the LOD files name; what the user typed for one stays
		void read_model_materials(model_dialog& d)
		{
			std::string key;
			for (const auto& lod : d.lods) { key += lod.file + "|"; }

			if (key == d.materials_of)
			{
				return;
			}

			d.materials_of = key;
			std::vector<std::string> names;

			for (const auto& lod : d.lods)
			{
				std::error_code ec;
				std::string error;

				if (!lod.file.empty() && fs::is_regular_file(fs::u8path(lod.file), ec))
				{
					read_export_materials(fs::u8path(lod.file), names, error);
				}
			}

			std::vector<std::pair<std::string, std::string>> table;

			for (const auto& name : names)
			{
				const auto known = std::find_if(d.materials.begin(), d.materials.end(), [&](const auto& row) { return row.first == name; });
				table.emplace_back(name, known != d.materials.end() ? known->second : default_material_for(name));
			}

			d.materials = std::move(table);
		}

		void draw_model_dialog()
		{
			auto& d = g_model;
			if (!d.open)
			{
				return;
			}

			ImGui::SetNextWindowSize(ImVec2(720.0f, 0.0f), ImGuiCond_Appearing);
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			if (d.appearing)
			{
				ImGui::SetNextWindowFocus();
				d.appearing = false;
			}

			if (ImGui::Begin("Import model##assets", &d.open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Name");
				ImGui::SameLine(110.0f);
				ImGui::SetNextItemWidth(330.0f);
				ImGui::InputTextWithHint("##name", "name of the model", &d.name);
				ImGui::SameLine();
				ImGui::Checkbox("Overwrite", &d.overwrite);

				ImGui::Spacing();
				ImGui::TextColored(dim, "One xmodel_export per LOD, the nearest first; each is drawn up to its distance.");

				int remove = -1;

				for (int i = 0; i < static_cast<int>(d.lods.size()); i++)
				{
					auto& lod = d.lods[static_cast<std::size_t>(i)];
					ImGui::PushID(i);
					ImGui::AlignTextToFramePadding();
					ImGui::Text("LOD %d", i);
					ImGui::SameLine(110.0f);
					ImGui::SetNextItemWidth(380.0f);
					ImGui::InputText("##file", &lod.file);
					ImGui::SameLine();

					if (ImGui::Button("..", ImVec2(36.0f, 0.0f)))
					{
						browse(export_filter, lod.file);
					}

					ImGui::SameLine();
					ImGui::SetNextItemWidth(100.0f);
					ImGui::InputFloat("##distance", &lod.distance, 0.0f, 0.0f, "%.0f");
					ImGui::SameLine();

					if (d.lods.size() > 1 && ImGui::Button("x", ImVec2(28.0f, 0.0f)))
					{
						remove = i;
					}

					ImGui::PopID();
				}

				if (remove >= 0)
				{
					d.lods.erase(d.lods.begin() + remove);
					reset_distances(d);
					d.collision = std::min(d.collision, static_cast<int>(d.lods.size()));
				}

				if (d.lods.size() < 4 && ImGui::Button("Add LOD"))
				{
					d.lods.emplace_back();
					reset_distances(d);
				}

				// a name from the first file
				if (d.name.empty() && !d.lods.empty() && !d.lods[0].file.empty())
				{
					std::string stem = lower(to_utf8(fs::u8path(d.lods[0].file).stem()));
					for (const char* tail : { "_lod0", "_lod1", "_lod2", "_lod3" })
					{
						if (ends_with(stem, tail)) { stem.resize(stem.size() - std::strlen(tail)); }
					}

					std::error_code ec;
					if (fs::is_regular_file(fs::u8path(d.lods[0].file), ec)) { d.name = stem; }
				}

				ImGui::Spacing();
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Collision");
				ImGui::SameLine(110.0f);
				ImGui::SetNextItemWidth(330.0f);

				const auto collision_label = [](const int index) { return index == 0 ? std::string("none ( bullets and players pass )") : "from LOD " + std::to_string(index - 1) + " ( solid )"; };

				if (ImGui::BeginCombo("##collision", collision_label(d.collision).c_str()))
				{
					for (int i = 0; i <= static_cast<int>(d.lods.size()); i++)
					{
						if (ImGui::Selectable(collision_label(i).c_str(), i == d.collision)) { d.collision = i; }
					}

					ImGui::EndCombo();
				}

				// ---- materials
				read_model_materials(d);
				ImGui::Spacing();
				bool all_exist = true;

				if (d.materials.empty())
				{
					ImGui::TextColored(dim, "Materials: none read yet ( name a LOD file ).");
				}
				else if (ImGui::BeginTable("##materials", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit))
				{
					ImGui::TableSetupColumn("material of the export", ImGuiTableColumnFlags_WidthFixed, 230.0f);
					ImGui::TableSetupColumn("material in the game", ImGuiTableColumnFlags_WidthFixed, 300.0f);
					ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 110.0f);
					ImGui::TableHeadersRow();

					for (int i = 0; i < static_cast<int>(d.materials.size()); i++)
					{
						auto& row = d.materials[static_cast<std::size_t>(i)];
						ImGui::PushID(i);
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::AlignTextToFramePadding();
						ImGui::TextUnformatted(row.first.c_str());
						ImGui::TableNextColumn();
						ImGui::SetNextItemWidth(-1.0f);
						ImGui::InputText("##to", &row.second);
						ImGui::TableNextColumn();

						const bool exists = material_exists(row.second);
						all_exist &= exists;
						ImGui::TextColored(exists ? green : red, exists ? "exists" : "does not exist");
						ImGui::PopID();
					}

					ImGui::EndTable();
				}

				if (!all_exist)
				{
					ImGui::PushTextWrapPos(680.0f);
					ImGui::TextColored(yellow, "A material that does not exist fails the model: make it first ( Materials > New material, kind Model ) or name one of the game.");
					ImGui::PopTextWrapPos();
				}

				ImGui::Spacing();

				const bool can = !d.name.empty() && !d.lods.empty() && !d.lods[0].file.empty() && !project().empty();
				if (!can) { ImGui::BeginDisabled(); }

				if (ImGui::Button("Import and build", ImVec2(170.0f, 0.0f)))
				{
					model_request request;
					request.name = d.name;
					request.collision_lod = d.collision - 1;
					request.overwrite = d.overwrite;

					for (const auto& lod : d.lods)
					{
						if (!lod.file.empty()) { request.lods.push_back({ fs::u8path(lod.file), lod.distance }); }
					}

					for (const auto& [from, to] : d.materials)
					{
						if (from != to) { request.materials[from] = to; }
					}

					if (import_model(request, d.error))
					{
						ui::get().show_assets = true;
						d.open = false;
						d.error.clear();
					}
				}

				if (!can) { ImGui::EndDisabled(); }

				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(110.0f, 0.0f)))
				{
					d.open = false;
				}

				if (project().empty())
				{
					ImGui::TextColored(yellow, "Open a map first: the assets belong to the project of a map.");
				}

				if (!d.error.empty())
				{
					ImGui::PushTextWrapPos(680.0f);
					ImGui::TextColored(red, "%s", d.error.c_str());
					ImGui::PopTextWrapPos();
				}
			}

			ImGui::End();
		}

		// ---- Import animation --------------------------------------------------------------------------------

		struct anim_dialog
		{
			bool open = false;
			bool appearing = false;
			std::string file, name, model, tree;
			int type = 0;				// relative, delta, delta3d
			bool loop = false;
			bool overwrite = false;
			std::string error;
		};

		anim_dialog g_anim;

		void draw_anim_dialog()
		{
			auto& d = g_anim;
			if (!d.open)
			{
				return;
			}

			ImGui::SetNextWindowSize(ImVec2(680.0f, 0.0f), ImGuiCond_Appearing);
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			if (d.appearing)
			{
				ImGui::SetNextWindowFocus();
				d.appearing = false;
			}

			if (ImGui::Begin("Import animation##assets", &d.open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize))
			{
				static const char* const types[] = { "relative", "delta", "delta3d" };
				static const char* const type_labels[] = { "Relative ( every part moves on its parent )", "Delta ( the movement of tag_origin moves the entity )",
					"Delta 3D ( as delta, with the full rotation )" };

				ImGui::PushItemWidth(520.0f);
				path_row("xanim_export", "file", d.file, anim_filter);
				ImGui::PopItemWidth();

				if (d.name.empty() && !d.file.empty())
				{
					std::error_code ec;
					if (fs::is_regular_file(fs::u8path(d.file), ec)) { d.name = lower(to_utf8(fs::u8path(d.file).stem())); }
				}

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Name");
				ImGui::SameLine(110.0f);
				ImGui::SetNextItemWidth(330.0f);
				ImGui::InputTextWithHint("##name", "name of the animation: %name in script", &d.name);
				ImGui::SameLine();
				ImGui::Checkbox("Overwrite", &d.overwrite);

				// the skeleton: a model of the project, or a file
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Model");
				ImGui::SameLine(110.0f);
				ImGui::SetNextItemWidth(330.0f);
				ImGui::InputTextWithHint("##model", "a model of the project, or an .xmodel_export", &d.model);
				ImGui::SameLine();

				if (ImGui::BeginCombo("##models", "", ImGuiComboFlags_NoPreview))
				{
					if (ImGui::Selectable("( none: every part is a root )", d.model.empty())) { d.model.clear(); }

					for (const auto& model : project_models())
					{
						if (ImGui::Selectable(model.c_str(), model == d.model)) { d.model = model; }
					}

					ImGui::EndCombo();
				}

				ImGui::SameLine();
				if (ImGui::Button("..", ImVec2(36.0f, 0.0f)))
				{
					browse(skeleton_filter, d.model);
				}

				ImGui::PushTextWrapPos(640.0f);
				ImGui::TextColored(dim, "An xanim_export has no bone hierarchy: it comes from the model the animation is for. Without one every part is taken "
					"as a root - right for a model of one bone that moves as a whole.");
				ImGui::PopTextWrapPos();

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Type");
				ImGui::SameLine(110.0f);
				ImGui::SetNextItemWidth(330.0f);

				if (ImGui::BeginCombo("##type", type_labels[d.type]))
				{
					for (int i = 0; i < 3; i++)
					{
						if (ImGui::Selectable(type_labels[i], i == d.type)) { d.type = i; }
					}

					ImGui::EndCombo();
				}

				ImGui::SameLine();
				ImGui::Checkbox("Looping", &d.loop);

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Animtree");
				ImGui::SameLine(110.0f);
				ImGui::SetNextItemWidth(330.0f);
				ImGui::InputTextWithHint("##tree", default_anim_tree().c_str(), &d.tree);
				ImGui::SameLine();
				ImGui::TextColored(dim, "#using_animtree( \"%s\" );", d.tree.empty() ? default_anim_tree().c_str() : d.tree.c_str());

				ImGui::Spacing();

				const bool can = !d.file.empty() && !d.name.empty() && !project().empty();
				if (!can) { ImGui::BeginDisabled(); }

				if (ImGui::Button("Import and build", ImVec2(170.0f, 0.0f)))
				{
					anim_request request;
					request.file = fs::u8path(d.file);
					request.name = d.name;
					request.model = d.model;
					request.type = types[d.type];
					request.loop = d.loop;
					request.tree = d.tree;
					request.overwrite = d.overwrite;

					if (import_anim(request, d.error))
					{
						ui::get().show_assets = true;
						d.open = false;
						d.error.clear();
					}
				}

				if (!can) { ImGui::EndDisabled(); }

				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(110.0f, 0.0f)))
				{
					d.open = false;
				}

				if (project().empty())
				{
					ImGui::TextColored(yellow, "Open a map first: the assets belong to the project of a map.");
				}

				if (!d.error.empty())
				{
					ImGui::PushTextWrapPos(640.0f);
					ImGui::TextColored(red, "%s", d.error.c_str());
					ImGui::PopTextWrapPos();
				}
			}

			ImGui::End();
		}

		// ---- Sounds, Effect: a file and what the import takes besides it ----------------------------------------

		struct sound_dialog
		{
			bool open = false;
			bool appearing = false;
			std::string file, root;
			bool overwrite = false;
			std::string error;
		};

		struct fx_dialog
		{
			bool open = false;
			bool appearing = false;
			std::string file, name;
			bool overwrite = false;
			std::string error;
		};

		sound_dialog g_sound;
		fx_dialog g_fx;

		void draw_sound_dialog()
		{
			auto& d = g_sound;
			if (!d.open)
			{
				return;
			}

			ImGui::SetNextWindowSize(ImVec2(680.0f, 0.0f), ImGuiCond_Appearing);
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			if (d.appearing)
			{
				ImGui::SetNextWindowFocus();
				d.appearing = false;
			}

			if (ImGui::Begin("Import sounds##assets", &d.open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::PushItemWidth(520.0f);
				path_row("Aliases ( .csv )", "file", d.file, csv_filter, 130.0f);
				ImGui::PopItemWidth();

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Sound folder");
				ImGui::SameLine(130.0f);
				codol::ui::path_input("##root", d.root, codol::ui::path_kind::folder, nullptr, "the folder that holds sound\\ and soundaliases\\ ( empty: above the csv's soundaliases\\ )", -8.0f, "Sound folder");

				ImGui::PushTextWrapPos(640.0f);
				ImGui::TextColored(dim, "The columns of the CoD4 mod tools. The .wav / .mp3 files the file column names are copied from sound\\, curves ( .vfcurve ) and "
					"speaker maps ( .spkrmap ) from soundaliases\\. A file that is not there is a sound of the game: it is referenced by name.");
				ImGui::PopTextWrapPos();
				ImGui::Checkbox("Overwrite", &d.overwrite);
				ImGui::Spacing();

				const bool can = !d.file.empty() && !project().empty();
				if (!can) { ImGui::BeginDisabled(); }

				if (ImGui::Button("Import and build", ImVec2(170.0f, 0.0f)))
				{
					if (import_sounds(fs::u8path(d.file), fs::u8path(d.root), d.overwrite, d.error))
					{
						ui::get().show_assets = true;
						d.open = false;
						d.error.clear();
					}
				}

				if (!can) { ImGui::EndDisabled(); }

				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(110.0f, 0.0f)))
				{
					d.open = false;
				}

				if (project().empty())
				{
					ImGui::TextColored(yellow, "Open a map first: the assets belong to the project of a map.");
				}

				if (!d.error.empty())
				{
					ImGui::PushTextWrapPos(640.0f);
					ImGui::TextColored(red, "%s", d.error.c_str());
					ImGui::PopTextWrapPos();
				}
			}

			ImGui::End();
		}

		void draw_fx_dialog()
		{
			auto& d = g_fx;
			if (!d.open)
			{
				return;
			}

			ImGui::SetNextWindowSize(ImVec2(680.0f, 0.0f), ImGuiCond_Appearing);
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			if (d.appearing)
			{
				ImGui::SetNextWindowFocus();
				d.appearing = false;
			}

			if (ImGui::Begin("Import effect##assets", &d.open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::PushItemWidth(520.0f);
				path_row("Effect ( .efx )", "file", d.file, efx_filter, 130.0f);
				ImGui::PopItemWidth();

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Name");
				ImGui::SameLine(130.0f);
				ImGui::SetNextItemWidth(330.0f);
				ImGui::InputTextWithHint("##name", "folder/name, as loadfx takes it ( empty: by its place below an fx folder )", &d.name);
				ImGui::SameLine();
				ImGui::Checkbox("Overwrite", &d.overwrite);

				ImGui::PushTextWrapPos(640.0f);
				ImGui::TextColored(dim, "An .efx of the effects editor. ..\\fx\\misc\\x.efx is misc/x; a file outside an fx folder becomes <project>/<file>. Effects it emits "
					"are read from fx\\<name>.efx of the project.");
				ImGui::PopTextWrapPos();
				ImGui::Spacing();

				const bool can = !d.file.empty() && !project().empty();
				if (!can) { ImGui::BeginDisabled(); }

				if (ImGui::Button("Import and build", ImVec2(170.0f, 0.0f)))
				{
					if (import_fx(fs::u8path(d.file), d.name, d.overwrite, d.error))
					{
						ui::get().show_assets = true;
						d.open = false;
						d.error.clear();
					}
				}

				if (!can) { ImGui::EndDisabled(); }

				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(110.0f, 0.0f)))
				{
					d.open = false;
				}

				if (project().empty())
				{
					ImGui::TextColored(yellow, "Open a map first: the assets belong to the project of a map.");
				}

				if (!d.error.empty())
				{
					ImGui::PushTextWrapPos(640.0f);
					ImGui::TextColored(red, "%s", d.error.c_str());
					ImGui::PopTextWrapPos();
				}
			}

			ImGui::End();
		}

		// ---- GDT -------------------------------------------------------------------------------------------

		struct gdt_dialog
		{
			bool open = false;
			bool appearing = false;
			std::string file, root;
			std::string read_for;
			std::vector<gdt_entry> entries;
			std::vector<char> checked;
			bool overwrite = false;
			std::string error;
			std::vector<std::string> log;
		};

		gdt_dialog g_gdt;

		void draw_gdt_dialog()
		{
			auto& d = g_gdt;
			if (!d.open)
			{
				return;
			}

			ImGui::SetNextWindowSize(ImVec2(820.0f, 560.0f), ImGuiCond_Appearing);
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			if (d.appearing)
			{
				ImGui::SetNextWindowFocus();
				d.appearing = false;
			}

			if (ImGui::Begin("GDT materials##assets", &d.open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
			{
				path_row("GDT file", "gdt", d.file, gdt_filter, 120.0f);

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Image folder");
				ImGui::SameLine(120.0f);
				codol::ui::path_input("##root", d.root, codol::ui::path_kind::folder, nullptr, "where the image paths of the GDT start; empty: the folder of the GDT and the two above it", -1.0f, "Image folder");

				if (const std::string key = d.file + "|" + d.root; key != d.read_for)
				{
					d.read_for = key;
					d.entries.clear();
					d.error.clear();
					d.log.clear();

					std::error_code ec;
					if (!d.file.empty() && fs::is_regular_file(fs::u8path(d.file), ec))
					{
						read_gdt(fs::u8path(d.file), fs::u8path(d.root), d.entries, d.error);
					}

					d.checked.assign(d.entries.size(), 0);
				}

				if (!d.error.empty())
				{
					ImGui::TextColored(red, "%s", d.error.c_str());
				}

				ImGui::TextColored(dim, "%d material entries. A CoD4 world material becomes wc/<name>, a model material mc/<name>.", static_cast<int>(d.entries.size()));

				const float footer = ImGui::GetFrameHeightWithSpacing() * (d.log.empty() ? 1.4f : 4.4f);

				if (ImGui::BeginTable("##entries", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable,
					ImVec2(0.0f, -footer)))
				{
					ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 28.0f);
					ImGui::TableSetupColumn("material", ImGuiTableColumnFlags_WidthFixed, 240.0f);
					ImGui::TableSetupColumn("type", ImGuiTableColumnFlags_WidthFixed, 110.0f);
					ImGui::TableSetupColumn("images");
					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableHeadersRow();

					for (int i = 0; i < static_cast<int>(d.entries.size()); i++)
					{
						const auto& entry = d.entries[static_cast<std::size_t>(i)];
						ImGui::PushID(i);
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						bool on = d.checked[static_cast<std::size_t>(i)] != 0;
						if (ImGui::Checkbox("##on", &on)) { d.checked[static_cast<std::size_t>(i)] = on ? 1 : 0; }
						ImGui::TableNextColumn();
						ImGui::TextUnformatted(entry.name.c_str());
						ImGui::TableNextColumn();
						ImGui::TextUnformatted(entry.type.c_str());
						ImGui::TableNextColumn();

						for (std::size_t k = 0; k < entry.images.size(); k++)
						{
							const bool found = !entry.found[k].empty();
							ImGui::TextColored(found ? dim : red, "%s %s%s", entry.images[k].first.c_str(), entry.images[k].second.c_str(), found ? "" : "  ( not found )");
						}

						ImGui::PopID();
					}

					ImGui::EndTable();
				}

				if (ImGui::Button("All")) { d.checked.assign(d.entries.size(), 1); }
				ImGui::SameLine();
				if (ImGui::Button("None")) { d.checked.assign(d.entries.size(), 0); }
				ImGui::SameLine();
				ImGui::Checkbox("Overwrite", &d.overwrite);
				ImGui::SameLine();

				const int chosen = static_cast<int>(std::count(d.checked.begin(), d.checked.end(), 1));
				const bool can = chosen > 0 && !project().empty();
				if (!can) { ImGui::BeginDisabled(); }

				if (ImGui::Button(("Convert " + std::to_string(chosen) + " and build").c_str()))
				{
					std::vector<gdt_entry> list;
					for (std::size_t i = 0; i < d.entries.size(); i++)
					{
						if (d.checked[i]) { list.push_back(d.entries[i]); }
					}

					d.log.clear();
					convert_gdt(list, d.overwrite, d.log);
					ui::get().show_assets = true;
				}

				if (!can) { ImGui::EndDisabled(); }

				for (const auto& line : d.log)
				{
					ImGui::TextColored(line.find("NOT converted") != std::string::npos ? red : green, "%s", line.c_str());
				}
			}

			ImGui::End();
		}

		// ---- Assets ----------------------------------------------------------------------------------------

		void draw_assets_window()
		{
			auto& st = ui::get();
			if (!st.show_assets)
			{
				return;
			}

			auto& s = get_scene();
			ImGui::SetNextWindowSize(ImVec2(520.0f, 600.0f), ImGuiCond_FirstUseEver);

			if (!ImGui::Begin("Assets##window", &st.show_assets, ImGuiWindowFlags_NoCollapse))
			{
				ImGui::End();
				return;
			}

			if (project().empty())
			{
				ImGui::PushTextWrapPos(0.0f);
				ImGui::TextDisabled("Open a map first. Its own textures, materials, models, animations, sounds, effects and scripts are a project in "
					"mod\\<map> next to the editor; they are built into <map>_assets.ff, which the editor shows and the map build takes them from.");
				ImGui::PopTextWrapPos();
				ImGui::End();
				return;
			}

			// the list follows the folder when the window comes up
			if (ImGui::IsWindowAppearing() && !busy())
			{
				rescan();
			}

			// the window is often docked in a narrow column: buttons go to the next line when they do not fit
			const auto next_button = [](const char* label)
			{
				const float width = ImGui::CalcTextSize(label).x + ImGui::GetStyle().FramePadding.x * 2.0f;
				ImGui::SameLine();

				if (ImGui::GetContentRegionAvail().x < width)
				{
					ImGui::NewLine();
				}
			};

			ImGui::Text("Project %s", project().c_str());
			ImGui::SameLine();
			ImGui::TextColored(dim, "mod\\%s", project().c_str());

			const bool building = busy();

			if (building) { ImGui::BeginDisabled(); }

			if (ImGui::Button("Build", ImVec2(90.0f, 0.0f)))
			{
				std::string error;
				if (!build(error))
				{
					ImGui::Toast(ImGuiToastType_Warning, "Assets", error.c_str());
				}
			}

			next_button("Rescan");
			if (ImGui::Button("Rescan"))
			{
				rescan();
			}

			if (building) { ImGui::EndDisabled(); }

			next_button("Folder");
			if (ImGui::Button("Folder"))
			{
				std::error_code ec;
				fs::create_directories(project_dir() / "images", ec);
				fs::create_directories(project_dir() / "materials", ec);
				fs::create_directories(project_dir() / "models", ec);
				fs::create_directories(project_dir() / "anims", ec);
				fs::create_directories(project_dir() / "soundaliases", ec);
				fs::create_directories(project_dir() / "sound", ec);
				fs::create_directories(project_dir() / "fx", ec);
				fs::create_directories(project_dir() / "scripts" / "maps" / "mp", ec);
				ShellExecuteW(main_window(), L"open", project_dir().wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s\nimages\\ materials\\*.mtl models\\*.model, *.xmodel_export\nanims\\*.anim, *.xanim_export soundaliases\\*.csv sound\\ fx\\<folder>\\*.efx\n"
					"scripts\\maps\\mp\\<map>.gsc ..: taken into the built map as they are", to_utf8(project_dir()).c_str());
			}

			if (building)
			{
				static const char spinner[] = "|/-\\";
				ImGui::PushTextWrapPos(0.0f);
				ImGui::TextColored(yellow, "%c building: %s", spinner[static_cast<int>(ImGui::GetTime() * 8.0) & 3], build_step().c_str());
				ImGui::PopTextWrapPos();
			}
			else if (zone_exists())
			{
				ImGui::TextColored(green, "%s.ff", zone_name().c_str());
			}
			else
			{
				ImGui::TextColored(dim, "no zone yet: Build makes %s.ff", zone_name().c_str());
			}

			if (ImGui::Button("New material ..")) { open_material_dialog(); }
			next_button("Import model ..");
			if (ImGui::Button("Import model ..")) { open_model_dialog(); }
			next_button("GDT ..");
			if (ImGui::Button("GDT ..")) { open_gdt_dialog(); }

			// what the editor does not draw: built into the zone, carried into the map by File > Build map
			if (ImGui::Button("Import animation ..")) { open_anim_dialog(); }
			next_button("Sounds ..");

			if (ImGui::Button("Sounds .."))
			{
				open_sounds_dialog();
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("A soundaliases .csv ( columns as in the CoD4 mod tools ). The .wav files its file column names are taken\nfrom sound\\ next to the "
					"csv's soundaliases\\ folder, curves ( .vfcurve ) and speaker maps ( .spkrmap ) from that folder.");
			}

			next_button("Effect ..");

			if (ImGui::Button("Effect .."))
			{
				open_fx_dialog();
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("An .efx of the effects editor. It is named by its place below an fx folder ( fx\\misc\\x.efx: misc/x ),\nwhich is what loadfx takes.");
			}

			next_button("Level script");

			if (ImGui::Button("Level script"))
			{
				fs::path file;
				std::string error;

				if (!level_script(file, error))
				{
					ImGui::Toast(ImGuiToastType_Error, "Level script", error.c_str(), 8000);
				}
				else
				{
					ShellExecuteW(main_window(), L"open", L"explorer.exe", (L"/select,\"" + file.wstring() + L"\"").c_str(), nullptr, SW_SHOWNORMAL);
				}
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("scripts\\maps\\mp\\<map>.gsc of the project: made with what the compiler would generate when it is not there.\n"
					"File > Build map takes every file of scripts\\ in front of the generated one of the same name.");
			}

			ImGui::Separator();

			const auto& list = items();
			const auto& problems = compile_problems();
			const auto& log = build_log();
			const float footer = (problems.empty() ? 0.0f : 150.0f) + (log.empty() ? 0.0f : ImGui::GetFrameHeightWithSpacing() + 4.0f);

			if (list.empty())
			{
				ImGui::PushTextWrapPos(0.0f);
				ImGui::TextDisabled("Nothing yet. \"New material ..\" makes a material from an image, \"Import model ..\" a model from xmodel_export files, "
					"\"Import animation ..\" an animation from an xanim_export, \"Sounds ..\" takes a soundaliases .csv with its .wav files, \"Effect ..\" an .efx; "
					"the files can also be dropped on the window. Sources put into the folder by hand show up with Rescan.");
				ImGui::PopTextWrapPos();
			}
			else if (ImGui::BeginChild("##assets", ImVec2(0.0f, -footer), false))
			{
				// one block per asset: kind, name and status on a line, below it what it is or what is wrong
				int built = 0, failed = 0, waiting = 0;
				for (const auto& entry : list)
				{
					built += entry.state == status::built;
					failed += entry.state == status::failed;
					waiting += entry.state == status::fresh || entry.state == status::changed;
				}

				ImGui::TextColored(dim, "%d built, %d failed, %d to build", built, failed, waiting);

				for (int i = 0; i < static_cast<int>(list.size()); i++)
				{
					const auto& entry = list[static_cast<std::size_t>(i)];
					ImGui::PushID(i);

					const char* tag = entry.type == kind::image ? "image" : entry.type == kind::material ? "material" : entry.type == kind::model ? "model"
						: entry.type == kind::anim ? "anim" : entry.type == kind::sound ? "sound" : entry.type == kind::fx ? "fx" : "script";
					const ImVec4 color = entry.state == status::built ? green : entry.state == status::failed ? red : entry.state == status::unused || entry.state == status::raw ? dim : yellow;
					const char* word = entry.state == status::fresh ? "new" : entry.state == status::changed ? "changed" : entry.state == status::built ? "built"
						: entry.state == status::failed ? "failed" : entry.state == status::raw ? "as is" : "unused";

					ImGui::Separator();
					ImGui::TextColored(color, "%s", word);
					ImGui::SameLine(62.0f);
					ImGui::TextColored(dim, "%s", tag);
					ImGui::SameLine(124.0f);

					const bool clicked = ImGui::Selectable(entry.name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick);

					if (clicked && !ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						// a look ( or a listen ) before it is used: the Preview window
						switch (entry.type)
						{
						case kind::image: preview::show_image(entry.file); break;
						case kind::material: if (entry.state == status::built) { preview::show_material(entry.name); } break;
						case kind::model: if (entry.state == status::built) { preview::show_model(entry.name); } break;
						case kind::anim: preview::show_anim(entry.name); break;
						case kind::sound: preview::show_sounds(entry.file); break;
						case kind::fx: preview::show_fx(entry.name); break;
						default: break;
						}
					}

					if (clicked && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && entry.state == status::built)
					{
						// use it: the material of new brushes ( and of the selection ), or a new entity with the model
						if (entry.type == kind::material)
						{
							s.doc.set_default_material(entry.name);

							if (!s.doc.selection().empty() || !s.doc.selected_faces().empty())
							{
								ui::report(s.doc.set_material(entry.name), "Set material");
							}
						}
						else if (entry.type == kind::model)
						{
							const auto target = edit::snap_point(s.ray_target(s.cam.origin, s.cam.axes().forward), s.snap ? s.grid : 0.0);
							s.create_model_entity(entry.name, target, 0.0);
						}
					}

					if (ImGui::IsItemHovered())
					{
						// how it is used: in the editor, or by the level script of the built map
						const std::string use = entry.type == kind::material ? "Click: preview. Double click: the material of new brushes and of the selection."
							: entry.type == kind::model ? "Click: preview. Double click: a new entity with the model where the camera looks."
							: entry.type == kind::image ? "Click: the picture."
							: entry.type == kind::anim ? "Click: plays it on its model ( Preview window ). File > Build map puts it into the map. Script: #using_animtree( \"<its animtree>\" ); in front of main(), then\n"
								"ent setAnim( %" + entry.name + ", 1, 0, 1 ); on a script_model that has its model"
							: entry.type == kind::sound ? "Click: its aliases, to listen to. File > Build map puts the aliases into the map. Script: ent playSound( \"<alias>\" ); ent playLoopSound( \"<alias>\" );\n"
								"ambientPlay( \"<alias>\" ); in the map: a script_sound entity"
							: entry.type == kind::fx ? "File > Build map puts it into the map. Script: level._effect[ \"x\" ] = loadfx( \"" + entry.name + "\" ); playfx( level._effect[ \"x\" ], origin );\n"
								"in the map: a script_fx entity with the key fx " + entry.name
							: std::string();
						ImGui::SetTooltip("%s\n%s", to_utf8(entry.file).c_str(), use.c_str());
					}

					ImGui::Indent(12.0f);
					ImGui::PushTextWrapPos(0.0f);

					if (entry.state == status::failed)
					{
						ImGui::TextColored(red, "%s", entry.message.c_str());
					}
					else if (entry.state == status::unused)
					{
						ImGui::TextColored(dim, "no material of the project names it: not part of the zone");
					}
					else if (!entry.detail.empty())
					{
						ImGui::TextUnformatted(entry.detail.c_str());
					}
					else
					{
						ImGui::TextColored(dim, "not built yet");
					}

					for (const auto& note : entry.notes)
					{
						ImGui::TextColored(dim, "%s", note.c_str());
					}

					if (entry.type == kind::material && entry.state != status::failed && undrawn_in_game(entry.name))
					{
						ImGui::TextColored(yellow, "On brushes: editor only. codol_mapc draws new faces only with materials the world of %s already uses; faces with this one "
							"collide but are not drawn in the game.", s.map_name.c_str());
					}

					ImGui::PopTextWrapPos();
					ImGui::Unindent(12.0f);
					ImGui::PopID();
				}
			}

			if (!list.empty())
			{
				ImGui::EndChild();
			}

			if (!problems.empty())
			{
				ImGui::TextColored(yellow, "Last compile: not in the game");

				if (ImGui::BeginChild("##compile_problems", ImVec2(0.0f, 120.0f), true))
				{
					ImGui::PushTextWrapPos(0.0f);
					for (const auto& line : problems)
					{
						ImGui::TextColored(yellow, "%s", line.c_str());
					}
					ImGui::PopTextWrapPos();
				}

				ImGui::EndChild();
			}

			if (!log.empty() && ImGui::CollapsingHeader("Last build"))
			{
				for (const auto& line : log)
				{
					ImGui::TextColored(line.find("FAILED") != std::string::npos ? red : dim, "%s", line.c_str());
				}
			}

			ImGui::End();
		}
	}

	// ---- opening the windows -----------------------------------------------------------------------------------

	void open_material_dialog(const std::vector<fs::path>& images)
	{
		auto& d = g_material;

		if (!d.open || !images.empty())
		{
			d = {};
		}

		d.open = d.appearing = true;

		// which map an image is: by the end of its name
		for (const auto& file : images)
		{
			const std::string stem = lower(to_utf8(file.stem()));
			const std::string path = to_utf8(file);
			const bool is_normal = ends_with(stem, "_n") || ends_with(stem, "_nml") || ends_with(stem, "_nrm") || ends_with(stem, "_normal");
			const bool is_specular = ends_with(stem, "_s") || ends_with(stem, "_spc") || ends_with(stem, "_spec") || ends_with(stem, "_specular");

			if (is_normal && d.normal.empty()) { d.normal = path; }
			else if (is_specular && d.specular.empty()) { d.specular = path; }
			else if (d.color.empty()) { d.color = path; }
		}

		if (!d.color.empty() && d.name.empty())
		{
			std::string stem = lower(to_utf8(fs::u8path(d.color).stem()));

			for (const char* tail : { "_col", "_color", "_colour", "_diffuse", "_c", "_d" })
			{
				if (ends_with(stem, tail))
				{
					stem.resize(stem.size() - std::strlen(tail));
					break;
				}
			}

			d.name = stem;
		}
	}

	void open_model_dialog(const std::vector<fs::path>& lods)
	{
		auto& d = g_model;

		if (!d.open || !lods.empty())
		{
			d = {};
		}

		d.open = d.appearing = true;

		for (const auto& file : lods)
		{
			if (d.lods.size() < 4) { d.lods.push_back({ to_utf8(file), 0.0f }); }
		}

		if (d.lods.empty())
		{
			d.lods.emplace_back();
		}

		std::sort(d.lods.begin(), d.lods.end(), [](const auto& a, const auto& b) { return a.file < b.file; });
		reset_distances(d);
	}

	void open_anim_dialog(const fs::path& file)
	{
		auto& d = g_anim;

		if (!d.open || !file.empty())
		{
			d = {};
		}

		d.open = d.appearing = true;

		if (!file.empty())
		{
			d.file = to_utf8(file);
		}
	}

	void open_gdt_dialog(const fs::path& file)
	{
		auto& d = g_gdt;

		if (!d.open || !file.empty())
		{
			d = {};
		}

		d.open = d.appearing = true;

		if (!file.empty())
		{
			d.file = to_utf8(file);
		}
	}

	void open_sounds_dialog(const fs::path& file)
	{
		auto& d = g_sound;

		if (!d.open || !file.empty())
		{
			d = {};
		}

		d.open = d.appearing = true;

		if (!file.empty())
		{
			d.file = to_utf8(file);
		}
	}

	void open_fx_dialog(const fs::path& file)
	{
		auto& d = g_fx;

		if (!d.open || !file.empty())
		{
			d = {};
		}

		d.open = d.appearing = true;

		if (!file.empty())
		{
			d.file = to_utf8(file);
		}
	}

	void close_dialogs()
	{
		g_material.open = g_model.open = g_gdt.open = g_anim.open = g_sound.open = g_fx.open = false;
	}

	void draw_windows()
	{
		draw_assets_window();
		draw_material_dialog();
		draw_model_dialog();
		draw_gdt_dialog();
		draw_anim_dialog();
		draw_sound_dialog();
		draw_fx_dialog();
	}
}
