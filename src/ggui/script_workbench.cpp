#include "std_include.hpp"
#include "codol/path_picker.hpp"

// windows.h macros
#undef min
#undef max

using namespace codol::script;

namespace ggui
{
	namespace
	{
		constexpr int MONO_FONT_INDEX = 5; // see components::imgui_init_fonts

		const ImU32 TOKEN_COLORS[static_cast<int>(tok::count)] =
		{
			IM_COL32(212, 212, 212, 255),	// text
			IM_COL32(106, 153, 85, 255),	// comment
			IM_COL32(206, 145, 120, 255),	// string
			IM_COL32(181, 206, 168, 255),	// number
			IM_COL32(197, 134, 192, 255),	// keyword
			IM_COL32(86, 156, 214, 255),	// builtin
			IM_COL32(215, 186, 125, 255),	// preproc
			IM_COL32(78, 201, 176, 255),	// anim
			IM_COL32(220, 220, 170, 255),	// funcref
			IM_COL32(209, 105, 105, 255),	// localized
			IM_COL32(79, 193, 255, 255),	// section
			IM_COL32(170, 170, 170, 255),	// punct
		};

		const ImVec4 COL_ERROR = ImVec4(0.96f, 0.36f, 0.33f, 1.0f);
		const ImVec4 COL_WARNING = ImVec4(0.95f, 0.76f, 0.26f, 1.0f);
		const ImVec4 COL_ANIM = ImVec4(0.31f, 0.79f, 0.69f, 1.0f);
		const ImVec4 COL_SECTION = ImVec4(0.31f, 0.76f, 1.0f, 1.0f);

		ImFont* mono_font()
		{
			const auto& fonts = ImGui::GetIO().Fonts->Fonts;
			return fonts.Size > MONO_FONT_INDEX ? fonts[MONO_FONT_INDEX] : ImGui::GetFont();
		}

		bool contains_ci(std::string_view haystack, std::string_view needle)
		{
			if (needle.empty())
			{
				return true;
			}

			const auto it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), [](const char a, const char b)
			{
				return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
			});

