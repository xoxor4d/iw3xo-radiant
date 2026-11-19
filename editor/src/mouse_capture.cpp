#include "std_include.hpp"

// windows.h macros
#undef min
#undef max

#include "host.hpp"
#include "ui.hpp"
#include "mouse_capture.hpp"

namespace editor::mouse_capture
{
	namespace
	{
		struct state
		{
			const void* owner = nullptr;
			POINT restore = {};			// where the drag started ( screen )
			POINT center = {};			// the middle of the view ( screen ), where the cursor is put back
			int frame = -1;				// the last frame the owner asked for it
			bool suppressed = false;	// let go with Escape / focus loss: not again before the buttons are up
		};

		state g;

		bool real_button_down()
		{
			return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) || (GetAsyncKeyState(VK_RBUTTON) & 0x8000) || (GetAsyncKeyState(VK_MBUTTON) & 0x8000);
		}

		bool window_has_focus()
		{
			const HWND hwnd = main_window();
			return hwnd && GetForegroundWindow() == hwnd;
		}

		// the view in screen coordinates ( ImGui positions are client pixels of the main window )
		RECT screen_rect(const ImVec2 rect_min, const ImVec2 rect_max)
		{
			POINT a = { static_cast<LONG>(std::ceil(rect_min.x)), static_cast<LONG>(std::ceil(rect_min.y)) };
			POINT b = { static_cast<LONG>(std::floor(rect_max.x)), static_cast<LONG>(std::floor(rect_max.y)) };
			ClientToScreen(main_window(), &a);
			ClientToScreen(main_window(), &b);
			return { a.x, a.y, std::max(a.x + 1, b.x), std::max(a.y + 1, b.y) };
		}

		void let_go(const bool suppress)
		{
			if (!g.owner)
			{
				return;
			}

			ClipCursor(nullptr);
			SetCursorPos(g.restore.x, g.restore.y);

			// ImGui keeps the position it last saw: give it the restored one, so the next frame has no jump
			POINT client = g.restore;
			if (ScreenToClient(main_window(), &client) && !ui::mouse_injected())
			{
				ImGui::GetIO().AddMousePosEvent(static_cast<float>(client.x), static_cast<float>(client.y));
			}

			g.owner = nullptr;
			g.suppressed = suppress;
		}
	}

	ImVec2 update(const void* owner, bool want, const ImVec2 rect_min, const ImVec2 rect_max)
	{
		const auto& io = ImGui::GetIO();
		const ImVec2 normal = io.MouseDelta;

		if (g.suppressed)
		{
			if (!real_button_down()) { g.suppressed = false; }
			else { want = false; }
		}

		// never the real cursor for injected input, never without the editor in front
		want = want && real_button_down() && !ui::mouse_injected() && window_has_focus();

		if (want && ImGui::IsKeyPressed(ImGuiKey_Escape, false))
		{
			let_go(true);
			return normal;
		}

		if (g.owner && g.owner != owner)
		{
			return normal;		// another view holds the mouse
		}

		if (!want)
		{
			if (g.owner == owner) { let_go(false); }
			return normal;
		}

		const RECT rect = screen_rect(rect_min, rect_max);
		const POINT center = { (rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2 };
		POINT cursor = {};
		GetCursorPos(&cursor);

		ImVec2 delta = normal;

		if (g.owner != owner)
		{
			// the drag starts: this frame's motion is ImGui's, from now on it is measured from the middle
			g.owner = owner;
			g.restore = cursor;
		}
		else
		{
			delta = ImVec2(static_cast<float>(cursor.x - g.center.x), static_cast<float>(cursor.y - g.center.y));
		}

		// again every frame: the window or the view may have moved
		ClipCursor(&rect);
		SetCursorPos(center.x, center.y);
		g.center = center;
		g.frame = ImGui::GetFrameCount();

		ImGui::SetMouseCursor(ImGuiMouseCursor_None);
		return delta;
	}

	bool held_by(const void* owner)
	{
		return g.owner && g.owner == owner;
	}

	void end_frame()
	{
		if (!g.owner)
		{
			return;
		}

		if (g.frame != ImGui::GetFrameCount() || !window_has_focus() || !real_button_down())
		{
			let_go(!window_has_focus());
		}
	}

	void release()
	{
		let_go(true);
	}
}
