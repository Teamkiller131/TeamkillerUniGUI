# C API (`unigui_capi.h`)

A stable **C ABI** over the UniGUI C++ library, for hosts that cannot (or
prefer not to) consume C++ directly: C applications, C#/Python/Go FFI
bindings, embedding layers, and ABI-stability-sensitive integrations.

- **Header:** `#include <unigui/capi/unigui_capi.h>` (plain C99; usable from
  C and C++)
- **Implementation:** compiled into `libunigui` — no extra library to link
- **First ABI version:** 1 (4.9.0+)

The C surface is deliberately a **subset** of the C++ API and grows
additively as demand appears. The full C++ API remains the first-class
surface.

## ABI stability rules

- `UNIGUI_CAPI_ABI_VERSION` is the ABI contract revision. It is bumped **only
  by additive change**: new functions, new trailing struct fields, new enum
  values. Existing function signatures, struct layouts, and enum values are
  frozen within an ABI version.
- A binding compiled against ABI version *N* works against any library whose
  reported ABI version is `>= N`. Check at startup:

  ```c
  if (!unigui_capi_abi_compatible(UNIGUI_CAPI_ABI_VERSION)) {
      fprintf(stderr, "unigui C ABI mismatch (want %d)\n", UNIGUI_CAPI_ABI_VERSION);
      return 1;
  }
  ```

- The ABI version is independent of the library version: a 4.9.1 patch
  release does not change the ABI number unless the C surface changed.
- Every function and type is plain C99 — no `bool` in signatures, no C++
  types, no exceptions cross the boundary. Strings are UTF-8 `const char*`,
  owned by the caller unless documented otherwise. Boolean results are `int`
  (0 = false, non-zero = true).
- All calls must come from the thread that created the app; none of the
  functions are thread-safe by themselves.
- An app handle owns one application. The C++ app layer is process-global,
  so at most one live app exists at a time.

## Quick start (C)

```c
#include <unigui/capi/unigui_capi.h>

static void draw(void* userdata) {
    int* clicks = (int*)userdata;
    if (unigui_begin("Hello", NULL)) {
        unigui_text("C says hello — frame %d", *clicks);
        if (unigui_button("Click me"))
            ++*clicks;
    }
    unigui_end();
}

int main(void) {
    if (!unigui_capi_abi_compatible(UNIGUI_CAPI_ABI_VERSION))
        return 1;

    unigui_app_config cfg;
    unigui_app_config_init(&cfg);
    cfg.title = "C host";

    unigui_app* app = unigui_app_create(&cfg);
    if (app == NULL)
        return 1;

    int clicks = 0;
    /* Run until the window closes, or cap frames for a smoke run: */
    int rc = unigui_app_run(app, draw, &clicks, 0);

    unigui_app_destroy(app);
    return rc;
}
```

A manual loop is the same contract split into pieces:

```c
while (!unigui_app_should_close(app)) {
    if (!unigui_app_new_frame(app)) break;
    draw(&clicks);
    unigui_app_render(app);
}
```

## Reference

### Version & ABI gate

| Function | Meaning |
|----------|---------|
| `unigui_version_major/minor/patch()` | Library version components |
| `unigui_version_string()` | Static string, e.g. `"4.9.0"` |
| `unigui_capi_abi_compatible(int)` | 1 when the library can run a binding compiled against that ABI version |

### App lifecycle

| Function | Meaning |
|----------|---------|
| `unigui_app_config_init(unigui_app_config*)` | Fill the documented defaults |
| `unigui_app_create(const unigui_app_config*)` | Create window + backend + context; `NULL` on failure (already logged) |
| `unigui_app_destroy(unigui_app*)` | Shut down (if still running) and free; `NULL` is a no-op |
| `unigui_app_new_frame(unigui_app*)` | Poll events + start the backend frame + `NewFrame()`; 0 when not initialised |
| `unigui_app_render(unigui_app*)` | `Render()` + present |
| `unigui_app_should_close(const unigui_app*)` | 1 when the window is closing (also 1 for a dead handle) |
| `unigui_app_run(app, frame, userdata, max_frames)` | Full loop; `frame` runs once per frame; `max_frames = 0` runs until close; shuts down when the loop ends |
| `unigui_app_native_window_handle(const unigui_app*)` | `HWND` on Windows, `GLFWwindow*` elsewhere; `NULL` before init / after shutdown |
| `unigui_app_set_content_scale(app, float)` / `unigui_app_get_content_scale` | HiDPI content scale (1.0 = 100%, 1.5 = 150%, …) |

`unigui_app_config` fields mirror `unigui::AppConfig`: `width`, `height`,
`title` (must remain valid for the app's lifetime), `backend`
(`UNIGUI_BACKEND_DEFAULT` keeps the platform default — DX11 on Windows,
GLFW+GL3 elsewhere), `dpi_scale_fonts`, `accessibility`, `multi_viewport`.

### Immediate-mode drawing subset (ABI v1)

The first tranche of the `unigui::im` layer — enough for a "hello world"
host. All of these require a live app (or at least a current ImGui context)
and run between `unigui_app_new_frame()`/`unigui_app_render()` or inside the
frame callback.

| Function | Notes |
|----------|-------|
| `unigui_begin(title, int* p_open)` | Returns 1 while visible; still call `unigui_end()` when it returns 0. `*p_open` is a close hook (title-bar X writes 0 — stop drawing the window on subsequent frames), not a visibility switch |
| `unigui_end()` | Pairs every `unigui_begin` |
| `unigui_text_unformatted(text)` / `unigui_text(fmt, ...)` | UTF-8, printf-style |
| `unigui_button(label)` | 1 on the clicked frame |
| `unigui_checkbox(label, int* value)` | Toggles in place; 1 on the changed frame |
| `unigui_slider_float(label, float* value, min, max)` | Writes `*value`; 1 on the changed frame |
| `unigui_separator()` | Horizontal rule |

## Growth policy

New surface lands behind a **C ABI version bump** (additive-only; see the
stability rules above) and in the same order the C++ API grew it: version
gate → app lifecycle → a drawing subset (this release) → more `im` calls,
then retained widgets / localization / events as binding demand appears.
Candidate next tranches: text input (`unigui_input_text` with a caller-owned
buffer), locale lookup (`unigui_tr`), more basic controls (radio, combo,
selectable), and window flags.

## Bindings

Language bindings (C#, Python, Go, …) are expected to be built over this
surface, not over the C++ headers. A C# P/Invoke or Python ctypes mapping is
mechanical: every function here is a plain symbol with C types. No binding
ships in-tree yet.
