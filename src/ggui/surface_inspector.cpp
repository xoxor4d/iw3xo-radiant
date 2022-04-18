#include "std_include.hpp"

// typedef game::MaterialDef*(__thiscall* csurfdlg_call_t)(CSurfaceDlg*);
//auto brush_get_materialdef = reinterpret_cast<csurfdlg_call_t>(0x457950);

// Select_SetTexture_2 - NEEDS SurfaceInspector
// auto selection_update_textureinfo = reinterpret_cast<csurfdlg_call_t>(0x4572D0);

// Brush_SetTexture (does indeed set the texture defined in mat_def
// utils::hook::call<void(__cdecl)(MaterialDef* _def, int _unk)>(0x48F170)(mat_def, 0);

// Brush_SetTextureMapping (keeps diff. textures but applies scaling)
// utils::hook::call<void(__cdecl)(texdef_sub_t* _texdef)>(0x48F4F0)(mat_def->mat_texDef);

// SurfaceInspector::SetTexMods
// cdeclcall(void, 0x458270);

namespace ggui
{
	void surface_dialog::edit_texture_info(game::texdef_sub_t* texdef, bool set_specific, TEXMODE mode, int dir)
	{
		const float PATCH_SCALAR = 0.005f;
		//auto& g_patch_texdef = *reinterpret_cast<patch_texdef_t*>(0x23F15F8);

		FOR_ALL_SELECTED_BRUSHES(sb)
		{
			// selection is patch
			if (sb->patch)
			{
				if (set_specific)
				{
					game::printf_to_console("[!] specific texture edits are not supported for patches. skipping patch ...");
					continue;
				}

				//texdef->shift[0] *= PATCH_SCALAR;
				//texdef->shift[1] *= PATCH_SCALAR;

				if (dir != 0)
				{
					if ((mode & TEXMODE_SHIFT_HORZ) != 0)
					{
						texdef->shift[0] = -texdef->shift[0] * PATCH_SCALAR;
					}

					if ((mode & TEXMODE_SHIFT_VERT) != 0)
					{
						texdef->shift[1] = -texdef->shift[1] * PATCH_SCALAR;
					}

					if ((mode & TEXMODE_SIZE_HORZ) != 0)
					{
						texdef->size[0] = 1.0f - (texdef->size[0] * PATCH_SCALAR);
					}

					if ((mode & TEXMODE_SIZE_VERT) != 0)
					{
						texdef->size[1] = 1.0f - (texdef->size[1] * PATCH_SCALAR);
					}
				}
				else
				{
					texdef->size[0] *= PATCH_SCALAR;
					texdef->size[1] *= PATCH_SCALAR;
					texdef->shift[0] *= PATCH_SCALAR;
					texdef->shift[1] *= PATCH_SCALAR;
				}

				//texdef->rotate	 *= PATCH_SCALAR;

				// increases version itself
				Patch_SetTextureInfo(texdef);
			}
			else
			{
				// entire brush selected (not single faces)
				// offset texture for each face, ++version to update visuals
				if (sb->def && sb->def->brush_faces)
				{
					for (auto f = 0; f < sb->def->facecount; f++)
					{
						const auto mat_def = &sb->def->brush_faces[f].mtldef[game::g_qeglobals->current_edit_layer];

						if (set_specific)
						{
							if ((mode & TEXMODE_SIZE_HORZ) != 0)
							{
								mat_def->mat_texDef->size[0] = texdef->size[0];
							}

							if ((mode & TEXMODE_SIZE_VERT) != 0)
							{
								mat_def->mat_texDef->size[1] = texdef->size[1];
							}

							if ((mode & TEXMODE_SHIFT_HORZ) != 0)
							{
								mat_def->mat_texDef->shift[0] = texdef->shift[0];
							}

							if ((mode & TEXMODE_SHIFT_VERT) != 0)
							{
								mat_def->mat_texDef->shift[1] = texdef->shift[1];
							}

							if ((mode & TEXMODE_ROTATE) != 0)
							{
								mat_def->mat_texDef->rotate = texdef->rotate;
							}
						}
						else
						{
							mat_def->mat_texDef->shift[0] += texdef->shift[0];
							mat_def->mat_texDef->shift[1] += texdef->shift[1];
							mat_def->mat_texDef->size[0] += texdef->size[0];
							mat_def->mat_texDef->size[1] += texdef->size[1];
							mat_def->mat_texDef->rotate += texdef->rotate;
						}

						++sb->def->version;
					}
				}
			}
		}

		const int  selected_faces_count = *reinterpret_cast<int*>(0x73C714);
		const auto selected_faces = game::g_selected_faces();

		if (selected_faces_count > 0)
		{
			for (auto f = 0; f < selected_faces_count; f++)
			{
				const auto face_index = selected_faces[f].index;
				const auto face_brush = selected_faces[f].brush->def;

				const auto mat_def = &face_brush->brush_faces[face_index].mtldef[game::g_qeglobals->current_edit_layer];

				if (set_specific)
				{
					if ((mode & TEXMODE_SIZE_HORZ) != 0)
					{
						mat_def->mat_texDef->size[0] = texdef->size[0];
					}

					if ((mode & TEXMODE_SIZE_VERT) != 0)
					{
						mat_def->mat_texDef->size[1] = texdef->size[1];
					}

					if ((mode & TEXMODE_SHIFT_HORZ) != 0)
					{
						mat_def->mat_texDef->shift[0] = texdef->shift[0];
					}

					if ((mode & TEXMODE_SHIFT_VERT) != 0)
					{
						mat_def->mat_texDef->shift[1] = texdef->shift[1];
					}

					if ((mode & TEXMODE_ROTATE) != 0)
					{
						mat_def->mat_texDef->rotate = texdef->rotate;
					}
				}
				else
				{
					mat_def->mat_texDef->shift[0] += texdef->shift[0];
					mat_def->mat_texDef->shift[1] += texdef->shift[1];
					mat_def->mat_texDef->size[0] += texdef->size[0];
					mat_def->mat_texDef->size[1] += texdef->size[1];
					mat_def->mat_texDef->rotate += texdef->rotate;
				}

				++face_brush->version;
			}
		}
	}

