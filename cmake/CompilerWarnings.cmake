# ─────────────────────────────────────────────────────────────────────────────
# CompilerWarnings.cmake — centralised warning configuration for UniGUI targets.
#
# unigui_set_warnings(<target>) applies a sensible base warning level to one of
# UniGUI's *own* targets. The flags are PRIVATE, so they never leak onto
# downstream consumers of the library — embedders keep their own warning policy.
#
# Base warnings are always on so problems are visible during development. Turning
# them into hard errors is opt-in via the UNIGUI_WARNINGS_AS_ERRORS option (off
# by default) so a transient warning — or a noisy compiler/STL upgrade — can't
# break the build for everyone. Enable it in CI / strict presets once a target
# tree is known to be warning-clean.
# ─────────────────────────────────────────────────────────────────────────────

function(unigui_set_warnings target)
    if(MSVC)
        set(_warnings /W4)
        set(_werror /WX)
        # MSVC's C4996 deprecates portable Standard C functions (fopen, sscanf,
        # _wfopen, …) in favour of its non-portable *_s variants. We use the
        # Standard forms deliberately for cross-platform parity, so silence the
        # nag for our own TUs (PRIVATE — never leaks to downstream consumers).
        # Public headers avoid the deprecated calls outright so consumer builds
        # stay clean without relying on this define.
        target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
    else()
        # GCC and Clang share this common set.
        #
        # -Wno-missing-field-initializers: -Wextra enables this, but it conflicts
        # with our clang-tidy policy (readability-redundant-member-init flags the
        # explicit `{}` defaults it would have us add). We use idiomatic partial
        # aggregate initialisation freely (trailing members fall back to their
        # type's default), so suppress the warning rather than fight the linters.
        set(_warnings -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor
                      -Wno-missing-field-initializers)
        set(_werror -Werror)
    endif()

    target_compile_options(${target} PRIVATE ${_warnings})

    if(UNIGUI_WARNINGS_AS_ERRORS)
        target_compile_options(${target} PRIVATE ${_werror})
    endif()
endfunction()
