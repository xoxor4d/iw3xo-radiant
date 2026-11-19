#pragma once

// Windows Explorer pickers for path fields: a text box ( still editable ) and a ".." button next to it that opens
// the system's folder picker or its open / save file dialog ( IFileOpenDialog / IFileSaveDialog ), started at the
// path that is in the box. Header only, so the standalone editor and the radiant plugin share it.
//
// Call on the UI thread. The dialog is modal ( as GetOpenFileName ): the frame waits until it is closed.
// COM: the host initialises the thread as an STA ( CoInitializeEx in wWinMain ); a thread that is not initialised
// yet is initialised here for the call and uninitialised again.

#include <windows.h>
#include <shobjidl.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace codol::ui
{
	enum class path_kind
	{
		folder,			// FOS_PICKFOLDERS
		open_file,		// an existing file
		save_file,		// a file that is written ( may not exist yet )
	};

	namespace path_picker_detail
	{
		inline std::wstring widen(const std::string_view text)
		{
			if (text.empty()) { return {}; }
			const int n = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
			std::wstring out(static_cast<std::size_t>(n > 0 ? n : 0), L'\0');
			if (n > 0) { MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), out.data(), n); }
			return out;
		}

		inline std::string narrow(const std::wstring_view text)
		{
			if (text.empty()) { return {}; }
			const int n = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
			std::string out(static_cast<std::size_t>(n > 0 ? n : 0), '\0');
			if (n > 0) { WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), out.data(), n, nullptr, nullptr); }
			return out;
		}

		// "Images|*.png;*.tga|All files|*.*" -> name / spec pairs
		inline std::vector<std::wstring> split_filter(const char* filter)
		{
			std::vector<std::wstring> parts;
			if (!filter || !*filter) { return parts; }

			std::string_view rest(filter);
			while (!rest.empty())
			{
				const auto bar = rest.find('|');
				parts.push_back(widen(rest.substr(0, bar)));
				if (bar == std::string_view::npos) { break; }
				rest.remove_prefix(bar + 1);
			}

			if (parts.size() % 2) { parts.pop_back(); }
			return parts;
		}

		// the value of a field as a starting point: a folder to open in and, for files, a name to show
		inline void split_start(const std::string& value, const path_kind kind, std::wstring& folder, std::wstring& name)
		{
			namespace fs = std::filesystem;
			std::error_code ec;

			// a command line: the program is the first, possibly quoted, word
			std::string text = value;
			if (!text.empty() && text.front() == '"')
			{
				const auto end = text.find('"', 1);
				text = text.substr(1, end == std::string::npos ? std::string::npos : end - 1);
			}

			while (!text.empty() && (text.back() == ' ' || text.back() == '\t')) { text.pop_back(); }
			if (text.empty()) { return; }

			fs::path path(widen(text));

			if (fs::is_directory(path, ec))
			{
				folder = fs::absolute(path, ec).wstring();
				return;
			}

			if (kind != path_kind::folder) { name = path.filename().wstring(); }

			// the nearest folder that exists
			for (fs::path dir = path.parent_path(); !dir.empty(); dir = dir.parent_path())
			{
				if (fs::is_directory(dir, ec))
				{
					folder = fs::absolute(dir, ec).wstring();
					break;
				}

				if (dir == dir.parent_path()) { break; }
			}
		}
	}

	// Opens the picker; on OK writes the chosen path ( utf-8 ) to `path` and returns true. `filter`: pairs of
	// "name|patterns" separated by '|', eg. "Programs|*.exe;*.cmd;*.bat|All files|*.*"; ignored for folders.
	inline bool pick_path(const path_kind kind, std::string& path, const char* filter = nullptr, const char* title = nullptr, HWND owner = nullptr)
	{
		using namespace path_picker_detail;

		const HRESULT init = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		const bool uninit = SUCCEEDED(init);		// S_OK or S_FALSE: balance it; RPC_E_CHANGED_MODE: COM is usable as it is

		bool picked = false;
		IFileDialog* dialog = nullptr;

		const HRESULT made = kind == path_kind::save_file
			? CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(reinterpret_cast<IFileSaveDialog**>(&dialog)))
			: CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(reinterpret_cast<IFileOpenDialog**>(&dialog)));

		if (SUCCEEDED(made) && dialog)
		{
			FILEOPENDIALOGOPTIONS options = 0;
			dialog->GetOptions(&options);
			options |= FOS_FORCEFILESYSTEM | FOS_NOCHANGEDIR;

			switch (kind)
			{
			case path_kind::folder:    options |= FOS_PICKFOLDERS | FOS_PATHMUSTEXIST; break;
			case path_kind::open_file: options |= FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST; break;
			case path_kind::save_file: options |= FOS_OVERWRITEPROMPT | FOS_PATHMUSTEXIST; break;
			}

			dialog->SetOptions(options);

			if (title && *title) { dialog->SetTitle(widen(title).c_str()); }

			const auto parts = kind == path_kind::folder ? std::vector<std::wstring>() : split_filter(filter);
			std::vector<COMDLG_FILTERSPEC> specs;
			for (std::size_t i = 0; i + 1 < parts.size(); i += 2) { specs.push_back({ parts[i].c_str(), parts[i + 1].c_str() }); }
			if (!specs.empty()) { dialog->SetFileTypes(static_cast<UINT>(specs.size()), specs.data()); }

			std::wstring folder, name;
			split_start(path, kind, folder, name);

			if (!folder.empty())
			{
				IShellItem* item = nullptr;
				if (SUCCEEDED(SHCreateItemFromParsingName(folder.c_str(), nullptr, IID_PPV_ARGS(&item))) && item)
				{
					dialog->SetFolder(item);
					item->Release();
				}
			}

			if (!name.empty()) { dialog->SetFileName(name.c_str()); }

			if (SUCCEEDED(dialog->Show(owner ? owner : GetActiveWindow())))
			{
				IShellItem* result = nullptr;
				if (SUCCEEDED(dialog->GetResult(&result)) && result)
				{
					PWSTR chosen = nullptr;
					if (SUCCEEDED(result->GetDisplayName(SIGDN_FILESYSPATH, &chosen)) && chosen)
					{
						path = narrow(chosen);
						picked = true;
						CoTaskMemFree(chosen);
					}

					result->Release();
				}
			}

			dialog->Release();
		}

		if (uninit) { CoUninitialize(); }
		return picked;
	}

	// The width the ".." button of path_input takes ( with its spacing ).
	inline float path_button_width()
	{
		return ImGui::GetFrameHeight() * 1.6f + ImGui::GetStyle().ItemSpacing.x;
	}

	// One path field: the text box ( `width` as SetNextItemWidth: <= 0 fills the line up to the button ) and a ".."
	// button that opens the picker at its value. Returns true when the value changed ( typed or picked ).
	inline bool path_input(const char* id, std::string& value, const path_kind kind, const char* filter = nullptr, const char* hint = nullptr,
		const float width = -1.0f, const char* title = nullptr)
	{
		ImGui::PushID(id);

		const float button = path_button_width();
		ImGui::SetNextItemWidth(width > 0.0f ? width : (width < 0.0f ? width - button : -button));
		bool changed = hint ? ImGui::InputTextWithHint("##path", hint, &value) : ImGui::InputText("##path", &value);

		ImGui::SameLine();
		if (ImGui::Button("..", ImVec2(button - ImGui::GetStyle().ItemSpacing.x, 0.0f)))
		{
			const char* fallback = kind == path_kind::folder ? "Select a folder" : (kind == path_kind::save_file ? "Save as" : "Select a file");
			changed |= pick_path(kind, value, filter, title ? title : fallback);
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(kind == path_kind::folder ? "Browse for a folder .." : "Browse for a file ..");
		}

		ImGui::PopID();
		return changed;
	}

	// common filters
	inline constexpr const char* filter_programs = "Programs and scripts ( *.exe, *.cmd, *.bat, *.ps1, *.py )|*.exe;*.cmd;*.bat;*.ps1;*.py|All files|*.*";
	inline constexpr const char* filter_exe = "Programs ( *.exe )|*.exe|All files|*.*";
	inline constexpr const char* filter_text = "Text files ( *.txt )|*.txt|All files|*.*";
}
