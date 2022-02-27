//
//  imgui_color_gradient.cpp
//  imgui extension
//
//  Created by David Gallardo on 11/06/16.

#include "std_include.hpp"
#include "imgui_internal.h"

#include <algorithm>

static const float GRADIENT_BAR_WIDGET_HEIGHT = 25;
static const float GRADIENT_BAR_EDITOR_HEIGHT = 40;
static const float GRADIENT_MARK_DELETE_DIFFY = 40;

ImGradient::ImGradient()
{
    addMark(0.0f, ImColor(0.0f,0.0f,0.0f));
    addMark(1.0f, ImColor(1.0f,1.0f,1.0f));
}

ImGradient::~ImGradient()
{
}

ImGradient::markptr_t ImGradient::addMark(float position, ImColor const color)
{
    position = ImClamp(position, 0.0f, 1.0f);

    markptr_t newMark(new ImGradientMark);
    
    newMark->position = position;
    newMark->color[0] = color.Value.x;
    newMark->color[1] = color.Value.y;
    newMark->color[2] = color.Value.z;
    
    m_marks.push_back(newMark);
    
    refreshCache();
    return newMark;
}

void ImGradient::removeMark(markptr_t mark)
{
    for(auto i = m_marks.begin(); i != m_marks.end(); ++i)
    {
        if((*i) == mark)
        {
            m_marks.erase(i);
            break;
        }
    }
    
    refreshCache();
}

void ImGradient::clear()
{
    m_marks.clear();

    m_selectedMark = nullptr;
    m_draggingMark = nullptr;
}

void ImGradient::getColorAt(float position, float* color) const
{
    position = ImClamp(position, 0.0f, 1.0f);    

	int cachePos = static_cast<int>(position * 255);
    cachePos *= 3;

    color[0] = m_cachedValues[cachePos+0];
    color[1] = m_cachedValues[cachePos+1];
    color[2] = m_cachedValues[cachePos+2];
}

void ImGradient::computeColorAt(float position, float* color) const
{
    position = ImClamp(position, 0.0f, 1.0f);
    
    markptr_t lower = nullptr;
    markptr_t upper = nullptr;
    
    for(const auto &mark : m_marks)
    {
        if(mark->position < position)
        {
            if(!lower || lower->position < mark->position)
            {
                lower = mark;
            }
        }
        
        if(mark->position >= position)
        {
            if(!upper || upper->position > mark->position)
            {
                upper = mark;
            }
        }
    }
    
    if(upper && !lower)
    {
        lower = upper;
    }
    else if(!upper && lower)
    {
        upper = lower;
    }
    else if(!lower && !upper)
    {
        color[0] = color[1] = color[2] = 0;
        return;
    }
    
    if(upper == lower)
    {
        color[0] = upper->color[0];
        color[1] = upper->color[1];
        color[2] = upper->color[2];
    }
    else
    {
        const float distance = upper->position - lower->position;
        const float delta = (position - lower->position) / distance;
        
        //lerp
        color[0] = ((1.0f - delta) * lower->color[0]) + ((delta) * upper->color[0]);
        color[1] = ((1.0f - delta) * lower->color[1]) + ((delta) * upper->color[1]);
        color[2] = ((1.0f - delta) * lower->color[2]) + ((delta) * upper->color[2]);
    }
}

void ImGradient::refreshCache()
{
    std::sort(m_marks.begin(), m_marks.end(), [](const markptr_t &a, const markptr_t &b) { return a->position < b->position; });
    
    for(int i = 0; i < 256; ++i)
    {
        computeColorAt(i/255.0f, &m_cachedValues[i*3]);
    }
}

ImGradient::markvector_t &ImGradient::getMarks()
{
    return m_marks;
}

ImGradient::markptr_t ImGradient::getSelected()
{
    return m_selectedMark;
}

ImGradient::markptr_t ImGradient::getDragging()
{
    return m_draggingMark;
}

void ImGradient::setSelected(markptr_t mark)
{
    m_selectedMark = mark;
}

