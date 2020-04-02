#pragma once

class CCamWnd : public CWnd
{
private:
	// Do not allow this class to be instanced
	CCamWnd() = delete;
	~CCamWnd() = delete;

public:
	//char	_pad[0x1C];
	char	_pad[0x10]; // diff. pad then bo1
	float	cameraOrigin[3];
	float	cameraAngles[3];

	//static CCamWnd * __stdcall ctor(CCamWnd *This);
	//static CCamWnd *(__stdcall *ctor_o)(CCamWnd *This);

	static CCamWnd *ActiveWindow;

	void Cam_MouseControl(float dtime);
};
STATIC_ASSERT_OFFSET(CCamWnd, cameraOrigin, 0x64); // diff. offset then bo1
STATIC_ASSERT_OFFSET(CCamWnd, cameraAngles, 0x70); // ^