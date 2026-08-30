// Interaction tests — the `unigui::im` immediate layer (the 2026-08 wrapper batch).
// Engine-driven: a real click on a sortable table header must arm ImGui's sort specs,
// and typing into an EnterReturnsTrue input must persist on every keystroke (the flag
// only changes the return value — never the write-back). Compiled only when
// UNIGUI_TEST_ENGINE=ON.
#include <unigui/im/im.h>

#include <string>

#include "interaction_harness.h"

namespace im = unigui::im;

class ImInteractionTest : public itest::InteractionFixture {};

TEST_F(ImInteractionTest, SortableTableHeader_ClickArmsSortSpecs) {
    // im::BeginTable with a DefaultSort column; clicking its header must make
    // TableGetSortSpecs() return a live sort-spec block the caller can query.
    bool sawSpecs = false;
    float specCount = -1.0f;
    const auto st = Run(
        "im_table_sort_header",
        [&] {
            if (im::BeginTable("##tsort", 2, ImGuiTableFlags_Sortable)) {
                im::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort);
                im::TableSetupColumn("Qty", ImGuiTableColumnFlags_PreferSortDescending);
                im::TableHeadersRow();
                for (int r = 0; r < 3; ++r) {
                    im::TableNextRow();
                    im::TableNextColumn();
                    im::TextUnformatted(r == 0 ? "c" : r == 1 ? "a" : "b");
                    im::TableNextColumn();
                    im::TextUnformatted(std::to_string(3 - r));
                }
                ImGuiTableSortSpecs* specs = im::TableGetSortSpecs();
                sawSpecs = specs != nullptr;
                specCount = specs ? static_cast<float>(specs->SpecsCount) : -1.0f;
                im::EndTable();
            }
        },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/Name");
            ctx->Yield();
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_TRUE(sawSpecs) << "clicking the sortable header must arm TableGetSortSpecs";
    EXPECT_GE(specCount, 1.0f);
}

TEST_F(ImInteractionTest, EnterReturnsTrue_TypingPersistsEveryKeystroke) {
    // Interaction twin of the headless regression: with EnterReturnsTrue the return
    // value fires only on the Enter frame, so a write-back keyed on it would discard
    // every keystroke (a password box you cannot type into). The engine types "abc"
    // with Enter never pressed; the bound string must still be "abc".
    std::string value;
    const ImGuiInputTextFlags flags =
        ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue;
    const auto st = Run(
        "im_inputtext_enterreturn",
        [&] { im::InputText("pwd", &value, 128, flags); },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/pwd");
            ctx->Yield(); // the frame acquiring keyboard focus swallows one char — idle past it
            ctx->KeyChars("abc");
            ctx->Yield();
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(value, "abc")
        << "typing was discarded: EnterReturnsTrue must not disable write-back";
}