void ImGradient::setDragging(markptr_t mark)
{
    m_draggingMark = mark;
}

namespace ImGui
{
    static void DrawGradientBar(ImGradient* gradient,
                                struct ImVec2 const & bar_pos,
                                float maxWidth,
                                float height)
    {
        ImVec4 colorA = {1,1,1,1};
        ImVec4 colorB = {1,1,1,1};
        float prevX = bar_pos.x;
        float barBottom = bar_pos.y + height;
        ImGradient::markptr_t prevMark = nullptr;
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        
        /*draw_list->AddRectFilled(ImVec2(bar_pos.x - 2, bar_pos.y - 2),
                                 ImVec2(bar_pos.x + maxWidth + 2, barBottom + 2),
                                 IM_COL32(100, 100, 100, 255));*/
        
        if(gradient->getMarks().empty())
        {
            draw_list->AddRectFilled(ImVec2(bar_pos.x, bar_pos.y),
                                     ImVec2(bar_pos.x + maxWidth, barBottom),
                                     IM_COL32(255, 255, 255, 255));
            
        }
        
        ImU32 colorAU32 = 0;
        ImU32 colorBU32 = 0;
        
        for(auto markIt = gradient->getMarks().begin(); markIt != gradient->getMarks().end(); ++markIt )
        {
            auto mark = (*markIt);

            float from = prevX;
            float to = prevX = bar_pos.x + mark->position * maxWidth;
            
            if(prevMark == nullptr)
            {
                colorA.x = mark->color[0];
                colorA.y = mark->color[1];
                colorA.z = mark->color[2];
            }
            else
            {
                colorA.x = prevMark->color[0];
                colorA.y = prevMark->color[1];
                colorA.z = prevMark->color[2];
            }
            
            colorB.x = mark->color[0];
            colorB.y = mark->color[1];
            colorB.z = mark->color[2];
            
            colorAU32 = ImGui::ColorConvertFloat4ToU32(colorA);
            colorBU32 = ImGui::ColorConvertFloat4ToU32(colorB);
            
            if(mark->position > 0.0f)
            {
                
                draw_list->AddRectFilledMultiColor(ImVec2(from, bar_pos.y),
                                                   ImVec2(to, barBottom),
                                                   colorAU32, colorBU32, colorBU32, colorAU32);
            }
            
            prevMark = mark;
        }
        
        if(prevMark && prevMark->position < 1.0)
        {
            
            draw_list->AddRectFilledMultiColor(ImVec2(prevX, bar_pos.y),
                                               ImVec2(bar_pos.x + maxWidth, barBottom),
                                               colorBU32, colorBU32, colorBU32, colorBU32);
        }
        
        ImGui::SetCursorScreenPos(ImVec2(bar_pos.x, bar_pos.y + height + 10.0f));
    }
    
