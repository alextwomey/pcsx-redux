/***************************************************************************
 *   Copyright (C) 2019 PCSX-Redux authors                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#include "gui/widgets/breakpoints.h"

#include "fmt/format.h"
#include "imgui.h"
#include "support/imgui-helpers.h"

static ImVec4 s_currentColor = ImColor(0xff, 0xeb, 0x3b);

void PCSX::Widgets::Breakpoints::showEditLabelPopup(const Debug::Breakpoint* bp, int counter) {
    std::string name = bp->name();
    std::string title = fmt::format(f_("Edit label of breakpoint {}##{}"), name, counter);
    if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text(_("Change the label of breakpoint %s:"), name.c_str());
        if (ImGui::InputText(_("Label"), m_bpEditPopupLabel, sizeof(m_bpEditPopupLabel),
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
            bp->label(m_bpEditPopupLabel);
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button(_("Cancel"))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void PCSX::Widgets::Breakpoints::draw(const char* title) {
    ImGui::SetNextWindowPos(ImVec2(520, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(title, &m_show, ImGuiWindowFlags_MenuBar)) {
        ImGui::End();
        return;
    }
    auto& debugger = PCSX::g_emulator->m_debug;
    if (ImGui::Button(_("Clear maps"))) {
        debugger->clearMaps();
    }
    ImGuiHelpers::ShowHelpMarker(
        _("The mapping feature is a simple concept, but requires some amount of explanation. See the documentation "
          "website for more details, in the Misc Features subsection of the Debugging section."));
    ImGui::Checkbox(_("Map execution"), &debugger->m_mapping_e);
    ImGui::Checkbox(_("Map byte reads         "), &debugger->m_mapping_r8);
    ImGui::SameLine();
    ImGui::Checkbox(_("Map half reads         "), &debugger->m_mapping_r16);
    ImGui::SameLine();
    ImGui::Checkbox(_("Map word reads         "), &debugger->m_mapping_r32);
    ImGui::Checkbox(_("Map byte writes        "), &debugger->m_mapping_w8);
    ImGui::SameLine();
    ImGui::Checkbox(_("Map half writes        "), &debugger->m_mapping_w16);
    ImGui::SameLine();
    ImGui::Checkbox(_("Map word writes        "), &debugger->m_mapping_w32);
    ImGui::Separator();
    ImGui::Checkbox(_("Break on execution map"), &debugger->m_breakmp_e);
    ImGui::Checkbox(_("Break on byte read map "), &debugger->m_breakmp_r8);
    ImGui::SameLine();
    ImGui::Checkbox(_("Break on half read map "), &debugger->m_breakmp_r16);
    ImGui::SameLine();
    ImGui::Checkbox(_("Break on word read map "), &debugger->m_breakmp_r32);
    ImGui::Checkbox(_("Break on byte write map"), &debugger->m_breakmp_w8);
    ImGui::SameLine();
    ImGui::Checkbox(_("Break on half write map"), &debugger->m_breakmp_w16);
    ImGui::SameLine();
    ImGui::Checkbox(_("Break on word write map"), &debugger->m_breakmp_w32);
    ImGui::Separator();
    ImGui::TextUnformatted(_("Breakpoints"));

    ImGuiStyle& style = ImGui::GetStyle();
    const float heightSeparator = style.ItemSpacing.y;
    float footerHeight = 0;
    footerHeight += (heightSeparator * 2 + ImGui::GetTextLineHeightWithSpacing()) * 5;  // 5 footer rows
    float glyphWidth = ImGui::GetFontSize();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGui::BeginChild("BreakpointsList", ImVec2(0, -footerHeight), true);
    const Debug::Breakpoint* toErase = nullptr;
    std::string editorToOpen = "";
    auto& tree = debugger->getTree();
    int counter = 0;
    for (auto bp = tree.begin(); bp != tree.end(); bp++, counter++) {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        std::string name = bp->name();
        if (bp->enabled()) {
            ImGui::Text("  %s", name.c_str());
        } else {
            ImGui::TextDisabled("  %s", name.c_str());
        }
        // there can be multiple breakpoints with the same name, so we need the counter
        // to make widget IDs unique
        std::string uniqueId = fmt::format("{}{}", name, counter);
        ImGui::SameLine();
        std::string buttonLabel = _("Remove##") + uniqueId;
        if (ImGui::Button(buttonLabel.c_str())) toErase = &*bp;
        ImGui::SameLine();
        if (bp->enabled()) {
            buttonLabel = _("Disable##") + uniqueId;
            if (ImGui::Button(buttonLabel.c_str())) bp->disable();
        } else {
            buttonLabel = _("Enable##") + uniqueId;
            if (ImGui::Button(buttonLabel.c_str())) bp->enable();
        }

        ImGui::SameLine();
        const std::string& label = bp->label();
        const std::string uniqueLabel = label + "##" + uniqueId;

        // make the label edit button look like normal text until the user hovers over it
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_WindowBg));
        if (!bp->enabled()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
        }
        if (ImGui::Button(uniqueLabel.c_str())) {
            editorToOpen = fmt::format(f_("Edit label of breakpoint {}##{}"), name, counter);
            strncpy(m_bpEditPopupLabel, label.c_str(), sizeof(m_bpEditPopupLabel));
            m_bpEditPopupLabel[sizeof(m_bpEditPopupLabel) - 1] = 0;
        }
        ImGui::PopStyleColor(bp->enabled() ? 1 : 2);

        if (debugger->lastBP() != &*bp) continue;
        ImVec2 a, b, c, d, e;
        const float dist = glyphWidth / 2;
        const float w2 = ImGui::GetTextLineHeight() / 4;
        a.x = pos.x + dist;
        a.y = pos.y;
        b.x = pos.x + dist;
        b.y = pos.y + ImGui::GetTextLineHeight();
        c.x = pos.x + glyphWidth;
        c.y = pos.y + ImGui::GetTextLineHeight() / 2;
        d.x = pos.x;
        d.y = pos.y + ImGui::GetTextLineHeight() / 2 - w2;
        e.x = pos.x + dist;
        e.y = pos.y + ImGui::GetTextLineHeight() / 2 + w2;
        drawList->AddTriangleFilled(a, b, c, ImColor(s_currentColor));
        drawList->AddRectFilled(d, e, ImColor(s_currentColor));
    }
    ImGui::EndChild();
    if (toErase) g_emulator->m_debug->removeBreakpoint(toErase);
    bool addBreakpoint = ImGui::InputText(_("Address"), m_bpAddressString, 20,
                                          ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue);
    if (ImGui::BeginCombo(_("Breakpoint Type"), Debug::s_breakpoint_type_names[m_breakpointType]())) {
        for (int i = 0; i < 3; i++) {
            if (ImGui::Selectable(Debug::s_breakpoint_type_names[i](), m_breakpointType == i)) {
                m_breakpointType = i;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SliderInt(_("Breakpoint Width"), &m_breakpointWidth, 1, 4);
    addBreakpoint = addBreakpoint || ImGui::InputText(_("Label"), m_bpLabelString, sizeof(m_bpLabelString),
                                                      ImGuiInputTextFlags_EnterReturnsTrue);
    if (ImGui::Button(_("Add Breakpoint")) || addBreakpoint) {
        char* endPtr;
        uint32_t breakpointAddress = strtoul(m_bpAddressString, &endPtr, 16);
        if (*m_bpAddressString && !*endPtr) {
            debugger->addBreakpoint(breakpointAddress, Debug::BreakpointType(m_breakpointType), m_breakpointWidth,
                                    _("GUI"), m_bpLabelString);
            // we clear the label string because it seems more likely that the user would forget to clear the field
            // than that they want to use the same label twice
            m_bpLabelString[0] = 0;
        }
    }
    ImGui::End();

    if (!editorToOpen.empty()) {
        ImGui::OpenPopup(editorToOpen.c_str());
        editorToOpen = "";
    }
    counter = 0;
    for (auto bp = tree.begin(); bp != tree.end(); bp++, counter++) {
        showEditLabelPopup(&*bp, counter);
    }
}
