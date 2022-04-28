#include "std_include.hpp"
#include "commdlg.h"

namespace components
{
	const float fxcurve_default_keys[] =
	{
		0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f, // color - r g b
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f
	};

	bool effects_editor::editor_can_add_segment()
	{
		return fx_system::get_editor_effect()->elemCount != 32;
	}

	bool effects_editor::editor_can_delete_segment()
	{
		return fx_system::get_editor_effect()->elemCount != 0;
	}

	
	// CMainframe::OnAddSegment
	void effects_editor::editor_add_new_segment()
	{
		if (effects_editor::editor_can_add_segment())
		{
			// #NOT_IMPLEMENTED
			// UNDO

			effects::stop();

			const auto editor_effect = fx_system::get_editor_effect();

			fx_system::FxEditorElemDef* free_elem = &editor_effect->elems[editor_effect->elemCount];
			++editor_effect->elemCount;

			memset(free_elem, 0, sizeof(fx_system::FxEditorElemDef));
			sprintf(free_elem->name, "segment %i", editor_effect->elemCount);

			free_elem->elemType = 0;
			free_elem->flags = fx_system::FX_ELEM_RUN_RELATIVE_TO_SPAWN;
			free_elem->spawnLooping.intervalMsec = 200;
			free_elem->lifeSpanMsec.base = 1000;

			for (auto i = 0; i < 2; i++)
			{
				for (auto x = 0; x < 2; x++)
				{
					for (auto y = 0; y < 3; y++)
					{
						free_elem->velShape[x][y][i] = fx_system::FxCurve_AllocAndCreateWithKeys(&fxcurve_default_keys[4], 1, 2);
					}

					free_elem->sizeShape[x][i] = fx_system::FxCurve_AllocAndCreateWithKeys(fxcurve_default_keys, 1, 2);
				}

				free_elem->rotationShape[i] = fx_system::FxCurve_AllocAndCreateWithKeys(&fxcurve_default_keys[4], 1, 2);
				free_elem->scaleShape[i] = fx_system::FxCurve_AllocAndCreateWithKeys(fxcurve_default_keys, 1, 2);
				free_elem->color[i] = fx_system::FxCurve_AllocAndCreateWithKeys(&fxcurve_default_keys[8], 3, 2);
				free_elem->alpha[i] = fx_system::FxCurve_AllocAndCreateWithKeys(fxcurve_default_keys, 1, 2);
			}

			free_elem->trailScrollTime = 0.0f;
			free_elem->trailDef.indCount = 0;
			free_elem->trailDef.vertCount = 0;
			free_elem->trailSplitDist = 100;
			free_elem->trailRepeatDist = 10;

			//effects::play();
			components::effects::apply_changes();
		}
	}

	// CMainframe::OnDeleteSegment
	void effects_editor::editor_delete_segment(int index)
	{
		// #NOT_IMPLEMENTED
		// UNDO

		effects::stop();

		const auto editor_effect = fx_system::get_editor_effect();

		fx_system::FxEditorElemDef* elem = &editor_effect->elems[index];

		for (auto i = 0; i < 2; i++)
		{
			for (auto x = 0; x < 2; x++)
			{
				for (auto y = 0; y < 3; y++)
				{
					 fx_system::FxCurveIterator_FreeRef(elem->velShape[x][y][i]);
				}

				fx_system::FxCurveIterator_FreeRef(elem->sizeShape[x][i]);
			}

			fx_system::FxCurveIterator_FreeRef(elem->rotationShape[i]);
			fx_system::FxCurveIterator_FreeRef(elem->scaleShape[i]);
			fx_system::FxCurveIterator_FreeRef(elem->color[i]);
			fx_system::FxCurveIterator_FreeRef(elem->alpha[i]);
		}

		memcpy(elem, &editor_effect->elems[index + 1], sizeof(fx_system::FxEditorElemDef) * (--editor_effect->elemCount - index));

		//effects::play();
		components::effects::apply_changes();
	}

	bool effects_editor::save_as()
	{
		char filename[MAX_PATH];
		OPENFILENAMEA ofn;
		ZeroMemory(&filename, sizeof(filename));
		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = cmainframe::activewnd->GetWindow();
		ofn.lpstrFilter = "Effect Files\0*.efx\0";
		ofn.lpstrFile = filename;
		ofn.lpstrDefExt = ".efx";
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = "Save effect as ...";
		ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

		if (GetSaveFileNameA(&ofn))
		{
			if(fx_system::FX_SaveEditorEffect(filename))
			{
				/*const std::string filepath = filename;
				const std::string replace_path = "raw\\fx\\";
				const std::size_t pos = filepath.find(replace_path) + replace_path.length();

				std::string loc_filepath = filepath.substr(pos);
				utils::erase_substring(loc_filepath, ".efx"s);
				utils::replace(loc_filepath, "\\", "/");

				effects::load_effect(loc_filepath.c_str());*/

				game::printf_to_console("[*] Successfully saved effect: %s", filename);

				return true;
			}
		}

		return false;
	}

	bool effects_editor::is_editor_active()
	{
		return GET_GUI(ggui::effects_editor_dialog)->is_active();
	}

	bool effects_editor::has_unsaved_changes()
	{
		return GET_GUI(ggui::effects_editor_dialog)->m_effect_was_modified;
	}

	effects_editor::effects_editor()
	{ }

	effects_editor::~effects_editor()
	{ }
}
