#pragma once

struct ImGuiContext;

namespace unigui {

/// Creates the ImGui context. Returns false if already initialized.
/// Safe to call multiple times — returns true on first call only.
bool CreateContext();

/// Destroys the ImGui context. Safe to call even if not initialized.
void DestroyContext();

/// Returns the raw ImGuiContext pointer, or nullptr if not initialized.
ImGuiContext* GetContext();

} // namespace unigui
