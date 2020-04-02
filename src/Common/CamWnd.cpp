#include "STDInclude.hpp"

CCamWnd *CCamWnd::ActiveWindow;

void CCamWnd::Cam_MouseControl(float dtime)
{
	static DWORD dwCall = 0x403950;

	_asm
	{
		push esi
		mov esi, ecx // esi = this
		push dtime
		call dwCall // Automatically fixes the stack
		pop esi
	}

	return;
}