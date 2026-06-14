#!/usr/bin/env python3
"""coverage_vs_imgui.py — report how much of Dear ImGui's public API has a
first-class ``unigui::im`` wrapper.

This backs the "Wrapper coverage" success metric in ``DEVELOPMENT_PLAN.md``:
the first-class-wrapped % of the *practical* ImGui surface should trend up,
never down. It parses the function names declared inside ``namespace ImGui`` in
``imgui.h`` and the free functions declared in ``include/unigui/im/im.h``, then
compares them by name.

Not every ImGui function is meant to get an ``im`` wrapper. The immediate-mode
layer deliberately skips:

  * context / IO / backend / allocator / font-atlas / ini plumbing,
  * docking, viewport and multi-platform window management,
  * the ``printf``-style ``*V`` (``va_list``) overloads,
  * the generic ``*Scalar``/``*ScalarN`` forms (we wrap the typed variants),
  * the style / ID / font *stacks* (covered by the RAII scopes in
    ``core/scope.h`` + the theme engine), and
  * functions that already have a richer *retained-mode* widget equivalent
    (windows, tables, images, the high-level ``ListBox``/``Combo``, …).

Everything else is "practical surface" and counts toward the coverage number.

Usage::

    python3 scripts/coverage_vs_imgui.py [--imgui PATH] [--threshold PCT]
                                         [--list] [--json]

Exit code is non-zero only when ``--threshold`` is given and coverage is below
it (so CI can run it advisory-first, then flip to a hard gate).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# ── Exclusion policy ─────────────────────────────────────────────────────────
# Names matched exactly are out of the *practical* im surface. Keep this list
# explanatory — it documents the wrapper's intentional boundaries.
EXCLUDE_EXACT = {
    # Context / frame / IO / backend / allocator plumbing.
    "CreateContext", "DestroyContext", "GetCurrentContext", "SetCurrentContext",
    "GetIO", "GetPlatformIO", "GetStyle", "NewFrame", "EndFrame", "Render",
    "GetDrawData", "GetDrawListSharedData", "GetVersion",
    "DebugCheckVersionAndDataLayout", "MemAlloc", "MemFree",
    "SetAllocatorFunctions", "GetAllocatorFunctions", "GetMainViewport",
    "GetStateStorage", "SetStateStorage",
    "GetFont", "GetFontSize", "GetFontTexUvWhitePixel",
    # Persisted settings.
    "LoadIniSettingsFromDisk", "LoadIniSettingsFromMemory",
    "SaveIniSettingsToDisk", "SaveIniSettingsToMemory",
    # Style / ID / font stacks — use core/scope.h RAII guards + the theme engine.
    "PushStyleColor", "PopStyleColor", "PushStyleVar", "PopStyleVar",
    "PushStyleVarX", "PushStyleVarY", "PushFont", "PopFont", "PushID", "PopID",
    "PushItemFlag", "PopItemFlag", "PushTextWrapPos", "PopTextWrapPos",
    "GetID", "GetItemID", "GetColorU32", "GetStyleColorVec4",
    "GetStyleColorName", "SetColorEditOptions",
    "StyleColorsDark", "StyleColorsLight", "StyleColorsClassic",
    # Retained-mode widget equivalents (windows / tables / columns / images).
    "Begin", "End", "BeginTable", "EndTable", "Columns", "NextColumn",
    "GetColumnIndex", "GetColumnOffset", "GetColumnWidth", "GetColumnsCount",
    "SetColumnOffset", "SetColumnWidth", "Image", "ImageButton", "ImageWithBg",
    "ListBox", "Value", "TabItemButton", "SetTabItemClosed",
    # Drag & drop.
    "BeginDragDropSource", "EndDragDropSource", "BeginDragDropTarget",
    "EndDragDropTarget", "SetDragDropPayload", "AcceptDragDropPayload",
    "GetDragDropPayload",
    # Multi-select protocol.
    "BeginMultiSelect", "EndMultiSelect", "IsItemToggledSelection",
    "SetNextItemSelectionUserData",
    # Clipboard (not a draw call; belongs in a platform/core helper).
    "GetClipboardText", "SetClipboardText",
    # Logging / capture.
    "LogToTTY", "LogToFile", "LogToClipboard", "LogFinish", "LogButtons",
    "LogText",
    # Debug tool windows beyond the three we expose (demo/metrics/style editor).
    "DebugLog", "DebugStartItemPicker", "DebugTextEncoding",
    "DebugFlashStyleColor", "ShowAboutWindow", "ShowDebugLogWindow",
    "ShowIDStackToolWindow", "ShowStyleSelector", "ShowFontSelector",
    "ShowUserGuide",
    # Low-level / niche helpers without a clean immediate-mode shape.
    "GetKeyName", "GetKeyPressedAmount", "GetMouseClickedCount",
    "GetMousePosOnOpeningCurrentPopup", "IsAnyMouseDown", "IsMousePosValid",
    "IsMouseReleasedWithDelay", "IsRectVisible", "IsKeyChordPressed",
    "SetItemKeyOwner", "SetNavCursorVisible", "SetNextFrameWantCaptureKeyboard",
    "SetNextFrameWantCaptureMouse", "SetNextItemAllowOverlap",
    "SetNextItemShortcut", "SetNextItemStorageID", "Shortcut",
    "GetTreeNodeToLabelSpacing", "TreePush",
    # Window setters with overload-heavy / niche forms.
    "SetWindowPos", "SetWindowSize", "SetWindowCollapsed", "SetWindowFocus",
    "SetWindowFontScale",
}

# Names matched by prefix (covers families like docking, viewports, tables).
EXCLUDE_PREFIX = (
    "Table",        # retained-mode Table/DataTable widgets own this surface
    "DockSpace",    # docking
    "GetWindowDock", "GetWindowDpiScale", "GetWindowViewport", "IsWindowDocked",
    "SetNextWindowClass", "SetNextWindowDockID", "SetNextWindowViewport",
    "FindViewport", "UpdatePlatformWindows", "RenderPlatformWindowsDefault",
    "DestroyPlatformWindows",
)

# Names matched by suffix.
EXCLUDE_SUFFIX = (
    "V",        # printf-style va_list overloads (TextV, SetTooltipV, …)
    "Scalar",   # generic DragScalar/InputScalar/SliderScalar (typed wrapped)
    "ScalarN",
)


def excluded(name: str) -> bool:
    if name in EXCLUDE_EXACT:
        return True
    if any(name.startswith(p) for p in EXCLUDE_PREFIX):
        return True
    # Suffix rules: only treat as va_list/scalar when it really is one.
    if name.endswith("ScalarN") or name.endswith("Scalar"):
        return True
    if name.endswith("V") and name[:-1] and name[:-1][-1].islower():
        # e.g. TextV, SetTooltipV, DebugLogV — preceding char is lowercase.
        return True
    return False


def find_imgui_header(explicit: str | None) -> str:
    if explicit:
        return explicit
    candidates = [
        os.path.join(REPO_ROOT, "build", "vcpkg_installed", "x64-linux",
                     "include", "imgui.h"),
    ]
    # Fall back to any imgui.h under build/vcpkg_installed/*/include.
    base = os.path.join(REPO_ROOT, "build", "vcpkg_installed")
    if os.path.isdir(base):
        for triplet in sorted(os.listdir(base)):
            candidates.append(os.path.join(base, triplet, "include", "imgui.h"))
    for c in candidates:
        if os.path.isfile(c):
            return c
    sys.exit("error: could not locate imgui.h; pass --imgui PATH "
             "(configure the build first so vcpkg installs ImGui)")


def parse_imgui(path: str) -> set[str]:
    src = open(path, encoding="utf-8", errors="replace").read()
    m = re.search(r"namespace ImGui\b.*?\n\}", src, re.S)
    if not m:
        sys.exit(f"error: no `namespace ImGui` block found in {path}")
    names: set[str] = set()
    for line in m.group(0).splitlines():
        mm = re.match(r"\s*IMGUI_API\s+[\w:<>&*\s]+?([A-Za-z_]\w*)\s*\(", line)
        if mm:
            names.add(mm.group(1))
    return names


def parse_im(path: str) -> set[str]:
    names: set[str] = set()
    decl = re.compile(
        r"\s*(?:bool|void|float|int|double|unsigned|std::string|ImVec2|ImVec4|"
        r"ImU32|ImGuiID|ImDrawList\*|ImGuiMouseCursor|const\s+\w+)\s*\*?\s*&?\s*"
        r"([A-Za-z_]\w*)\s*\(")
    for line in open(path, encoding="utf-8").read().splitlines():
        mm = decl.match(line)
        if mm:
            names.add(mm.group(1))
    return names


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--imgui", help="path to imgui.h (default: autodetect under build/)")
    ap.add_argument("--im", default=os.path.join(REPO_ROOT, "include", "unigui", "im", "im.h"),
                    help="path to im.h")
    ap.add_argument("--threshold", type=float, default=None,
                    help="fail (exit 1) if practical coverage is below this %%")
    ap.add_argument("--list", action="store_true",
                    help="list the practical-but-unwrapped functions")
    ap.add_argument("--json", action="store_true", help="emit machine-readable JSON")
    args = ap.parse_args()

    imgui = parse_imgui(find_imgui_header(args.imgui))
    im = parse_im(args.im)

    practical = {n for n in imgui if not excluded(n)}
    excluded_set = imgui - practical
    wrapped = practical & im
    gap = sorted(practical - im)
    pct = 100.0 * len(wrapped) / len(practical) if practical else 0.0

    if args.json:
        print(json.dumps({
            "imgui_total": len(imgui),
            "excluded": len(excluded_set),
            "practical": len(practical),
            "wrapped": len(wrapped),
            "coverage_pct": round(pct, 1),
            "gap": gap,
        }, indent=2))
    else:
        print("UniGUI im-layer coverage vs Dear ImGui")
        print("─" * 44)
        print(f"  ImGui public functions : {len(imgui)}")
        print(f"  Out of im scope        : {len(excluded_set)}")
        print(f"  Practical surface      : {len(practical)}")
        print(f"  Wrapped by unigui::im  : {len(wrapped)}")
        print(f"  Coverage               : {pct:.1f}%")
        if args.list and gap:
            print("\n  Practical but unwrapped:")
            for n in gap:
                print(f"    - {n}")
        elif gap:
            print(f"\n  ({len(gap)} practical functions unwrapped; pass --list to see them)")

    if args.threshold is not None and pct < args.threshold:
        print(f"\nFAIL: coverage {pct:.1f}% < threshold {args.threshold:.1f}%",
              file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
