#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// API stability annotations
//
// These macros make UniGUI's public-API contract explicit and machine-greppable.
// See docs/API_STABILITY.md for the policy they encode (semver scope, stability
// tiers, and the deprecation lifecycle).
// ─────────────────────────────────────────────────────────────────────────────

// UNIGUI_DEPRECATED("message")
//   Marks a public symbol as deprecated. Using it still compiles but emits a
//   compiler warning carrying the migration hint. Deprecated symbols are removed
//   no earlier than the next MAJOR release (see the deprecation lifecycle in
//   docs/API_STABILITY.md).
//
//   Usage:
//     UNIGUI_DEPRECATED("use NewThing() instead") void OldThing();
#if defined(__cplusplus) && __cplusplus >= 201402L
#define UNIGUI_DEPRECATED(msg) [[deprecated(msg)]]
#elif defined(__GNUC__) || defined(__clang__)
#define UNIGUI_DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
#define UNIGUI_DEPRECATED(msg) __declspec(deprecated(msg))
#else
#define UNIGUI_DEPRECATED(msg)
#endif

// UNIGUI_EXPERIMENTAL
//   Documentation marker for an experimental API: it works, but its shape may
//   change in a MINOR release without a deprecation cycle. Expands to nothing —
//   it exists so experimental surfaces are greppable and self-documenting at the
//   declaration site. Place it on the line above the declaration:
//
//     UNIGUI_EXPERIMENTAL
//     class NodeEditor { ... };
//
//   Opt-in builds can define UNIGUI_WARN_ON_EXPERIMENTAL to surface a one-time
//   note when an experimental header is included (off by default to keep clean
//   builds quiet).
#define UNIGUI_EXPERIMENTAL /* experimental: API may change without notice */

// UNIGUI_INTERNAL
//   Documentation marker for a symbol that is technically reachable from a public
//   header but is NOT part of the supported API surface. Anything in a `detail`
//   namespace is internal even without this marker.
#define UNIGUI_INTERNAL /* internal: not part of the public API contract */
