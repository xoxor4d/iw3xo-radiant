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

namespace ImGui::FileDialog
{
	enum class FileDialogType
	{
		OpenFile,
		SelectFolder
	};

	extern bool file_dialog_open;
	extern FileDialogType file_dialog_open_type;

	bool ShowFileDialog(bool* open, std::string& path_str, FileDialogType type = FileDialogType::OpenFile, std::string ext_str = "");
}