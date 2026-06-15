#pragma once

#include <unigui/theme/color_tokens.h>

#include <imgui.h>

#include <string>
#include <string_view>

namespace unigui {

/// Sign- and status-colored value text — the most-repeated trading-UI idiom
/// (`TextColored(v >= 0 ? up : down, fmt(v))`) centralised so call sites stop
/// re-declaring `ImVec4` literals. Colours come from the active theme semantic
/// tokens and honour the active up/down `Polarity` (CN red-up by default), so a
/// rise renders red in a CN client and green in a Western one with no call-site
/// change. Free functions (immediate-mode), not a retained widget.
///
/// The sign/threshold → role mapping is exposed as a pure function so it is
/// unit-testable without an ImGui frame.

/// Pure: classify a value into an Up/Down/Flat-style semantic role. `Flat`
/// returns false through `isDirectional` (caller uses the neutral text colour).
struct DirectionRole {
    theme::Semantic role = theme::Semantic::Up;
    bool isDirectional = false; ///< false for a flat value (use neutral colour)
};
inline DirectionRole PnlRole(double value, double eps = 0.0) {
    if (value > eps)
        return {theme::Semantic::Up, true};
    if (value < -eps)
        return {theme::Semantic::Down, true};
    return {theme::Semantic::Up, false};
}

/// Pure: two-threshold "traffic-light" role. value < warn → Success,
/// < crit → Warning, else Danger. Set `inverted` when lower is worse.
inline theme::Semantic GradedRole(double value, double warn, double crit, bool inverted = false) {
    const bool a = inverted ? value > warn : value < warn;
    const bool b = inverted ? value > crit : value < crit;
    if (a)
        return theme::Semantic::Success;
    if (b)
        return theme::Semantic::Warning;
    return theme::Semantic::Danger;
}

/// Render `display`, coloured by the sign of `value` (Up/Down via active
/// polarity; flat values use the normal text colour).
void PnlText(double value, std::string_view display, double eps = 0.0);

/// Render `value` formatted with an explicit leading sign (e.g. "+1.50"),
/// coloured by its sign. `decimals` controls precision.
void PnlText(double value, int decimals = 2, double eps = 0.0);

/// Binary status text: `on` → Success(onLabel), else Danger(offLabel).
void StatusText(bool on, std::string_view onLabel, std::string_view offLabel);

/// Two-threshold traffic-light text: colours `display` Success/Warning/Danger
/// via GradedRole(value, warn, crit, inverted).
void GradedText(double value, double warn, double crit, std::string_view display,
                bool inverted = false);

} // namespace unigui
