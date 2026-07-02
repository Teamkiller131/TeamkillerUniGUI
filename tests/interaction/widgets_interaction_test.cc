// Interaction tests — selection/input + data/composite widgets. The Dear ImGui test
// engine drives real clicks/typing through the widgets and the tests assert BEHAVIOR
// (selection changed, callback fired, value round-tripped, a11y announcement queued).
// Compiled only when UNIGUI_TEST_ENGINE=ON (see tests/interaction_harness.h and the
// windows-msvc-debug-testengine preset / linux-testengine CI lane).
#include <unigui/core/accessibility.h>
#include <unigui/widgets/combobox.h>
#include <unigui/widgets/datatable.h>
#include <unigui/widgets/form.h>
#include <unigui/widgets/inputfloat.h>
#include <unigui/widgets/inputint.h>
#include <unigui/widgets/listbox.h>
#include <unigui/widgets/listview.h>
#include <unigui/widgets/radiogroup.h>
#include <unigui/widgets/searchbox.h>
#include <unigui/widgets/segmentedcontrol.h>
#include <unigui/widgets/toggleswitch.h>
#include <unigui/widgets/treeview.h>

#include <string>
#include <vector>

#include "interaction_harness.h"

class SelectionInputTest : public itest::InteractionFixture {};
class DataCompositeInteractionTest : public itest::InteractionFixture {};

