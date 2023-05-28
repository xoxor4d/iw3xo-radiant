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
	void surface_dialog::edit_texture_info(game::texdef_sub_t* texdef, texedit_helper* helper, TEXMODE mode, int dir)
	{
		const float PATCH_SCALAR = 0.005f;
		//auto& g_patch_texdef = *reinterpret_cast<patch_texdef_t*>(0x23F15F8);

		//bool is_dirty = false;

		//bool use_additive_logic = false;

		if (!helper->original_logic)
		{
			FOR_ALL_SELECTED_BRUSHES(sb)
			{
				// selection is patch
				if (sb->patch)
				{
					if (helper->specific_mode)
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

				// not patch
				else
				{
					// entire brush selected (not single faces)
					// offset texture for each face, ++version to update visuals
					if (sb->def && sb->def->brush_faces)
					{
						for (auto f = 0; f < sb->def->facecount; f++)
						{
							const auto mat_def = &sb->def->brush_faces[f].mtldef[game::g_qeglobals->current_edit_layer];

							if (helper->specific_mode)
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
							//utils::hook::call<void(__cdecl)(game::MaterialDef* _def, int _unk)>(0x48F170)(mat_def, 0);
							//is_dirty = true;
						}
					}
				}
			}

			// single faces

			const int  selected_faces_count = *reinterpret_cast<int*>(0x73C714);
			const auto selected_faces = game::g_selected_faces();

			if (selected_faces_count > 0)
			{
				for (auto f = 0; f < selected_faces_count; f++)
				{
					const auto face_index = selected_faces[f].index;
					const auto face_brush = selected_faces[f].brush->def;

					const auto mat_def = &face_brush->brush_faces[face_index].mtldef[game::g_qeglobals->current_edit_layer];

					if (helper->specific_mode)
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

					//utils::hook::call<void(__cdecl)(game::MaterialDef* _def, int _unk)>(0x48F170)(mat_def, 0);
					++face_brush->version;
					//is_dirty = true;
				}
			}
		}

		


		// original surface inspector logic which uses texdef settings of the first face
		// and applies these onto all other selected surfaces / brushes
		else 
		{
			game::MaterialDef* mtl_def = nullptr;

			const int  selected_faces_count = *reinterpret_cast<int*>(0x73C714);
			const auto selected_faces = game::g_selected_faces();

			// Part of 'Brush_GetMaterialDef'
			if (selected_faces_count > 0)
			{
				const auto b = selected_faces[0].brush;
				const auto i = selected_faces[0].index;

				if (selected_faces[0].face != &selected_faces[0].brush->faces[i])
				{
					AssertS("selFace.face == &selFace.brush->faces[selFace.index]");
				}
				if (b->version != b->def->version)
				{
					AssertS("selFace.brush->version == selFace.brush->def->version");
				}
				// pt = &selected_face->texdef;
				mtl_def = &b->def->brush_faces[i].mtldef[game::g_qeglobals->current_edit_layer];
			}
			else
			{
				// pt = &g_qeglobals.d_texturewin.texdef;
				mtl_def = (game::MaterialDef*)&game::g_qeglobals->random_texture_stuff[2100 * game::g_qeglobals->current_edit_layer];
			}

			if (helper->specific_mode)
			{
				if ((mode & TEXMODE_SIZE_HORZ) != 0)
				{
					mtl_def->mat_texDef->size[0] = texdef->size[0];
				}

				if ((mode & TEXMODE_SIZE_VERT) != 0)
				{
					mtl_def->mat_texDef->size[1] = texdef->size[1];
				}

				if ((mode & TEXMODE_SHIFT_HORZ) != 0)
				{
					mtl_def->mat_texDef->shift[0] = texdef->shift[0];
				}

				if ((mode & TEXMODE_SHIFT_VERT) != 0)
				{
					mtl_def->mat_texDef->shift[1] = texdef->shift[1];
				}

				if ((mode & TEXMODE_ROTATE) != 0)
				{
					mtl_def->mat_texDef->rotate = texdef->rotate;
				}
			}
			else
			{
				mtl_def->mat_texDef->shift[0] += texdef->shift[0];
				mtl_def->mat_texDef->shift[1] += texdef->shift[1];
				mtl_def->mat_texDef->size[0] += texdef->size[0];
				mtl_def->mat_texDef->size[1] += texdef->size[1];
				mtl_def->mat_texDef->rotate += texdef->rotate;
			}

			utils::hook::call<void(__cdecl)(game::MaterialDef* _def, int _unk)>(0x48F170)(mtl_def, 0);
		}
	}

	void surface_dialog::inspector_controls(bool is_toolbox, float max_width)
	{
		int style_colors = 0;
		int style_vars = 0;

		bool treenode_state = false;

		const float inner_item_spacing = 4.0f;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(inner_item_spacing, 0.0f));

		const float clamped_min_widget_width = ImMax(186.0f, ImGui::CalcItemWidth() + 2 * inner_item_spacing);

		static texedit_helper texhelp = {};
		texhelp.scalar_direction = 0; // reset each frame

		float tex_scalar_width = 1.0f;
		float tex_scalar_height = 1.0f;

		auto& g_patch_texdef = *reinterpret_cast<patch_texdef_t*>(0x23F15F8);
		const int selected_faces_count = *reinterpret_cast<int*>(0x73C714);

		static std::string last_texture_name = ctexwnd::get_name_for_selection(); // initial
		static int last_texture_width = 64;
		static int last_texture_height = 64;

		if (is_toolbox)
		{
			treenode_state = toolbox_dialog::treenode_begin("Info", true, style_colors, style_vars);
		}

		if (!is_toolbox || (is_toolbox && treenode_state))
		{
			static float info_section_width = 0.0f;
			static float info_tex_dims_width = 0.0f;

			if (is_toolbox)
			{
				toolbox_dialog::center_horz_begin(info_section_width);
				ImGui::SetNextItemWidth(max_width);
			}

			ImGui::InputText("##curr_tex", &last_texture_name, ImGuiInputTextFlags_ReadOnly);

			if (is_toolbox)
			{
				toolbox_dialog::center_horz_end(info_section_width);
				toolbox_dialog::center_horz_begin(info_tex_dims_width);
				ImGui::SetNextItemWidth(max_width);
				ImGui::Text("Width [ %d ] - Height [ %d ]", last_texture_width, last_texture_height);
			}
			else
			{
				static float group_width = 80.0f;
				ImGui::SetCursorPosX(clamped_min_widget_width * 0.5f - (group_width * 0.5f));

				ImGui::BeginGroup();
				{
					ImGui::Text("Width [ %d ] - Height [ %d ]", last_texture_width, last_texture_height);
					ImGui::EndGroup();
				}

				group_width = ImGui::GetItemRectSize().x;
				SPACING(0.0f, 4.0f);

			}

			if(is_toolbox)
			{
				toolbox_dialog::center_horz_end(info_tex_dims_width);
				toolbox_dialog::treenode_end(style_colors, style_vars);
			}
		}

		if (is_toolbox)
		{
			treenode_state = toolbox_dialog::treenode_begin("Manual", true, style_colors, style_vars);
		}

		if (selected_faces_count > 0)
		{
			const auto selected_faces = game::g_selected_faces();

			if (selected_faces && selected_faces->brush && selected_faces->brush->def)
			{
				const auto mat_def = &selected_faces->brush->def->brush_faces[selected_faces->index].mtldef[game::g_qeglobals->current_edit_layer];

				if (mat_def->radMtl)
				{
					last_texture_name = mat_def->radMtl->name;
					last_texture_width = mat_def->radMtl->width;
					last_texture_height = mat_def->radMtl->height;

					tex_scalar_width = 1.0f / static_cast<float>(mat_def->radMtl->width);
					tex_scalar_height = 1.0f / static_cast<float>(mat_def->radMtl->height);
				}

				texhelp.size_horz = mat_def->mat_texDef->size[0] * tex_scalar_width;
				texhelp.size_vert = mat_def->mat_texDef->size[1] * tex_scalar_height;
				texhelp.shift_horz = mat_def->mat_texDef->shift[0] /** tex_scalar_width*/;
				texhelp.shift_vert = mat_def->mat_texDef->shift[1] /** tex_scalar_height*/;
				texhelp.rotation = mat_def->mat_texDef->rotate;
			}
		}
		else if (const auto	sb = game::g_selected_brushes();
							sb)
		{
			if (sb->patch)
			{
				if(sb->patch->def && sb->patch->def->texture.radMtl)
				{
					last_texture_name = sb->patch->def->texture.radMtl->name;
					last_texture_width = sb->patch->def->texture.radMtl->width;
					last_texture_height = sb->patch->def->texture.radMtl->height;
				}

				texhelp.reset_values();
				//texhelp.rotation = g_patch_texdef.def.mat_texDef->rotate;
			}
			else if (sb->def && sb->def->brush_faces && !sb->def->owner->eclass->fixedsize)
			{
				if(sb->def->brush_faces->mtldef[game::g_qeglobals->current_edit_layer].radMtl)
				{
					last_texture_name = sb->def->brush_faces->mtldef[game::g_qeglobals->current_edit_layer].radMtl->name;
					last_texture_width = sb->def->brush_faces->mtldef[game::g_qeglobals->current_edit_layer].radMtl->width;
					last_texture_height = sb->def->brush_faces->mtldef[game::g_qeglobals->current_edit_layer].radMtl->height;
				}

				if (sb->def->brush_faces->mtldef[game::g_qeglobals->current_edit_layer].radMtl)
				{
					tex_scalar_width = 1.0f / static_cast<float>(sb->def->brush_faces->mtldef[game::g_qeglobals->current_edit_layer].radMtl->width);
					tex_scalar_height = 1.0f / static_cast<float>(sb->def->brush_faces->mtldef[game::g_qeglobals->current_edit_layer].radMtl->height);
				}

				texhelp.size_horz = sb->def->brush_faces->mtldef[game::g_qeglobals->current_edit_layer].mat_texDef->size[0] * tex_scalar_width;
				texhelp.size_vert = sb->def->brush_faces->mtldef[game::g_qeglobals->current_edit_layer].mat_texDef->size[1] * tex_scalar_height;
				texhelp.shift_horz = sb->def->brush_faces->mtldef[game::g_qeglobals->current_edit_layer].mat_texDef->shift[0] /** tex_scalar_width*/;
				texhelp.shift_vert = sb->def->brush_faces->mtldef[game::g_qeglobals->current_edit_layer].mat_texDef->shift[1] /** tex_scalar_height*/;
				texhelp.rotation = sb->def->brush_faces->mtldef[game::g_qeglobals->current_edit_layer].mat_texDef->rotate;
			}
		}

		if (!is_toolbox || (is_toolbox && treenode_state))
		{
			static float manual_section_width = 0.0f;
			const float max_width_manual = max_width - 100.0f;

			if (is_toolbox)
			{
				ImGui::SetNextItemWidth(max_width_manual);
				toolbox_dialog::center_horz_begin(manual_section_width);
			}

			if (ImGui::InputScalarDir(" Shift Horz", ImGuiDataType_Float, &texhelp.shift_horz, &texhelp.scalar_direction, &texhelp.amount_shift, nullptr, true, "%.2f", !texhelp.specific_mode ? ImGuiInputTextFlags_ReadOnly : 0))
			{
				const float amount_specific = texhelp.shift_horz;
				const float amount_inc = texhelp.amount_shift;

				const float edit_amount = texhelp.specific_mode ? amount_specific
					: texhelp.scalar_direction == -1 ? -amount_inc
					: texhelp.scalar_direction == 1 ? amount_inc : 0.0f;

				game::texdef_sub_t texdef_edit = {};
				texdef_edit.shift[0] = edit_amount /*/ tex_scalar_width*/;

				edit_texture_info(&texdef_edit, &texhelp, TEXMODE_SHIFT_HORZ, texhelp.scalar_direction);
			}

			if (is_toolbox)
			{
				ImGui::SetNextItemWidth(max_width_manual);
			}

			if (ImGui::InputScalarDir(" Shift Vert", ImGuiDataType_Float, &texhelp.shift_vert, &texhelp.scalar_direction, &texhelp.amount_shift, nullptr, true, "%.2f", !texhelp.specific_mode ? ImGuiInputTextFlags_ReadOnly : 0))
			{
				const float amount_specific = texhelp.shift_vert;
				const float amount_inc = texhelp.amount_shift;

				const float edit_amount = texhelp.specific_mode ? amount_specific
					: texhelp.scalar_direction == -1 ? -amount_inc
					: texhelp.scalar_direction == 1 ? amount_inc : 0.0f;

				game::texdef_sub_t texdef_edit = {};
				texdef_edit.shift[1] = edit_amount /*/ tex_scalar_height*/;

				edit_texture_info(&texdef_edit, &texhelp, TEXMODE_SHIFT_VERT, texhelp.scalar_direction);
			}

			SPACING(0.0f, 4.0f);

			if (is_toolbox)
			{
				ImGui::SetNextItemWidth(max_width_manual);
			}

			if (ImGui::InputScalarDir(" Size Horz", ImGuiDataType_Float, &texhelp.size_horz, &texhelp.scalar_direction, &texhelp.amount_size, nullptr, true, "%.2f", !texhelp.specific_mode ? ImGuiInputTextFlags_ReadOnly : 0))
			{
				const float amount_specific = texhelp.size_horz;
				const float amount_inc = texhelp.amount_size;

				const float edit_amount = texhelp.specific_mode ? amount_specific
					: texhelp.scalar_direction == -1 ? -amount_inc
					: texhelp.scalar_direction == 1 ? amount_inc : 0.0f;

				game::texdef_sub_t texdef_edit = {};
				texdef_edit.size[0] = edit_amount / tex_scalar_width;

				edit_texture_info(&texdef_edit, &texhelp, TEXMODE_SIZE_HORZ, texhelp.scalar_direction);
			}

			if (is_toolbox)
			{
				ImGui::SetNextItemWidth(max_width_manual);
			}

			if (ImGui::InputScalarDir(" Size Vert", ImGuiDataType_Float, &texhelp.size_vert, &texhelp.scalar_direction, &texhelp.amount_size, nullptr, true, "%.2f", !texhelp.specific_mode ? ImGuiInputTextFlags_ReadOnly : 0))
			{
				const float amount_specific = texhelp.size_vert;
				const float amount_inc = texhelp.amount_size;

				const float edit_amount = texhelp.specific_mode ? amount_specific
					: texhelp.scalar_direction == -1 ? -amount_inc
					: texhelp.scalar_direction == 1 ? amount_inc : 0.0f;

				game::texdef_sub_t texdef_edit = {};
				texdef_edit.size[1] = edit_amount / tex_scalar_height;

				edit_texture_info(&texdef_edit, &texhelp, TEXMODE_SIZE_VERT, texhelp.scalar_direction);
			}

			SPACING(0.0f, 4.0f);

			if (is_toolbox)
			{
				ImGui::SetNextItemWidth(max_width_manual);
			}

			if (ImGui::InputScalarDir(" Rotation", ImGuiDataType_Float, &texhelp.rotation, &texhelp.scalar_direction, &texhelp.amount_rotate, nullptr, true, "%.2f", !texhelp.specific_mode ? ImGuiInputTextFlags_ReadOnly : 0))
			{
				const float amount_specific = texhelp.rotation;
				const float amount_inc = texhelp.amount_rotate;

				const float edit_amount = texhelp.specific_mode ? amount_specific
					: texhelp.scalar_direction == -1 ? -amount_inc
					: texhelp.scalar_direction == 1 ? amount_inc : 0.0f;

				game::texdef_sub_t texdef_edit = {};
				texdef_edit.rotate = edit_amount;

				edit_texture_info(&texdef_edit, &texhelp, TEXMODE_ROTATE);
			}

			if (is_toolbox)
			{
				ImGui::SetNextItemWidth(max_width_manual);
			}

			if (ImGui::InputScalarDir(" Sample", ImGuiDataType_Float, &g_patch_texdef.sample_size, nullptr, &texhelp.amount_sample, nullptr, true, "%.2f"))
			{
				if (g_patch_texdef.sample_size < 4.0f)
				{
					g_patch_texdef.sample_size = 4.0f;
				}

				game::Brush_SetSampleSize(static_cast<int>(g_patch_texdef.sample_size));
			}

			SPACING(0.0f, 4.0f);

			static float specific_values_width = 0.0f;
			static float group_width = 80.0f;

			if (is_toolbox)
			{
				toolbox_dialog::center_horz_end(manual_section_width);
				toolbox_dialog::center_horz_begin(specific_values_width);
			}
			else
			{
				ImGui::SetCursorPosX(clamped_min_widget_width * 0.5f - (group_width * 0.5f));
			}
			
			ImGui::BeginGroup();
			{
				ImGui::Checkbox(" Set Specific Values", &texhelp.specific_mode);
				ImGui::EndGroup();
			}

			group_width = ImGui::GetItemRectSize().x;
			ImGui::SameLine();
			ImGui::HelpMarker(
				"On: (Specific Mode)\n"
				"- Enables editing values within the textboxes to directly set texture values.\n"
				"- Set all selected faces/brushes to the specified value.\n\n"
				"Off: (Inc/Dec Mode)\n"
				"- Texture edits are only possible via the + and - buttons\n"
				"- Inc/Dec values per face/brush\n  (eg: brush1 value @ 0.0f + offset, brush2 value @ 45.0f + offset)\n\n"
				"Displayed values are always from the first face that was selected.");

			// ------

			ImGui::BeginGroup();
			{
				ImGui::Checkbox(" Use Original Logic", &texhelp.original_logic);
				ImGui::EndGroup();
			}

			// original surface inspector logic which uses texdef settings of the first face
		// and applies these onto all other selected surfaces / brushes

			group_width = ImGui::GetItemRectSize().x;
			ImGui::SameLine();
			ImGui::HelpMarker(
				"On: (Original Logic)\n"
				"- Uses settings of the first face and applies\n"
				"  these onto all other selected surfaces / brushes.\n"
				"- Supports UNDO/REDO.\n\n"
				"Off: (Additive - per Face)\n"
				"- Inc/Dec values per face/brush\n"
				"  (eg: brush1 value @ 0.0f + offset, brush2 value @ 45.0f + offset)\n\n"
				"Displayed values are always from the first face that was selected.");

			// ------

			if (!is_toolbox)
			{
				SPACING(0.0f, 4.0f);
			}
			else
			{
				toolbox_dialog::center_horz_end(specific_values_width);
			}
		}

		// #
		// end and begin next node

		if (treenode_state)
		{
			toolbox_dialog::treenode_end(style_colors, style_vars);
		}

		if (is_toolbox)
		{
			treenode_state = toolbox_dialog::treenode_begin("Texture Operations", true, style_colors, style_vars);
		}

		// ---

		if (!is_toolbox || (is_toolbox && treenode_state))
		{
			static float texop_section_width = 0.0f;
			const ImVec2 button_size = is_toolbox
					? ImVec2(max_width * 0.5f - 2.0f, 0.0f)
					: ImVec2(ImMax(182.0f, ImGui::CalcItemWidth() + inner_item_spacing) * 0.5f, 0.0f);

			if (is_toolbox)
			{
				toolbox_dialog::center_horz_begin(texop_section_width);
			}

			if (ImGui::Button("Cap", button_size))
			{
				const float sample_size = game::g_qeglobals->current_edit_layer == 1 ? g_patch_texdef.sample_size : 0.25f;

				game::Patch_NaturalizeSelected(true, false, texhelp.repeatx * sample_size, texhelp.repeaty * sample_size);
			}

			ImGui::SameLine(0.0f, inner_item_spacing);
			if (ImGui::Button("Natural", button_size))
			{
				const float sample_size = game::g_qeglobals->current_edit_layer == 1 ? g_patch_texdef.sample_size : 0.25f;

				game::Patch_NaturalizeSelected(false, false, texhelp.repeatx * sample_size, texhelp.repeaty * sample_size);
			}

			if (ImGui::Button("Fit", button_size))
			{
				game::Brush_FitTexture(1.0f, 1.0f, 0);
			}

			ImGui::SameLine(0.0f, inner_item_spacing);
			if (ImGui::Button("Lmap", button_size))
			{
				game::Patch_Lightmap_Texturing();
			}

			if (!is_toolbox)
			{
				if (ImGui::Button("Cycle Texture Layer", ImVec2(clamped_min_widget_width, ImGui::GetFrameHeight())))
				{
					cdeclcall(void, 0x424010); // CMainFrame::OnEditLayerCycle
				}

				SPACING(0.0f, 12.0f);
			}
			else
			{
				toolbox_dialog::center_horz_end(texop_section_width);
			}
		}


		// #
		// end and begin next node

		if (treenode_state)
		{
			toolbox_dialog::treenode_end(style_colors, style_vars);
		}

		if (is_toolbox)
		{
			treenode_state = toolbox_dialog::treenode_begin("Set", true, style_colors, style_vars);
		}

		if (!is_toolbox || (is_toolbox && treenode_state))
		{
			const float max_width_set = max_width - 80.0f;

			static float set_section_width = 0.0f;

			if (is_toolbox)
			{
				ImGui::SetNextItemWidth(max_width_set + 15.0f);
				toolbox_dialog::center_horz_begin(set_section_width);
			}
			else
			{
				ImGui::SetNextItemWidth(clamped_min_widget_width);
			}

			const char* set_mode_items[] = { "2D", "2D - Auto Set", "3D", "3D - Auto Set", "Patch Curve", "Patch Curve - Auto Set" };
			static int set_mode_current = 1;

			imgui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.125f));
			ImGui::Combo(" Mode", &set_mode_current, set_mode_items, IM_ARRAYSIZE(set_mode_items));
			imgui::PopStyleColor();

			if (is_toolbox)
			{
				ImGui::SetNextItemWidth(max_width_set);
			}

			if (ImGui::InputScalarDir(" Repeat X", ImGuiDataType_Float, &texhelp.repeatx, nullptr, &texhelp.amount_repeatx, nullptr, true, "%.2f"))
			{
				if (texhelp.amount_repeatx < 0.0f)
				{
					texhelp.amount_repeatx = 0.0f;
				}

				// if auto set mode
				if (set_mode_current == 1 || set_mode_current == 3 || set_mode_current == 5)
				{
					game::Patch_SetTexturing(texhelp.repeatx, texhelp.repeaty, set_mode_current == 1 ? 0 : set_mode_current == 3 ? 1 : 2);
				}
			}

			if (is_toolbox)
			{
				ImGui::SetNextItemWidth(max_width_set);
			}

			if (ImGui::InputScalarDir(" Repeat Y", ImGuiDataType_Float, &texhelp.repeaty, nullptr, &texhelp.amount_repeaty, nullptr, true, "%.2f"))
			{
				if (texhelp.amount_repeaty < 0.0f)
				{
					texhelp.amount_repeaty = 0.0f;
				}

				// if auto set mode
				if (set_mode_current == 1 || set_mode_current == 3 || set_mode_current == 5)
				{
					game::Patch_SetTexturing(texhelp.repeatx, texhelp.repeaty, set_mode_current == 1 ? 0 : set_mode_current == 3 ? 1 : 2);
				}
			}

			const ImVec2 button_size = is_toolbox
				? ImVec2(max_width, 0.0f)
				: ImVec2(clamped_min_widget_width, 0.0f);

			if (ImGui::Button("Set##set", button_size))
			{
				game::Patch_SetTexturing(texhelp.repeatx, texhelp.repeaty, set_mode_current == 1 ? 0 : set_mode_current == 3 ? 1 : 2);
			}

			if (is_toolbox)
			{
				toolbox_dialog::center_horz_end(set_section_width);
			}
		}

		if (treenode_state)
		{
			toolbox_dialog::treenode_end(style_colors, style_vars);
		}

		ImGui::PopStyleVar(); // ImGuiStyleVar_ItemInnerSpacing
		//ImGui::PopItemWidth();
	}

	bool surface_dialog::gui()
	{
		if (dvars::gui_props_surfinspector && !dvars::gui_props_surfinspector->current.integer)
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
				return false;
			}

			ImGui::Indent(8.0f);
			SPACING(0.0f, 4.0f);
			surface_dialog::inspector_controls();

			ImGui::PopStyleColor(stylecolors);
			ImGui::PopStyleVar(stylevars);
			ImGui::End();

			return true;
		}

		return false;
	}

	void surface_dialog::on_open()
	{ }

	void surface_dialog::on_close()
	{ }

	// CMainFrame::OnTexturesInspector
	void surface_dialog::on_surfaceinspector_command()
	{
		if (dvars::gui_use_new_surfinspector && dvars::gui_props_surfinspector && dvars::gui_use_new_surfinspector->current.enabled)
		{
			// within entity properties
			if (dvars::gui_props_surfinspector->current.integer == 1)
			{
				const auto ent = GET_GUI(ggui::entity_dialog);

				if (ent->is_inactive_tab() && ent->is_active())
				{
					ent->set_bring_to_front(true);
					return;
				}

				ent->toggle();
			}
			// within toolbox
			else if (dvars::gui_props_surfinspector->current.integer == 2)
			{
				const auto tb = GET_GUI(ggui::toolbox_dialog);

				tb->set_bring_to_front(true);
				tb->focus_child(toolbox_dialog::TB_CHILD::SURFACE_INSP);
				tb->open();
			}
			// separate 
			else
			{
				const auto surf = GET_GUI(ggui::surface_dialog);

				if (surf->is_inactive_tab() && surf->is_active())
				{
					surf->set_bring_to_front(true);
					return;
				}

				surf->toggle();
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
		// 0 : off
		// 1 : within entity property window
		// 2 : within toolbox
		dvars::gui_props_surfinspector = dvars::register_int(
			/* name		*/ "gui_props_surfinspector",
			/* default	*/ 2,
			/* mins		*/ 0,
			/* maxs		*/ 2,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "integrate surface inspector window into entity properties (1) or toolbox (2)");

		dvars::gui_use_new_surfinspector = dvars::register_bool(
			/* name		*/ "gui_use_new_surfinspector",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "use the new experimental surface inspector window (replaces the original hotkey)");
	}

	REGISTER_GUI(surface_dialog);
}
