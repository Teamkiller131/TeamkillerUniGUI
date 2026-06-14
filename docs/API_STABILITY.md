# API Stability Policy

_This document is the public-API contract for TeamkillerUniGUI. It defines what
"stable" means, which parts of the codebase are covered, how experimental APIs
are marked, and how an API is deprecated and removed. It complements
`CHANGELOG.md` (what changed) and `RELEASE.md` (per-release notes)._

## 1. Semantic versioning

UniGUI follows [Semantic Versioning 2.0.0](https://semver.org/) for its public
API. Given a version `MAJOR.MINOR.PATCH`:

| Bump | Meaning for the public API |
|------|----------------------------|
| **MAJOR** | May contain breaking changes. Symbols deprecated in a prior MINOR may be removed here. |
| **MINOR** | Adds functionality in a backward-compatible way. Existing **stable** APIs keep compiling and behaving the same. May add deprecations. |
| **PATCH** | Backward-compatible bug fixes only. No API surface changes. |

"Backward-compatible" means source compatibility: code that compiled and worked
against the previous version of a **stable** API continues to compile and behave
the same. UniGUI does **not** currently guarantee binary (ABI) compatibility —
relink against each release.

### Checking the version at compile time

`<unigui/core/version.h>` exposes both string and numeric forms:

```cpp
#include <unigui/core/version.h>

#if UNIGUI_VERSION_AT_LEAST(3, 5, 0)
    // use an API introduced in 3.5.0
#endif

static_assert(UNIGUI_VERSION_NUMBER >= UNIGUI_MAKE_VERSION(3, 0, 0),
              "UniGUI 3.x required");
```

## 2. What the contract covers

**Covered (the public API surface):**

- Everything under `include/unigui/**` that is *not* marked experimental or
  internal, and is *not* in a `detail` namespace.

**Not covered (no stability guarantee):**

- Anything under `src/**` — these are implementation files.
- Any symbol in a `detail` namespace, or marked `UNIGUI_INTERNAL`.
- Anything marked `UNIGUI_EXPERIMENTAL` (see §3).
- Third-party headers pulled in transitively (Dear ImGui, ImPlot, etc.). Use
  their own stability guarantees, not ours.
- Build-system options, internal CMake targets, and file layout. (User-facing
  CMake options like `UNIGUI_MODULE_*` / `UNIGUI_BACKEND_*` are treated as part
  of the contract and follow the same deprecation process.)

## 3. Stability tiers

Each public symbol falls into one of three tiers. Tiers are declared at the
declaration site with the markers in `<unigui/core/api.h>` and summarised in the
table below.

| Tier | Marker | Guarantee |
|------|--------|-----------|
| **Stable** | _(default — no marker)_ | Covered by semver as in §1. |
| **Experimental** | `UNIGUI_EXPERIMENTAL` + `@experimental` doc tag | Works today, but the shape may change in a **MINOR** release without a deprecation cycle. Pin a version if you depend on it. |
| **Internal** | `UNIGUI_INTERNAL` or a `detail` namespace | Not part of the contract; may change or disappear at any time. |

`UNIGUI_EXPERIMENTAL` expands to nothing — it is a greppable, self-documenting
marker, not a compile gate. To audit every experimental surface:

```bash
grep -rn "UNIGUI_EXPERIMENTAL" include/unigui
```

### Current classification

| Area (`include/unigui/…`) | Tier | Notes |
|---------------------------|------|-------|
| `core/`, `theme/`, `theme/presets/`, `widgets/`, `im/`, `app/`, `fx/` | **Stable** | The toolkit's core surface. |
| `dsl/`, `styling/`, `events/`, `fonts/` | **Stable** | Optional modules, but API-frozen. |
| `config/`, `sqlite/`, `ipc/`, `network/`, `plugin/` | **Stable** | Optional modules gated by `UNIGUI_MODULE_*`; the headers wrap external deps but their UniGUI-facing API is stable. |
| `backend/` (platform/renderer abstraction, factory, types) | **Stable** | The abstraction is stable. |
| Backend **renderers**: Metal, WebGPU, Emscripten | **Experimental** | Non-functional stubs today (no public header); do not depend on them. See `DEVELOPMENT_PLAN.md` Horizon 2. |
| `ext/` (`node_editor.h`, `plot.h`) | **Experimental** | Thin wrappers over third-party libs; marked `UNIGUI_EXPERIMENTAL`. |

When a tier changes, the change is recorded in `CHANGELOG.md`. Promoting an
experimental API to stable is **not** a breaking change; demoting a stable API to
experimental requires a deprecation cycle (§4).

## 4. Deprecation lifecycle

We never silently break a **stable** API. Removal follows a fixed sequence:

1. **Deprecate (a MINOR release).** The symbol is annotated with
   `UNIGUI_DEPRECATED("use X instead")` so every use site emits a compiler
   warning carrying the migration hint. The replacement ships in the same
   release. A `### Deprecated` entry is added to `CHANGELOG.md`.

   ```cpp
   UNIGUI_DEPRECATED("use SetItemWidth(level, w) instead")
   void SetWidth(int level, float w);
   ```

2. **Grace period.** The deprecated symbol keeps working for **at least one
   MINOR release** so downstreams have a version where both old and new APIs are
   available and compiling.

3. **Remove (the next MAJOR release).** The symbol is deleted. The removal is
   listed under a `### Removed` / breaking-changes heading in `CHANGELOG.md` and
   called out in that release's notes in `RELEASE.md`, with the migration path.

Experimental and internal symbols may skip this lifecycle (§3).

## 5. Author checklist (when changing the public API)

- [ ] **Adding** a stable API → MINOR bump; document it in the relevant `docs/`
      file, add an example, add a GoogleTest (see `CLAUDE.md`), update README
      counts/badges.
- [ ] **Adding** an unsettled API → mark it `UNIGUI_EXPERIMENTAL` with an
      `@experimental` doc comment and list it in §3's table.
- [ ] **Changing** a stable API incompatibly → not allowed in MINOR/PATCH; go
      through the deprecation lifecycle (§4) and schedule removal for the next
      MAJOR.
- [ ] **Removing** a symbol → confirm it has been `UNIGUI_DEPRECATED` for at
      least one MINOR; MAJOR bump; record under `### Removed`.
- [ ] Update `UNIGUI_VERSION_*` in `include/unigui/core/version.h`, the
      `vcpkg.json` version, and any version strings in docs in the same change.

## 6. Reporting

If a release breaks a **stable** API without following this policy, that is a
bug — please open an issue referencing this document.
