#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// ContextRegistry — per-ImGui-context instance holder for the ::Instance()
// singletons (the first step of the multi-context work).
//
// A singleton migrated to this template keeps its public shape (`T::Instance()`)
// but resolves through the CURRENT ImGui context: two independent UniGUI surfaces
// in one process get independent instances, and parallel tests stop sharing
// global state. With no context current (pre-Init), the nullptr key serves the
// same default instance the old function-local static provided.
//
// Lifetime: instances live until Reset(ctx) / ResetAll(). The map is LRU-capped
// at kMaxEntries so test suites that churn thousands of ImGui contexts cannot
// grow it unboundedly (real apps use 1–2 contexts and never notice the cap);
// the app loop calls Reset for its context on Shutdown.
// ─────────────────────────────────────────────────────────────────────────────

#include <imgui.h>

#include <cstddef>
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>

namespace unigui::detail {

template <typename T>
class ContextRegistry {
public:
    static constexpr std::size_t kMaxEntries = 16;

    static T& Instance() {
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        std::lock_guard<std::mutex> lock(Mutex());
        auto& map = Map();
        auto it = map.find(ctx);
        if (it == map.end()) {
            std::unique_ptr<T> inst = Make();
            it = map.emplace(ctx, std::move(inst)).first;
            Lru().push_front(ctx);
            EvictLocked();
        } else {
            // Touch LRU position (cheap; entries rarely exceed 2 in practice).
            Lru().remove(ctx);
            Lru().push_front(ctx);
        }
        return *it->second;
    }

    /// Drop the instance bound to @p ctx (the app loop calls this on Shutdown,
    /// before DestroyContext). nullptr drops the no-context default.
    static void Reset(ImGuiContext* ctx) {
        std::lock_guard<std::mutex> lock(Mutex());
        Map().erase(ctx);
        Lru().remove(ctx);
    }

    /// Drop every instance (process teardown / test fixtures that want a clean slate).
    static void ResetAll() {
        std::lock_guard<std::mutex> lock(Mutex());
        Map().clear();
        Lru().clear();
    }

    /// Number of live instances (tests/diagnostics).
    static std::size_t Count() {
        std::lock_guard<std::mutex> lock(Mutex());
        return Map().size();
    }

    /// Overridable factory — specialise for singletons with a non-default
    /// constructor (e.g. Toast, whose name matters for its ImGui ID).
    static std::unique_ptr<T> Make() { return std::make_unique<T>(); }

private:
    using MapType = std::unordered_map<ImGuiContext*, std::unique_ptr<T>>;

    static MapType& Map() {
        static MapType map;
        return map;
    }
    static std::list<ImGuiContext*>& Lru() {
        static std::list<ImGuiContext*> lru;
        return lru;
    }
    static std::mutex& Mutex() {
        static std::mutex m;
        return m;
    }

    // Evict the least-recently-used entries above the cap (never the current one).
    static void EvictLocked() {
        MapType& map = Map();
        std::list<ImGuiContext*>& lru = Lru();
        while (map.size() > kMaxEntries && !lru.empty()) {
            ImGuiContext* victim = lru.back();
            lru.pop_back();
            if (victim == ImGui::GetCurrentContext())
                continue; // never evict the live instance
            map.erase(victim);
        }
    }
};

} // namespace unigui::detail
