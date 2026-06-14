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
    else()
        # GCC and Clang share this common set.
        set(_warnings -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor)
        set(_werror -Werror)
    endif()

    target_compile_options(${target} PRIVATE ${_warnings})

    if(UNIGUI_WARNINGS_AS_ERRORS)
        target_compile_options(${target} PRIVATE ${_werror})
    endif()
endfunction()
