#pragma once
#include "imgui_internal.h"

// forward decl
namespace fx_system
{
	enum   FX_ED_FLAG_ : int;
	enum   FX_ELEM_ : int;
	struct FxEditorElemDef;
	struct FxFloatRange;
}

namespace ImGui
{
	enum class CurveEditorFlags
	{
		NO_TANGENTS = 1 << 0,
		SHOW_GRID = 1 << 1,
		RESET = 1 << 2
	};

	bool	IsVertScollbarVisible();

	bool	Checkbox_FxElemFlag(const char* name, fx_system::FxEditorElemDef* elem, fx_system::FX_ED_FLAG_ flag, bool* result = nullptr);
	bool	Checkbox_FxElemFlag(const char* name, fx_system::FxEditorElemDef* elem, fx_system::FX_ELEM_ flag, bool* result = nullptr);
	bool	DragFloat2_FxFloatRange(const char* name, fx_system::FxFloatRange* range, float speed, float min, float max, const char* format);

	int		CurveEditor(const char* label, float* values, int points_count, const ImVec2& editor_size, ImU32 flags, int* new_count);

	ImGuiID FindNodeByID(ImGuiID id);
	void	DockBuilderDockWindow_FirstUseOrSaved(const char* window_name, ImGuiID node_id);
	void	PushStyleCompact();
	void	PopStyleCompact();
	bool	BeginTabItem_SmallGap(const char* label, bool* p_open = nullptr, ImGuiTabItemFlags flags = 0);
	bool	IsItemHoveredDelay(float delay_in_seconds);
	bool	IsResizing();
	void	HandleKeyIO(HWND hwnd, UINT key, SHORT zDelta = 0, UINT nChar = 0);
	ImVec4	ToImVec4(const float* in);
	void	HelpMarker(const char* desc);
	void	PushFontFromIndex(int font);
	
	void	AddUnterline(ImColor col);
	void	TextURL(const char* name, const char* url);
	void	SetCursorForCenteredText(const char* text);
	void	title_with_background(const char* title_text, const ImVec2& pos, const float width, const float height, const float* bg_color, const float* border_color, bool pre_spacing = true, const float text_indent = 8.0f);
	void	title_with_seperator(const char* title_text, bool pre_spacing = true, float width = 0.0f, float height = 2.0f, float post_spacing = 0.0f);
	bool    InputScalarDir(const char* label, ImGuiDataType data_type, void* p_data, int* dir, void* p_step = nullptr, const void* p_step_fast = nullptr, bool display_p_step = false, const char* format = nullptr, ImGuiInputTextFlags flags = 0);

	void	debug_table_entry_vec3(const char* label, const float* vec3);
	void	debug_table_entry_float(const char* label, const float* val);
	void	debug_table_entry_int(const char* label, const int val);
	void	debug_table_entry_int2(const char* label, const int* val);
	void	StyleColorsDevgui(ImGuiStyle* = nullptr);
	
}
