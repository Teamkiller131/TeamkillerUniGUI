#pragma once
#include <imgui.h>

#include <functional>
#include <string>
namespace unigui {
template <typename T> inline bool BeginDragSource(const char* type, const T& data) {
    if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload(type, &data, sizeof(T));
        ImGui::TextUnformatted(type);
        ImGui::EndDragDropSource();
        return true;
    }
    return false;
}
template <typename T> inline const T* AcceptDragDrop(const char* type) {
    if (ImGui::BeginDragDropTarget()) {
        auto* payload = ImGui::AcceptDragDropPayload(type);
        ImGui::EndDragDropTarget();
        if (payload)
            return static_cast<const T*>(payload->Data);
    }
    return nullptr;
}
} // namespace unigui