    static void DrawGradientMarks(ImGradient* gradient,
                                  ImGradient::markptr_t &draggingMark,
                                  ImGradient::markptr_t &selectedMark,
                                  struct ImVec2 const & bar_pos,
                                  float maxWidth,
                                  float height)
    {
        ImVec4 colorA = {1,1,1,1};
        ImVec4 colorB = {1,1,1,1};
        float barBottom = bar_pos.y + height;
        ImGradient::markptr_t prevMark = nullptr;
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImU32 colorAU32 = 0;
        ImU32 colorBU32 = 0;
        
        for(const auto& mark : gradient->getMarks() )
        {
            if(!selectedMark)
            {
                selectedMark = mark;
            }
            
            const float to = bar_pos.x + mark->position * maxWidth;
            
            if(prevMark == nullptr)
            {
                colorA.x = mark->color[0];
                colorA.y = mark->color[1];
                colorA.z = mark->color[2];
            }
            else
            {
                colorA.x = prevMark->color[0];
                colorA.y = prevMark->color[1];
                colorA.z = prevMark->color[2];
            }
            
            colorB.x = mark->color[0];
            colorB.y = mark->color[1];
            colorB.z = mark->color[2];
            
            colorAU32 = ImGui::ColorConvertFloat4ToU32(colorA);
            colorBU32 = ImGui::ColorConvertFloat4ToU32(colorB);
            
            draw_list->AddTriangleFilled(ImVec2(to, bar_pos.y + (height - 6)),
                                         ImVec2(to - 6, barBottom),
                                         ImVec2(to + 6, barBottom), IM_COL32(100, 100, 100, 255));
            
            draw_list->AddRectFilled(ImVec2(to - 6, barBottom),
                                     ImVec2(to + 6, bar_pos.y + (height + 12)),
                                     IM_COL32(100, 100, 100, 255), 1.0f);
            
            draw_list->AddRectFilled(ImVec2(to - 5, bar_pos.y + (height + 1)),
                                     ImVec2(to + 5, bar_pos.y + (height + 11)),
                                     IM_COL32(0, 0, 0, 255), 1.0f);
            
            if(selectedMark == mark)
            {
                draw_list->AddTriangleFilled(ImVec2(to, bar_pos.y + (height - 3)),
                                             ImVec2(to - 4, barBottom + 1),
                                             ImVec2(to + 4, barBottom + 1), IM_COL32(0, 255, 0, 255));
                
                draw_list->AddRect(ImVec2(to - 5, bar_pos.y + (height + 1)),
                                   ImVec2(to + 5, bar_pos.y + (height + 11)),
                                   IM_COL32(0, 255, 0, 255), 1.0f);
            }
            
            draw_list->AddRectFilledMultiColor(ImVec2(to - 3, bar_pos.y + (height + 3)),
                                               ImVec2(to + 3, bar_pos.y + (height + 9)),
                                               colorBU32, colorBU32, colorBU32, colorBU32);
            
            ImGui::SetCursorScreenPos(ImVec2(to - 6, barBottom));
            ImGui::InvisibleButton("mark", ImVec2(12, 12));
            
            if(ImGui::IsItemHovered())
            {
                if(ImGui::IsMouseClicked(0))
                {
                    selectedMark = mark;
                    draggingMark = mark;
                }
            }
            
            
            prevMark = mark;
        }
        
        ImGui::SetCursorScreenPos(ImVec2(bar_pos.x, bar_pos.y + height + 20.0f));
    }
    
    bool GradientButton(const char* str_id, ImGradient* gradient)
    {
        if(!gradient)
        {
	        return false;
        }

        ImGui::PushID(str_id);
        
        const ImVec2 widget_pos = ImGui::GetCursorScreenPos();
        const float maxWidth = ImMax(250.0f, ImGui::GetContentRegionAvailWidth() - 100.0f);
        const bool clicked = ImGui::InvisibleButton("InvisibleButton", ImVec2(maxWidth, GRADIENT_BAR_WIDGET_HEIGHT));
        
        DrawGradientBar(gradient, widget_pos, maxWidth, GRADIENT_BAR_WIDGET_HEIGHT);

        ImGui::PopID();
        
        return clicked;
    }
    
