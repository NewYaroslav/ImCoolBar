/*
MIT License

Copyright (c) 2023 Stephane Cuillerdier (aka Aiekick)
Copyright (c) 2025 NewYaroslav

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

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "ImCoolBar.h"
#include "imgui_internal.h"
#include <cmath>
#include <vector>
#include <array>

#ifndef IMCOOLBAR_HAS_DOCKING
#if defined(IMGUI_HAS_DOCK)
#define IMCOOLBAR_HAS_DOCKING
#endif
#endif

#ifdef IMCOOLBAR_HAS_DOCKING
#define ICB_DOCKING_HOST_FLAGS (ImGuiWindowFlags_DockNodeHost | ImGuiWindowFlags_NoDocking)
#else
#define ICB_DOCKING_HOST_FLAGS 0
#endif

#define ICB_PREFIX "ICB"
//#define ENABLE_IMCOOLBAR_DEBUG

#ifdef _MSC_VER
#include <Windows.h>
#define ICB_DEBUG_BREAK       \
    if (IsDebuggerPresent()) \
    __debugbreak()
#else
#define ICB_DEBUG_BREAK
#endif

#define BREAK_ON_KEY(KEY)         \
    if (ImGui::IsKeyPressed(KEY)) \
    ICB_DEBUG_BREAK
    
namespace {

    static float bubbleEffect(const float vValue, const float vStength) {
        return pow(cos(vValue * IM_PI * vStength), 12.0f);
    }

    // https://codesandbox.io/s/motion-dock-forked-hs4p8d?file=/src/Dock.tsx
    static float getHoverSize(const float vValue, const float vNormalSize, const float vHoveredSize, const float vStength, const float vScale) {
        return ImClamp(vNormalSize + (vHoveredSize - vNormalSize) * bubbleEffect(vValue, vStength) * vScale, vNormalSize, vHoveredSize);
    }

    static bool isWindowHovered(ImGuiWindow* vWindow) {
        return ImGui::IsMouseHoveringRect(vWindow->Rect().Min, vWindow->Rect().Max);
    }

    static float getBarSize(const float vNormalSize, const float vHoveredSize, const float vScale) {
        return vNormalSize + (vHoveredSize - vNormalSize) * vScale;
    }

    static float getChannel(const ImVec2& vVec, const ImCoolBarFlags vCBFlags) {
        if (vCBFlags & ImCoolBarFlags_Horizontal) {
            return vVec.x;
        }
        return vVec.y;
    }

    static float getChannelInv(const ImVec2& vVec, const ImCoolBarFlags vCBFlags) {
        if (vCBFlags & ImCoolBarFlags_Horizontal) {
            return vVec.y;
        }
        return vVec.x;
    }

    static void emaReset(ImGuiID id, float value) {
        ImGui::GetStateStorage()->SetFloat(id, value);
    }

    static float emaUpdate(ImGuiID id, float sample, float alpha) {
        ImGuiStorage* st = ImGui::GetStateStorage();
        float current = st->GetFloat(id, sample);
        current += alpha * (sample - current);
        st->SetFloat(id, current);
        return current;
    }

    //static float emaValue(ImGuiID id, float default_value) {
    //    return ImGui::GetStateStorage()->GetFloat(id, default_value);
    //}

}; // namespace

IMGUI_API bool ImGui::BeginCoolBar(const char* vLabel, ImCoolBarFlags vCBFlags, const ImCoolBarConfig& vConfig, ImGuiWindowFlags vFlags) {
    ImGuiWindowFlags flags =                   //
        vFlags |                               //
        ImGuiWindowFlags_NoTitleBar |          //
        ImGuiWindowFlags_NoScrollbar |         //
        ImGuiWindowFlags_AlwaysAutoResize |    //
        ImGuiWindowFlags_NoCollapse |          //
        ImGuiWindowFlags_NoMove |              //
        ImGuiWindowFlags_NoSavedSettings |     //
#       ifndef ENABLE_IMCOOLBAR_DEBUG          //
        ImGuiWindowFlags_NoBackground |        //
#       endif                                  //
        ImGuiWindowFlags_NoFocusOnAppearing |  //
        ICB_DOCKING_HOST_FLAGS;                //
    bool res = ImGui::Begin(vLabel, nullptr, flags);
    if (!res) {
        ImGui::End();
    } else {
        // Only one orientation flag (horizontal or vertical) may be active
        IM_ASSERT(                                                                    //
            ((vCBFlags & ImCoolBarFlags_Horizontal) == ImCoolBarFlags_Horizontal) ||  //
            ((vCBFlags & ImCoolBarFlags_Vertical) == ImCoolBarFlags_Vertical)         //
        );

        ImGuiContext& g = *GImGui;
        ImGuiWindow* window_ptr = GetCurrentWindow();

        // --- Force local AA for this bar (and remember previous flags) ---
        {
            ImDrawList* dl = ImGui::GetWindowDrawList();

            const ImGuiID dl_flags_id     = window_ptr->GetID(ICB_PREFIX "PrevDLFlags");
            const ImGuiID dl_flags_set_id = window_ptr->GetID(ICB_PREFIX "PrevDLFlagsSet");
            const ImGuiID pushed_round_id = window_ptr->GetID(ICB_PREFIX "PushedRounding");
            
            
            if (vConfig.local_antialiasing) {
                window_ptr->StateStorage.SetInt (dl_flags_id, (int)dl->Flags);
                window_ptr->StateStorage.SetBool(dl_flags_set_id, true);
                dl->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

                if (vConfig.frame_rounding_override >= 0.0f) {
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, vConfig.frame_rounding_override);
                    window_ptr->StateStorage.SetBool(pushed_round_id, true);
                } else {
                    window_ptr->StateStorage.SetBool(pushed_round_id, false);
                }
            } else {
                // Ensure EndCoolBar has no state to restore
                window_ptr->StateStorage.SetBool(dl_flags_set_id, false);
                window_ptr->StateStorage.SetBool(pushed_round_id, false);
            }
            
            window_ptr->StateStorage.SetBool(window_ptr->GetID(ICB_PREFIX "SnapItemsToPixels"),  vConfig.snap_items_to_pixels);
            window_ptr->StateStorage.SetBool(window_ptr->GetID(ICB_PREFIX "SnapWindowToPixels"), vConfig.snap_window_to_pixels);
        }

        window_ptr->StateStorage.SetVoidPtr(window_ptr->GetID(ICB_PREFIX "Type"), (void*)"ImCoolBar");
        window_ptr->StateStorage.SetInt(window_ptr->GetID(ICB_PREFIX "ItemIdx"), 0);
        window_ptr->StateStorage.SetInt(window_ptr->GetID(ICB_PREFIX "Flags"), vCBFlags);

        const float anchor = ImClamp(getChannelInv(vConfig.anchor, vCBFlags), 0.0f, 1.0f);
        window_ptr->StateStorage.SetFloat(window_ptr->GetID(ICB_PREFIX "Anchor"), anchor);

        window_ptr->StateStorage.SetFloat(window_ptr->GetID(ICB_PREFIX "NormalSize"), vConfig.normal_size);
        window_ptr->StateStorage.SetFloat(window_ptr->GetID(ICB_PREFIX "HoveredSize"), vConfig.hovered_size);
        window_ptr->StateStorage.SetFloat(window_ptr->GetID(ICB_PREFIX "EffectStrength"), vConfig.effect_strength);

        // --- Time-based smoothing for anim_scale (EMA) --------------------------------
        float anim_scale = 0.0;
        {
            const ImGuiID anim_scale_id = window_ptr->GetID(ICB_PREFIX "AnimScale");
            anim_scale = window_ptr->StateStorage.GetFloat(anim_scale_id); // prev value (0 by default)

            const bool  hovered_now = isWindowHovered(window_ptr);
            const float target      = hovered_now ? 1.0f : 0.0f;

            // alpha = 1 - exp(-ln(2) * dt / HL)
            float anim_alpha = 1.0f;
            if (vConfig.anim_smoothing_ms > 0.0f) {
                const float dt_ms = ImGui::GetIO().DeltaTime * 1000.0f;
                const float k     = 0.69314718056f; // ln(2)
                anim_alpha = 1.0f - expf(-k * (dt_ms / vConfig.anim_smoothing_ms));
                anim_alpha = ImClamp(anim_alpha, 0.0f, 1.0f);

                anim_scale += anim_alpha * (target - anim_scale);     // EMA step
            } else {
                // legacy step-based ramp
                if (hovered_now)  anim_scale = ImMin(1.0f, anim_scale + vConfig.anim_step);
                else              anim_scale = ImMax(0.0f, anim_scale - vConfig.anim_step);
            }

            anim_scale = ImClamp(anim_scale, 0.0f, 1.0f);
            window_ptr->StateStorage.SetFloat(anim_scale_id, anim_scale);

            window_ptr->StateStorage.SetFloat(window_ptr->GetID(ICB_PREFIX "AnimSmoothingMs"), vConfig.anim_smoothing_ms);
            window_ptr->StateStorage.SetFloat(window_ptr->GetID(ICB_PREFIX "AnimSmoothingAlpha"),      anim_alpha);
        }
        
        // --- Time-based smoothing setup for mouse (compute per-frame alpha, EMA) ------------------------
        {
            ImGuiIO& io = ImGui::GetIO();
            const ImGuiID ema_hl_id    = window_ptr->GetID(ICB_PREFIX "MouseSmoothingMs");
            const ImGuiID ema_alpha_id = window_ptr->GetID(ICB_PREFIX "MouseSmoothingAlpha");
            const ImGuiID ema_init_id  = window_ptr->GetID(ICB_PREFIX "MouseSmoothingInit");
            const ImGuiID reseed_id    = window_ptr->GetID(ICB_PREFIX "MouseReseedPending");

            window_ptr->StateStorage.SetFloat(ema_hl_id, vConfig.mouse_smoothing_ms);

            float alpha = 1.0f; // disabled by default -> pass-through
            if (vConfig.mouse_smoothing_ms > 0.0f) {
                // alpha = 1 - exp(-ln(2) * dt / HL)
                float dt_ms = ImGui::GetIO().DeltaTime * 1000.0f;
                dt_ms = ImMin(dt_ms, 100.0f);
                const float k     = 0.69314718056f; // ln(2)
                alpha = 1.0f - expf(-k * (dt_ms / vConfig.mouse_smoothing_ms));
                alpha = ImClamp(alpha, 0.0f, 1.0f);
            }
            window_ptr->StateStorage.SetFloat(ema_alpha_id, alpha);

            // NOTE: do NOT reset EMA on leave — we want continuity across hover boundaries.
            if (io.AppFocusLost || !ImGui::IsMousePosValid())
                window_ptr->StateStorage.SetBool(ema_init_id, false), window_ptr->StateStorage.SetBool(reseed_id, true);
        }
        
        // --- Update filtered mouse (once per frame), independent of hover ------
        {
            const ImGuiID ema_id      = window_ptr->GetID(ICB_PREFIX "MouseSmoothing");
            const ImGuiID ema_init_id = window_ptr->GetID(ICB_PREFIX "MouseSmoothingInit");
            const ImGuiID reseed_id   = window_ptr->GetID(ICB_PREFIX "MouseReseedPending");
            const ImGuiID last_m_id   = window_ptr->GetID(ICB_PREFIX "LastMousePos");
            const float   alpha       = window_ptr->StateStorage.GetFloat(window_ptr->GetID(ICB_PREFIX "MouseSmoothingAlpha"));
            const float   hl_ms       = window_ptr->StateStorage.GetFloat(window_ptr->GetID(ICB_PREFIX "MouseSmoothingMs"));

            if (ImGui::IsMousePosValid()) {
                float m_raw = getChannel(ImGui::GetMousePos(), vCBFlags);
                float m_flt = m_raw;
                const bool need_seed = !window_ptr->StateStorage.GetBool(ema_init_id) ||
                                        window_ptr->StateStorage.GetBool(reseed_id);
                if (hl_ms > 0.0f && alpha > 0.0f) {
                    if (need_seed) {
                        emaReset(ema_id, m_raw);                 // seed once on first valid
                        window_ptr->StateStorage.SetBool(ema_init_id, true);
                        window_ptr->StateStorage.SetBool(reseed_id, false);
                    } else {
                        m_flt = emaUpdate(ema_id, m_raw, alpha); // EMA step
                    }
                }
                window_ptr->StateStorage.SetFloat(last_m_id, m_flt);
            }
        }

        // --- Position with predicted cross-axis size for THIS frame ---
        ImVec2 pad = ImGui::GetStyle().WindowPadding * 2.0f;
        ImVec2 bar_size = window_ptr->ContentSize + pad; // along main axis ok
        const float normal_size  = window_ptr->StateStorage.GetFloat(window_ptr->GetID(ICB_PREFIX "NormalSize"));
        const float hovered_size = window_ptr->StateStorage.GetFloat(window_ptr->GetID(ICB_PREFIX "HoveredSize"));
        const float cross = getBarSize(normal_size, hovered_size, anim_scale);
        if (vCBFlags & ImCoolBarFlags_Horizontal) {
            bar_size.y = cross + pad.y;
        } else {
            bar_size.x = cross + pad.x;
        }
        const ImGuiViewport* vp = window_ptr->Viewport;
        ImVec2 new_pos = vp->Pos + (vp->Size - bar_size) * vConfig.anchor;
        if (vConfig.snap_window_to_pixels) {
            new_pos.x = ImFloor(new_pos.x);
            new_pos.y = ImFloor(new_pos.y);
        }
        ImGui::SetWindowPos(new_pos);
        
        window_ptr->StateStorage.SetBool(window_ptr->GetID(ICB_PREFIX "SnapItemsToPixels"),
                                         vConfig.snap_items_to_pixels);

    }
    return res;
}

IMGUI_API void ImGui::EndCoolBar() {
    // Restore AA flags and rounding we pushed in BeginCoolBar()
    ImGuiWindow* window_ptr = GetCurrentWindow();
    if (window_ptr) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImGuiID dl_flags_set_id = window_ptr->GetID(ICB_PREFIX "PrevDLFlagsSet");
        if (window_ptr->StateStorage.GetBool(dl_flags_set_id)) {
            const ImGuiID dl_flags_id = window_ptr->GetID(ICB_PREFIX "PrevDLFlags");
            dl->Flags = (ImDrawListFlags)window_ptr->StateStorage.GetInt(dl_flags_id);
            window_ptr->StateStorage.SetBool(dl_flags_set_id, false);
        }
        const ImGuiID pushed_round_id = window_ptr->GetID(ICB_PREFIX "PushedRounding");
        if (window_ptr->StateStorage.GetBool(pushed_round_id)) {
            ImGui::PopStyleVar(); // FrameRounding
            window_ptr->StateStorage.SetBool(pushed_round_id, false);
        }
    }
    ImGui::End();
}

IMGUI_API bool ImGui::CoolBarItem() {
    ImGuiWindow* window_ptr = GetCurrentWindow();
    if (window_ptr->SkipItems)
        return false;

    const auto item_index_id = window_ptr->GetID(ICB_PREFIX "ItemIdx");
    const auto idx = window_ptr->StateStorage.GetInt(item_index_id);
    const auto coolbar_item_id = window_ptr->GetID(window_ptr->ID + idx + 1);
    float current_item_size = window_ptr->StateStorage.GetFloat(coolbar_item_id);
    const auto flags             = window_ptr->StateStorage.GetInt(window_ptr->GetID(ICB_PREFIX "Flags"));
    const auto anim_scale        = window_ptr->StateStorage.GetFloat(window_ptr->GetID(ICB_PREFIX "AnimScale"));
    const auto normal_size       = window_ptr->StateStorage.GetFloat(window_ptr->GetID(ICB_PREFIX "NormalSize"));
    const auto hovered_size      = window_ptr->StateStorage.GetFloat(window_ptr->GetID(ICB_PREFIX "HoveredSize"));
    const auto effect_strength = window_ptr->StateStorage.GetFloat(window_ptr->GetID(ICB_PREFIX "EffectStrength"));
    const auto last_mouse_pos_id = window_ptr->GetID(ICB_PREFIX "LastMousePos");
    auto last_mouse_pos = window_ptr->StateStorage.GetFloat(last_mouse_pos_id);

    assert(normal_size > 0.0f);

    if (flags & ImCoolBarFlags_Horizontal) {
        if (idx) {
            ImGui::SameLine();
        }
    }

    float current_size = normal_size;

    ImGuiContext& g = *GImGui;
    
    // Read filtered mouse, already updated once in BeginCoolBar()
    last_mouse_pos = window_ptr->StateStorage.GetFloat(window_ptr->GetID(ICB_PREFIX "LastMousePos"));
    
    if (current_item_size <= 0.0f) current_item_size = normal_size;

    if (anim_scale > 0.0f) {
        const auto csp = getChannel(ImGui::GetCursorScreenPos(), flags);
        const auto ws = getChannel(window_ptr->Size, flags);
        const auto wp = getChannel(g.Style.WindowPadding, flags);
        const float btn_center = csp + current_item_size * 0.5f;
        const float diff_pos = (last_mouse_pos - btn_center) / ws;
        current_size = getHoverSize(diff_pos, normal_size, hovered_size, effect_strength, anim_scale);
        const float anchor = window_ptr->StateStorage.GetFloat(window_ptr->GetID(ICB_PREFIX "Anchor"));
        const float bar_height = getBarSize(normal_size, hovered_size, anim_scale);
        float btn_offset = (bar_height - current_size) * anchor + wp;
        if (window_ptr->StateStorage.GetBool(window_ptr->GetID(ICB_PREFIX "SnapItemsToPixels"))) {
            btn_offset = ImFloor(btn_offset);
        }
        if (flags & ImCoolBarFlags_Horizontal) {
            ImGui::SetCursorPosY(btn_offset);
        } else if (flags & ImCoolBarFlags_Vertical) {
            ImGui::SetCursorPosX(btn_offset);
        }
    }

    BREAK_ON_KEY(ImGuiKey_D);
    window_ptr->StateStorage.SetInt(item_index_id, idx + 1);
    window_ptr->StateStorage.SetFloat(coolbar_item_id, current_size);
    window_ptr->StateStorage.SetFloat(last_mouse_pos_id, last_mouse_pos);
    window_ptr->StateStorage.SetFloat(window_ptr->GetID(ICB_PREFIX "ItemCurrentSize"), current_size);
    window_ptr->StateStorage.SetFloat(window_ptr->GetID(ICB_PREFIX "ItemCurrentScale"), current_size / normal_size);

    return true;
}

IMGUI_API float ImGui::GetCoolBarItemWidth() {
    ImGuiWindow* window_ptr = GetCurrentWindow();
    if (window_ptr->SkipItems) {
        return 0.0f;
    }
    return window_ptr->StateStorage.GetFloat(  //
        window_ptr->GetID(ICB_PREFIX "ItemCurrentSize"));
}

IMGUI_API float ImGui::GetCoolBarItemScale() {
    ImGuiWindow* window_ptr = GetCurrentWindow();
    if (window_ptr->SkipItems) {
        return 0.0f;
    }

    return window_ptr->StateStorage.GetFloat(  //
        window_ptr->GetID(ICB_PREFIX "ItemCurrentScale"));
}

IMGUI_API void ImGui::ShowCoolBarMetrics(bool* vOpened) {
    if (ImGui::Begin("ImCoolBar Metrics", vOpened)) {
        ImGuiContext& g = *GImGui;
        for (auto* window_ptr : g.Windows) {
            const char* type = (const char*)window_ptr->StateStorage.GetVoidPtr(window_ptr->GetID(ICB_PREFIX "Type"));
            if (type != nullptr && strcmp(type, "ImCoolBar") == 0) {
                if (!TreeNode(window_ptr, "ImCoolBar %s", window_ptr->Name)) {
                    continue;
                }

                const auto flags_id = window_ptr->GetID(ICB_PREFIX "Flags");
                const auto anchor_id = window_ptr->GetID(ICB_PREFIX "Anchor");
                const auto anim_scale_id = window_ptr->GetID(ICB_PREFIX "AnimScale");
                const auto item_index_id = window_ptr->GetID(ICB_PREFIX "ItemIdx");
                const auto normal_size_id = window_ptr->GetID(ICB_PREFIX "NormalSize");
                const auto hovered_size_id = window_ptr->GetID(ICB_PREFIX "HoveredSize");
                const auto effect_strength_id = window_ptr->GetID(ICB_PREFIX "EffectStrength");
                const auto item_current_size_id = window_ptr->GetID(ICB_PREFIX "ItemCurrentSize");
                const auto item_current_scale_id = window_ptr->GetID(ICB_PREFIX "ItemCurrentScale");
                const auto ema_hl_id = window_ptr->GetID(ICB_PREFIX "MouseSmoothingMs");
                const auto ema_alpha_id = window_ptr->GetID(ICB_PREFIX "MouseSmoothingAlpha");
                const auto ema_anim_hl_id = window_ptr->GetID(ICB_PREFIX "AnimSmoothingMs");
                const auto ema_anim_alpha_id = window_ptr->GetID(ICB_PREFIX "AnimSmoothingAlpha");

                const auto flags = window_ptr->StateStorage.GetInt(flags_id);
                const auto anchor = window_ptr->StateStorage.GetFloat(anchor_id);
                const auto max_idx = window_ptr->StateStorage.GetInt(item_index_id);
                const auto anim_scale = window_ptr->StateStorage.GetFloat(anim_scale_id);
                const auto normal_size = window_ptr->StateStorage.GetFloat(normal_size_id);
                const auto hovered_size = window_ptr->StateStorage.GetFloat(hovered_size_id);
                const auto effect_strength = window_ptr->StateStorage.GetFloat(effect_strength_id);
                const auto item_current_size = window_ptr->StateStorage.GetFloat(item_current_size_id);
                const auto item_current_scale = window_ptr->StateStorage.GetFloat(item_current_scale_id);

#define SetColumnLabel(a, fmt, v) \
    ImGui::TableNextColumn();     \
    ImGui::Text("%s", a);         \
    ImGui::TableNextColumn();     \
    ImGui::Text(fmt, v);          \
    ImGui::TableNextRow()

                if (ImGui::BeginTable("CoolbarDebugDatas", 2)) {
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    SetColumnLabel("MaxIdx ", "%i", max_idx);
                    SetColumnLabel("Anchor ", "%f", anchor);
                    SetColumnLabel("AnimScale ", "%f", anim_scale);
                    SetColumnLabel("NormalSize ", "%f", normal_size);
                    SetColumnLabel("HoveredSize ", "%f", hovered_size);
                    SetColumnLabel("EffectStrength ", "%f", effect_strength);
                    SetColumnLabel("ItemCurrentSize ", "%f", item_current_size);
                    SetColumnLabel("ItemCurrentScale ", "%f", item_current_scale);
                    SetColumnLabel("MouseSmoothingMs  ", "%f", window_ptr->StateStorage.GetFloat(ema_hl_id));
                    SetColumnLabel("MouseSmoothingAlpha ", "%f", window_ptr->StateStorage.GetFloat(ema_alpha_id));
                    SetColumnLabel("AnimSmoothingMs ", "%f", window_ptr->StateStorage.GetFloat(ema_anim_hl_id));
                    SetColumnLabel("AnimSmoothingAlpha ", "%f", window_ptr->StateStorage.GetFloat(ema_anim_alpha_id));
                    
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", "Flags ");
                    ImGui::TableNextColumn();
                    if (flags & ImCoolBarFlags_None) {
                        ImGui::Text("None");
                    }
                    if (flags & ImCoolBarFlags_Vertical) {
                        ImGui::Text("Vertical");
                    }
                    if (flags & ImCoolBarFlags_Horizontal) {
                        ImGui::Text("Horizontal");
                    }
                    ImGui::TableNextRow();

                    for (int idx = 0; idx < max_idx; ++idx) {
                        const auto coolbar_item_id = window_ptr->GetID(window_ptr->ID + idx + 1);
                        const auto current_item_size = window_ptr->StateStorage.GetFloat(coolbar_item_id);
                        ImGui::TableNextColumn();
                        ImGui::Text("Item %i Size ", idx);
                        ImGui::TableNextColumn();
                        ImGui::Text("%f", current_item_size);
                        ImGui::TableNextRow();
                    }

                    ImGui::EndTable();
                }

#undef SetColumnLabel
                TreePop();
            } 
        }
    }
    ImGui::End();
}
