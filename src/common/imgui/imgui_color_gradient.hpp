//
//  imgui_color_gradient.h
//  imgui extension
//
//  Created by David Gallardo on 11/06/16.

/*
 
 Usage:
 
 ::GRADIENT DATA::
 ImGradient gradient;
 
 ::BUTTON::
 if(ImGui::GradientButton(&gradient))
 {
    //set show editor flag to true/false
 }
 
 ::EDITOR::
 static ImGradientMark* draggingMark = nullptr;
 static ImGradientMark* selectedMark = nullptr;
 
 bool updated = ImGui::GradientEditor(&gradient, draggingMark, selectedMark);
 
 ::GET A COLOR::
 float color[3];
 gradient.getColorAt(0.3f, color); //position from 0 to 1
 
 ::MODIFY GRADIENT WITH CODE::
 gradient.getMarks().clear();
 gradient.addMark(0.0f, ImColor(0.2f, 0.1f, 0.0f));
 gradient.addMark(0.7f, ImColor(120, 200, 255));
 
 ::WOOD BROWNS PRESET::
 gradient.getMarks().clear();
 gradient.addMark(0.0f, ImColor(0xA0, 0x79, 0x3D));
 gradient.addMark(0.2f, ImColor(0xAA, 0x83, 0x47));
 gradient.addMark(0.3f, ImColor(0xB4, 0x8D, 0x51));
 gradient.addMark(0.4f, ImColor(0xBE, 0x97, 0x5B));
 gradient.addMark(0.6f, ImColor(0xC8, 0xA1, 0x65));
 gradient.addMark(0.7f, ImColor(0xD2, 0xAB, 0x6F));
 gradient.addMark(0.8f, ImColor(0xDC, 0xB5, 0x79));
 gradient.addMark(1.0f, ImColor(0xE6, 0xBF, 0x83));
 
 */

#pragma once

#include "imgui.h"

#include <vector>
#include <memory>

class ImGradient;

namespace ImGui
{
    bool GradientButton(const char *str_id, ImGradient *gradient);
    bool GradientEditor(const char *str_id, ImGradient *gradient);
}

struct ImGradientMark
{
    float color[4];
    float position; //0 to 1
};

class ImGradient
{
public:
    typedef std::shared_ptr<ImGradientMark> markptr_t;
    typedef std::vector<markptr_t> markvector_t;

    ImGradient();
    ~ImGradient();

    void setGreyscale(bool greyscale) { m_greyscale = greyscale; }
    
    void getColorAt(float position, float* color) const;
    markptr_t addMark(float position, ImColor const color);
    void removeMark(markptr_t mark);
    void clear();

    markvector_t &getMarks();
    markptr_t getSelected();
    markptr_t getDragging();

private:
    friend bool ImGui::GradientEditor(const char *str_id, ImGradient* gradient);

    void setSelected(markptr_t mark);
    void setDragging(markptr_t mark);

    void refreshCache();

    void computeColorAt(float position, float* color) const;

    markvector_t m_marks;
    markptr_t m_selectedMark;
    markptr_t m_draggingMark;
    float m_cachedValues[256 * 3];
    bool m_greyscale = false;
};
