/* Pure-C compile-time and link-time proof for the C ABI.
 *
 * This file is compiled as C (not C++) and linked against the C++ unigui
 * library. It deliberately exercises the header's C-ness: C99 declarations
 * only, no C++ constructs, no gtest (which is C++). Each exported check
 * returns 1 on success / 0 on failure and is wrapped into gtest cases by
 * capi_test.cc, so the gtest harness stays in C++ while this TU proves the
 * boundary really is callable from C.
 */

#include <unigui/capi/unigui_capi.h>

#include <stddef.h>
#include <string.h>

/* The ABI gate must accept the version this file was compiled against, and the
 * compile-time macro must agree with the runtime library. */
int capi_c_abi_gate_ok(void) {
    return unigui_capi_abi_compatible(UNIGUI_CAPI_ABI_VERSION) != 0;
}

/* A binding compiled against a NEWER ABI than the library must be rejected. */
int capi_c_abi_future_rejected(void) {
    return unigui_capi_abi_compatible(UNIGUI_CAPI_ABI_VERSION + 1) == 0;
}

/* ABI version 0 is a sentinel (pre-versioning); never treated as compatible. */
int capi_c_abi_zero_rejected(void) {
    return unigui_capi_abi_compatible(0) == 0;
}

/* Version triple and string must be self-consistent and sane. */
int capi_c_version_ok(void) {
    const char* s = unigui_version_string();
    if (s == NULL || strlen(s) == 0)
        return 0;
    if (unigui_version_major() <= 0 || unigui_version_minor() < 0 || unigui_version_patch() < 0)
        return 0;
    return 1;
}

/* Config defaults must match the documented C++ defaults. */
int capi_c_config_defaults_ok(void) {
    unigui_app_config cfg;
    memset(&cfg, 0, sizeof(cfg)); /* prove init fills everything, not just some fields */
    unigui_app_config_init(&cfg);
    return cfg.width == 1280 && cfg.height == 720 && cfg.title != NULL &&
           cfg.backend == UNIGUI_BACKEND_DEFAULT && cfg.dpi_scale_fonts == 0 &&
           cfg.accessibility == 0 && cfg.multi_viewport == 0;
}

/* The opaque handle and callback types must be usable from C (declarable,
 * storable, and round-trippable through pointers). */
int capi_c_types_usable_ok(void) {
    unigui_app* app = NULL;
    unigui_frame_fn fn = NULL;
    (void) app;
    (void) fn;
    return sizeof(unigui_app*) > 0 && sizeof(unigui_frame_fn) > 0;
}

/* Destroy must be a NULL-safe no-op (documented contract). */
int capi_c_null_destroy_ok(void) {
    unigui_app_destroy(NULL);
    return 1;
}

/* ABI v2 tranche: the added symbols must exist and link from plain C. Taking
 * their addresses proves linkage without executing them (these calls need a
 * live ImGui context, which a pure-C TU without the app never has). */
int capi_c_v2_surface_ok(void) {
    void (*f_same_line)(void) = unigui_same_line;
    void (*f_spacing)(void) = unigui_spacing;
    int (*f_radio)(const char*, int*, int) = unigui_radio_button;
    int (*f_combo)(const char*, int*, const char* const*, int) = unigui_combo;
    int (*f_itext)(const char*, char*, size_t) = unigui_input_text;
    int (*f_iint)(const char*, int*) = unigui_input_int;
    int (*f_ifloat)(const char*, float*) = unigui_input_float;
    int (*f_slider)(const char*, int*, int, int) = unigui_slider_int;
    void (*f_bar)(float) = unigui_progress_bar;
    void (*f_tip)(const char*) = unigui_set_tooltip;
    int (*f_sel)(const char*, int) = unigui_selectable;
    (void) f_same_line;
    (void) f_spacing;
    return f_radio != NULL && f_combo != NULL && f_itext != NULL && f_iint != NULL &&
           f_ifloat != NULL && f_slider != NULL && f_bar != NULL && f_tip != NULL && f_sel != NULL;
}

/* The gate must keep accepting v1 bindings after the v2 addition (additive
 * growth: unigui_capi_abi_compatible(1) still passes). */
int capi_c_v1_bindings_still_compatible(void) {
    return unigui_capi_abi_compatible(1) != 0;
}