	void surface_dialog::inspector_controls()
	{
		//ImGui::Indent(8.0f);

		//const auto avail_width = ImGui::GetContentRegionAvailWidth() * 0.65f;
		//ImGui::PushItemWidth(avail_width);

		const float inner_item_spacing = 4.0f;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(inner_item_spacing, 0.0f));

		const float clamped_min_widget_width = ImMax(186.0f, ImGui::CalcItemWidth() + 2 * inner_item_spacing);

		static texedit_helper texhelp = {};
		texhelp.scalar_direction = 0; // reset each frame

		auto& g_patch_texdef = *reinterpret_cast<patch_texdef_t*>(0x23F15F8);
		const int selected_faces_count = *reinterpret_cast<int*>(0x73C714);

		if (selected_faces_count > 0)
		{
			const auto selected_faces = game::g_selected_faces();

			if (selected_faces && selected_faces->brush && selected_faces->brush->def)
			{
				const auto mat_def = &selected_faces->brush->def->brush_faces[selected_faces->index].mtldef[game::g_qeglobals->current_edit_layer];

				texhelp.size_horz = mat_def->mat_texDef->size[0];
				texhelp.size_vert = mat_def->mat_texDef->size[1];
				texhelp.shift_horz = mat_def->mat_texDef->shift[0];
				texhelp.shift_vert = mat_def->mat_texDef->shift[1];
				texhelp.rotation = mat_def->mat_texDef->rotate;
			}
		}
		else if (const auto	sb = game::g_selected_brushes();
							sb)
		{
			if (sb->patch)
			{
				texhelp.reset_values();
				//texhelp.rotation = g_patch_texdef.def.mat_texDef->rotate;
			}
			else if (sb->def && sb->def->brush_faces)
			{
				texhelp.size_horz = sb->def->brush_faces->mtldef[game::g_qeglobals->current_edit_layer].mat_texDef->size[0];
				texhelp.size_vert = sb->def->brush_faces->mtldef[game::g_qeglobals->current_edit_layer].mat_texDef->size[1];
				texhelp.shift_horz = sb->def->brush_faces->mtldef[game::g_qeglobals->current_edit_layer].mat_texDef->shift[0];
				texhelp.shift_vert = sb->def->brush_faces->mtldef[game::g_qeglobals->current_edit_layer].mat_texDef->shift[1];
				texhelp.rotation = sb->def->brush_faces->mtldef[game::g_qeglobals->current_edit_layer].mat_texDef->rotate;
			}
		}

