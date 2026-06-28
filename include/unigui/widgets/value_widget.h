#pragma once
#include <unigui/core/observable.h>
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <utility>

namespace unigui {

template <typename T> class ValueWidget : public Widget {
public:
    using OnChange = std::function<void(T)>;

    ValueWidget(std::string name, T initial = T{})
            : Widget(std::move(name))
            , value_(std::move(initial)) {}

    // Address-sensitivity note: BindValue() registers a `this`-capturing observer
    // inside the source observable, so a *bound* widget must not be relocated
    // (bind only after the widget reaches its final home). Unbound widgets stay
    // movable — copy is ill-formed via the move-only Subscription member — so they
    // can still live in containers.

    T GetValue() const { return value_; }
    void SetValue(T val) { value_ = std::move(val); }

    void SetOnChange(OnChange fn) { onChange_ = std::move(fn); }

    /// Bind this widget's value to an Observable. On bind the widget adopts the
    /// observable's current value; afterwards observable changes flow into the
    /// widget. When `twoWay` (the default), a user edit also writes the new value
    /// back to the observable. The widget owns the subscription, so the binding
    /// auto-detaches when the widget is destroyed (no dangling). The widget must
    /// not be moved or copied after binding (the binding captures `this`).
    /// Calling BindValue again rebinds to the new source.
    ///
    /// The bound *source* `Observable` must likewise not be moved while the binding is
    /// live: a two-way binding writes back through the source's address, and the
    /// liveness token (`Lifetime()`) tracks destruction, not moves — a moved-from
    /// source would leave the write-back targeting an empty husk.
    void BindValue(Observable<T>& src, bool twoWay = true) {
        bound_ = twoWay ? &src : nullptr;
        boundLife_ = src.Lifetime();
        binding_ = src.SubscribeAndFire([this](const T& v) { ApplyBoundValue(v); });
    }

    /// Detach any active binding.
    void Unbind() {
        binding_.Reset();
        bound_ = nullptr;
        boundLife_.reset();
    }

protected:
    T value_;
    OnChange onChange_;

    /// Apply a value pushed from a bound Observable. Defaults to setting the
    /// internal value; widgets that cache derived state (e.g. an input buffer)
    /// override this to keep that state in sync.
    virtual void ApplyBoundValue(T v) { value_ = std::move(v); }

    /// Call this in Render when the value changed (pass the pre-edit value): it
    /// fires the OnChange callback and pushes the new value to any two-way
    /// binding. The push is change-detecting (Observable::Set), so it cannot feed
    /// back into a loop.
    void NotifyChange(T oldVal) {
        if (value_ == oldVal)
            return;
        if (onChange_)
            onChange_(value_);
        // Push back to a two-way binding, but only if the source is still alive —
        // boundLife_ expires when the observable is destroyed, so a source that
        // outlived its widget binding can't be dereferenced through a stale bound_.
        if (bound_ && !boundLife_.expired())
            bound_->Set(value_);
    }

private:
    Observable<T>* bound_ = nullptr;      // non-owning; valid only while a binding is live
    std::weak_ptr<const void> boundLife_; // expires when *bound_ is destroyed
    Subscription binding_;                // declared last → torn down first (stops callbacks)
};

} // namespace unigui
