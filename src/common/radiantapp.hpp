#pragma once
#include "afx.hpp"

class radiantapp
{
private:
	// Do not allow this class to be instanced
	radiantapp() = delete;
	~radiantapp() = delete;

public:
	static void		set_default_savedinfo_colors();

    static void     hooks();
};