		if (ImGui::InputScalarDir("Shift Horz", ImGuiDataType_Float, &texhelp.shift_horz, &texhelp.scalar_direction, &texhelp.amount_shift, nullptr, true, nullptr, !texhelp.specific_mode ? ImGuiInputTextFlags_ReadOnly : 0))
		{
			const float amount_specific = texhelp.shift_horz;
			const float amount_inc = texhelp.amount_shift;

			const float edit_amount = texhelp.specific_mode ? amount_specific
				: texhelp.scalar_direction == -1 ? -amount_inc
				: texhelp.scalar_direction == 1 ? amount_inc : 0.0f;

			game::texdef_sub_t texdef_edit = {};
			texdef_edit.shift[0] = edit_amount;

			edit_texture_info(&texdef_edit, texhelp.specific_mode, TEXMODE_SHIFT_HORZ, texhelp.scalar_direction);
		}

		if (ImGui::InputScalarDir("Shift Vert", ImGuiDataType_Float, &texhelp.shift_vert, &texhelp.scalar_direction, &texhelp.amount_shift, nullptr, true, nullptr, !texhelp.specific_mode ? ImGuiInputTextFlags_ReadOnly : 0))
		{
			const float amount_specific = texhelp.shift_vert;
			const float amount_inc = texhelp.amount_shift;

			const float edit_amount = texhelp.specific_mode ? amount_specific
				: texhelp.scalar_direction == -1 ? -amount_inc
				: texhelp.scalar_direction == 1 ? amount_inc : 0.0f;

			game::texdef_sub_t texdef_edit = {};
			texdef_edit.shift[1] = edit_amount;

			edit_texture_info(&texdef_edit, texhelp.specific_mode, TEXMODE_SHIFT_VERT, texhelp.scalar_direction);
		}

		SPACING(0.0f, 4.0f);

		if (ImGui::InputScalarDir("Size Horz", ImGuiDataType_Float, &texhelp.size_horz, &texhelp.scalar_direction, &texhelp.amount_size, nullptr, true, nullptr, !texhelp.specific_mode ? ImGuiInputTextFlags_ReadOnly : 0))
		{
			const float amount_specific = texhelp.size_horz;
			const float amount_inc = texhelp.amount_size;

			const float edit_amount = texhelp.specific_mode ? amount_specific
				: texhelp.scalar_direction == -1 ? -amount_inc
				: texhelp.scalar_direction == 1 ? amount_inc : 0.0f;

			game::texdef_sub_t texdef_edit = {};
			texdef_edit.size[0] = edit_amount;

			edit_texture_info(&texdef_edit, texhelp.specific_mode, TEXMODE_SIZE_HORZ, texhelp.scalar_direction);
		}

		if (ImGui::InputScalarDir("Size Vert", ImGuiDataType_Float, &texhelp.size_vert, &texhelp.scalar_direction, &texhelp.amount_size, nullptr, true, nullptr, !texhelp.specific_mode ? ImGuiInputTextFlags_ReadOnly : 0))
		{
			const float amount_specific = texhelp.size_vert;
			const float amount_inc = texhelp.amount_size;

			const float edit_amount = texhelp.specific_mode ? amount_specific
				: texhelp.scalar_direction == -1 ? -amount_inc
				: texhelp.scalar_direction == 1 ? amount_inc : 0.0f;

			game::texdef_sub_t texdef_edit = {};
			texdef_edit.size[1] = edit_amount;

			edit_texture_info(&texdef_edit, texhelp.specific_mode, TEXMODE_SIZE_VERT, texhelp.scalar_direction);
		}

		SPACING(0.0f, 4.0f);

		if (ImGui::InputScalarDir("Rotation", ImGuiDataType_Float, &texhelp.rotation, &texhelp.scalar_direction, &texhelp.amount_rotate, nullptr, true, nullptr, !texhelp.specific_mode ? ImGuiInputTextFlags_ReadOnly : 0))
		{
			const float amount_specific = texhelp.rotation;
			const float amount_inc = texhelp.amount_rotate;

			const float edit_amount = texhelp.specific_mode ? amount_specific
				: texhelp.scalar_direction == -1 ? -amount_inc
				: texhelp.scalar_direction == 1 ? amount_inc : 0.0f;

			game::texdef_sub_t texdef_edit = {};
			texdef_edit.rotate = edit_amount;

			edit_texture_info(&texdef_edit, texhelp.specific_mode, TEXMODE_ROTATE);
		}

