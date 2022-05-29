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

#pragma once

namespace ggui
{
	class file_dialog final : public ggui::ggui_module
	{
	public:
		enum class FileDialogType
		{
			OpenFile,
			SelectFolder,
			SaveFile
		};

	private:
		bool m_update_files_and_folders = false;
		bool m_update_sorting = false;
		bool m_initial_path_set = false;

		int  m_file_select_index = 0;
		int  m_folder_select_index = 0;
		char m_error[500] = {};

		std::string m_desktop_path;

		ImGuiTextFilter	m_filter;
		FileDialogType m_file_operation_type = FileDialogType::OpenFile;
		std::string m_file_ext;
		std::string m_default_path;
		std::string m_current_path;
		std::string m_current_file;
		std::string m_current_folder;
		bool m_blocking = false;

		std::string m_save_file_name;
		bool m_save_filename_valid = false;

		std::function<void()> m_callback;

		// persistent data after dialog was closed
		std::string m_last_filepath;
		int m_last_file_handler = 0;
		bool m_was_canceled = false;
		bool m_track_result = false;
		bool m_fix_on_close = false;

	public:
		file_dialog();

		void reset(bool reset_callback = false)
		{
			this->m_file_select_index = 0;
			this->m_folder_select_index = 0;
			strcpy_s(this->m_error, "");

			m_file_operation_type = FileDialogType::OpenFile;
			m_file_ext = "";

			set_default_path("");
			this->m_initial_path_set = false;
			this->m_current_path = "";
			this->m_current_file = "";
			this->m_current_folder = "";
			this->m_blocking = false;
			
			if(reset_callback)
			{
				this->m_callback = nullptr;
			}

			this->m_was_canceled = false;
			this->close();
		}

		void set_callback(const std::function<void()>& _callback)
		{
			if(!this->is_active())
				this->m_callback = _callback;
		}
		[[nodiscard]] bool do_callback()
		{
			if(this->m_callback != nullptr && !this->get_path_result().empty() && !this->was_canceled())
			{
				this->m_callback();
				this->m_callback = nullptr;
				return true;
			}

			return false;
		}

		void set_blocking()
		{
			this->m_blocking = true;
		}
		[[nodiscard]] bool is_blocking() const
		{
			return this->m_blocking;
		}

		void set_file_op_type(const FileDialogType type)
		{
			if(!this->is_active())
				m_file_operation_type = type;
		}
		[[nodiscard]] FileDialogType get_file_op_type() const
		{
			return m_file_operation_type;
		}

		void set_file_ext(const std::string& ext)
		{
			if (!this->is_active())
			{
				if(!ext.empty())
				{
					m_file_ext = ext.starts_with(".") ? ext : "." + ext;
				}
			}
		}
		std::string& get_file_ext()
		{
			return m_file_ext;
		}

		void set_default_path(const std::string& path)
		{
			if (!this->is_active()) 
				m_default_path = path;
		}
		const std::string& get_default_path()
		{
			return m_default_path;
		}

		[[nodiscard]] std::string get_path_result() const
		{
			return m_last_filepath;
		}

		// ggui::FILE_DIALOG_HANDLER
		void set_file_handler(const int handler)
		{
			if (!this->is_active())
				m_last_file_handler = handler;
		}

		// ggui::FILE_DIALOG_HANDLER
		[[nodiscard]] int get_file_handler() const
		{
			return m_last_file_handler;
		}

		 
		[[nodiscard]] bool was_canceled() const
		{
			return this->m_was_canceled;
		}

		// hack: do not re-set m_was_canceled
		void fix_on_close()
		{
			this->m_fix_on_close = true;
		}

		bool selection_build_path_to_file()
		{
			if (this->get_file_op_type() == FileDialogType::SelectFolder)
			{
				if (this->m_current_folder.empty())
				{
					strcpy_s(this->m_error, "Error: You must select a folder!");
				}
				else
				{
					m_last_filepath = this->m_current_path + (this->m_current_path.back() == '\\' ? "" : "\\") + this->m_current_folder;
					strcpy_s(this->m_error, "");

					this->reset();
					return true;
				}
			}
			else if (this->get_file_op_type() == FileDialogType::OpenFile)
			{
				if (this->m_current_file.empty())
				{
					strcpy_s(this->m_error, "Error: You must select a file!");
				}
				else
				{
					m_last_filepath = this->m_current_path + (this->m_current_path.back() == '\\' ? "" : "\\") + this->m_current_file;
					strcpy_s(this->m_error, "");

					this->reset();
					return true;
				}
			}
			else if (this->get_file_op_type() == FileDialogType::SaveFile)
			{
				if (this->m_save_file_name.empty())
				{
					strcpy_s(this->m_error, "Error: You must enter a name!");
				}
				else
				{
					m_last_filepath = this->m_current_path + (this->m_current_path.back() == '\\' ? "" : "\\") + this->m_save_file_name;
					if(!this->m_file_ext.empty())
					{
						if(!this->m_last_filepath.ends_with(this->m_file_ext))
						{
							m_last_filepath += this->m_file_ext;
						}
					}

					strcpy_s(this->m_error, "");

					this->reset();
					return true;
				}
			}

			return false;
		}

		bool dialog();

		void on_open() override;
		void on_close() override;
	};
}