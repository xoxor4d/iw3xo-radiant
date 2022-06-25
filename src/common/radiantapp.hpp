#pragma once
#include "afx.hpp"

class radiantapp
{
private:
	// Do not allow this class to be instanced
	radiantapp() = delete;
	~radiantapp() = delete;

public:
	static void		on_create_client();
	static void		on_shutdown();
	static void		set_default_savedinfo_colors();
    static void     hooks();
};