			return it != haystack.end();
		}

		bool is_script_file(const std::string& lower_name)
		{
			static const char* exts[] =
			{
				".gsc", ".csc", ".gsh", ".atr", ".script", ".csv", ".txt", ".cfg", ".vision", ".sun", ".shock",
				".def", ".str", ".menu", ".ents", ".json", ".arena", ".efx", ".map",
			};

			for (const auto* ext : exts)
			{
				if (lower_name.ends_with(ext))
				{
					return true;
				}
			}

			return false;
		}

		bool is_valid_utf8(const std::string& s)
		{
			std::size_t i = 0;
			const std::size_t n = s.size();

			while (i < n)
			{
				const auto c = static_cast<unsigned char>(s[i]);
				std::size_t extra;

				if (c < 0x80) { i++; continue; }
				if ((c & 0xE0) == 0xC0 && c >= 0xC2) { extra = 1; }
				else if ((c & 0xF0) == 0xE0) { extra = 2; }
				else if ((c & 0xF8) == 0xF0 && c <= 0xF4) { extra = 3; }
				else { return false; }

				if (i + extra >= n)
				{
					return false;
				}

				for (std::size_t k = 1; k <= extra; k++)
				{
					if ((static_cast<unsigned char>(s[i + k]) & 0xC0) != 0x80)
					{
						return false;
					}
				}

				i += extra + 1;
			}

			return true;
		}

		std::string convert_codepage(const std::string& in, const UINT from, const UINT to)
		{
			if (in.empty())
			{
				return {};
			}

			const int wlen = MultiByteToWideChar(from, 0, in.data(), static_cast<int>(in.size()), nullptr, 0);
			std::wstring wide(static_cast<std::size_t>(wlen), L'\0');
			MultiByteToWideChar(from, 0, in.data(), static_cast<int>(in.size()), wide.data(), wlen);

			const int len = WideCharToMultiByte(to, 0, wide.data(), wlen, nullptr, 0, nullptr, nullptr);
			std::string out(static_cast<std::size_t>(len), '\0');
			WideCharToMultiByte(to, 0, wide.data(), wlen, out.data(), len, nullptr, nullptr);
			return out;
		}

		int editor_callback(ImGuiInputTextCallbackData* data)
		{
			auto* doc = static_cast<script_workbench_dialog::document*>(data->UserData);

			if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
			{
				if (doc->pending_cursor >= 0)
				{
					const int start = std::min(doc->pending_cursor, data->BufTextLen);
					const int end = doc->pending_sel_end >= 0 ? std::min(doc->pending_sel_end, data->BufTextLen) : start;

					data->SelectionStart = start;
					data->SelectionEnd = end;
					data->CursorPos = end;

					doc->pending_cursor = -1;
					doc->pending_sel_end = -1;
				}

				doc->cursor = data->CursorPos;
				doc->sel_start = data->SelectionStart;
				doc->sel_end = data->SelectionEnd;
				doc->active_frame = ImGui::GetFrameCount();
			}

			return 0;
		}

		const char* severity_tag(const severity sev)
		{
			return sev == severity::error ? "E" : sev == severity::warning ? "W" : "I";
		}

		ImVec4 severity_color(const severity sev)
		{
			return sev == severity::error ? COL_ERROR : COL_WARNING;
		}
	}

	// ---------------------------------------------------------------------------------------
	// document

	int script_workbench_dialog::document::line_of(const int offset) const
	{
		if (line_starts.empty())
		{
			return 0;
		}

		const auto it = std::upper_bound(line_starts.begin(), line_starts.end(), static_cast<std::uint32_t>(std::max(0, offset)));
		return static_cast<int>(it - line_starts.begin()) - 1;
	}

	void script_workbench_dialog::document::rebuild_layout()
	{
		line_starts.clear();
		line_states.clear();
		line_starts.push_back(0);

		for (std::size_t i = 0; i < text.size(); i++)
		{
			if (text[i] == '\n')
			{
				line_starts.push_back(static_cast<std::uint32_t>(i + 1));
			}
		}

		// lexer state at the start of every line ( block comments span lines )
		line_states.resize(line_starts.size());
		std::uint8_t state = LS_NONE;
		std::vector<span> scratch;

		for (std::size_t l = 0; l < line_starts.size(); l++)
		{
			line_states[l] = state;

			const auto begin = line_starts[l];
			const auto end = (l + 1 < line_starts.size()) ? line_starts[l + 1] - 1 : static_cast<std::uint32_t>(text.size());
			const std::string_view line(text.data() + begin, end - begin);

			// only lines that can change the state need to be lexed
			if (state != LS_NONE || line.find('/') != std::string_view::npos)
			{
				lex_line(lang, line, state, scratch);
			}
		}

		layout_dirty = false;
	}

	// ---------------------------------------------------------------------------------------
	// scanning

	script_workbench_dialog::scan_result script_workbench_dialog::scan(std::vector<std::string> roots, std::string xanim_list)
	{
		scan_result result;
		result.roots = std::move(roots);

		std::unordered_set<std::string> seen;

		for (int r = 0; r < static_cast<int>(result.roots.size()); r++)
		{
			const auto& root = result.roots[r];
			std::error_code ec;

			auto it = std::filesystem::recursive_directory_iterator(std::filesystem::u8path(root),
				std::filesystem::directory_options::skip_permission_denied, ec);

			for (const auto end = std::filesystem::recursive_directory_iterator(); !ec && it != end; it.increment(ec))
			{
				const auto name_u8 = it->path().filename().u8string();
				const std::string name(name_u8.begin(), name_u8.end());

				if (it->is_directory(ec))
				{
					if (!name.empty() && name[0] == '.')
					{
						it.disable_recursion_pending();
					}

					continue;
				}

				if (!is_script_file(to_lower(name)))
				{
					continue;
				}

				const auto abs_u8 = it->path().u8string();

				file_entry entry;
				entry.abs.assign(abs_u8.begin(), abs_u8.end());
				entry.rel = relative_slash_path(root, entry.abs);
				entry.name = name;
				entry.root = r;

				if (seen.insert(to_lower(entry.rel)).second)
				{
					result.files.push_back(std::move(entry));
				}
			}
		}

		std::sort(result.files.begin(), result.files.end(), [](const file_entry& a, const file_entry& b)
		{
			// directories first, then by name
			const auto la = to_lower(a.rel), lb = to_lower(b.rel);
			const auto da = std::count(la.begin(), la.end(), '/'), db = std::count(lb.begin(), lb.end(), '/');
			const auto dir_a = la.substr(0, la.find_last_of('/') == std::string::npos ? 0 : la.find_last_of('/'));
			const auto dir_b = lb.substr(0, lb.find_last_of('/') == std::string::npos ? 0 : lb.find_last_of('/'));

			if (dir_a != dir_b)
			{
				// a file in a parent folder sorts after the sub folders of that folder
				if (dir_b.starts_with(dir_a) && db > da) { return false; }
				if (dir_a.starts_with(dir_b) && da > db) { return true; }
				return dir_a < dir_b;
			}

			return la < lb;
		});

		// folder tree
		result.dirs.emplace_back(); // root
		std::unordered_map<std::string, int> dir_lookup;
		dir_lookup.emplace("", 0);

		for (int f = 0; f < static_cast<int>(result.files.size()); f++)
		{
			const auto& rel = result.files[f].rel;
			int dir = 0;
			std::size_t start = 0;

			for (auto slash = rel.find('/'); slash != std::string::npos; slash = rel.find('/', start))
			{
				const auto key = to_lower(rel.substr(0, slash));

				if (const auto found = dir_lookup.find(key); found != dir_lookup.end())
				{
					dir = found->second;
				}
				else
				{
					const int index = static_cast<int>(result.dirs.size());
					result.dirs.emplace_back();
					result.dirs[index].name = rel.substr(start, slash - start);
					result.dirs[dir].dirs.push_back(index);
					dir_lookup.emplace(key, index);
					dir = index;
				}

				start = slash + 1;
			}

			result.dirs[dir].files.push_back(f);
		}

		if (!xanim_list.empty())
		{
			std::string text;
			if (read_text_file(xanim_list, text))
			{
				std::vector<std::string> names;
				std::size_t pos = 0;

				while (pos < text.size())
				{
					auto end = text.find('\n', pos);
					if (end == std::string::npos)
					{
						end = text.size();
					}

					// first word of the line
					std::size_t b = pos;
					while (b < end && (text[b] == ' ' || text[b] == '\t'))
					{
						b++;
					}

					std::size_t e = b;
					while (e < end && !std::isspace(static_cast<unsigned char>(text[e])) && text[e] != ',')
					{
						e++;
					}

					if (e > b && text[b] != '#' && text[b] != '/')
					{
						names.emplace_back(text.substr(b, e - b));
					}

					pos = end + 1;
				}

				result.index.set_known_xanims(std::move(names));
			}
		}

		result.index.build(result.roots);
		return result;
	}

	void script_workbench_dialog::rescan()
	{
		if (m_scanning)
		{
			return;
		}

		m_scanning = true;
		m_scan = std::async(std::launch::async, &script_workbench_dialog::scan, components::codol_game::script_roots(),
			std::string((dvars::codol_xanim_list && dvars::codol_xanim_list->current.string) ? dvars::codol_xanim_list->current.string : ""));
	}

	void script_workbench_dialog::on_open()
	{
		if (!m_scanned_once)
		{
			rescan();
		}
	}

	// ---------------------------------------------------------------------------------------
	// documents

	script_workbench_dialog::document* script_workbench_dialog::find_document(const std::string& abs_path)
	{
		const auto key = to_lower(abs_path);
		for (const auto& doc : m_docs)
		{
			if (to_lower(doc->path) == key)
			{
				return doc.get();
			}
		}

		return nullptr;
	}

	bool script_workbench_dialog::load_document(document& doc)
	{
		std::string raw;
		if (!read_text_file(doc.path, raw))
		{
			return false;
		}

		doc.crlf = raw.find("\r\n") != std::string::npos;
		raw.erase(std::remove(raw.begin(), raw.end(), '\r'), raw.end());

		// game text is GBK, imgui needs utf-8 ( and would mangle anything else on the first edit )
		doc.gbk = !is_valid_utf8(raw);
		doc.text = doc.gbk ? convert_codepage(raw, 936, CP_UTF8) : std::move(raw);

		doc.saved_hash = std::hash<std::string>{}(doc.text);
		doc.dirty = false;
		doc.changed_on_disk = false;
		doc.layout_dirty = true;

		std::error_code ec;
		doc.mtime = std::filesystem::last_write_time(std::filesystem::u8path(doc.path), ec);
		return true;
	}

	bool script_workbench_dialog::save_document(document& doc)
	{
		std::string out = doc.gbk ? convert_codepage(doc.text, CP_UTF8, 936) : doc.text;

		if (doc.crlf)
		{
			std::string crlf;
			crlf.reserve(out.size() + out.size() / 20);

			for (const char c : out)
			{
				if (c == '\n')
				{
					crlf += '\r';
				}

				crlf += c;
			}

			out.swap(crlf);
		}

		// A file of a reference tree ( eg. the unpacked copy of sh1x_svr_scripts.iwd ): the game reads only the script
		// folder, the first root. The edit is saved there as a file of its own, the override of the packed one; a copy
		// the script folder has already is never overwritten.
		bool moved = false;

		for (std::size_t r = 1; r < m_roots.size(); r++)
		{
			const auto rel = relative_slash_path(m_roots[r], doc.path);
			if (rel.empty() || rel.starts_with("..") || rel == doc.path)
			{
				continue;
			}

			const auto target = std::filesystem::u8path(m_roots[0]) / std::filesystem::u8path(rel).make_preferred();
			std::error_code ec;

			if (std::filesystem::exists(target, ec))
			{
				game::printf_to_console("^1[CoDOL] not saved: the script folder has its own %s ( open that one )\n", rel.c_str());
				ImGui::Toast(ImGuiToastType_Error, "Script Workbench", ("Not saved: the script folder has its own " + rel + " - open that one.").c_str());
				return false;
			}

			std::filesystem::create_directories(target.parent_path(), ec);
			const auto u8 = target.u8string();
			doc.path.assign(u8.begin(), u8.end());
			doc.rel = rel;
			moved = true;
			break;
		}

		{
			std::ofstream file(std::filesystem::u8path(doc.path), std::ios::binary | std::ios::trunc);
			if (!file.is_open())
			{
				game::printf_to_console("^1[CoDOL] could not write '%s'\n", doc.path.c_str());
				ImGui::Toast(ImGuiToastType_Error, "Script Workbench", ("Could not write " + doc.title).c_str());
				return false;
			}

			file.write(out.data(), static_cast<std::streamsize>(out.size()));
		}

		std::error_code ec;
		doc.mtime = std::filesystem::last_write_time(std::filesystem::u8path(doc.path), ec);
		doc.saved_hash = std::hash<std::string>{}(doc.text);
		doc.dirty = false;
		doc.changed_on_disk = false;

		m_index.update_file(doc.path);
		m_index_generation++;

		game::printf_to_console("[CoDOL] saved %s\n", doc.rel.c_str());

		if (moved)
		{
			game::printf_to_console("[CoDOL] %s is now a file of the script folder ( %s ): it overrides the packed / reference copy\n", doc.rel.c_str(), doc.path.c_str());
			rescan();
		}

		if (dvars::codol_reload_on_save && dvars::codol_reload_on_save->current.enabled && doc.lang != lang::plain)
		{
			components::codol_game::reload_scripts(doc.rel);
		}

		return true;
	}

	void script_workbench_dialog::open_file(const std::string& abs_path, const int line)
	{
		auto* doc = find_document(abs_path);

		if (!doc)
		{
			auto fresh = std::make_unique<document>();
			fresh->path = abs_path;
			fresh->rel = abs_path;

			for (const auto& root : m_roots)
			{
				const auto rel = relative_slash_path(root, abs_path);
				if (!rel.empty() && !rel.starts_with("..") && rel != abs_path)
				{
					fresh->rel = rel;
					break;
				}
			}

			const auto slash = abs_path.find_last_of("\\/");
			fresh->title = (slash == std::string::npos) ? abs_path : abs_path.substr(slash + 1);
			fresh->lang = lang_from_path(abs_path);

			if (!load_document(*fresh))
			{
				game::printf_to_console("^1[CoDOL] could not open '%s'\n", abs_path.c_str());
				return;
			}

			doc = fresh.get();
			m_docs.push_back(std::move(fresh));
		}

		doc->open = true;
		doc->select_tab = true;

		if (line > 0)
		{
			goto_line(*doc, line);
		}

		this->open();
	}

	void script_workbench_dialog::goto_line(document& doc, const int line)
	{
		if (doc.layout_dirty)
		{
			doc.rebuild_layout();
		}

		const int index = std::clamp(line - 1, 0, static_cast<int>(doc.line_starts.size()) - 1);
		const auto begin = doc.line_starts[index];
		const auto end = (index + 1 < static_cast<int>(doc.line_starts.size())) ? doc.line_starts[index + 1] - 1 : static_cast<std::uint32_t>(doc.text.size());

		// select the line so it is easy to spot. The cursor ends up at the start of the line, so a long
		// line does not scroll the view sideways
		doc.pending_cursor = static_cast<int>(end);
		doc.pending_sel_end = static_cast<int>(begin);
		doc.request_focus = true;
	}

	void script_workbench_dialog::find_next(document& doc, const bool backwards)
	{
		if (m_find.empty() || doc.text.empty())
		{
			return;
		}

		const auto pred = [](const char a, const char b)
		{
			return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
		};

		const auto& text = doc.text;
		std::size_t found = std::string::npos;

		if (!backwards)
		{
			const auto from = std::min<std::size_t>(static_cast<std::size_t>(std::max(doc.sel_start, doc.sel_end)), text.size());
			auto it = std::search(text.begin() + static_cast<std::ptrdiff_t>(from), text.end(), m_find.begin(), m_find.end(), pred);

			if (it == text.end())
			{
				it = std::search(text.begin(), text.end(), m_find.begin(), m_find.end(), pred); // wrap
			}

			if (it != text.end())
			{
				found = static_cast<std::size_t>(it - text.begin());
			}
		}
		else
		{
			const std::size_t before = static_cast<std::size_t>(std::max(0, std::min(doc.sel_start, doc.sel_end)));
			std::size_t pos = 0;
			auto it = text.begin();

			// last match that starts before the selection, or the last match of the file ( wrap )
			std::size_t last_before = std::string::npos, last_any = std::string::npos;
			while ((it = std::search(it, text.end(), m_find.begin(), m_find.end(), pred)) != text.end())
			{
				pos = static_cast<std::size_t>(it - text.begin());
				last_any = pos;

				if (pos < before)
				{
					last_before = pos;
				}

				++it;
			}

			found = (last_before != std::string::npos) ? last_before : last_any;
		}

		if (found != std::string::npos)
		{
			doc.pending_cursor = static_cast<int>(found);
			doc.pending_sel_end = static_cast<int>(found + m_find.size());
			doc.request_focus = true;
		}
		else
		{
			components::codol_game::set_status("'" + m_find + "' not found");
		}
	}

	void script_workbench_dialog::refresh_markers(document& doc)
	{
		if (doc.markers_generation == m_index_generation)
		{
			return;
		}

		doc.markers.clear();
		doc.markers_generation = m_index_generation;

		const auto key = to_lower(doc.path);
		for (const auto& d : m_index.diagnostics)
		{
			if (d.line > 0 && to_lower(d.path) == key)
			{
				auto& marker = doc.markers[d.line - 1];
				if (marker.second.empty() || static_cast<int>(d.sev) > static_cast<int>(marker.first))
				{
					marker.first = d.sev;
				}

				marker.second += (marker.second.empty() ? "" : "\n") + d.message;
			}
		}
	}

	// ---------------------------------------------------------------------------------------
	// editor

	void script_workbench_dialog::draw_editor(document& doc, const ImVec2& size)
	{
		if (doc.layout_dirty)
		{
			doc.rebuild_layout();
		}

		refresh_markers(doc);

		ImFont* font = mono_font();
		ImGui::PushFont(font);

		const float font_size = ImGui::GetFontSize();
		const float line_h = font_size;
		const auto& style = ImGui::GetStyle();

		char digits[16];
		ImFormatString(digits, sizeof(digits), "%d", static_cast<int>(doc.line_starts.size()));
		const float gutter_w = ImGui::CalcTextSize(digits).x + 22.0f;

		const ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGuiWindow* parent = ImGui::GetCurrentWindow();

		ImGui::SetCursorScreenPos(ImVec2(pos.x + gutter_w, pos.y));

		if (doc.request_focus)
		{
			ImGui::SetKeyboardFocusHere();
			doc.request_focus = false;
		}

		// the input draws its text transparent, the highlighted text is drawn on top of it below
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(24, 24, 27, 255));

		const bool edited = ImGui::InputTextMultiline("##src", &doc.text, ImVec2(size.x - gutter_w, size.y),
			ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackAlways, editor_callback, &doc);

		ImGui::PopStyleColor(2);

		if (edited)
		{
			doc.dirty = std::hash<std::string>{}(doc.text) != doc.saved_hash;
			doc.rebuild_layout();
		}

		// the child window InputTextMultiline created ( imgui: "%s/%s_%08X" )
		const ImGuiID id = parent->GetID("##src");
		char child_name[512];
		ImFormatString(child_name, sizeof(child_name), "%s/%s_%08X", parent->Name, "##src", id);

		if (ImGuiWindow* child = ImGui::FindWindowByName(child_name))
		{
			// a multiline input scrolls vertically through its child window, but horizontally through its own
			// state ( to keep the cursor in view ) - and only applies that while it is being edited
			float scroll_x = 0.0f;
			if (const ImGuiInputTextState* input_state = ImGui::GetInputTextState(id))
			{
				const ImGuiID active = ImGui::GetActiveID();
				if (active == id || active == child->GetID("#SCROLLY"))
				{
					scroll_x = input_state->ScrollX;
				}
			}

			const ImVec2 origin = ImVec2(child->DC.CursorStartPos.x + style.FramePadding.x - scroll_x, child->DC.CursorStartPos.y + style.FramePadding.y);
			const ImRect clip = child->InnerClipRect;
			const int line_count = static_cast<int>(doc.line_starts.size());

			const int first = std::clamp(static_cast<int>((clip.Min.y - origin.y) / line_h), 0, line_count - 1);
			const int last = std::clamp(static_cast<int>((clip.Max.y - origin.y) / line_h) + 1, 0, line_count - 1);

			ImDrawList* dl = child->DrawList;
			dl->PushClipRect(clip.Min, clip.Max, false);

			const bool active = doc.active_frame == ImGui::GetFrameCount();
			const int cursor_line = doc.line_of(doc.cursor);
			std::vector<span> spans;

			for (int l = first; l <= last; l++)
			{
				const auto begin = doc.line_starts[l];
				const auto end = (l + 1 < line_count) ? doc.line_starts[l + 1] - 1 : static_cast<std::uint32_t>(doc.text.size());
				const char* line_begin = doc.text.data() + begin;
				const std::string_view line(line_begin, end - begin);
				const float y = origin.y + static_cast<float>(l) * line_h;

				if (active && l == cursor_line && doc.sel_start == doc.sel_end)
				{
					dl->AddRectFilled(ImVec2(clip.Min.x, y), ImVec2(clip.Max.x, y + line_h), IM_COL32(255, 255, 255, 10));
				}

				if (const auto marker = doc.markers.find(l); marker != doc.markers.end())
				{
					const ImU32 col = marker->second.first == severity::error ? IM_COL32(245, 92, 84, 34) : IM_COL32(242, 194, 66, 22);
					dl->AddRectFilled(ImVec2(clip.Min.x, y), ImVec2(clip.Max.x, y + line_h), col);
				}

				std::uint8_t state = doc.line_states[l];
				lex_line(doc.lang, line, state, spans);

				float x = origin.x;
				const char* p = line_begin;

				for (const auto& s : spans)
				{
					const char* s_begin = line_begin + s.start;
					const char* s_end = s_begin + s.len;

					if (s_begin > p)
					{
						x += font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, p, s_begin).x;
					}

					dl->AddText(font, font_size, ImVec2(x, y), TOKEN_COLORS[static_cast<int>(s.kind)], s_begin, s_end);
					x += font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, s_begin, s_end).x;
					p = s_end;
				}

				// caret ( the one of the input is transparent like its text )
				if (active && l == cursor_line && std::fmod(ImGui::GetTime(), 1.1) < 0.7)
				{
					const char* caret = doc.text.data() + std::clamp(doc.cursor, static_cast<int>(begin), static_cast<int>(end));
					const float cx = origin.x + font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, line_begin, caret).x;
					dl->AddLine(ImVec2(cx, y + 1.0f), ImVec2(cx, y + line_h - 1.0f), IM_COL32(235, 235, 235, 255), 1.0f);
				}
			}

			dl->PopClipRect();

			// gutter: line numbers + problem markers
			ImDrawList* pdl = parent->DrawList;
			const ImVec2 g_min = pos;
			const ImVec2 g_max = ImVec2(pos.x + gutter_w, pos.y + size.y);

			pdl->AddRectFilled(g_min, g_max, IM_COL32(30, 30, 34, 255));
			pdl->PushClipRect(ImVec2(g_min.x, clip.Min.y), ImVec2(g_max.x, clip.Max.y), true);

			const ImVec2 mouse = ImGui::GetIO().MousePos;

			for (int l = first; l <= last; l++)
			{
				const float y = origin.y + static_cast<float>(l) * line_h;
				ImFormatString(digits, sizeof(digits), "%d", l + 1);

				const float w = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, digits).x;
				pdl->AddText(font, font_size, ImVec2(g_max.x - 8.0f - w, y),
					(active && l == cursor_line) ? IM_COL32(220, 220, 220, 255) : IM_COL32(120, 120, 128, 255), digits);

				if (const auto marker = doc.markers.find(l); marker != doc.markers.end())
				{
					const ImU32 col = ImGui::GetColorU32(severity_color(marker->second.first));
					pdl->AddCircleFilled(ImVec2(g_min.x + 7.0f, y + line_h * 0.5f), 3.5f, col);

					if (mouse.x >= g_min.x && mouse.x < clip.Max.x && mouse.y >= y && mouse.y < y + line_h
						&& ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
					{
						ImGui::PopFont();
						ImGui::SetTooltip("%s", marker->second.second.c_str());
						ImGui::PushFont(font);
					}
				}
			}

			pdl->PopClipRect();
		}

		ImGui::PopFont();
	}

	void script_workbench_dialog::draw_status(document* doc)
	{
		if (doc)
		{
			const int line = doc->line_of(doc->cursor);
			const int col = doc->cursor - static_cast<int>(doc->line_starts.empty() ? 0 : doc->line_starts[std::max(0, line)]);

			ImGui::Text("Ln %d, Col %d", line + 1, col + 1);
			ImGui::SameLine(0.0f, 16.0f);
			ImGui::TextDisabled("%s  |  %s  |  %s", lang_name(doc->lang), doc->gbk ? "GBK" : "UTF-8", doc->crlf ? "CRLF" : "LF");

			if (doc->lang == lang::gsc)
			{
				if (const auto* file = m_index.find_gsc(doc->path))
				{
					std::string tree;
					for (const auto& [name, tree_line] : file->trees)
					{
						if (tree_line <= line + 1)
						{
							tree = name;
						}
					}

					ImGui::SameLine(0.0f, 16.0f);

					if (tree.empty())
					{
						ImGui::TextDisabled("no animtree");
					}
					else
					{
						ImGui::TextColored(COL_ANIM, "animtree: %s", tree.c_str());
					}
				}
			}
		}

		if (const auto status = components::codol_game::status(); !status.empty())
		{
			ImGui::SameLine(0.0f, 16.0f);
			ImGui::TextDisabled("%s", status.c_str());
		}
	}

	void script_workbench_dialog::draw_documents()
	{
		// close finished tabs
		for (auto it = m_docs.begin(); it != m_docs.end();)
		{
			if (!(*it)->open && !(*it)->dirty)
			{
				if (m_active_doc == it->get())
				{
					m_active_doc = nullptr;
				}

				it = m_docs.erase(it);
			}
			else
			{
				++it;
			}
		}

		if (m_docs.empty())
		{
			m_active_doc = nullptr;
			ImGui::Dummy(ImVec2(0.0f, 30.0f));
			ImGui::Indent(20.0f);
			ImGui::TextDisabled("Open a script from the Files tab, or pick an entry in the Animations / Problems tabs.");

			if (m_roots.empty() && !m_scanning)
			{
				ImGui::Dummy(ImVec2(0.0f, 8.0f));
				ImGui::TextColored(COL_WARNING, "No script folder found. Use Setup to point the workbench at your script tree.");
			}

			ImGui::Unindent(20.0f);
			return;
		}

		if (!ImGui::BeginTabBar("##wb_docs", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_FittingPolicyScroll))
		{
			return;
		}

		for (const auto& holder : m_docs)
		{
			auto& doc = *holder;

			ImGuiTabItemFlags flags = doc.dirty ? ImGuiTabItemFlags_UnsavedDocument : 0;
			if (doc.select_tab)
			{
				flags |= ImGuiTabItemFlags_SetSelected;
				doc.select_tab = false;
			}

			bool keep_open = true;
			const bool selected = ImGui::BeginTabItem((doc.title + "###" + doc.path).c_str(), &keep_open, flags);

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", doc.path.c_str());
			}

			if (!keep_open)
			{
				doc.open = false;

				if (doc.dirty)
				{
					ImGui::OpenPopup(("Unsaved changes###close_" + doc.path).c_str());
				}
			}

			if (ImGui::BeginPopupModal(("Unsaved changes###close_" + doc.path).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("%s has unsaved changes.", doc.title.c_str());
				ImGui::Dummy(ImVec2(0.0f, 6.0f));

				if (ImGui::Button("Save", ImVec2(110.0f, 0.0f)))
				{
					save_document(doc);
					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();
				if (ImGui::Button("Discard", ImVec2(110.0f, 0.0f)))
				{
					doc.dirty = false;
					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(110.0f, 0.0f)))
				{
					doc.open = true;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			if (!selected)
			{
				continue;
			}

			m_active_doc = &doc;

			// changed outside of radiant?
			if ((ImGui::GetFrameCount() % 60) == 0)
			{
				std::error_code ec;
				const auto mtime = std::filesystem::last_write_time(std::filesystem::u8path(doc.path), ec);

				if (!ec && mtime != doc.mtime)
				{
					if (doc.dirty)
					{
						doc.changed_on_disk = true;
						doc.mtime = mtime;
					}
					else
					{
						load_document(doc);
					}
				}
			}

			if (doc.changed_on_disk)
			{
				ImGui::TextColored(COL_WARNING, "This file was changed outside of radiant.");
				ImGui::SameLine();

				if (ImGui::SmallButton("Reload from disk"))
				{
					load_document(doc);
				}

				ImGui::SameLine();
				if (ImGui::SmallButton("Keep my version"))
				{
					doc.changed_on_disk = false;
				}
			}

			// find / goto bar
			ImGui::SetNextItemWidth(220.0f);
			const bool find_enter = ImGui::InputTextWithHint("##find", "Find", &m_find, ImGuiInputTextFlags_EnterReturnsTrue);
			ImGui::SameLine();

			if (ImGui::Button("Next") || find_enter || (ImGui::IsKeyPressed(ImGuiKey_F3, false) && !ImGui::GetIO().KeyShift))
			{
				find_next(doc, false);
			}

			ImGui::SameLine();
			if (ImGui::Button("Prev") || (ImGui::IsKeyPressed(ImGuiKey_F3, false) && ImGui::GetIO().KeyShift))
			{
				find_next(doc, true);
			}

			ImGui::SameLine(0.0f, 18.0f);
			ImGui::SetNextItemWidth(90.0f);

			if (ImGui::InputInt("##goto", &m_goto_line_input, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				goto_line(doc, m_goto_line_input);
			}

			ImGui::SameLine();
			if (ImGui::Button("Go to line"))
			{
				goto_line(doc, m_goto_line_input);
			}

			if (!doc.markers.empty())
			{
				ImGui::SameLine(0.0f, 18.0f);
				ImGui::TextColored(COL_WARNING, "%d problem%s in this file", static_cast<int>(doc.markers.size()), doc.markers.size() == 1 ? "" : "s");
			}

			const float status_h = ImGui::GetFrameHeightWithSpacing();
			const ImVec2 avail = ImGui::GetContentRegionAvail();
			draw_editor(doc, ImVec2(avail.x, std::max(60.0f, avail.y - status_h)));
			draw_status(&doc);

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	// ---------------------------------------------------------------------------------------
	// left panel

	void script_workbench_dialog::draw_dir(const int dir_index)
	{
		const auto& dir = m_dirs[dir_index];

		for (const int child : dir.dirs)
		{
			if (ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(child)), ImGuiTreeNodeFlags_SpanAvailWidth, "%s", m_dirs[child].name.c_str()))
			{
				draw_dir(child);
				ImGui::TreePop();
			}
		}

		for (const int f : dir.files)
		{
			const auto& file = m_files[f];
			const bool is_open = m_active_doc && m_active_doc->path == file.abs;

			ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(100000 + f)),
				ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth | (is_open ? ImGuiTreeNodeFlags_Selected : 0),
				"%s", file.name.c_str());

			if (ImGui::IsItemClicked())
			{
				open_file(file.abs);
			}
		}
	}

	void script_workbench_dialog::draw_files_tab()
	{
		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputTextWithHint("##file_filter", "Filter files ( name or path )", &m_file_filter);

		ImGui::BeginChild("##wb_files", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_HorizontalScrollbar);

		if (m_scanning && m_files.empty())
		{
			ImGui::TextDisabled("Scanning ...");
		}
		else if (m_files.empty())
		{
			ImGui::TextDisabled("No scripts found.");
		}
		else if (!m_file_filter.empty())
		{
			std::vector<int> matches;
			for (int f = 0; f < static_cast<int>(m_files.size()); f++)
			{
				if (contains_ci(m_files[f].rel, m_file_filter))
				{
					matches.push_back(f);
				}
			}

			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int>(matches.size()));

			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					const auto& file = m_files[matches[i]];
					if (ImGui::Selectable((file.rel + "##" + std::to_string(matches[i])).c_str(), m_active_doc && m_active_doc->path == file.abs))
					{
						open_file(file.abs);
					}
				}
			}
		}
		else if (!m_dirs.empty())
		{
			draw_dir(0);
		}

		ImGui::EndChild();
	}

	void script_workbench_dialog::draw_player_anims()
	{
		if (!m_index.has_player)
		{
			ImGui::TextWrapped("mp/playeranim.script was not found in the script roots. It is the state machine that picks the player animations in multiplayer.");
			return;
		}

		const auto& pa = m_index.player;
		const auto* mp_tree = m_index.find_tree("multiplayer");

		ImGui::TextDisabled("%d states / conditions, %d animation commands", static_cast<int>(pa.blocks.size()), static_cast<int>(pa.command_count));

		if (ImGui::SmallButton("Open playeranim.script"))
		{
			open_file(pa.path);
		}

		ImGui::BeginChild("##wb_pa", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_HorizontalScrollbar);

		const auto draw_command = [&](const pa_command& cmd, const int id, const int block)
		{
			const bool missing = !cmd.anim.empty() && mp_tree && !mp_tree->contains(cmd.anim);
			const std::string label = cmd.part + " " + cmd.anim + (cmd.extra.empty() ? "" : "  " + cmd.extra);

			ImGui::PushStyleColor(ImGuiCol_Text, missing ? COL_WARNING : COL_ANIM);
			ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(1000000 + id)),
				ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth, "%s", label.c_str());
			ImGui::PopStyleColor();

			if (ImGui::IsItemClicked())
			{
				open_file(pa.path, cmd.line);
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s\nline %d%s", pa.block_path(block).c_str(), cmd.line, missing ? "\nnot part of animtrees/multiplayer.atr" : "");
			}
		};

		if (!m_anim_filter.empty())
		{
			int id = 0;
			for (int b = 0; b < static_cast<int>(pa.blocks.size()); b++)
			{
				for (const auto& cmd : pa.blocks[b].commands)
				{
					if (contains_ci(cmd.anim, m_anim_filter) || contains_ci(pa.blocks[b].label, m_anim_filter))
					{
						draw_command(cmd, id, b);
					}

					id++;
				}
			}
		}
		else
		{
			if (!pa.defines.empty() && ImGui::TreeNodeEx("##pa_defines", ImGuiTreeNodeFlags_SpanAvailWidth, "DEFINES ( %d )", static_cast<int>(pa.defines.size())))
			{
				for (const auto& def : pa.defines)
				{
					if (ImGui::Selectable((def.type + " " + def.name + " = " + def.value + "##def" + std::to_string(def.line)).c_str()))
					{
						open_file(pa.path, def.line);
					}
				}

				ImGui::TreePop();
			}

			int open_depth = 0;
			int skip_below = INT_MAX;
			int cmd_id = 0;
			std::string section;

			for (int b = 0; b < static_cast<int>(pa.blocks.size()); b++)
			{
				const auto& block = pa.blocks[b];

				if (block.depth > skip_below)
				{
					cmd_id += static_cast<int>(block.commands.size());
					continue;
				}

				skip_below = INT_MAX;

				while (open_depth > block.depth)
				{
					ImGui::TreePop();
					open_depth--;
				}

				if (block.depth == 0 && block.section != section)
				{
					section = block.section;
					ImGui::Dummy(ImVec2(0.0f, 4.0f));
					ImGui::TextColored(COL_SECTION, "%s", section.c_str());
					ImGui::Separator();
				}

				const bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(b)),
					ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth, "%s", block.label.c_str());

				if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
				{
					open_file(pa.path, block.line);
				}

				if (open)
				{
					open_depth++;

					for (const auto& cmd : block.commands)
					{
						draw_command(cmd, cmd_id++, b);
					}
				}
				else
				{
					skip_below = block.depth;
					cmd_id += static_cast<int>(block.commands.size());
				}
			}

			while (open_depth-- > 0)
			{
				ImGui::TreePop();
			}
		}

		ImGui::EndChild();
	}

	void script_workbench_dialog::draw_animtrees()
	{
		if (m_index.trees.empty())
		{
			ImGui::TextWrapped("No animtrees ( animtrees/*.atr ) found in the script roots.");
			return;
		}

		if (!m_index.trees.contains(m_selected_tree))
		{
			m_selected_tree = m_index.trees.contains("generic_human") ? "generic_human" : m_index.trees.begin()->first;
		}

		ImGui::SetNextItemWidth(-1.0f);
		if (ImGui::BeginCombo("##tree", (m_selected_tree + "  ( " + std::to_string(m_index.trees[m_selected_tree].nodes.size()) + " )").c_str()))
		{
			for (const auto& [name, tree] : m_index.trees)
			{
				if (ImGui::Selectable((name + "  ( " + std::to_string(tree.nodes.size()) + " )").c_str(), name == m_selected_tree))
				{
					m_selected_tree = name;
				}
			}

			ImGui::EndCombo();
		}

		const auto& tree = m_index.trees[m_selected_tree];

		if (ImGui::SmallButton("Open .atr"))
		{
			open_file(tree.path);
		}

		if (!tree.merged_paths.empty())
		{
			ImGui::SameLine();
			ImGui::TextDisabled("+ %d merged", static_cast<int>(tree.merged_paths.size()));
		}

		ImGui::BeginChild("##wb_atr", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_HorizontalScrollbar);
		const int count = static_cast<int>(tree.nodes.size());

		if (!m_anim_filter.empty())
		{
			std::vector<int> matches;
			for (int n = 0; n < count; n++)
			{
				if (contains_ci(tree.nodes[n].name, m_anim_filter))
				{
					matches.push_back(n);
				}
			}

			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int>(matches.size()));

			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					const auto& node = tree.nodes[matches[i]];
					ImGui::PushStyleColor(ImGuiCol_Text, node.group ? COL_SECTION : COL_ANIM);

					if (ImGui::Selectable((node.name + "##" + std::to_string(matches[i])).c_str()))
					{
						open_file(tree.path, node.line);
					}

					ImGui::PopStyleColor();
				}
			}
		}
		else
		{
			int open_depth = 0;
			int skip_below = INT_MAX;

			for (int n = 0; n < count; n++)
			{
				const auto& node = tree.nodes[n];

				if (node.depth > skip_below)
				{
					continue;
				}

				skip_below = INT_MAX;

				while (open_depth > node.depth)
				{
					ImGui::TreePop();
					open_depth--;
				}

				if (node.group)
				{
					const bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(n)),
						ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | (node.depth == 0 ? ImGuiTreeNodeFlags_DefaultOpen : 0),
						node.flags.empty() ? "%s" : "%s  : %s", node.name.c_str(), node.flags.c_str());

					if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
					{
						open_file(tree.path, node.line);
					}

					if (open)
					{
						open_depth++;
					}
					else
					{
						skip_below = node.depth;
					}
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Text, COL_ANIM);
					ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(n)),
						ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth, "%s", node.name.c_str());
					ImGui::PopStyleColor();

					if (ImGui::IsItemClicked())
					{
						open_file(tree.path, node.line);
					}
				}
			}

			while (open_depth-- > 0)
			{
				ImGui::TreePop();
			}
		}

		ImGui::EndChild();
	}

	void script_workbench_dialog::draw_scenes()
	{
		// files that define scripted animations
		std::vector<const gsc_file*> files;
		for (const auto& [rel, file] : m_index.gsc)
		{
			if (!file.scenes.empty() || !file.calls.empty())
			{
				files.push_back(&file);
			}
		}

		if (files.empty())
		{
			ImGui::TextWrapped("No scripted animations ( level.scr_anim[ .. ] tables or anim_* calls ) found in the script roots.");
			return;
		}

		// follow the active document if it has scenes
		if (m_active_doc)
		{
			if (const auto* file = m_index.find_gsc(m_active_doc->path); file && (!file->scenes.empty() || !file->calls.empty()))
			{
				if (ImGui::IsWindowAppearing() || m_selected_scene_file.empty())
				{
					m_selected_scene_file = to_lower(file->rel);
				}
			}
		}

		const gsc_file* current = nullptr;
		if (const auto it = m_index.gsc.find(m_selected_scene_file); it != m_index.gsc.end())
		{
			current = &it->second;
		}

		if (!current)
		{
			current = files.front();
			m_selected_scene_file = to_lower(current->rel);
		}

		ImGui::SetNextItemWidth(-1.0f);
		if (ImGui::BeginCombo("##scene_file", (current->rel + "  ( " + std::to_string(current->scenes.size()) + " )").c_str(), ImGuiComboFlags_HeightLarge))
		{
			for (const auto* file : files)
			{
				if (!m_anim_filter.empty() && !contains_ci(file->rel, m_anim_filter))
				{
					bool any = false;
					for (const auto& scene : file->scenes)
					{
						if (contains_ci(scene.animname, m_anim_filter) || contains_ci(scene.scene, m_anim_filter) || contains_ci(scene.value, m_anim_filter))
						{
							any = true;
							break;
						}
					}

					if (!any)
					{
						continue;
					}
				}

				if (ImGui::Selectable((file->rel + "  ( " + std::to_string(file->scenes.size()) + " )").c_str(), file == current))
				{
					m_selected_scene_file = to_lower(file->rel);
				}
			}

			ImGui::EndCombo();
		}

		if (ImGui::SmallButton("Open script"))
		{
			open_file(current->path);
		}

		ImGui::SameLine();
		ImGui::TextDisabled("%d table entries, %d anim calls", static_cast<int>(current->scenes.size()), static_cast<int>(current->calls.size()));

		const float half = std::max(120.0f, (ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing()) * 0.62f);

		if (ImGui::BeginTable("##scenes", 4, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV,
			ImVec2(0.0f, current->calls.empty() ? 0.0f : half)))
		{
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("who");
			ImGui::TableSetupColumn("scene");
			ImGui::TableSetupColumn("value");
			ImGui::TableSetupColumn("table");
			ImGui::TableHeadersRow();

			std::vector<int> rows;
			for (int s = 0; s < static_cast<int>(current->scenes.size()); s++)
			{
				const auto& scene = current->scenes[s];
				if (m_anim_filter.empty() || contains_ci(scene.animname, m_anim_filter) || contains_ci(scene.scene, m_anim_filter) || contains_ci(scene.value, m_anim_filter))
				{
					rows.push_back(s);
				}
			}

			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int>(rows.size()));

			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					const auto& scene = current->scenes[rows[i]];
					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					if (ImGui::Selectable((scene.animname + "##s" + std::to_string(rows[i])).c_str(), false, ImGuiSelectableFlags_SpanAllColumns))
					{
						open_file(current->path, scene.line);
					}

					ImGui::TableNextColumn();
					ImGui::TextUnformatted((scene.scene + (scene.index.empty() ? "" : " [" + scene.index + "]")).c_str());

					ImGui::TableNextColumn();
					const bool is_anim = scene.value.starts_with("%");
					bool missing = false;

					if (is_anim && !scene.tree.empty())
					{
						if (const auto* tree = m_index.find_tree(scene.tree))
						{
							missing = !tree->contains(std::string_view(scene.value).substr(1));
						}
					}

					ImGui::TextColored(missing ? COL_ERROR : is_anim ? COL_ANIM : ImGui::GetStyleColorVec4(ImGuiCol_Text), "%s", scene.value.c_str());

					ImGui::TableNextColumn();
					ImGui::TextDisabled("%s", scene.table.c_str());
				}
			}

			ImGui::EndTable();
		}

		if (!current->calls.empty() && ImGui::BeginTable("##calls", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("call");
			ImGui::TableSetupColumn("scene / args");
			ImGui::TableHeadersRow();

			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int>(current->calls.size()));

			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					const auto& call = current->calls[i];
					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					if (ImGui::Selectable((call.func + "##c" + std::to_string(i)).c_str(), false, ImGuiSelectableFlags_SpanAllColumns))
					{
						open_file(current->path, call.line);
					}

					ImGui::TableNextColumn();
					std::string args;
					for (const auto& s : call.strings)
					{
						args += (args.empty() ? "" : ", ") + s;
					}

					ImGui::TextUnformatted(args.c_str());
				}
			}

			ImGui::EndTable();
		}
	}

	void script_workbench_dialog::draw_animations_tab()
	{
		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputTextWithHint("##anim_filter", "Filter ( animation, scene, character )", &m_anim_filter);

		if (ImGui::BeginTabBar("##wb_anim_tabs"))
		{
			if (ImGui::BeginTabItem("Player"))
			{
				draw_player_anims();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Scripted"))
			{
				draw_scenes();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Animtrees"))
			{
				draw_animtrees();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}

	void script_workbench_dialog::draw_problems_tab()
	{
		int errors = 0, warnings = 0;
		for (const auto& d : m_index.diagnostics)
		{
			(d.sev == severity::error ? errors : warnings)++;
		}

		ImGui::TextColored(errors ? COL_ERROR : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled), "%d errors", errors);
		ImGui::SameLine();
		ImGui::TextColored(warnings ? COL_WARNING : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled), "%d warnings", warnings);
		ImGui::SameLine(0.0f, 14.0f);
		ImGui::Checkbox("Warnings", &m_show_warnings);

		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputTextWithHint("##problem_filter", "Filter ( file or message )", &m_problem_filter);

		ImGui::BeginChild("##wb_problems", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_HorizontalScrollbar);

		std::vector<int> rows;
		for (int i = 0; i < static_cast<int>(m_index.diagnostics.size()); i++)
		{
			const auto& d = m_index.diagnostics[i];
			if ((d.sev == severity::error || m_show_warnings) && (m_problem_filter.empty() || contains_ci(d.path, m_problem_filter) || contains_ci(d.message, m_problem_filter)))
			{
				rows.push_back(i);
			}
		}

		ImGuiListClipper clipper;
		clipper.Begin(static_cast<int>(rows.size()));

		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				const auto& d = m_index.diagnostics[rows[i]];

				std::string rel = d.path;
				for (const auto& root : m_roots)
				{
					if (to_lower(d.path).starts_with(to_lower(root)))
					{
						rel = d.path.substr(std::min(d.path.size(), root.size() + 1));
						break;
					}
				}

				ImGui::PushStyleColor(ImGuiCol_Text, severity_color(d.sev));
				const bool clicked = ImGui::Selectable((std::string("[") + severity_tag(d.sev) + "] " + rel + ":" + std::to_string(d.line) + "##p" + std::to_string(rows[i])).c_str());
				ImGui::PopStyleColor();

				if (clicked)
				{
					open_file(d.path, d.line);
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("%s", d.message.c_str());
				}

				ImGui::SameLine();
				ImGui::TextDisabled("%s", d.message.c_str());
			}
		}

		ImGui::EndChild();
	}

	void script_workbench_dialog::draw_left_panel(const float width)
	{
		ImGui::BeginChild("##wb_left", ImVec2(width, 0.0f), true);

		if (ImGui::BeginTabBar("##wb_left_tabs"))
		{
			if (ImGui::BeginTabItem("Files"))
			{
				draw_files_tab();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Animations"))
			{
				draw_animations_tab();
				ImGui::EndTabItem();
			}

			int errors = 0;
			for (const auto& d : m_index.diagnostics)
			{
				errors += d.sev == severity::error ? 1 : 0;
			}

			if (ImGui::BeginTabItem(errors ? ("Problems ( " + std::to_string(errors) + " )###wb_problems_tab").c_str() : "Problems###wb_problems_tab"))
			{
				draw_problems_tab();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::EndChild();
	}

	// ---------------------------------------------------------------------------------------
	// toolbar / setup

	void script_workbench_dialog::draw_setup_popup()
	{
		const auto dvar_str = [](const game::dvar_s* dvar) -> std::string
		{
			return (dvar && dvar->current.string) ? dvar->current.string : "";
		};

		if (m_open_setup)
		{
			m_open_setup = false;
			m_cfg_script_root = dvar_str(dvars::codol_script_root);
			m_cfg_ref_root = dvar_str(dvars::codol_script_ref_root);
			m_cfg_install = dvar_str(dvars::codol_install_path);
			m_cfg_pw_file = dvar_str(dvars::codol_rcon_password_file);
			m_cfg_precheck = dvar_str(dvars::codol_reload_precheck_cmd);
			m_cfg_precheck_cwd = dvar_str(dvars::codol_reload_precheck_cwd);
			m_cfg_launch = dvar_str(dvars::codol_launch_cmd);
			m_cfg_client_exe = dvar_str(dvars::codol_client_exe);
			m_cfg_asset_tool = dvar_str(dvars::codol_asset_tool);
			m_cfg_xanim_list = dvar_str(dvars::codol_xanim_list);
			m_cfg_port = dvars::codol_rcon_port ? dvars::codol_rcon_port->current.integer : 17733;
			ImGui::OpenPopup("CoDOL Setup##wb");
		}

		ImGui::SetNextWindowSize(ImVec2(760.0f, 0.0f), ImGuiCond_Appearing);
		if (!ImGui::BeginPopupModal("CoDOL Setup##wb", nullptr, ImGuiWindowFlags_NoSavedSettings))
		{
			return;
		}

		const auto field = [](const char* label, const char* hint, std::string& value, const char* tooltip)
		{
			ImGui::TextUnformatted(label);
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::InputTextWithHint((std::string("##") + label).c_str(), hint, &value);

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", tooltip);
			}
		};

		// a path: the text box and a ".." button with Explorer's folder picker / file dialog
		const auto path_field = [](const char* label, const char* hint, std::string& value, const char* tooltip,
			const codol::ui::path_kind kind, const char* filter = nullptr)
		{
			ImGui::TextUnformatted(label);

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", tooltip);
			}

			codol::ui::path_input((std::string("##") + label).c_str(), value, kind, filter, hint, -1.0f, label);
		};

		using codol::ui::path_kind;

		ImGui::TextColored(COL_SECTION, "Scripts");
		path_field("Script folder", "D:\\...\\codol_rawfiles", m_cfg_script_root,
			"The script tree you edit: the folder the server loads loose scripts from.\nContains maps\\, animscripts\\, animtrees\\, mp\\playeranim.script ...", path_kind::folder);
		path_field("Reference script folder ( optional )", "rawfiles exported from the zones", m_cfg_ref_root,
			"A second, read-mostly tree. Files of the script folder shadow it, animtrees of both are merged\n( the game also merges the tree from its zones with loose files ).", path_kind::folder);

		path_field("Game xanim list ( optional )", "D:\\...\\docs\\zone_assets\\ALL_xanim.txt", m_cfg_xanim_list,
			"Text file with the animations the game ships, one name per line ( anything after the name is ignored ).\nScripts that use an animation that is not in it get a warning.", path_kind::open_file, codol::ui::filter_text);

		ImGui::Dummy(ImVec2(0.0f, 6.0f));
		ImGui::TextColored(COL_SECTION, "Game");
		path_field("CoDOL install folder", "D:\\...\\Call of Duty Online 1.1.1.13", m_cfg_install, "Folder that contains main\\ and client_shipRetail\\.", path_kind::folder);
		path_field("Start command", "path to the script / exe that starts the client", m_cfg_launch, "Used by the Launch button of the Game window. Runs in its own folder.", path_kind::open_file, codol::ui::filter_programs);
		path_field("Asset tool ( optional )", "D:\\...\\CodolSoundDumper.exe", m_cfg_asset_tool,
			"CodolSoundDumper.exe of CODOL-AssetExplorer. Used by CoDOL > Map Entities to decrypt a map zone ( .ff ).", path_kind::open_file, codol::ui::filter_exe);
		field("Client process names", "codoMP_client_shipRetail.exe", m_cfg_client_exe, "Used to find the window of the running game ( ; separated ).");

		ImGui::Dummy(ImVec2(0.0f, 6.0f));
		ImGui::TextColored(COL_SECTION, "Local server ( rcon )");
		ImGui::TextUnformatted("Port");
		ImGui::SetNextItemWidth(140.0f);
		ImGui::InputInt("##rcon_port", &m_cfg_port, 0, 0);
		path_field("Password file", "native_server_rcon.txt ( leave empty to look in the install folder )", m_cfg_pw_file,
			"File that holds the rcon password of the local server.\nIf empty: <install>\\main\\k33_rcon.txt, <install>\\p2p\\state\\native_server_rcon.txt, %CODOL_STATE_DIR%.", path_kind::open_file, codol::ui::filter_text);
		path_field("Pre-reload check ( optional )", "python tools/codol_script_resolve.py --category mp --gametype war --map mp_killhouse_sh", m_cfg_precheck,
			"Command that has to succeed ( exit code 0 ) before the game is reloaded.\nA script the server can not compile makes the map load fail, so check first.", path_kind::open_file, codol::ui::filter_programs);
		path_field("Pre-reload check working folder", "D:\\...\\CoD4x_Server", m_cfg_precheck_cwd, "Working directory of the pre-reload check.", path_kind::folder);

		ImGui::Dummy(ImVec2(0.0f, 10.0f));

		if (ImGui::Button("Apply", ImVec2(120.0f, 0.0f)))
		{
			dvars::set_string(dvars::codol_script_root, m_cfg_script_root.c_str());
			dvars::set_string(dvars::codol_script_ref_root, m_cfg_ref_root.c_str());
			dvars::set_string(dvars::codol_install_path, m_cfg_install.c_str());
			dvars::set_string(dvars::codol_rcon_password_file, m_cfg_pw_file.c_str());
			dvars::set_string(dvars::codol_reload_precheck_cmd, m_cfg_precheck.c_str());
			dvars::set_string(dvars::codol_reload_precheck_cwd, m_cfg_precheck_cwd.c_str());
			dvars::set_string(dvars::codol_launch_cmd, m_cfg_launch.c_str());
			dvars::set_string(dvars::codol_client_exe, m_cfg_client_exe.c_str());
			dvars::set_string(dvars::codol_asset_tool, m_cfg_asset_tool.c_str());
			dvars::set_string(dvars::codol_xanim_list, m_cfg_xanim_list.c_str());
			dvars::set_int(dvars::codol_rcon_port, std::clamp(m_cfg_port, 1, 65535));

			rescan();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120.0f, 0.0f)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	void script_workbench_dialog::draw_toolbar()
	{
		const bool can_save = m_active_doc && m_active_doc->dirty;

		ImGui::BeginDisabled(!can_save);
		if (ImGui::Button("Save"))
		{
			save_document(*m_active_doc);
		}
		ImGui::EndDisabled();

		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("Save the current script ( Ctrl + S )");
		}

		ImGui::SameLine();
		if (ImGui::Button("Save All"))
		{
			for (const auto& doc : m_docs)
			{
				if (doc->dirty)
				{
					save_document(*doc);
				}
			}
		}

		ImGui::SameLine(0.0f, 16.0f);
		ImGui::BeginDisabled(m_scanning);
		if (ImGui::Button(m_scanning ? "Scanning ..." : "Rescan"))
		{
			rescan();
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Setup"))
		{
			m_open_setup = true;
		}

		ImGui::SameLine(0.0f, 24.0f);
		if (ImGui::Button("Reload in game"))
		{
			components::codol_game::reload_scripts("manual");
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Reload the level on the local server so it picks up the saved scripts");
		}

		ImGui::SameLine();
		if (dvars::codol_reload_on_save)
		{
			bool on_save = dvars::codol_reload_on_save->current.enabled;
			if (ImGui::Checkbox("on save", &on_save))
			{
				dvars::set_bool(dvars::codol_reload_on_save, on_save);
			}
		}

		ImGui::SameLine();
		if (dvars::codol_reload_mode)
		{
			static const char* modes[] = { "fast_restart", "map_restart", "map <current> ( recompiles scripts )" };
			int mode = std::clamp(dvars::codol_reload_mode->current.integer, 0, 2);

			ImGui::SetNextItemWidth(250.0f);
			if (ImGui::Combo("##reload_mode", &mode, modes, IM_ARRAYSIZE(modes)))
			{
				dvars::set_int(dvars::codol_reload_mode, mode);
			}
		}

		ImGui::SameLine(0.0f, 16.0f);
		if (ImGui::Button("Game window"))
		{
			GET_GUI(ggui::game_view_dialog)->open();
		}

		if (!m_roots.empty())
		{
			ImGui::SameLine(0.0f, 16.0f);
			ImGui::TextDisabled("%d files  |  %d %%anim refs", static_cast<int>(m_files.size()), static_cast<int>(m_index.total_refs));

			if (ImGui::IsItemHovered())
			{
				std::string tip;
				for (const auto& root : m_roots)
				{
					tip += (tip.empty() ? "" : "\n") + root;
				}

				ImGui::SetTooltip("%s", tip.c_str());
			}
		}
	}

	// ---------------------------------------------------------------------------------------

	bool script_workbench_dialog::gui()
	{
		// background scan finished?
		if (m_scanning && m_scan.valid() && m_scan.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		{
			auto result = m_scan.get();
			m_roots = std::move(result.roots);
			m_files = std::move(result.files);
			m_dirs = std::move(result.dirs);
			m_index = std::move(result.index);
			m_index_generation++;
			m_scanning = false;
			m_scanned_once = true;
		}

		const auto MIN_WINDOW_SIZE = ImVec2(640.0f, 360.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(1280.0f, 760.0f);

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		bool any_dirty = false;
		for (const auto& doc : m_docs)
		{
			any_dirty |= doc->dirty;
		}

		if (!ImGui::Begin("Script Workbench##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse | (any_dirty ? ImGuiWindowFlags_UnsavedDocument : 0)))
		{
			ImGui::End();
			return false;
		}

		// shortcuts
		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::GetIO().KeyCtrl && !ImGui::GetIO().KeyAlt)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_S, false) && m_active_doc && m_active_doc->dirty)
			{
				save_document(*m_active_doc);
			}
		}

		draw_toolbar();
		draw_setup_popup();
		ImGui::Separator();

		// left panel | splitter | documents
		draw_left_panel(m_left_width);

		ImGui::SameLine(0.0f, 0.0f);
		ImGui::InvisibleButton("##wb_splitter", ImVec2(6.0f, std::max(1.0f, ImGui::GetContentRegionAvail().y)));

		if (ImGui::IsItemActive())
		{
			m_left_width = std::clamp(m_left_width + ImGui::GetIO().MouseDelta.x, 180.0f, std::max(180.0f, ImGui::GetWindowWidth() - 300.0f));
		}

		if (ImGui::IsItemHovered() || ImGui::IsItemActive())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		}

		ImGui::SameLine(0.0f, 0.0f);
		ImGui::BeginChild("##wb_right", ImVec2(0.0f, 0.0f), false);
		draw_documents();
		ImGui::EndChild();

		ImGui::End();
		return true;
	}

	REGISTER_GUI(script_workbench_dialog);

	// ---------------------------------------------------------------------------------------
	// game view

	bool game_view_dialog::gui()
	{
		using components::codol_game;

		ImGui::SetNextWindowSizeConstraints(ImVec2(420.0f, 300.0f), ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(ImVec2(1024.0f, 640.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Game##codol_window", this->get_p_open(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			// hidden tab: hide the game with it
			codol_game::place_game_window(0, 0, 0, 0, false);
			ImGui::End();
			return false;
		}

		const bool embedded = codol_game::is_embedded();
		const auto* viewport = ImGui::GetMainViewport();
		const auto parent = static_cast<HWND>(viewport->PlatformHandleRaw ? viewport->PlatformHandleRaw : viewport->PlatformHandle);

		if (!embedded)
		{
			if (ImGui::Button("Launch"))
			{
				codol_game::launch_game();
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Start the game client ( the start command is set in Script Workbench > Setup )");
			}

			ImGui::SameLine();
			if (ImGui::Button("Attach"))
			{
				if (!codol_game::attach_game_window(parent))
				{
					codol_game::set_status("no running game window found");
					ImGui::Toast(ImGuiToastType_Warning, "CoDOL", "No running game window found. Start the game first.");
				}
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Move the window of the running game into this dock");
			}
		}
		else if (ImGui::Button("Detach"))
		{
			codol_game::detach_game_window();
		}

		ImGui::SameLine(0.0f, 20.0f);
		ImGui::SetNextItemWidth(200.0f);
		const bool enter = ImGui::InputTextWithHint("##gv_map", "map name", &m_map, ImGuiInputTextFlags_EnterReturnsTrue);

		ImGui::SameLine();
		if ((ImGui::Button("Load map") || enter) && !m_map.empty())
		{
			codol_game::load_map(m_map);
		}

		ImGui::SameLine();
		if (ImGui::Button("Reload scripts"))
		{
			codol_game::reload_scripts("manual");
		}

		if (const auto status = codol_game::status(); !status.empty())
		{
			ImGui::SameLine(0.0f, 16.0f);
			ImGui::TextDisabled("%s", status.c_str());
		}

		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 avail = ImGui::GetContentRegionAvail();

		if (codol_game::is_embedded())
		{
			// screen coordinates of imgui are client coordinates of the radiant window ( no multi-viewports )
			codol_game::place_game_window(static_cast<int>(pos.x), static_cast<int>(pos.y), static_cast<int>(avail.x), static_cast<int>(avail.y), true);
			ImGui::Dummy(avail);
		}
		else
		{
			ImGui::Dummy(ImVec2(0.0f, 20.0f));
			ImGui::Indent(20.0f);
			ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + std::max(200.0f, avail.x - 40.0f));
			ImGui::TextDisabled(
				"The running game is shown here once it is attached.\n\n"
				"1. Start your local server and the game client ( Launch, or start them the way you always do ).\n"
				"2. Press Attach: the game window moves into this dock and follows it around.\n"
				"3. Save a script in the Script Workbench with \"on save\" enabled, or press Reload scripts: "
				"the server reloads the level and the game in this window shows the result.\n\n"
				"Detach ( or closing this window ) gives the game its own window back.");
			ImGui::PopTextWrapPos();
			ImGui::Unindent(20.0f);
		}

		ImGui::End();
		return true;
	}

	void game_view_dialog::on_close()
	{
		components::codol_game::detach_game_window();
	}

	REGISTER_GUI(game_view_dialog);
}
