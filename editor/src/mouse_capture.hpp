#pragma once

// The mouse of a 3D view while it steers the camera ( look around, fly, pan, drive, orbit ): the cursor is hidden,
// kept inside the view ( ClipCursor ) and put back to the middle of the view every frame, so the motion never runs
// out at the edge of the view or the screen. When the drag ends the cursor comes back where the drag started.
// A capture is let go on Escape, when the editor loses the foreground ( Alt-Tab ) and when the view that holds it is
// not drawn in a frame, so it can never stay stuck.
//
// Only a drag with a real mouse button is captured: input injected with ui_mouse ( headless tests ) never moves
// the real cursor.

#include <imgui.h>

namespace editor::mouse_capture
{
	// Every frame the view is drawn, after the item that owns its mouse. `owner` tells the views apart, `want`: the
	// view is in a camera drag now. `rect_min` / `rect_max`: the view in ImGui coordinates. Returns the mouse
	// motion of this frame in pixels ( io.MouseDelta while nothing is captured ).
	ImVec2 update(const void* owner, bool want, ImVec2 rect_min, ImVec2 rect_max);

	// whether `owner` holds the mouse
	bool held_by(const void* owner);

	// Once per frame after all views are drawn: lets go a capture no view asked for in this frame.
	void end_frame();

	// lets go right away ( focus loss of the window )
	void release();
}
