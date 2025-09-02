/*
MIT License

Copyright (c) 2024 Stephane Cuillerdier (aka Aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "imgui.h"

#if !defined(IMCOOLBAR_HAS_DOCKING) && defined(IMGUI_HAS_DOCK)
#define IMCOOLBAR_HAS_DOCKING
#endif

typedef int ImCoolBarFlags;                //
enum ImCoolBarFlags_ {                     //
    ImCoolBarFlags_None       = 0,         //
    ImCoolBarFlags_Vertical   = (1 << 0),  //
    ImCoolBarFlags_Horizontal = (1 << 1),  //
};

struct ImCoolBarConfig {
    ImVec2 anchor                    = ImVec2(-1.0f, -1.0f);  //
    float normal_size                = 40.0f;                 //
    float hovered_size               = 60.0f;                 //
    float anim_step                  = 0.15f;                 //
    float effect_strength            = 0.5f;                  //
    float mouse_smoothing_ms         = 50.0f;                 // <=0 disables time-based smoothing (EMA half-life)
    float anim_smoothing_ms          = 50.0f;                 // <=0 uses step-based anim_step (EMA half-life)
    bool snap_window_to_pixels       = true;                  // snap window pos to integer pixels
    bool snap_items_to_pixels        = false;                 // snap inner offsets (buttons)
    bool local_antialiasing          = true;                  // локально включить AA для бара
    float frame_rounding_override    = -1.0f;                 // <0 = не трогать, >=0 = PushStyleVar(FrameRounding, value)
    ImCoolBarConfig(                                          //
        const ImVec2 vAnchor         = ImVec2(-1.0f, -1.0f),  //
        const float  vNormalSize     = 40.0f,                 //
        const float  vHoveredSize    = 60.0f,                 //
        const float  vAnimStep       = 0.15f,                 //
        const float  vEffectStrength = 0.5f,                  //
        const float  vMouseSmoothingMs      = 50.0f,          //
        const float  vAnimSmoothingMs       = 50.0f,          //
        const bool   vSnapWindowToPixels    = true,           //
        const bool   vSnapItemsToPixels     = false,
        const bool   vLocalAntialiasing     = true,
        const float  vFrameRoundingOverride = 50.0f)          //
        :                                                     //
          anchor(vAnchor),                                    //
          normal_size(vNormalSize),                           //
          hovered_size(vHoveredSize),                         //
          anim_step(vAnimStep),                               //
          effect_strength(vEffectStrength),                   //
          mouse_smoothing_ms(vMouseSmoothingMs),              //
          anim_smoothing_ms(vAnimSmoothingMs),                //
          snap_window_to_pixels(vSnapWindowToPixels),         //
          snap_items_to_pixels(vSnapItemsToPixels),
          local_antialiasing(vLocalAntialiasing),             //
          frame_rounding_override(vFrameRoundingOverride)
    {
    }
};

namespace ImGui {

IMGUI_API bool BeginCoolBar(const char* vLabel, ImCoolBarFlags vCBFlags = ImCoolBarFlags_Vertical, const ImCoolBarConfig& vConfig = {}, ImGuiWindowFlags vFlags = ImGuiWindowFlags_None);
IMGUI_API void EndCoolBar();
IMGUI_API bool CoolBarItem();
IMGUI_API float GetCoolBarItemWidth();
IMGUI_API float GetCoolBarItemScale();
IMGUI_API void ShowCoolBarMetrics(bool* vOpened);

}  // namespace ImGui
