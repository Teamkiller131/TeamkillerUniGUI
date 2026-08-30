#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// form_component.h — forms & validation in the Component idiom (unigui::dsl).
//
// A FormField<T> is a validated State cell: a value, a chain of rules, a
// touched flag, and a reactive error message. A FormComponent owns fields,
// exposes whole-form validity, and gates submission: Submit() touches every
// field (so errors become visible), revalidates, and calls OnSubmit() only
// when the whole form passes.
//
//     class Signup : public dsl::FormComponent {
//         dsl::FormField<std::string> user_{this, "Username"};
//         dsl::FormField<std::string> mail_{this, "Email"};
//         dsl::FormField<bool>        terms_{this, "Accept terms"};
//     public:
//         Signup() {
//             user_.Required().MinLength(3);
//             mail_.Required().Rule([](const std::string& v) {
//                 return v.find('@') == std::string::npos ? "Not an email" : "";
//             });
//             terms_.Required("You must accept the terms");
//         }
//         void OnSubmit() override { /* all fields valid here */ }
//         dsl::NodePtr Build() override {
//             return dsl::VBox({
//                 user_.Node(), mail_.Node(), terms_.Node(),
//                 dsl::Button("Create account", [this] { Submit(); }),
//             });
//         }
//     };
//
// Fields are address-sensitive members of their FormComponent (like State);
// neither copyable nor movable. Rules run in registration order; the first
// failing rule's message is the field's Error(). Errors render only once a
// field is Touched() — Submit() touches everything, giving the standard
// "errors appear on submit" UX without per-frame plumbing.
// ─────────────────────────────────────────────────────────────────────────────

#include <unigui/dsl/component.h>
#include <unigui/im/im.h>
#include <unigui/theme/theme.h>

#include <functional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace unigui::dsl {

class FormComponent;

/// Non-template face of FormField so a FormComponent can drive any field:
/// validity checks, touch-all-on-submit, and inspector-style enumeration.
class FormFieldBase {
public:
    virtual ~FormFieldBase() = default;
    /// Do the rules pass on the current value?
    virtual bool Valid() const = 0;
    /// Mark touched (errors become visible) and revalidate.
    virtual void Touch() = 0;
    /// First failing rule's message; empty when valid.
    virtual const std::string& Error() const = 0;
    virtual const std::string& Label() const = 0;
    virtual bool Touched() const = 0;
};

/// Component base with form plumbing. Fields self-register; Submit() is the
/// validation gate. Rebuilds follow the normal Component dirty tracking (field
/// writes mark the owner dirty via their inner State).
class FormComponent : public Component {
public:
    /// All registered fields currently valid?
    bool FormValid() const {
        for (const auto* f : fields_)
            if (!f->Valid())
                return false;
        return true;
    }

    /// Touch every field (reveal errors), revalidate, and if the whole form is
    /// valid call OnSubmit() and return true; otherwise OnSubmitRejected().
    bool Submit() {
        for (auto* f : fields_)
            f->Touch();
        MarkDirty(); // error visibility may have changed even if no value did
        if (FormValid()) {
            OnSubmit();
            return true;
        }
        OnSubmitRejected();
        return false;
    }

    /// Called by Submit() when every field passed validation.
    virtual void OnSubmit() {}
    /// Called by Submit() when validation failed (errors are now visible).
    virtual void OnSubmitRejected() {}

    /// Registered fields, in declaration order.
    const std::vector<FormFieldBase*>& Fields() const { return fields_; }

    const char* InspectorName() const override { return "FormComponent"; }

private:
    template <typename T> friend class FormField;
    void RegisterField(FormFieldBase* f) { fields_.push_back(f); }
    void DeregisterField(FormFieldBase* f) {
        fields_.erase(std::remove(fields_.begin(), fields_.end(), f), fields_.end());
    }
    std::vector<FormFieldBase*> fields_;
};