    bool GradientEditor(const char* str_id, ImGradient* gradient)
    {
		if (!gradient)
		{
			return false;
		}

		bool modified = false;
		const ImGuiStyle& style = ImGui::GetStyle();

		const ImGuiWindow* window = GetCurrentWindow();
		const auto indent = window->DC.Indent.x;

        ImVec2 bar_pos = ImGui::GetCursorScreenPos();
		bar_pos.x -= indent;
		bar_pos.x += style.FramePadding.x;

		const float maxWidth = ImGui::GetContentRegionAvailWidth() - (style.FramePadding.x * 2.0f) + indent;
        const float barBottom = bar_pos.y + GRADIENT_BAR_EDITOR_HEIGHT;

        ImGui::PushID(str_id);

        ImGui::InvisibleButton("GradientBar", ImVec2(maxWidth + 1.0f, GRADIENT_BAR_EDITOR_HEIGHT));

		if (ImGui::BeginPopupContextItem("gradiennt_context##bg"))
		{
			if (ImGui::MenuItem("Reset"))
			{
				gradient->getMarks().clear();
				gradient->addMark(0.0f, ImColor(1.0f, 1.0f, 1.0f));
				gradient->addMark(1.0f, ImColor(1.0f, 1.0f, 1.0f));
				modified = true;
			}

			ImGui::EndPopup();
		}

        if(ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
        {
			float newMarkCol[4];
			float pos = (ImGui::GetIO().MousePos.x - bar_pos.x) / maxWidth;
			
        	if (pos < 0.01f) {
				pos = 0.01f;
            }
			else if(pos > 0.99f) {
				pos = 0.99f;
			}


            gradient->getColorAt(pos, newMarkCol);
            const auto mark = gradient->addMark(pos, ImColor(newMarkCol[0], newMarkCol[1], newMarkCol[2]));
            gradient->setSelected(mark);

            modified = true;
        }
        
        DrawGradientBar(gradient, bar_pos, maxWidth, GRADIENT_BAR_EDITOR_HEIGHT);

        auto draggingMark = gradient->getDragging();
        auto selectedMark = gradient->getSelected();

        DrawGradientMarks(gradient, draggingMark, selectedMark, bar_pos, maxWidth, GRADIENT_BAR_EDITOR_HEIGHT);

        if(!ImGui::IsMouseDown(0) && draggingMark)
        {
            draggingMark = nullptr;
        }

        if(ImGui::IsMouseDragging(0) && draggingMark 
			&& draggingMark->position != 0.0f && draggingMark->position != 1.0f) // prevent first and last drag
        {
            const float increment = ImGui::GetIO().MouseDelta.x / maxWidth;
            const bool  insideZone = (ImGui::GetIO().MousePos.x > bar_pos.x) && (ImGui::GetIO().MousePos.x < bar_pos.x + maxWidth);
            
            if(increment != 0.0f && insideZone)
            {
                draggingMark->position += increment;
                draggingMark->position = ImClamp(draggingMark->position, 0.01f, 0.99f); // prevent additional keys reaching min/max
                gradient->refreshCache();
                modified = true;
            }
            
            const float diffY = ImGui::GetIO().MousePos.y - barBottom;
            
            if(diffY >= GRADIENT_MARK_DELETE_DIFFY)
            {
                gradient->removeMark(draggingMark);
                draggingMark = nullptr;
                selectedMark = nullptr;
                modified = true;
            }
        }
        
        if(!selectedMark && !gradient->getMarks().empty())
        {
            selectedMark = (gradient->getMarks().front());
        }
        
        if(selectedMark)
        {
            if(gradient->m_greyscale)
            {
                if(ImGui::SliderFloat("Value", &selectedMark->color[0], 0, 1, "%.2f", 1))
                {
                    modified = true;
                    selectedMark->color[2] = selectedMark->color[1] = selectedMark->color[0];
                    gradient->refreshCache();
                        
                }
            }
            else
            {
				
				float color_picker_width = ImGui::GetWindowContentRegionWidth() - (style.FramePadding.x * 2);
				float color_picker_width_uncapped = color_picker_width;

            	if (color_picker_width > 380)
				{
					color_picker_width = 380;
				}

				color_picker_width_uncapped -= color_picker_width;
				color_picker_width_uncapped *= 0.5f;
				color_picker_width_uncapped += style.FramePadding.x;

				ImGui::SetNextItemWidth(color_picker_width);
				ImGui::SetCursorPosX(color_picker_width_uncapped);

                if(ImGui::ColorPicker3("##ColorPicker3", selectedMark->color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHex | ImGuiColorEditFlags_Float))
                {
                    modified = true;
                    gradient->refreshCache();
                }
            }  
        }

        gradient->setDragging(draggingMark);
        gradient->setSelected(selectedMark);

        ImGui::PopID();
        
        return modified;
    }
};