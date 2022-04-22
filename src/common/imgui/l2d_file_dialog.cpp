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

namespace ImGui::FileDialog
{
	bool file_dialog_open = false;
	FileDialogType file_dialog_open_type = FileDialogType::OpenFile;

	bool ShowFileDialog(bool* open, std::string& path_str, FileDialogType type, std::string ext_str)
	{
		bool result = false;

		static int file_dialog_file_select_index = 0;
		static int file_dialog_folder_select_index = 0;
		static std::string file_dialog_current_path = std::filesystem::current_path().string();
		static std::string file_dialog_current_file = "";
		static std::string file_dialog_current_folder = "";
		static char file_dialog_error[500] = "";

		static bool update_files_and_folders = false;
		static bool initial_path_set = false;

		if (open) 
		{
			// Check if there was already something in the buffer. If so, try to use that path (if it exists).
			// If it doesn't exist, just put them into the current path.
			if (!initial_path_set && !path_str.empty())
			{
				auto path = std::filesystem::path(path_str);

				if (std::filesystem::is_directory(path)) 
				{
					file_dialog_current_path = path_str;
				}
				else 
				{
					// Check if this is just a file in a real path. If so, use the real path.
					// If that still doesn't work, use current path.
					if (std::filesystem::exists(path)) 
					{
						// It's a file! Take the path and set it.
						file_dialog_current_path = path.remove_filename().string();
					}
					else 
					{
						// An invalid path was entered
						file_dialog_current_path = std::filesystem::current_path().string();
					}
				}

				update_files_and_folders = true;
				initial_path_set = true;
			}

			ImGui::SetNextWindowSize(ImVec2(736.0f, 437.0f), ImGuiCond_Appearing);
			const char* window_title = (type == FileDialogType::OpenFile ? "Select a file" : "Select a folder");

			if(!ImGui::Begin(window_title, open, ImGuiWindowFlags_NoScrollbar /*| ImGuiWindowFlags_NoResize*/))
			{
				ImGui::PopStyleVar();
				ImGui::End();
			}

			static std::vector<std::filesystem::directory_entry> files;
			static std::vector<std::filesystem::directory_entry> folders;

			// add dot to file ext. string
			if(!ext_str.empty())
			{
				if(!ext_str.starts_with("."))
				{
					ext_str = "." + ext_str;
				}
			}

			// #
			//update files and folders (only when needed)

			if(files.empty() || folders.empty() || update_files_and_folders)
			{
				files.clear();
				folders.clear();

				try
				{
					for (auto& p : std::filesystem::directory_iterator(file_dialog_current_path))
					{
						if (p.is_directory())
						{
							folders.emplace_back(p);
						}
						else
						{
							if(ext_str.empty() || p.path().extension() == ext_str)
							{
								files.emplace_back(p);
							}
						}
					}
				}
				catch (...) {}

				update_files_and_folders = false;
			}

			// calc. child sizes to allow window resizing
			const auto content_size = ImGui::GetWindowSize();
			const auto left_child_size = ImClamp(ImVec2(content_size.x * 0.25f, content_size.y - 112.0f), ImVec2(100, 0), ImVec2(250.0f, 99999.0f));
			const auto right_child_size = ImVec2(content_size.x - left_child_size.x - 20.0f, left_child_size.y);


			// display current path (editable)
			std::string path_edit = file_dialog_current_path;
			if (ImGui::InputText("##text", &path_edit, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				if (!path_edit.empty())
				{
					if (auto lpath = std::filesystem::path(path_edit);
						std::filesystem::is_directory(lpath))
					{
						file_dialog_current_path = path_edit;
						update_files_and_folders = true;
					}
				}
			}

			// #
			// directories child

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(11.0f, 5.0f));
			ImGui::BeginChild("Directories##1", left_child_size, true, ImGuiWindowFlags_HorizontalScrollbar);
			{
				if (ImGui::Selectable("..", false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetContentRegionAvail().x, 0)))
				{
					if (ImGui::IsMouseDoubleClicked(0))
					{
						file_dialog_current_path = std::filesystem::path(file_dialog_current_path).parent_path().string();
						update_files_and_folders = true;
					}
				}

				for (int i = 0; i < folders.size(); ++i)
				{
					if (ImGui::Selectable(folders[i].path().stem().string().c_str(), i == file_dialog_folder_select_index, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
					{
						file_dialog_current_file = "";
						if (ImGui::IsMouseDoubleClicked(0))
						{
							file_dialog_current_path = folders[i].path().string();
							file_dialog_folder_select_index = 0;
							file_dialog_file_select_index = 0;
							ImGui::SetScrollHereY(0.0f);
							file_dialog_current_folder = "";

							update_files_and_folders = true;
						}
						else
						{
							file_dialog_folder_select_index = i;
							file_dialog_current_folder = folders[i].path().stem().string();
						}
					}
				}
			}
			ImGui::EndChild();
			ImGui::PopStyleVar();

			// #
			// files child

			static auto reset_everything = [&]()
			{
				file_dialog_file_select_index = 0;
				file_dialog_folder_select_index = 0;
				file_dialog_current_file = "";
				strcpy_s(file_dialog_error, "");
				initial_path_set = false;
				file_dialog_open = false;
			};

			static auto select_file_dblclick = [&]()
			{
				if (type == FileDialogType::SelectFolder)
				{
					if (file_dialog_current_folder.empty())
					{
						strcpy_s(file_dialog_error, "Error: You must select a folder!");
					}
					else
					{
						path_str = file_dialog_current_path + (file_dialog_current_path.back() == '\\' ? "" : "\\") + file_dialog_current_folder;
						strcpy_s(file_dialog_error, "");

						reset_everything();
						result = true;
					}
				}
				else if (type == FileDialogType::OpenFile)
				{
					if (file_dialog_current_file.empty())
					{
						strcpy_s(file_dialog_error, "Error: You must select a file!");
					}
					else
					{
						path_str = file_dialog_current_path + (file_dialog_current_path.back() == '\\' ? "" : "\\") + file_dialog_current_file;
						strcpy_s(file_dialog_error, "");

						reset_everything();
						result = true;
					}
				}
			};

			ImGui::SameLine();
			ImGui::BeginChild("Files##1", right_child_size, true, ImGuiWindowFlags_HorizontalScrollbar);
			{
				// fixed size header and proper sorting
				if (ImGui::BeginTable("##filestable", 4,
					ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_PadOuterX))
				{
					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableSetupColumn("Files", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 230.0f);
					ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthStretch, 80.0f);
					ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 40.0f);
					ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthStretch, 80.0f);
					ImGui::TableHeadersRow();

					if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
					{
						if (sorts_specs->SpecsDirty)
						{
							switch(sorts_specs->Specs->ColumnIndex)
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
						}
					}

					// use clipper to skip invisible files

					int filelist_row = 0;
					ImGuiListClipper filelist_clipper;
					filelist_clipper.Begin(static_cast<int>(files.size()));

					while (filelist_clipper.Step())
					{
						for (int i = filelist_clipper.DisplayStart; i < filelist_clipper.DisplayEnd; i++)
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
									if (ImGui::Selectable(files[i].path().filename().string().c_str(), i == file_dialog_file_select_index, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
									{
										file_dialog_file_select_index = i;
										file_dialog_current_file = files[i].path().filename().string();
										file_dialog_current_folder = "";

										if (ImGui::IsMouseDoubleClicked(0))
										{
											select_file_dblclick();
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

								// column
								ImGui::PopID();
							}

							// row
							ImGui::PopID();
						} 
					} filelist_clipper.End();

					ImGui::EndTable();
				}
			}
			ImGui::EndChild();

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
				disable_delete_button = file_dialog_current_folder.empty();

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

				ImVec2 center(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x * 0.5f, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y * 0.5f);
				ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

				if (ImGui::BeginPopup("NewFolderPopup", ImGuiWindowFlags_Modal))
				{
					ImGui::Text("Enter a name for the new folder");
					static char new_folder_name[500] = "";
					static char new_folder_error[500] = "";
					ImGui::InputText("##newfolder", new_folder_name, sizeof(new_folder_name));

					if (ImGui::Button("Create##1"))
					{
						if (strlen(new_folder_name) <= 0)
						{
							strcpy_s(new_folder_error, "Folder name can't be empty");
						}
						else 
						{
							std::string new_file_path = file_dialog_current_path + (file_dialog_current_path.back() == '\\' ? "" : "\\") + new_folder_name;
							std::filesystem::create_directory(new_file_path);
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
					ImGui::EndPopup();
				}

				ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

				if (ImGui::BeginPopup("DeleteFolderPopup", ImGuiWindowFlags_Modal))
				{
					ImGui::TextColored(ImColor(1.0f, 0.0f, 0.2f, 1.0f), "Are you sure you want to delete this folder?");
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
					ImGui::TextUnformatted(file_dialog_current_folder.c_str());
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
					if (ImGui::Button("Yes"))
					{
						std::filesystem::remove(file_dialog_current_path + (file_dialog_current_path.back() == '\\' ? "" : "\\") + file_dialog_current_folder);
						ImGui::CloseCurrentPopup();
					}

					ImGui::SameLine();
					if (ImGui::Button("No"))
					{
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				static float right_align_group_width = 120.0f;
				ImGui::SameLine(ImGui::GetWindowWidth() - right_align_group_width - 12.0f);

				ImGui::BeginGroup();
				{
					if (ImGui::Button("Select"))
					{
						select_file_dblclick();
					}

					ImGui::SameLine();
					if (ImGui::Button("Cancel"))
					{
						reset_everything();
					}
				}
				ImGui::EndGroup();
				right_align_group_width = ImGui::GetItemRectSize().x;


				if (strlen(file_dialog_error) > 0)
				{
					ImGui::TextColored(ImColor(1.0f, 0.0f, 0.2f, 1.0f), file_dialog_error);
				}
			}
			ImGui::EndGroup();
			bottom_align_group_height = ImGui::GetItemRectSize().y;

			ImGui::End();
		}

		return result;
	}
}