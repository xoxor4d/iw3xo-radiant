#pragma once

struct __declspec(align(4)) MaterialIter_t
{
	game::qtexture_s* radMtl;
	const char* sortedIndex;
	BYTE gap8[4];
	DWORD dwordC;
	DWORD thats_8;
	DWORD thats_8_too;
	BYTE gap18[8];
	DWORD dword20;
	DWORD font_pixelheight;
	float texwndscale_1p;
};

struct nPos_s
{
	int nPos_current;
	int nPos_max;
	int nPos_layered_current;
	int nPos_layered_max;
};

struct texwnd_s
{
	int textureOffset; //0x0000 
	int usageCount; //0x0004 
	int localeCount; //0x0008 
	bool usageFilter; //0x000C 
	bool localeFilter; //0x000D 
	bool surfaceTypeFilter; //0x000E 
	bool searchbar_filter; //0x000F 
	const char* searchbar_buffer; //0x0010 
	int materialCount; //0x0014 
	game::qtexture_s* sorted_materials[16384]; //0x0018 
	void* qtextures; //0x10018 
	int unk_8; //0x1001C 
	bool m_bNeedRange; //0x10020 
	char z_pad_0x10021[0x3]; //0x10021
	int m_nWidth; //0x10024 
	int m_nHeight; //0x10028 
	int lastButtonDown; //0x1002C 
	CPoint m_ptDown; //0x10030 
	bool m_was_mouse_dragged; //0x10038 
	bool unk_bool; //0x10039 
	char z_pad_0x1003A[0x2]; //0x1003A
	nPos_s nPos[3]; // (nPos + height < nPos to scroll) 1: tex, 2: lmap, 3: smoothing
};

extern texwnd_s* g_texwnd;
extern std::vector<std::vector<std::string>> texwnd_vector_of_favourites;

class ctexwnd : public CWnd
{
private:
	// Do not allow this class to be instanced
	ctexwnd() = delete;
	~ctexwnd() = delete;

public:
    static void				hooks();
	static void				register_dvars();
	static void				init();

	static const char*		get_name_for_selection();
	static void				add_selected_to_favourite_list(int list_id);
	static void				remove_selected_from_favourite_list(int list_id);
	static void				write_favourite_list(int list_id);
	static void				apply_favourite(int list);
	static void				load_favourites();

	static void				select_texture(CPoint point);
	static void				on_mousebutton_down(UINT nFlags);
	static void				on_mousebutton_up(UINT nFlags);
	static void				on_mousemove(UINT nFlags);
	static BOOL __fastcall	on_paint(ctexwnd* pThis);

	static void				rtt_texture_window();
};