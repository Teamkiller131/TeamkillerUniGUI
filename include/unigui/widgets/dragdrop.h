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
        const T* result = nullptr;
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(type)) {
            // Copy the value out *before* EndDragDropTarget(): on the delivery
            // frame EndDragDropTarget() calls ClearDragDrop(), which frees/zeros
            // ImGui's payload buffer that payload->Data points into. Returning a
            // pointer into that buffer (as the old code did) yielded a dangling /
            // nulled read, so the drop never actually delivered its value.
            if (payload->Data != nullptr && payload->DataSize == static_cast<int>(sizeof(T))) {
                static thread_local T storage;
                storage = *static_cast<const T*>(payload->Data);
                result = &storage;
            }
        }
        ImGui::EndDragDropTarget();
        return result;
    }
    return nullptr;
}
} // namespace unigui