		if (ImGui::InputScalarDir("Sample Size", ImGuiDataType_Float, &g_patch_texdef.sample_size, nullptr, &texhelp.amount_sample, nullptr, true))
		{
			if (g_patch_texdef.sample_size < 4.0f) 
			{
				g_patch_texdef.sample_size = 4.0f;
			}

			// Brush_SetSampleSize
			utils::hook::call<void(__cdecl)(int sample_size)>(0x48F800)((int)g_patch_texdef.sample_size);
		}

		SPACING(0.0f, 4.0f);

		static float group_width = 80.0f;
		ImGui::SetCursorPosX(clamped_min_widget_width * 0.5f - (group_width * 0.5f));

		ImGui::BeginGroup();
		{
			ImGui::Checkbox(" Set Specific Values", &texhelp.specific_mode);
		}
		ImGui::EndGroup();
		group_width = ImGui::GetItemRectSize().x;
		ImGui::SameLine();
		ImGui::HelpMarker(
			"On: (Specific Mode)\n"
			"- Enables editing values within the textboxes to directly set texture values.\n"
			"- Set all selected faces/brushes to the specified value.\n\n"
			"Off: (Inc/Dec Mode)\n"
			"- Texture edits are only possible via the + and - buttons\n"
			"- Inc/Dec values per face/brush (eg: brush1 value @ 0.0f + offset, brush2 value @ 45.0f + offset\n\n"
			"Displayed values are always from the first face that was selected.");

		SPACING(0.0f, 4.0f);

		const float button_size = ImMax(182.0f, ImGui::CalcItemWidth() + inner_item_spacing) * 0.5f;

		if (ImGui::Button("Cap", ImVec2(button_size, ImGui::GetFrameHeight())))
		{
			const float sample_size = game::g_qeglobals->current_edit_layer == 1 ? g_patch_texdef.sample_size : 0.25f;

			//Patch_NaturalizeSelected
			utils::hook::call<void(__cdecl)(bool _unk, bool cap, float _x, float _y)>(0x447FD0)(true, false, texhelp.repeatx * sample_size, texhelp.repeaty * sample_size);
		}

		ImGui::SameLine(0.0f, inner_item_spacing);
		if (ImGui::Button("Natural", ImVec2(button_size, ImGui::GetFrameHeight())))
		{
			const float sample_size = game::g_qeglobals->current_edit_layer == 1 ? g_patch_texdef.sample_size : 0.25f;

			//Patch_NaturalizeSelected
			utils::hook::call<void(__cdecl)(bool _unk, bool cap, float _x, float _y)>(0x447FD0)(false, false, texhelp.repeatx * sample_size, texhelp.repeaty * sample_size);
		}


		if (ImGui::Button("Fit", ImVec2(button_size, ImGui::GetFrameHeight())))
		{
			//Brush_FitTexture
			utils::hook::call<void(__cdecl)(float _x, float _y, int _unk)>(0x4939E0)(1.0f, 1.0f, 0);
		}

		ImGui::SameLine(0.0f, inner_item_spacing);
		if (ImGui::Button("Lmap", ImVec2(button_size, ImGui::GetFrameHeight())))
		{
			//Patch_Lightmap_Texturing
			utils::hook::call<void(__cdecl)()>(0x448110)();
		}

		if (ImGui::Button("Cycle Texture Layer", ImVec2(clamped_min_widget_width, ImGui::GetFrameHeight())))
		{
			cdeclcall(void, 0x424010);
		}

		SPACING(0.0f, 12.0f);

		const char* set_mode_items[] = { "2D", "2D - Auto Set", "3D", "3D - Auto Set", "Patch Curve", "Patch Curve - Auto Set" };
		static int set_mode_current = 1;

		ImGui::SetNextItemWidth(clamped_min_widget_width);
		ImGui::Combo("Mode", &set_mode_current, set_mode_items, IM_ARRAYSIZE(set_mode_items));

