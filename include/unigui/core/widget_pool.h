#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

namespace unigui {

/// WidgetPool — a keyed cache of retained widget instances for per-row / per-item
/// stateful widgets inside data-driven views (tables, trees, lists).
///
/// Data-driven UIs that host stateful widgets (a `ComboBox`/`InputInt`/`Button`
/// bound to a row) otherwise cache each one in a hand-rolled
/// `static std::map<int, Widget>` keyed by *visible row index* — which is
/// fragile (index reuse after sort/delete) and leaks on row removal. WidgetPool
/// fixes both: key by a **stable identity** (a row id), and evict entries not
/// touched in a frame.
///
/// Usage per frame:
/// ```cpp
/// pool.BeginFrame();
/// for (auto& row : rows) {
///     ComboBox& cb = pool.GetOrCreate(row.id, [&] {
///         return std::make_unique<ComboBox>("##mode" + std::to_string(row.id), "", items);
///     });
///     cb.Render();
/// }
/// pool.EndFrame();   // drops widgets for rows that disappeared
/// ```
///
/// Header-only and ImGui-free, so the lifetime/eviction logic is unit-testable
/// without a frame.
template <class W> class WidgetPool {
public:
    /// Begin a frame: marks the current generation. Call before the GetOrCreate
    /// loop so EndFrame can tell which entries were touched this frame.
    void BeginFrame() { ++gen_; }

    /// Return the widget for `key`, constructing it via `make` (which returns a
    /// `std::unique_ptr<W>`) the first time the key is seen. The returned
    /// reference is stable until the entry is evicted.
    template <class Factory> W& GetOrCreate(std::uint64_t key, Factory&& make) {
        auto it = map_.find(key);
        if (it == map_.end()) {
            Entry e;
            e.w = make();
            e.gen = gen_;
            it = map_.emplace(key, std::move(e)).first;
        } else {
            it->second.gen = gen_;
        }
        return *it->second.w;
    }

    /// Evict entries not requested since the last BeginFrame (i.e. rows that were
    /// removed/filtered out). Call after the GetOrCreate loop.
    void EndFrame() {
        for (auto it = map_.begin(); it != map_.end();) {
            if (it->second.gen != gen_)
                it = map_.erase(it);
            else
                ++it;
        }
    }

    /// True if a widget is currently cached for `key`.
    bool Contains(std::uint64_t key) const { return map_.find(key) != map_.end(); }
    std::size_t Size() const { return map_.size(); }
    void Clear() { map_.clear(); }

private:
    struct Entry {
        std::unique_ptr<W> w;
        std::uint64_t gen = 0;
    };
    std::unordered_map<std::uint64_t, Entry> map_;
    std::uint64_t gen_ = 0;
};

} // namespace unigui
