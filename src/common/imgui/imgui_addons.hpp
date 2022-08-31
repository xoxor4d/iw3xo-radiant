#pragma once
#include "imgui_internal.h"

#pragma warning(push)
#pragma warning(disable: 4172)
#include "imgui_notify.hpp"
#pragma warning(pop)

// forward decl
namespace fx_system
{
	enum	FX_ED_FLAG_ : int;
	enum	FX_ELEM_ : int;
	struct	FxEditorElemDef;
	struct	FxFloatRange;
}

// forward decl
namespace game
{
	struct dvar_s;
}

namespace ImGui
{
	bool	pre_description_button(const char* label, const ImVec2& button_size, const ImVec4& text_color, const ImVec4& bg_color);

	void	Toast(const ImGuiToastType_ type, const char* title, const char* content, int time = 2500);
	int		popup_close_map();

	bool	ImageButtonScaled(ImTextureID user_texture_id, const ImVec2& size, const float scale, const ImVec2& uv0 = ImVec2(0.0f, 0.0f), const ImVec2& uv1 = ImVec2(1.0f, 1.0f), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0.0f, 0.0f, 0.0f, 0.0f), const ImVec4& tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

	bool	TreeNodeEx(const char* label, bool* is_hovered, bool* is_pressed, ImGuiTreeNodeFlags flags);
	bool	Spinner(const char* label, float radius, float thickness, const ImU32& color);
	bool	BufferingBar(const char* label, float value, const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col);
	
	bool	IsVertScollbarVisible();
	void	left_label_drag(const char* label, const float text_y_offset, const float sameline_offset = 80.0f);
	void	left_label_checkbox(const char* label, const float text_y_offset);

	void	Dvar(const char* label, game::dvar_s* dvar);
	void	DvarBool_External(const char* checkbox_label, const char* dvar_name);
	void	DvarInt_External(const char* checkbox_label, const char* dvar_name);
	void	DvarFloat_External(const char* checkbox_label, const char* dvar_name);
	void	DvarEnum_External(const char* label, const char* dvar_name);

	bool	Checkbox_FxElemFlag(const char* name, fx_system::FxEditorElemDef* elem, fx_system::FX_ED_FLAG_ flag, bool* result = nullptr, bool invert_selected = false);
	bool	Checkbox_FxElemFlag(const char* name, fx_system::FxEditorElemDef* elem, fx_system::FX_ELEM_ flag, bool* result = nullptr);
	bool	DragFloat2_FxFloatRange(const char* name, fx_system::FxFloatRange* range, float speed, float min, float max, const char* format);

	bool    TabItemEx_Hovered(ImGuiTabBar* tab_bar, const char* label, bool* p_open, ImGuiTabItemFlags flags, bool* is_hovered, ImGuiWindow* docked_window);
	bool	MenuItemFlags(const char* label, bool selected, bool enabled, ImGuiSelectableFlags flags);

	ImGuiID FindNodeByID(ImGuiID id);
	void	DockBuilderDockWindow_FirstUseOrSaved(const char* window_name, ImGuiID node_id);
	void	PushCompactButtonInvBg();
	void	PopCompactButtonInvBg();
	void	PushStyleCompact();
	void	PopStyleCompact();
	bool	BeginTabItem_SmallGap(const char* label, const char* tooltip = nullptr, bool* p_open = nullptr, ImGuiTabItemFlags flags = 0);
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
	void	title_inside_seperator(const char* title_text, bool pre_spacing, float width, float post_spacing, float thickness = 1.0f);
	void	title_with_seperator_helpmark(const char* title_text, bool pre_spacing, float width, float height, float post_spacing, const char* helper_text);
	bool    InputScalarDir(const char* label, ImGuiDataType data_type, void* p_data, int* dir, void* p_step = nullptr, const void* p_step_fast = nullptr, bool display_p_step = false, const char* format = nullptr, ImGuiInputTextFlags flags = 0);

	void	debug_table_entry_vec3(const char* label, const float* vec3);
	void	debug_table_entry_float(const char* label, const float* val);
	void	debug_table_entry_int(const char* label, const int val);
	void	debug_table_entry_int2(const char* label, const int* val);
	void	StyleColorsDevgui(ImGuiStyle* = nullptr);
	
}