		if (ImGui::InputScalarDir("Repeat X", ImGuiDataType_Float, &texhelp.repeatx, nullptr, &texhelp.amount_repeatx, nullptr, true))
		{
			if (texhelp.amount_repeatx < 0.0f) 
			{
				texhelp.amount_repeatx = 0.0f;
			}

			// if auto set mode
			if (set_mode_current == 1 || set_mode_current == 3 || set_mode_current == 5)
			{
				// Patch_SetTexturing
				utils::hook::call<void(__cdecl)(float _x, float _y, int _unk)>(0x446B60)(texhelp.repeatx, texhelp.repeaty, set_mode_current == 1 ? 0 : set_mode_current == 3 ? 1 : 2);
			}
		}

		if (ImGui::InputScalarDir("Repeat Y", ImGuiDataType_Float, &texhelp.repeaty, nullptr, &texhelp.amount_repeaty, nullptr, true))
		{
			if (texhelp.amount_repeaty < 0.0f) 
			{
				texhelp.amount_repeaty = 0.0f;
			}

			// if auto set mode
			if (set_mode_current == 1 || set_mode_current == 3 || set_mode_current == 5)
			{
				// Patch_SetTexturing
				utils::hook::call<void(__cdecl)(float _x, float _y, int _unk)>(0x446B60)(texhelp.repeatx, texhelp.repeaty, set_mode_current == 1 ? 0 : set_mode_current == 3 ? 1 : 2);
			}
		}

		if (ImGui::Button("Set##set", ImVec2(clamped_min_widget_width, ImGui::GetFrameHeight())))
		{
			// Patch_SetTexturing
			utils::hook::call<void(__cdecl)(float _x, float _y, int _unk)>(0x446B60)(texhelp.repeatx, texhelp.repeaty, set_mode_current == 1 ? 0 : set_mode_current == 3 ? 1 : 2);
		}

		ImGui::PopStyleVar(); // ImGuiStyleVar_ItemInnerSpacing
		//ImGui::PopItemWidth();
	}

	void surface_dialog::gui()
	{
		if (dvars::gui_props_surfinspector && !dvars::gui_props_surfinspector->current.enabled)
		{
			const auto MIN_WINDOW_SIZE = ImVec2(210.0f, 290.0f);
			const auto INITIAL_WINDOW_SIZE = ImVec2(345.0f, 510.0f);
			ggui::set_next_window_initial_pos_and_constraints(MIN_WINDOW_SIZE, INITIAL_WINDOW_SIZE);

			int stylevars = 0, stylecolors = 0;
			//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f)); stylevars++;
			//ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.08f, 0.08f, 0.31f)); stylecolors++;

			if (!ImGui::Begin("Surface Inspector##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
			{
				ImGui::PopStyleColor(stylecolors);
				ImGui::PopStyleVar(stylevars);
				ImGui::End();
				return;
			}

			ImGui::Indent(8.0f);
			SPACING(0.0f, 4.0f);
			surface_dialog::inspector_controls();

			ImGui::PopStyleColor(stylecolors);
			ImGui::PopStyleVar(stylevars);
			ImGui::End();
		}
	}

	void surface_dialog::on_open()
	{ }

	void surface_dialog::on_close()
	{ }

	// CMainFrame::OnTexturesInspector
	void surface_dialog::on_surfaceinspector_command()
	{
		if (dvars::gui_use_new_surfinspector && dvars::gui_use_new_surfinspector->current.enabled)
		{
			if (dvars::gui_props_surfinspector && dvars::gui_props_surfinspector->current.enabled)
			{
				GET_GUI(ggui::entity_dialog)->toggle();
			}
			else
			{
				GET_GUI(ggui::surface_dialog)->toggle();
			}
		}
		else
		{
			// SurfaceInspector::DoSurface
			cdeclcall(void, 0x4585D0);
		}
	}

	void surface_dialog::hooks()
	{
		// detour CMainFrame::OnPrefs (hotkey to open the original dialog)
		utils::hook::detour(0x424B60, surface_dialog::on_surfaceinspector_command, HK_JUMP);
	}

	void surface_dialog::register_dvars()
	{
		dvars::gui_props_surfinspector = dvars::register_bool(
			/* name		*/ "gui_props_surfinspector",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "property editor - incorporate surface inspector window");

		dvars::gui_use_new_surfinspector = dvars::register_bool(
			/* name		*/ "gui_use_new_surfinspector",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "use the new experimental surface inspector window (replaces the original hotkey)");
	}

	REGISTER_GUI(surface_dialog);
}
