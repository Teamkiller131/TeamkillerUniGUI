#pragma once

#include <unigui/dsl/dsl.h>

#include <string>
#include <string_view>

namespace unigui::dsl {

/// Result of parsing a scene description (see docs/DSL.md, "Scene text format").
struct SceneParseResult {
    /// The built tree; null when parsing failed.
    NodePtr tree;
    /// Human-readable error, "<line>: <message>"; empty on success.
    std::string error;
};

/// Parse an indentation-based scene description into a DSL tree — the designer
/// tool's in-app scene-editing format. Lines indent with spaces (any consistent
/// step; each line becomes a child of the nearest preceding line with a
/// smaller indent), `#` starts a comment, and the root must be a `window`.
///
///     window "Settings"
///       vbox
///         text "Welcome"
///         separator
///         hbox
///           checkbox "Wireframe"
///           button "Save" primary
///         slider_float "Gain" 0 1
///         for 3
///           label "item"
///
/// Keywords: window "title" | vbox | hbox | flex | label/text/text_wrapped/
/// text_disabled/bullet_text "text" | button "label" [default|primary|danger|
/// success|warning] | checkbox "label" | slider_float "label" min max |
/// input_text "label" | separator | spacing | for count. Callbacks cannot be
/// expressed in text: `if`/`custom` are rejected with a clear error, and a
/// `for` clones its child template once per iteration (stateful template
/// children get fresh copies).
///
/// Never throws; malformed input yields a null tree plus a line-numbered error.
SceneParseResult ParseScene(std::string_view text);

} // namespace unigui::dsl
