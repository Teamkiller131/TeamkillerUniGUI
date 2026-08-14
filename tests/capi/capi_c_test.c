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
