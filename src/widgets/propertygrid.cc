#include <unigui/widgets/propertygrid.h>
#include <imgui.h>
#include <cstdio>

namespace unigui {

PropertyGrid::PropertyGrid(std::string name) : Widget(std::move(name)) {}

void PropertyGrid::AddProperty(PropertyDef prop) { props_.push_back(std::move(prop)); }
void PropertyGrid::Clear() { props_.clear(); }

void PropertyGrid::RenderProp(PropertyDef& prop) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(prop.label.c_str());
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-1);

    bool changed = false;
    switch (prop.type) {
    case PropType::Bool: {
        bool v = std::get<bool>(prop.value);
        if (ImGui::Checkbox(("##" + prop.name).c_str(), &v)) { prop.value = v; changed = true; }
        break;
    }
    case PropType::Int: {
        int v = std::get<int>(prop.value);
        if (ImGui::SliderInt(("##" + prop.name).c_str(), &v, (int)prop.minVal, (int)prop.maxVal)) { prop.value = v; changed = true; }
        break;
    }
    case PropType::Float: {
        float v = std::get<float>(prop.value);
        if (ImGui::SliderFloat(("##" + prop.name).c_str(), &v, prop.minVal, prop.maxVal, "%.2f")) { prop.value = v; changed = true; }
        break;
    }
    case PropType::String: {
        char buf[256];
        snprintf(buf, sizeof(buf), "%s", std::get<std::string>(prop.value).c_str());
        if (ImGui::InputText(("##" + prop.name).c_str(), buf, sizeof(buf))) { prop.value = std::string(buf); changed = true; }
        break;
    }
    case PropType::Combo: {
        int idx = std::get<int>(prop.value);
        std::string preview = (idx >= 0 && idx < (int)prop.options.size()) ? prop.options[idx] : "";
        if (ImGui::BeginCombo(("##" + prop.name).c_str(), preview.c_str())) {
            for (int i = 0; i < (int)prop.options.size(); i++) {
                if (ImGui::Selectable(prop.options[i].c_str(), i == idx)) { prop.value = i; changed = true; }
            }
            ImGui::EndCombo();
        }
        break;
    }
    default: break;
    }

    if (changed && onChange_) onChange_(prop.name, prop.value);
}

void PropertyGrid::SetValue(const std::string& name, PropValue val) {
    for (auto& p : props_) { if (p.name == name) { p.value = std::move(val); return; } }
}

void PropertyGrid::Render() {
    if (!IsVisible()) return;
    if (ImGui::BeginTable(GetName().c_str(), 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Property");
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        for (auto& p : props_) RenderProp(p);
        ImGui::EndTable();
    }
}

} // namespace unigui
