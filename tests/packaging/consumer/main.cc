// Downstream consumer smoke test for `find_package(unigui)`.
//
// Built OUT-OF-TREE against an *installed* UniGUI (not the source build), this
// verifies the whole packaging contract end to end: the exported `unigui::unigui`
// target resolves, its installed headers are on the include path, `unigui_export.h`
// is present, the transitively-propagated dependency includes (imgui, …) are
// reachable, and the re-resolved `find_dependency()` link interface links.

#include <unigui/core/observable.h>
#include <unigui/core/version.h>
#include <unigui/widgets/label.h>

#include <cstdio>
#include <string>

int main() {
    // Header-only reactive core.
    unigui::Observable<int> n{41};
    n.Set(42);

    // A compiled widget — forces linking the installed library, exercising the
    // exported link interface (imgui/glfw/implot/… via find_dependency) and the
    // transitive include propagation (label.h pulls in imgui headers).
    unigui::Label label("consumer", "ok");

    std::printf("unigui %s consumer: label='%s' n=%d\n", UNIGUI_VERSION_STRING,
                label.GetText().c_str(), n.Get());
    return (n.Get() == 42 && label.GetText() == "ok") ? 0 : 1;
}