/// A validated form field: State<T> + rule chain + touched flag + error.
/// Declare as a member of a FormComponent with `{this, "Label"}`.
template <typename T> class FormField final : public FormFieldBase {
public:
    FormField(FormComponent* owner, std::string label, T initial = T{})
            : value_(owner, std::move(initial))
            , label_(std::move(label))
            , owner_(owner) {
        if (owner_)
            owner_->RegisterField(this);
    }
    ~FormField() override {
        if (owner_)
            owner_->DeregisterField(this);
    }
    FormField(const FormField&) = delete;
    FormField& operator=(const FormField&) = delete;

    // ── Rules (chainable; evaluated in order, first failure wins) ───────────

    /// Custom rule: return "" when the value is OK, else the error message.
    FormField& Rule(std::function<std::string(const T&)> rule) {
        rules_.push_back(std::move(rule));
        Revalidate();
        return *this;
    }

    /// Required: non-empty for strings, true for bools, always-pass otherwise
    /// (numeric fields are "present" by construction — use Range()).
    FormField& Required(std::string msg = "This field is required") {
        return Rule([msg = std::move(msg)](const T& v) -> std::string {
            if constexpr (std::is_same_v<T, std::string>)
                return v.empty() ? msg : "";
            else if constexpr (std::is_same_v<T, bool>)
                return v ? "" : msg;
            else {
                (void) v;
                return "";
            }
        });
    }

    /// Inclusive numeric range.
    template <typename U = T>
        requires std::is_arithmetic_v<U>
    FormField& Range(U lo, U hi, std::string msg = "") {
        if (msg.empty())
            msg = "Must be between " + std::to_string(lo) + " and " + std::to_string(hi);
        return Rule([lo, hi, msg = std::move(msg)](const T& v) -> std::string {
            return (v < lo || v > hi) ? msg : "";
        });
    }

    /// Minimum string length.
    template <typename U = T>
        requires std::is_same_v<U, std::string>
    FormField& MinLength(std::size_t n, std::string msg = "") {
        if (msg.empty())
            msg = "Must be at least " + std::to_string(n) + " characters";
        return Rule([n, msg = std::move(msg)](const std::string& v) -> std::string {
            return v.size() < n ? msg : "";
        });
    }

    // ── Value (mirrors State) ────────────────────────────────────────────────

    const T& operator()() const { return value_.Get(); }
    const T& Get() const { return value_.Get(); }

    /// Set the value: marks the field touched and revalidates. The inner State
    /// marks the owning component dirty on a real change.
    void Set(T v) {
        value_.Set(std::move(v));
        touched_ = true;
        Revalidate();
    }
    FormField& operator=(T v) {
        Set(std::move(v));
        return *this;
    }

    // ── Validation state ─────────────────────────────────────────────────────

    bool Valid() const override { return error_.empty(); }
    bool Touched() const override { return touched_; }
    const std::string& Error() const override { return error_; }
    const std::string& Label() const override { return label_; }

    void Touch() override {
        touched_ = true;
        Revalidate();
    }

    // ── View ─────────────────────────────────────────────────────────────────

    /// Ready-made DSL row: a bound input for the field plus its error message
    /// (danger-coloured, shown only once the field is touched). Available for
    /// std::string (InputText) and bool (CheckBox); compose other types
    /// manually from the field's value + Error().
    NodePtr Node() {
        static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, bool>,
                      "FormField<T>::Node() is provided for std::string and bool; "
                      "compose other field types manually (see form_component.h)");
        NodePtr input;
        if constexpr (std::is_same_v<T, std::string>) {
            input = InputText(label_, [this](const std::string& v) { Set(v); });
        } else if constexpr (std::is_same_v<T, bool>) {
            input = CheckBox(label_, [this](bool v) { Set(v); });
        }
        NodePtr error = Custom([this] {
            if (touched_ && !error_.empty())
                im::TextColored(GetColorTokens().danger, error_);
        });
        return VBox({std::move(input), std::move(error)});
    }

private:
    void Revalidate() {
        const T& v = value_.Get();
        for (const auto& r : rules_) {
            std::string e = r ? r(v) : std::string();
            if (!e.empty()) {
                error_ = std::move(e);
                return;
            }
        }
        error_.clear();
    }

    State<T> value_;
    std::string label_;
    FormComponent* owner_;
    std::vector<std::function<std::string(const T&)>> rules_;
    std::string error_;
    bool touched_ = false;
};

} // namespace unigui::dsl