TEST_F(SelectionInputTest, RadioGroupClickChangesSelection) {
    // RadioGroup renders one ImGui::RadioButton per option with the option text as
    // the visible label, scoped under PushID(name) — so "**/Beta" resolves.
    unigui::RadioGroup rg("rg", {"Alpha", "Beta", "Gamma"}, 0);
    int lastChange = -1;
    rg.SetOnChange([&](int idx) { lastChange = idx; });
    const auto st = Run(
        "selinput_radiogroup_click", [&] { rg.Render(); },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/Beta");
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(rg.GetSelected(), 1);
    EXPECT_EQ(lastChange, 1);
}

TEST_F(SelectionInputTest, ToggleSwitchCheckAndUncheck) {
    // ToggleSwitch is an ImGui::Checkbox under the hood, so the engine's
    // check/uncheck actions apply directly.
    unigui::ToggleSwitch sw("tsw", "Enable", false);
    bool lastValue = false;
    int fires = 0;
    sw.SetOnChange([&](bool v) {
        lastValue = v;
        ++fires;
    });
    const auto st = Run(
        "selinput_toggle_check_uncheck", [&] { sw.Render(); },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemCheck("**/Enable");
            ctx->ItemUncheck("**/Enable");
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_FALSE(sw.IsOn()); // net effect: on, then back off
    EXPECT_EQ(fires, 2);     // both edges observed
    EXPECT_FALSE(lastValue); // last transition was the uncheck
}

TEST_F(SelectionInputTest, ComboBoxPickItem) {
    // ComboBox = BeginCombo(label, preview) + Selectable(item). ComboClick splits
    // combo-vs-item at the FIRST '/', so the widget's PushID scope must live in the
    // ref: ref "TW/cb" + "Fruit/Cherry" = combo "Fruit", popup item "Cherry".
    unigui::ComboBox cb("cb", "Fruit", {"Apple", "Banana", "Cherry"}, 0);
    int lastChange = -1;
    cb.SetOnChange([&](int idx) { lastChange = idx; });
    const auto st = Run(
        "selinput_combo_pick", [&] { cb.Render(); },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW/cb");
            ctx->ComboClick("Fruit/Cherry");
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(cb.GetSelectedIndex(), 2);
    EXPECT_EQ(cb.GetSelectedValue(), "Cherry");
    EXPECT_EQ(lastChange, 2);
}

TEST_F(SelectionInputTest, SegmentedControlClickSegment) {
    // Each segment is an InvisibleButton("##seg") under PushID(name) + PushID(int i);
    // the engine addresses integer PushIDs with the documented "$$<int>" path
    // segment, so segment 2 is the explicit path "seg/$$2/##seg".
    unigui::SegmentedControl sc("seg", {"1D", "1W", "1M"});
    int lastIndex = -1;
    std::string lastLabel;
    sc.SetOnChange([&](int idx, const std::string& label) {
        lastIndex = idx;
        lastLabel = label;
    });
    const auto st = Run(
        "selinput_segmented_click", [&] { sc.Render(); },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("seg/$$2/##seg");
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(sc.GetSelected(), 2);
    EXPECT_EQ(sc.GetSelectedLabel(), "1M");
    EXPECT_EQ(lastIndex, 2);
    EXPECT_EQ(lastLabel, "1M");
}

TEST_F(SelectionInputTest, InputIntTypeValue) {
    // InputInt renders ImGui::InputInt(label) with step 0 under PushID(name);
    // ItemInputValue clicks, select-all-replaces the text, and presses Enter.
    unigui::InputInt in("qty", "Count", 5, 0, 100);
    int lastChange = -1;
    in.SetOnChange([&](int v) { lastChange = v; });
    const auto st = Run(
        "selinput_inputint_type", [&] { in.Render(); },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemInputValue("qty/Count", 42);
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(in.GetValue(), 42);
    EXPECT_EQ(lastChange, 42); // final NotifyChange carried the typed value
}

TEST_F(SelectionInputTest, InputFloatTypeValue) {
    // Same idiom as InputInt but through the float ItemInputValue overload.
    // 2.5 is exactly representable and survives the widget's "%.3f" format.
    unigui::InputFloat in("amt", "Amount", 1.0f, 0.0f, 100.0f);
    float lastChange = -1.0f;
    in.SetOnChange([&](float v) { lastChange = v; });
    const auto st = Run(
        "selinput_inputfloat_type", [&] { in.Render(); },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemInputValue("amt/Amount", 2.5f);
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_FLOAT_EQ(in.GetValue(), 2.5f);
    EXPECT_FLOAT_EQ(lastChange, 2.5f);
}

TEST_F(SelectionInputTest, SearchBoxTypeUpdatesQuery) {
    // SearchBox renders InputTextWithHint(GetName()) inside PushID(GetName()), so
    // the item path is "<name>/<name>". The label is the plain name (clipped by
    // SetNextItemWidth(-1), but the ID is unaffected). No items are set, so the
    // suggestion tooltip never opens mid-type.
    unigui::SearchBox sb("sbox", "Search...");
    std::string lastQuery;
    sb.SetOnChange([&](const std::string& q) { lastQuery = q; });
    const auto st = Run(
        "selinput_searchbox_type", [&] { sb.Render(); },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemInputValue("sbox/sbox", "imgui");
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(sb.GetQuery(), "imgui");
    EXPECT_EQ(lastQuery, "imgui");
}
// ── DataTable: clicking a row's first-column cell selects the row ────────────
// The first column renders as a SpanAllColumns Selectable labeled with the
// formatted cell text, so the engine targets "**/<cell text>".
TEST_F(DataCompositeInteractionTest, DataTable_ClickRowCell_SelectsAndFiresCallback) {
    std::vector<int> data{10, 20, 30};
    unigui::DataTable<int> dt("it_dt", {{"A"}, {"B"}});
    dt.SetDataSource(&data);
    dt.SetCellFormatter([](int, int col, const int& v) {
        return col == 0 ? "Item " + std::to_string(v) : std::to_string(v);
    });
    int selectedRow = -1;
    dt.SetOnSelect([&](int row) { selectedRow = row; });

    const auto status = Run(
        "datatable_row_click", [&] { dt.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/Item 20");
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_EQ(selectedRow, 1);
    EXPECT_EQ(dt.GetSelectedRow(), 1);
}

// ── DataTable × a11y: a driven row click queues the live announcement ────────
// engine input → Selectable → a11y::Announce("Row N selected: <cell>") — the
// cross-subsystem path a screen reader depends on.
TEST_F(DataCompositeInteractionTest, DataTable_ClickRow_QueuesA11yAnnouncement) {
    unigui::a11y::SetEnabled(true);
    unigui::a11y::BeginFrame();
    unigui::a11y::DrainAnnouncements();

    std::vector<int> data{10, 20, 30};
    unigui::DataTable<int> dt("it_dt_a11y", {{"A"}, {"B"}});
    dt.SetDataSource(&data);
    dt.SetCellFormatter([](int, int col, const int& v) {
        return col == 0 ? "Item " + std::to_string(v) : std::to_string(v);
    });

    const auto status = Run(
        "datatable_row_a11y_announce", [&] { dt.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/Item 30");
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_EQ(dt.GetSelectedRow(), 2);
    bool announced = false;
    for (const auto& a : unigui::a11y::DrainAnnouncements())
        if (a.message == "Row 2 selected: Item 30")
            announced = true;
    EXPECT_TRUE(announced);
    unigui::a11y::SetEnabled(false);
}

// ── TreeView: clicking a node label selects it and announces ─────────────────
// Nodes render via TreeNodeEx with the label visible; HideRoot puts the
// children at top level so the target is on screen without an ItemOpen first.
TEST_F(DataCompositeInteractionTest, TreeView_ClickNodeLabel_SelectsAndAnnounces) {
    unigui::a11y::SetEnabled(true);
    unigui::a11y::BeginFrame();
    unigui::a11y::DrainAnnouncements();

    unigui::TreeNode root;
    root.label = "root";
    unigui::TreeNode alpha;
    alpha.label = "Alpha";
    unigui::TreeNode beta;
    beta.label = "Beta";
    root.children = {alpha, beta};

    unigui::TreeView tv("it_tree");
    tv.SetRoot(root);
    tv.SetHideRoot(true);

    const auto status = Run(
        "treeview_node_select", [&] { tv.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/Alpha");
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    ASSERT_FALSE(tv.GetSelectedNodes().empty());
    EXPECT_EQ(tv.GetSelectedNodes()[0], 0); // "Alpha" is the first node walked
    bool announced = false;
    for (const auto& a : unigui::a11y::DrainAnnouncements())
        if (a.message == "Alpha selected")
            announced = true;
    EXPECT_TRUE(announced);
    unigui::a11y::SetEnabled(false);
}

// ── ListView: clicking an item updates the selection getter + callback ───────
TEST_F(DataCompositeInteractionTest, ListView_ClickItem_UpdatesSelection) {
    unigui::ListView lv("it_list", {"One", "Two", "Three"});
    int callbackIndex = -1;
    lv.SetOnSelect([&](int i) { callbackIndex = i; });

    const auto status = Run(
        "listview_item_select", [&] { lv.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/Two");
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_EQ(lv.GetSelected(), 1);
    EXPECT_EQ(callbackIndex, 1);
}

// ── ListBox: clicking an item changes index/value and fires OnChange ─────────
// ImGui::ListBox draws each item as a Selectable inside the list's child frame,
// so the wildcard path reaches "Banana" directly.
TEST_F(DataCompositeInteractionTest, ListBox_ClickItem_ChangesSelection) {
    unigui::ListBox lb("it_lb", "Fruits", {"Apple", "Banana", "Cherry"});
    int changedIndex = -1;
    lb.SetOnChange([&](int i) { changedIndex = i; });

    const auto status = Run(
        "listbox_item_select", [&] { lb.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/Banana");
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_EQ(lb.GetSelectedIndex(), 1);
    EXPECT_EQ(lb.GetSelectedValue(), "Banana");
    EXPECT_EQ(changedIndex, 1);
}

// ── Form: typing into a required field, then Submit → OnSubmit fires ─────────
// Form::Render opens its OWN window titled with the form title, so the driver
// refs that window (not "TW").
TEST_F(DataCompositeInteractionTest, Form_FillRequiredField_SubmitFires) {
    unigui::Form form("it_form", "LoginForm");
    form.AddTextField("user", "User", /*required=*/true);
    bool submitted = false;
    form.SetOnSubmit([&] { submitted = true; });

    const auto status = Run(
        "form_submit_valid", [&] { form.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("LoginForm");
            ctx->ItemInputValue("**/User", "alice");
            ctx->ItemClick("**/Submit");
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_EQ(form.GetFieldValue("user"), "alice");
    EXPECT_TRUE(submitted);
    EXPECT_TRUE(form.GetErrors().empty());
}

// ── Form: Submit with an empty required field is blocked with an error ───────
TEST_F(DataCompositeInteractionTest, Form_EmptyRequiredField_SubmitBlocked) {
    unigui::Form form("it_form2", "SignupForm");
    form.AddTextField("email", "Email", /*required=*/true);
    bool submitted = false;
    form.SetOnSubmit([&] { submitted = true; });

    const auto status = Run(
        "form_submit_required_blocked", [&] { form.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("SignupForm");
            ctx->ItemClick("**/Submit");
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_FALSE(submitted);
    ASSERT_EQ(form.GetErrors().size(), 1u);
    EXPECT_EQ(form.GetErrors()[0].field_name, "email");
    EXPECT_EQ(form.GetErrors()[0].message, "Required field");
}