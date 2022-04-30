/*
	Copyright 2020 Limeoats

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

// based on https://github.com/Limeoats/L2DFileDialog/tree/b902d7faae37a2b1ab9b6b7ce3c123f42963061e

#include "std_include.hpp"

namespace ggui
{
	bool file_dialog::dialog()
	{
		this->m_track_result = false;

		// add dot to file ext. string
		if (!this->get_file_ext().empty())
		{
			if (!this->get_file_ext().starts_with("."))
			{
				this->set_file_ext("." + this->get_file_ext());
			}
		}

		ImGui::SetNextWindowSizeConstraints(ImVec2(640.0f, 420.0f), ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(ImVec2(924.0f, 510.0f), ImGuiCond_Appearing);

		std::string window_title = "Select a ";
		window_title += !this->get_file_ext().empty() ? this->get_file_ext() + " " : "";
		window_title += this->get_file_op_type() == FileDialogType::SelectFolder ? "folder" : "file";

		if (!ImGui::Begin(window_title.c_str(), this->get_p_open(), ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
		{
			ImGui::End();
			return false;
		}

		static std::vector<std::filesystem::directory_entry> files;
		static std::vector<std::filesystem::directory_entry> folders;


		// #
		//update files and folders (only when needed)

		if (files.empty() || folders.empty() || this->m_update_files_and_folders)
		{
			files.clear();
			folders.clear();

			try
			{
				for (auto& p : std::filesystem::directory_iterator(this->m_current_path))
				{
					if (p.is_directory())
					{
						folders.emplace_back(p);
					}
					else
					{
						if (this->get_file_ext().empty() || p.path().extension() == this->get_file_ext())
						{
							files.emplace_back(p);
						}
					}
				}
			}
			catch (...) {}

			this->m_update_files_and_folders = false;
			this->m_update_sorting = true;
		}

		// calc. child sizes to allow window resizing
		const auto content_size = ImGui::GetWindowSize();
		const auto left_child_size = ImClamp(ImVec2(content_size.x * 0.25f, content_size.y - 112.0f), ImVec2(100, 0), ImVec2(250.0f, 99999.0f));
		const auto right_child_size = ImVec2(content_size.x - left_child_size.x - 20.0f, left_child_size.y);

		if (ImGui::Button("<"))
		{
			this->m_current_path = std::filesystem::path(this->m_current_path).parent_path().string();
			this->m_update_files_and_folders = true;
		}

		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 6.0f - ImGui::GetCursorPos().x);

		// display current path (editable)
		std::string path_edit = this->m_current_path;
		if (ImGui::InputText("##text", &path_edit, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			if (!path_edit.empty())
			{
				if (auto lpath = std::filesystem::path(path_edit);
					std::filesystem::is_directory(lpath))
				{
					this->m_current_path = path_edit;
					this->m_update_files_and_folders = true;
				}
			}
		}

		// #
		// directories child

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(11.0f, 5.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f));
		ImGui::BeginChild("Directories##1", left_child_size, true, ImGuiWindowFlags_HorizontalScrollbar);
		{
			if (!this->m_current_path.empty())
			{
				ImGui::TextUnformatted("v");
				ImGui::SameLine();
				ImGui::PushFontFromIndex(ggui::BOLD_18PX);

				const std::size_t pos = this->m_current_path.find_last_of('\\') + 1;
				std::string parent_folder = this->m_current_path.substr(pos);

				ImGui::TextUnformatted(parent_folder.c_str());
				ImGui::PopFont();
			}
			else
			{
				ImGui::TextUnformatted("...");
			}


			for (size_t i = 0u; i < folders.size(); ++i)
			{
				if (ImGui::Selectable(utils::va("| %s", folders[i].path().stem().string().c_str()), static_cast<int>(i) == this->m_folder_select_index, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
				{
					this->m_current_file = "";
					if (ImGui::IsMouseDoubleClicked(0))
					{
						this->m_current_path = folders[i].path().string();
						this->m_folder_select_index = 0;
						this->m_file_select_index = 0;
						ImGui::SetScrollHereY(0.0f);
						this->m_current_folder = "";

						this->m_update_files_and_folders = true;
					}
					else
					{
						this->m_folder_select_index = static_cast<int>(i);
						this->m_current_folder = folders[i].path().stem().string();
					}
				}
			}
		}
		ImGui::EndChild();
		ImGui::PopStyleVar(2);




		




		// early out upon selecting a valid file or folder
		bool early_out = false;

		ImGui::SameLine();
		ImGui::BeginChild("Files##1", right_child_size, true, ImGuiWindowFlags_HorizontalScrollbar);
		{
			// fixed size header and proper sorting
			if (ImGui::BeginTable("##filestable", 4,
				ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_PadOuterX))
			{
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("Files", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_DefaultSort, 230.0f);
				ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthStretch, 80.0f);
				ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 40.0f);
				ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthStretch, 80.0f);
				ImGui::TableHeadersRow();

				if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
				{
					// sorting was changed manually or file dialog was just opened and the table might still have sorting features applied from the last dialog -> sort
					if (sorts_specs->SpecsDirty || this->m_update_sorting)
					{
						switch (sorts_specs->Specs->ColumnIndex)
						{
						case 0 /*File*/:
							std::ranges::sort(files.begin(), files.end(), [&](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
								{
									if (sorts_specs->Specs->SortDirection == ImGuiSortDirection_Descending)
									{
										return a.path().filename().string() > b.path().filename().string();
									}
									else
									{
										return a.path().filename().string() < b.path().filename().string();
									}
								});
							break;

						case 1 /*Size*/:
							std::ranges::sort(files.begin(), files.end(), [&](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
								{
									if (sorts_specs->Specs->SortDirection == ImGuiSortDirection_Descending)
									{
										return a.file_size() > b.file_size();
									}
									else
									{
										return a.file_size() < b.file_size();
									}
								});
							break;

						case 2 /*Type*/:
							std::ranges::sort(files.begin(), files.end(), [&](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
								{
									if (sorts_specs->Specs->SortDirection == ImGuiSortDirection_Descending)
									{
										return a.path().extension().string() > b.path().extension().string();
									}
									else
									{
										return a.path().extension().string() < b.path().extension().string();
									}
								});
							break;

						case 3:
							std::ranges::sort(files.begin(), files.end(), [&](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
								{
									if (sorts_specs->Specs->SortDirection == ImGuiSortDirection_Descending)
									{
										return a.last_write_time() > b.last_write_time();
									}
									else
									{
										return a.last_write_time() < b.last_write_time();
									}
								});

							break;
						}

						sorts_specs->SpecsDirty = false;
						this->m_update_sorting = false;
					}
				}

				// use clipper to skip invisible files

				int filelist_row = 0;

				// #
				// #

				auto list_file = [&](const int i) -> void
				{
					// unique widget id's for each row
					ImGui::PushID(filelist_row); filelist_row++;
					ImGui::TableNextRow();

					for (int column = 0; column < 4; column++)
					{
						ImGui::PushID(column);
						ImGui::TableNextColumn();

						switch (column)
						{
						case 0:
							if (ImGui::Selectable(files[i].path().filename().string().c_str(), i == this->m_file_select_index, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
							{
								this->m_file_select_index = i;
								this->m_current_file = files[i].path().filename().string();
								this->m_save_file_name = this->m_current_file;
								this->m_save_filename_valid = !this->m_save_file_name.empty();
								this->m_current_folder = "";

								if (this->get_file_handler() == ggui::FILE_DIALOG_HANDLER::MISC_MODEL)
								{
									if (const auto	msd = GET_GUI(ggui::modelselector_dialog);
													msd->is_active())
									{
										msd->set_bring_to_front(true);
										msd->m_preview_model_name = this->m_current_file;
									}
								}

								if (ImGui::IsMouseDoubleClicked(0))
								{
									this->m_track_result = this->selection_build_path_to_file();
									if (this->m_track_result)
									{
										ImGui::PopID(); // column
										ImGui::PopID(); // row
										early_out = true;
									}
								}
							}
							break;

						case 1:
							ImGui::TextUnformatted(utils::va("%d KB", files[i].file_size() / 1024));
							break;

						case 2:
							ImGui::TextUnformatted(files[i].path().extension().string().c_str());
							break;

						case 3:
							auto ftime = files[i].last_write_time();
							auto st = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now());
							std::time_t tt = std::chrono::system_clock::to_time_t(st);
							std::tm* mt = std::localtime(&tt);
							std::stringstream ss;
							ss << std::put_time(mt, "%F %R");
							ImGui::TextUnformatted(ss.str().c_str());
							break;
						}

						if (early_out)
						{
							break;
						}

						// column
						ImGui::PopID();
					}

					if (!early_out)
					{
						// row
						ImGui::PopID();
					}
				};

				// #
				// #

				{
					
					static std::vector<int> filtered_list;

					if (m_filter.IsActive())
					{
						static std::string last_filter;

						if (m_filter.InputBuf != last_filter)
						{
							filtered_list.clear();

							for (size_t i = 0u; i < files.size(); i++)
							{
								std::string item_name = files[i].path().filename().string();

								if (!m_filter.PassFilter(item_name.c_str()))
								{
									continue;
								}

								filtered_list.emplace_back(i);
							}

							last_filter = m_filter.InputBuf;
						}
					}

					ImGuiListClipper filelist_clipper;
					filelist_clipper.Begin(
						m_filter.IsActive() ? static_cast<int>(filtered_list.size()) : static_cast<int>(files.size()));

					while (filelist_clipper.Step() && !early_out)
					{
						for (int i = filelist_clipper.DisplayStart; i < filelist_clipper.DisplayEnd; i++)
						{
							list_file(m_filter.IsActive() ? filtered_list[i] : i);

							if(this->m_track_result)
							{
								break;
							}
						}
					}

					filelist_clipper.End();
				}

				ImGui::EndTable();
			}
		}
		ImGui::EndChild();
		
		if(this->m_track_result)
		{
			ImGui::End();
			return this->m_track_result;
		}

		// #
		// bottom buttons                                                         

		static float bottom_align_group_height = 40.0f;
		float bottom_y = ImGui::GetWindowHeight() - bottom_align_group_height - 10.0f;
		ImGui::SetCursorPosY(bottom_y);

		ImGui::BeginGroup();
		{
			if (ImGui::Button("New folder"))
			{
				ImGui::OpenPopup("NewFolderPopup");
			}
			ImGui::SameLine();

			static bool disable_delete_button = false;
			disable_delete_button = this->m_current_folder.empty();

			if (disable_delete_button)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (ImGui::Button("Delete folder"))
			{
				ImGui::OpenPopup("DeleteFolderPopup");
			}

			if (disable_delete_button)
			{
				ImGui::PopStyleVar();
				ImGui::PopItemFlag();
			}

			static float right_align_group_width = 120.0f;


			if(this->get_file_op_type() == FileDialogType::SaveFile)
			{
				ImGui::SameLine();
				float calc_width = ImGui::GetWindowWidth() - right_align_group_width - 18.0f - ImGui::GetCursorPos().x;

				this->m_save_file_name = this->m_current_file;

				ImGui::SetNextItemWidth(calc_width);
				if (ImGui::InputText("##filename", &m_save_file_name))
				{
					this->m_save_filename_valid = !this->m_save_file_name.empty();
					if(this->m_save_filename_valid)
					{
						this->m_current_file = this->m_save_file_name;
					}
				}

				// check if enter was pressed
				if(ImGui::IsItemDeactivatedAfterEdit()) 
				{
					if (this->m_save_filename_valid)
					{
						this->m_track_result = this->selection_build_path_to_file();
					}
				}
			}
			else // filter 
			{
				ImGui::SameLine();

				const float calc_width = ImGui::GetWindowWidth() - right_align_group_width - 18.0f - ImGui::GetCursorPos().x;

				const auto pre_filter_pos = ImGui::GetCursorScreenPos();
				m_filter.Draw("##filedialog_filter", calc_width);

				if (!m_filter.IsActive())
				{
					ImGui::GetWindowDrawList()->AddText(
						ImVec2(pre_filter_pos.x + 12.0f, pre_filter_pos.y + 4.0f), 
						ImGui::GetColorU32(ImGuiCol_ButtonHovered),
						"Filter ..");
				}
			}


			// # Folder creation popup

			ImVec2 center(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x * 0.5f, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y * 0.5f);
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 5.0f));
			if (ImGui::BeginPopup("NewFolderPopup", ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoMove))
			{
				const ImGuiStyle& style = ImGui::GetStyle();

				static float label_width = 120.0f;
				ImGui::PushFontFromIndex(ggui::BOLD_18PX);
				ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() - label_width + style.FramePadding.x) * 0.5f);
				ImGui::TextUnformatted("Enter a name for the new folder");
				label_width = ImGui::GetItemRectSize().x;
				ImGui::PopFont();

				ImGui::Spacing();

				static char new_folder_name[500] = "";
				static char new_folder_error[500] = "";

				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetColorU32(ImGuiCol_Button));
				ImGui::InputText("##newfolder", new_folder_name, sizeof(new_folder_name));
				ImGui::PopStyleColor();

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 16);

				static float modal_button_group_width = 100.0f;
				ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() - modal_button_group_width + style.FramePadding.x) * 0.5f);
				ImGui::BeginGroup();
				{
					if (ImGui::Button("Create##1"))
					{
						if (strlen(new_folder_name) <= 0)
						{
							strcpy_s(new_folder_error, "Folder name can't be empty");
						}
						else
						{
							std::string new_file_path = this->m_current_path + (this->m_current_path.back() == '\\' ? "" : "\\") + new_folder_name;
							std::filesystem::create_directory(new_file_path);

							this->m_update_files_and_folders = true;
							ImGui::CloseCurrentPopup();
						}
					}

					ImGui::SameLine();
					if (ImGui::Button("Cancel##1"))
					{
						strcpy_s(new_folder_name, "");
						strcpy_s(new_folder_error, "");
						ImGui::CloseCurrentPopup();
					}

					ImGui::TextColored(ImColor(1.0f, 0.0f, 0.2f, 1.0f), new_folder_error);
				}
				ImGui::EndGroup();
				modal_button_group_width = ImGui::GetItemRectSize().x;

				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();

			ImGui::SetNextWindowSize(ImVec2(180.0f, 120.0f));
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 5.0f));
			if (ImGui::BeginPopup("DeleteFolderPopup", ImGuiWindowFlags_Modal))
			{
				const ImGuiStyle& style = ImGui::GetStyle();

				static float label_width = 120.0f;
				ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() - label_width + style.FramePadding.x) * 0.5f);
				ImGui::TextColored(ImColor(1.0f, 0.0f, 0.2f, 1.0f), "Delete this folder?");
				label_width = ImGui::GetItemRectSize().x;

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);

				static float folder_str_width = 120.0f;
				ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() - folder_str_width + style.FramePadding.x) * 0.5f);
				ImGui::PushFontFromIndex(ggui::BOLD_18PX);
				ImGui::TextUnformatted(this->m_current_folder.c_str());
				folder_str_width = ImGui::GetItemRectSize().x;
				ImGui::PopFont();
				ImGui::AddUnterline(ImGui::GetColorU32(ImGuiCol_ButtonActive));

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 14);

				static float modal_button_group_width = 100.0f;
				ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() - modal_button_group_width + style.FramePadding.x) * 0.5f);
				ImGui::BeginGroup();
				{
					if (ImGui::Button("Yes"))
					{
						std::filesystem::remove(this->m_current_path + (this->m_current_path.back() == '\\' ? "" : "\\") + this->m_current_folder);

						this->m_update_files_and_folders = true;
						ImGui::CloseCurrentPopup();
					}

					ImGui::SameLine();
					if (ImGui::Button("No"))
					{
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::EndGroup();
				modal_button_group_width = ImGui::GetItemRectSize().x;

				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();

			
			ImGui::SameLine(ImGui::GetWindowWidth() - right_align_group_width - 12.0f);
			ImGui::BeginGroup();
			{
				const bool op_save = this->get_file_op_type() == FileDialogType::SaveFile;
				ImGui::BeginDisabled(!this->m_save_filename_valid && op_save);
				{
					if (ImGui::Button(op_save ? "Save" : "Select"))
					{
						this->m_track_result = this->selection_build_path_to_file();
					}
				}
				ImGui::EndDisabled();
				

				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					this->reset();
					this->m_was_canceled = true;
				}
			}
			ImGui::EndGroup();
			right_align_group_width = ImGui::GetItemRectSize().x;
		}
		ImGui::EndGroup();
		bottom_align_group_height = ImGui::GetItemRectSize().y;

		// currently renders "out of bounds"
		if (strlen(this->m_error) > 0)
		{
			ImGui::TextColored(ImColor(1.0f, 0.0f, 0.2f, 1.0f), this->m_error);
		}

		ImGui::End();
		return this->m_track_result;
	}

	void file_dialog::on_open()
	{
		this->m_was_canceled = false;

		// setup default path or use working directory
		if (!this->m_initial_path_set && !get_default_path().empty())
		{
			auto defpath = std::filesystem::path(get_default_path());

			if (std::filesystem::is_directory(defpath))
			{
				this->m_current_path = get_default_path();
			}
			else
			{
				// Check if this is just a file in a real path. If so, use the real path.
				// If that still doesn't work, use current path.
				if (std::filesystem::exists(defpath))
				{
					// It's a file! Take the path and set it.
					this->m_current_path = defpath.remove_filename().string();
				}
				else
				{
					// An invalid path was entered
					this->m_current_path = std::filesystem::current_path().string();
				}
			}

			this->m_update_files_and_folders = true;
			this->m_initial_path_set = true;
		}
	}

	void file_dialog::on_close()
	{
		if(!this->m_fix_on_close)
		{
			if (!this->m_track_result)
			{
				this->m_was_canceled = true;
			}
		}
		else
		{
			this->m_fix_on_close = false;
		}
	}

	REGISTER_GUI(file_dialog);
}