#include <unigui/im/im.h>

#include <imgui.h>

#include <gtest/gtest.h>
#include <functional>
#include <string>
#include <vector>

// Immediate-mode free functions (unigui::im). These exercise the wrappers for
// crashes and correct value binding within a headless ImGui frame, mirroring the
// existing widget tests' fixture pattern.
class ImTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }
};

TEST_F(ImTest, Button_DoesNotCrash) {
    unigui::im::Button("Click");
    unigui::im::Button("Primary", unigui::im::ButtonVariant::Primary);
    unigui::im::Button("Danger", unigui::im::ButtonVariant::Danger);
    unigui::im::Button("Success", unigui::im::ButtonVariant::Success);
    unigui::im::SmallButton("Small");
}

TEST_F(ImTest, Text_VariantsDoNotCrash) {
    unigui::im::Text("plain");
    unigui::im::TextWrapped("wrapped text that is reasonably long");
    unigui::im::TextDisabled("disabled");
    unigui::im::TextColored(ImVec4(1, 0, 0, 1), "red");
    unigui::im::BulletText("bullet");
    unigui::im::LabelText("Label", "Value");
}

TEST_F(ImTest, Checkbox_BindsValue) {
    bool v = false;
    unigui::im::Checkbox("flag", &v); // no click: stays false
    EXPECT_FALSE(v);
}

TEST_F(ImTest, SliderFloat_DoesNotCrash) {
    float f = 0.5f;
    unigui::im::SliderFloat("gain", &f, 0.f, 1.f);
    EXPECT_GE(f, 0.f);
    EXPECT_LE(f, 1.f);
}

TEST_F(ImTest, SliderInt_DoesNotCrash) {
    int i = 3;
    unigui::im::SliderInt("count", &i, 0, 10);
    EXPECT_EQ(i, 3);
}

TEST_F(ImTest, DragAndInputNumeric_DoNotCrash) {
    float f = 1.0f;
    int i = 2;
    unigui::im::DragFloat("df", &f);
    unigui::im::DragInt("di", &i);
    unigui::im::InputFloat("if", &f);
    unigui::im::InputInt("ii", &i);
}

TEST_F(ImTest, InputText_BindsStringAndStaysUnchanged) {
    std::string s = "hello";
    bool changed = unigui::im::InputText("name", &s);
    EXPECT_FALSE(changed);
    EXPECT_EQ(s, "hello");
}

TEST_F(ImTest, InputText_NullPointerReturnsFalse) {
    EXPECT_FALSE(unigui::im::InputText("name", nullptr));
    EXPECT_FALSE(unigui::im::InputTextMultiline("ml", nullptr));
}

TEST_F(ImTest, InputTextMultiline_DoesNotCrash) {
    std::string s = "line1\nline2";
    unigui::im::InputTextMultiline("body", &s);
    EXPECT_EQ(s, "line1\nline2");
}

TEST_F(ImTest, Combo_BindsIndex) {
    int idx = 1;
    std::vector<std::string> items = {"A", "B", "C"};
    unigui::im::Combo("pick", &idx, items);
    EXPECT_EQ(idx, 1);
}

TEST_F(ImTest, Combo_NullPointerReturnsFalse) {
    std::vector<std::string> items = {"A"};
    EXPECT_FALSE(unigui::im::Combo("pick", nullptr, items));
}

TEST_F(ImTest, Combo_LeadingEmptyItemBindsIndex) {
    // Regression: a leading "" (blank/clear option many callers prepend) must NOT
    // collapse the list to zero items. The old packed-zero-string impl fed
    // ImGui::Combo(const char*) whose `while (*p)` counter stopped on the leading
    // '\0' and reported 0 items -> empty popup.
    int idx = 2;
    std::vector<std::string> items = {"", "IH", "IF", "IC"};
    unigui::im::Combo("pick", &idx, items);
    EXPECT_EQ(idx, 2);  // in-range index preserved, no crash
}

TEST_F(ImTest, Combo_ClampsOutOfRangeIndex) {
    int idx = 99;
    std::vector<std::string> items = {"", "A"};
    unigui::im::Combo("pick", &idx, items);
    EXPECT_GE(idx, 0);
    EXPECT_LT(idx, static_cast<int>(items.size()));
}

TEST_F(ImTest, DrawActiveInputCaret_NoActiveInputIsNoOp) {
    // Must be safe with no active input item (IsItemActive()==false -> early return).
    unigui::im::DrawActiveInputCaret();
    SUCCEED();
}

TEST_F(ImTest, DrawActiveInputCaret_AfterInputDoesNotCrash) {
    std::string v = "abc";
    unigui::im::InputText("field", &v);   // calls DrawActiveInputCaret() internally
    SUCCEED();
}

TEST_F(ImTest, Layout_HelpersDoNotCrash) {
    unigui::im::Text("a");
    unigui::im::SameLine();
    unigui::im::Text("b");
    unigui::im::NewLine();
    unigui::im::Spacing();
    unigui::im::Separator();
    unigui::im::SeparatorText("section");
    unigui::im::Dummy(10, 10);
    unigui::im::Indent();
    unigui::im::Unindent();
    unigui::im::Bullet();
}

// ── A1: Vector/scalar slider variants ────────────────────────────────────────

TEST_F(ImTest, SliderFloat_VectorVariants_DoNotCrash) {
    float v2[2] = {0.1f, 0.2f};
    float v3[3] = {0.1f, 0.2f, 0.3f};
    float v4[4] = {0.1f, 0.2f, 0.3f, 0.4f};
    unigui::im::SliderFloat2("v2", v2, 0.f, 1.f);
    unigui::im::SliderFloat3("v3", v3, 0.f, 1.f);
    unigui::im::SliderFloat4("v4", v4, 0.f, 1.f);
    // values unchanged without interaction
    EXPECT_FLOAT_EQ(v2[0], 0.1f);
    EXPECT_FLOAT_EQ(v3[1], 0.2f);
    EXPECT_FLOAT_EQ(v4[3], 0.4f);
}

TEST_F(ImTest, SliderAngle_DoesNotCrash) {
    float rad = 1.0f; // ~57 degrees
    unigui::im::SliderAngle("angle", &rad);
    EXPECT_FLOAT_EQ(rad, 1.0f); // no interaction → unchanged
}

TEST_F(ImTest, SliderInt_VectorVariants_DoNotCrash) {
    int v2[2] = {1, 2};
    int v3[3] = {1, 2, 3};
    int v4[4] = {1, 2, 3, 4};
    unigui::im::SliderInt2("v2i", v2, 0, 10);
    unigui::im::SliderInt3("v3i", v3, 0, 10);
    unigui::im::SliderInt4("v4i", v4, 0, 10);
    EXPECT_EQ(v2[1], 2);
    EXPECT_EQ(v3[2], 3);
    EXPECT_EQ(v4[3], 4);
}

TEST_F(ImTest, VSlider_DoNotCrash) {
    float f = 0.5f;
    int i = 3;
    unigui::im::VSliderFloat("vf", ImVec2(18, 80), &f, 0.f, 1.f);
    unigui::im::VSliderInt("vi", ImVec2(18, 80), &i, 0, 10);
    EXPECT_FLOAT_EQ(f, 0.5f);
    EXPECT_EQ(i, 3);
}

// ── A1: Vector drag variants ──────────────────────────────────────────────────

TEST_F(ImTest, DragFloat_VectorVariants_DoNotCrash) {
    float v2[2] = {1.f, 2.f};
    float v3[3] = {1.f, 2.f, 3.f};
    float v4[4] = {1.f, 2.f, 3.f, 4.f};
    unigui::im::DragFloat2("df2", v2);
    unigui::im::DragFloat3("df3", v3);
    unigui::im::DragFloat4("df4", v4);
    EXPECT_FLOAT_EQ(v2[0], 1.f);
    EXPECT_FLOAT_EQ(v3[2], 3.f);
    EXPECT_FLOAT_EQ(v4[3], 4.f);
}

TEST_F(ImTest, DragFloatRange2_DoesNotCrash) {
    float lo = 0.2f, hi = 0.8f;
    unigui::im::DragFloatRange2("range", &lo, &hi);
    EXPECT_FLOAT_EQ(lo, 0.2f);
    EXPECT_FLOAT_EQ(hi, 0.8f);
}

TEST_F(ImTest, DragInt_VectorVariants_DoNotCrash) {
    int v2[2] = {1, 2};
    int v3[3] = {1, 2, 3};
    int v4[4] = {1, 2, 3, 4};
    unigui::im::DragInt2("di2", v2);
    unigui::im::DragInt3("di3", v3);
    unigui::im::DragInt4("di4", v4);
    EXPECT_EQ(v2[1], 2);
    EXPECT_EQ(v3[2], 3);
    EXPECT_EQ(v4[3], 4);
}

TEST_F(ImTest, DragIntRange2_DoesNotCrash) {
    int lo = 2, hi = 8;
    unigui::im::DragIntRange2("irange", &lo, &hi);
    EXPECT_EQ(lo, 2);
    EXPECT_EQ(hi, 8);
}

// ── A1: Vector input variants ─────────────────────────────────────────────────

TEST_F(ImTest, InputFloat_VectorVariants_DoNotCrash) {
    float v2[2] = {1.f, 2.f};
    float v3[3] = {1.f, 2.f, 3.f};
    float v4[4] = {1.f, 2.f, 3.f, 4.f};
    unigui::im::InputFloat2("if2", v2);
    unigui::im::InputFloat3("if3", v3);
    unigui::im::InputFloat4("if4", v4);
    EXPECT_FLOAT_EQ(v2[0], 1.f);
    EXPECT_FLOAT_EQ(v3[1], 2.f);
    EXPECT_FLOAT_EQ(v4[3], 4.f);
}

TEST_F(ImTest, InputInt_VectorVariants_DoNotCrash) {
    int v2[2] = {10, 20};
    int v3[3] = {10, 20, 30};
    int v4[4] = {10, 20, 30, 40};
    unigui::im::InputInt2("ii2", v2);
    unigui::im::InputInt3("ii3", v3);
    unigui::im::InputInt4("ii4", v4);
    EXPECT_EQ(v2[1], 20);
    EXPECT_EQ(v3[2], 30);
    EXPECT_EQ(v4[3], 40);
}

TEST_F(ImTest, InputDouble_DoesNotCrash) {
    double d = 3.14159;
    unigui::im::InputDouble("pi", &d);
    EXPECT_DOUBLE_EQ(d, 3.14159);
}

// ── A1: InputTextWithHint ─────────────────────────────────────────────────────

TEST_F(ImTest, InputTextWithHint_BindsStringAndStaysUnchanged) {
    std::string s = "hello";
    bool changed = unigui::im::InputTextWithHint("search", "Type to search…", &s);
    EXPECT_FALSE(changed);
    EXPECT_EQ(s, "hello");
}

TEST_F(ImTest, InputTextWithHint_NullPointerReturnsFalse) {
    EXPECT_FALSE(unigui::im::InputTextWithHint("x", "hint", nullptr));
}

TEST_F(ImTest, InputTextWithHint_EmptyStringDoesNotCrash) {
    std::string s;
    unigui::im::InputTextWithHint("empty", "placeholder", &s);
    EXPECT_TRUE(s.empty());
}

// ── A2: Item width ────────────────────────────────────────────────────────────

TEST_F(ImTest, ItemWidth_PushPopDoesNotCrash) {
    unigui::im::PushItemWidth(120.f);
    float w = unigui::im::CalcItemWidth();
    EXPECT_GT(w, 0.f);
    unigui::im::PopItemWidth();
}

TEST_F(ImTest, SetNextItemWidth_DoesNotCrash) {
    unigui::im::SetNextItemWidth(80.f);
    float f = 0.5f;
    unigui::im::SliderFloat("w", &f, 0.f, 1.f);
}

// ── A2: Group ─────────────────────────────────────────────────────────────────

TEST_F(ImTest, Group_BeginEndDoesNotCrash) {
    unigui::im::BeginGroup();
    unigui::im::Text("inside group");
    unigui::im::EndGroup();
}

// ── A2: Child windows ─────────────────────────────────────────────────────────

TEST_F(ImTest, BeginChild_StringId_DoesNotCrash) {
    bool visible = unigui::im::BeginChild("child1", ImVec2(200, 100));
    if (visible)
        unigui::im::Text("inside child");
    unigui::im::EndChild();
}

TEST_F(ImTest, BeginChild_NumericId_DoesNotCrash) {
    ImGui::Begin("child_id_parent");
    bool visible = unigui::im::BeginChild(static_cast<ImGuiID>(42), ImVec2(100, 50));
    if (visible)
        unigui::im::Text("child 42");
    unigui::im::EndChild();
    ImGui::End();
}

// ── A2: Next-window hints ─────────────────────────────────────────────────────

TEST_F(ImTest, SetNextWindow_HintsDoNotCrash) {
    unigui::im::SetNextWindowPos(ImVec2(100, 100));
    unigui::im::SetNextWindowSize(ImVec2(300, 200));
    unigui::im::SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(800, 600));
    unigui::im::SetNextWindowContentSize(ImVec2(0, 0));
    unigui::im::SetNextWindowCollapsed(false);
    unigui::im::SetNextWindowScroll(ImVec2(0, 0));
    unigui::im::SetNextWindowBgAlpha(0.9f);
    // SetNextWindowFocus is safe to call; begin a window to consume the hints
    ImGui::Begin("hint_test_wnd");
    ImGui::End();
}

// ── A2: Scrolling ─────────────────────────────────────────────────────────────

TEST_F(ImTest, Scroll_GettersReturnNonNegative) {
    ImGui::Begin("scroll_wnd");
    EXPECT_GE(unigui::im::GetScrollX(), 0.f);
    EXPECT_GE(unigui::im::GetScrollY(), 0.f);
    EXPECT_GE(unigui::im::GetScrollMaxX(), 0.f);
    EXPECT_GE(unigui::im::GetScrollMaxY(), 0.f);
    ImGui::End();
}

TEST_F(ImTest, Scroll_SetAndHereDoNotCrash) {
    ImGui::Begin("scroll_wnd2");
    unigui::im::SetScrollX(0.f);
    unigui::im::SetScrollY(0.f);
    unigui::im::Text("line");
    unigui::im::SetScrollHereY();
    unigui::im::SetScrollHereX();
    unigui::im::SetScrollFromPosX(0.f);
    unigui::im::SetScrollFromPosY(0.f);
    ImGui::End();
}

// ── A2: Cursor & content region ───────────────────────────────────────────────

TEST_F(ImTest, CursorScreenPos_GetSet) {
    ImGui::Begin("cursor_wnd");
    ImVec2 pos = unigui::im::GetCursorScreenPos();
    EXPECT_GT(pos.x, 0.f);
    unigui::im::SetCursorScreenPos(pos);
    ImGui::End();
}

TEST_F(ImTest, CursorPos_WindowLocal) {
    ImGui::Begin("cursor_wnd2");
    ImVec2 cp = unigui::im::GetCursorPos();
    EXPECT_GE(cp.x, 0.f);
    EXPECT_GE(cp.y, 0.f);
    float x = unigui::im::GetCursorPosX();
    float y = unigui::im::GetCursorPosY();
    EXPECT_FLOAT_EQ(x, cp.x);
    EXPECT_FLOAT_EQ(y, cp.y);
    unigui::im::SetCursorPos(cp);
    unigui::im::SetCursorPosX(x);
    unigui::im::SetCursorPosY(y);
    ImGui::End();
}

TEST_F(ImTest, ContentRegionAndWindowMetrics_DoNotCrash) {
    ImGui::Begin("metrics_wnd");
    ImVec2 avail = unigui::im::GetContentRegionAvail();
    EXPECT_GE(avail.x, 0.f);
    EXPECT_GE(avail.y, 0.f);
    ImVec2 start = unigui::im::GetCursorStartPos();
    (void) start;
    ImVec2 wpos = unigui::im::GetWindowPos();
    (void) wpos;
    ImVec2 wsz = unigui::im::GetWindowSize();
    EXPECT_GT(wsz.x, 0.f);
    EXPECT_GT(unigui::im::GetWindowWidth(), 0.f);
    EXPECT_GT(unigui::im::GetWindowHeight(), 0.f);
    ImGui::End();
}

// ── A2: Clip rect ─────────────────────────────────────────────────────────────

TEST_F(ImTest, ClipRect_DoesNotCrash) {
    ImGui::Begin("clip_wnd");
    ImVec2 pos = unigui::im::GetCursorScreenPos();
    unigui::im::PushClipRect(pos, ImVec2(pos.x + 100, pos.y + 50), true);
    unigui::im::Text("clipped");
    unigui::im::PopClipRect();
    ImGui::End();
}

// ── A2: Alignment & line metrics ─────────────────────────────────────────────

TEST_F(ImTest, LineMetrics_ReturnPositive) {
    EXPECT_GT(unigui::im::GetTextLineHeight(), 0.f);
    EXPECT_GT(unigui::im::GetTextLineHeightWithSpacing(), 0.f);
    EXPECT_GT(unigui::im::GetFrameHeight(), 0.f);
    EXPECT_GT(unigui::im::GetFrameHeightWithSpacing(), 0.f);
}

TEST_F(ImTest, AlignTextToFramePadding_DoesNotCrash) {
    unigui::im::AlignTextToFramePadding();
    unigui::im::Text("aligned label");
    unigui::im::SameLine();
    float f = 0.f;
    unigui::im::InputFloat("value", &f);
}

// ── A3: Popups ────────────────────────────────────────────────────────────────

TEST_F(ImTest, Popup_OpenAndIsOpen) {
    // OpenPopup marks it open; IsPopupOpen should reflect that within the frame.
    unigui::im::OpenPopup("test_popup");
    EXPECT_TRUE(unigui::im::IsPopupOpen("test_popup"));
}

TEST_F(ImTest, Popup_BeginEndWhenClosed_DoesNotCrash) {
    // Not opened → BeginPopup returns false → must NOT call EndPopup.
    bool open = unigui::im::BeginPopup("closed_popup");
    EXPECT_FALSE(open);
}

TEST_F(ImTest, PopupModal_BeginEndWhenClosed_DoesNotCrash) {
    bool open = unigui::im::BeginPopupModal("closed_modal");
    EXPECT_FALSE(open);
}

TEST_F(ImTest, Popup_OpenThenBeginEnd) {
    // Open and immediately begin in the same frame (valid in headless).
    unigui::im::OpenPopup("live_popup");
    if (unigui::im::BeginPopup("live_popup")) {
        unigui::im::Text("inside popup");
        unigui::im::EndPopup();
    }
}

TEST_F(ImTest, PopupModal_OpenThenBeginEnd) {
    unigui::im::OpenPopup("live_modal");
    if (unigui::im::BeginPopupModal("live_modal")) {
        unigui::im::Text("inside modal");
        unigui::im::CloseCurrentPopup();
        unigui::im::EndPopup();
    }
}

TEST_F(ImTest, IsPopupOpen_UnknownReturnsFalse) {
    EXPECT_FALSE(unigui::im::IsPopupOpen("no_such_popup"));
}

TEST_F(ImTest, OpenPopup_NumericId_DoesNotCrash) {
    unigui::im::OpenPopup(static_cast<ImGuiID>(0xBEEF));
    // Not checking IsPopupOpen for numeric ID — just verifying no crash.
}

TEST_F(ImTest, ContextPopups_DoNotCrashWhenNotTriggered) {
    // These return false (no right-click) in headless; must not crash.
    EXPECT_FALSE(unigui::im::BeginPopupContextItem("ctx_item"));
    EXPECT_FALSE(unigui::im::BeginPopupContextWindow("ctx_win"));
    EXPECT_FALSE(unigui::im::BeginPopupContextVoid("ctx_void"));
}

// ── A3: Menus ─────────────────────────────────────────────────────────────────

TEST_F(ImTest, MainMenuBar_BeginEnd) {
    if (unigui::im::BeginMainMenuBar()) {
        if (unigui::im::BeginMenu("File")) {
            unigui::im::MenuItem("Open", "Ctrl+O");
            bool checked = true;
            unigui::im::MenuItem("Autosave", "", &checked);
            EXPECT_TRUE(checked);
            unigui::im::EndMenu();
        }
        unigui::im::EndMainMenuBar();
    }
}

TEST_F(ImTest, MenuItem_ReturnsFalseWithoutClick) {
    if (unigui::im::BeginMainMenuBar()) {
        if (unigui::im::BeginMenu("Edit")) {
            bool activated = unigui::im::MenuItem("Cut", "Ctrl+X");
            EXPECT_FALSE(activated); // no user interaction in headless
            unigui::im::EndMenu();
        }
        unigui::im::EndMainMenuBar();
    }
}

TEST_F(ImTest, MenuBar_InsideWindow) {
    ImGui::Begin("menubar_wnd", nullptr, ImGuiWindowFlags_MenuBar);
    if (unigui::im::BeginMenuBar()) {
        if (unigui::im::BeginMenu("View")) {
            unigui::im::MenuItem("Zoom In", "Ctrl++", false, false);
            unigui::im::EndMenu();
        }
        unigui::im::EndMenuBar();
    }
    ImGui::End();
}

// ── A4: Item queries ──────────────────────────────────────────────────────────

TEST_F(ImTest, ItemQueries_AfterButton_DoNotCrash) {
    unigui::im::Button("q_btn");
    // No click in headless — all queries should return false without crashing.
    EXPECT_FALSE(unigui::im::IsItemHovered());
    EXPECT_FALSE(unigui::im::IsItemActive());
    EXPECT_FALSE(unigui::im::IsItemFocused());
    EXPECT_FALSE(unigui::im::IsItemClicked());
    EXPECT_FALSE(unigui::im::IsItemEdited());
    EXPECT_FALSE(unigui::im::IsItemActivated());
    EXPECT_FALSE(unigui::im::IsItemDeactivated());
    EXPECT_FALSE(unigui::im::IsItemDeactivatedAfterEdit());
    EXPECT_FALSE(unigui::im::IsItemToggledOpen());
}

TEST_F(ImTest, ItemVisibility_AfterButton_IsTrue) {
    unigui::im::Button("vis_btn");
    // In a headless frame the button is rendered but not clipped.
    EXPECT_TRUE(unigui::im::IsItemVisible());
}

TEST_F(ImTest, AnyItem_Queries_DoNotCrash) {
    unigui::im::Button("a_btn");
    (void) unigui::im::IsAnyItemHovered();
    (void) unigui::im::IsAnyItemActive();
    (void) unigui::im::IsAnyItemFocused();
}

TEST_F(ImTest, GetItemRect_AfterButton_HasPositiveSize) {
    unigui::im::Button("rect_btn");
    ImVec2 sz = unigui::im::GetItemRectSize();
    EXPECT_GT(sz.x, 0.f);
    EXPECT_GT(sz.y, 0.f);
    ImVec2 lo = unigui::im::GetItemRectMin();
    ImVec2 hi = unigui::im::GetItemRectMax();
    EXPECT_LE(lo.x, hi.x);
    EXPECT_LE(lo.y, hi.y);
}

// ── A4: Keyboard & mouse queries ──────────────────────────────────────────────

TEST_F(ImTest, KeyQueries_NoInput_ReturnFalse) {
    EXPECT_FALSE(unigui::im::IsKeyDown(ImGuiKey_A));
    EXPECT_FALSE(unigui::im::IsKeyPressed(ImGuiKey_Enter));
    EXPECT_FALSE(unigui::im::IsKeyReleased(ImGuiKey_Space));
}

TEST_F(ImTest, MouseQueries_NoInput_ReturnFalse) {
    EXPECT_FALSE(unigui::im::IsMouseDown(ImGuiMouseButton_Left));
    EXPECT_FALSE(unigui::im::IsMouseClicked(ImGuiMouseButton_Left));
    EXPECT_FALSE(unigui::im::IsMouseReleased(ImGuiMouseButton_Right));
    EXPECT_FALSE(unigui::im::IsMouseDoubleClicked(ImGuiMouseButton_Left));
    EXPECT_FALSE(unigui::im::IsMouseDragging(ImGuiMouseButton_Left));
}

TEST_F(ImTest, MousePos_ReturnsValue) {
    ImVec2 pos = unigui::im::GetMousePos();
    // Mouse pos is -FLT_MAX when no input; just verify it's a valid float.
    EXPECT_EQ(pos.x, pos.x); // NaN check
}

TEST_F(ImTest, MouseDragDelta_NoInput_IsZero) {
    ImVec2 delta = unigui::im::GetMouseDragDelta();
    EXPECT_FLOAT_EQ(delta.x, 0.f);
    EXPECT_FLOAT_EQ(delta.y, 0.f);
    unigui::im::ResetMouseDragDelta(); // must not crash
}

TEST_F(ImTest, IsMouseHoveringRect_NoInput_ReturnsFalse) {
    EXPECT_FALSE(unigui::im::IsMouseHoveringRect(ImVec2(0, 0), ImVec2(100, 100)));
}

// ── A5: Misc widgets ──────────────────────────────────────────────────────────

TEST_F(ImTest, InvisibleButton_DoesNotCrash) {
    bool clicked = unigui::im::InvisibleButton("inv_btn", ImVec2(100, 30));
    EXPECT_FALSE(clicked); // no mouse input in headless
}

TEST_F(ImTest, ArrowButton_AllDirections_DoNotCrash) {
    unigui::im::ArrowButton("arr_l", ImGuiDir_Left);
    unigui::im::ArrowButton("arr_r", ImGuiDir_Right);
    unigui::im::ArrowButton("arr_u", ImGuiDir_Up);
    unigui::im::ArrowButton("arr_d", ImGuiDir_Down);
}

TEST_F(ImTest, CheckboxFlags_Int_TogglesCorrectBit) {
    int flags = 0b0101;
    unigui::im::CheckboxFlags("bit1", &flags, 0b0010); // add bit 1 (no click → unchanged)
    EXPECT_EQ(flags, 0b0101);
}

TEST_F(ImTest, CheckboxFlags_UInt_DoesNotCrash) {
    unsigned int flags = 0xF0u;
    unigui::im::CheckboxFlags("ubits", &flags, 0x01u);
    EXPECT_EQ(flags, 0xF0u); // unchanged without interaction
}

TEST_F(ImTest, ColorButton_DoesNotCrash) {
    bool clicked = unigui::im::ColorButton("col_btn", ImVec4(1.f, 0.f, 0.f, 1.f));
    EXPECT_FALSE(clicked);
}

// ── A5: Debug windows ─────────────────────────────────────────────────────────

TEST_F(ImTest, ShowDemoWindow_DoesNotCrash) {
    bool open = true;
    unigui::im::ShowDemoWindow(&open);
}

TEST_F(ImTest, ShowMetricsWindow_DoesNotCrash) {
    bool open = true;
    unigui::im::ShowMetricsWindow(&open);
}

TEST_F(ImTest, ShowStyleEditor_DoesNotCrash) {
    unigui::im::ShowStyleEditor();
}

// ── A5: Draw-list access ──────────────────────────────────────────────────────

TEST_F(ImTest, GetWindowDrawList_ReturnsNonNull) {
    ImGui::Begin("dl_wnd");
    ImDrawList* dl = unigui::im::GetWindowDrawList();
    ASSERT_NE(dl, nullptr);
    // Draw a line via the returned pointer to confirm it is usable.
    dl->AddLine(ImVec2(0, 0), ImVec2(10, 10), IM_COL32(255, 0, 0, 255));
    ImGui::End();
}

TEST_F(ImTest, GetBackgroundAndForegroundDrawList_ReturnNonNull) {
    EXPECT_NE(unigui::im::GetBackgroundDrawList(), nullptr);
    EXPECT_NE(unigui::im::GetForegroundDrawList(), nullptr);
}

// ── A6: Text extras ───────────────────────────────────────────────────────────

TEST_F(ImTest, TextExtras_DoNotCrash) {
    ImGui::Begin("a6_text");
    unigui::im::TextUnformatted("raw unformatted text with % signs not parsed");
    unigui::im::TextLink("a link");
    unigui::im::TextLinkOpenURL("homepage"); // url defaults to nullptr
    unigui::im::TextLinkOpenURL("docs", "https://example.com");
    ImGui::End();
}

// ── A6: Tooltips ──────────────────────────────────────────────────────────────

TEST_F(ImTest, Tooltips_DoNotCrash) {
    ImGui::Begin("a6_tip");
    if (unigui::im::BeginTooltip()) {
        unigui::im::Text("inside tooltip");
        unigui::im::EndTooltip();
    }
    unigui::im::SetTooltip("one-shot tooltip");
    unigui::im::Button("hover me");
    if (unigui::im::BeginItemTooltip()) {
        unigui::im::Text("item tooltip");
        unigui::im::EndTooltip();
    }
    unigui::im::Button("hover me 2");
    unigui::im::SetItemTooltip("item one-shot");
    ImGui::End();
}

// ── A6: Disabled block ────────────────────────────────────────────────────────

TEST_F(ImTest, Disabled_BalancesStack) {
    ImGui::Begin("a6_disabled");
    unigui::im::BeginDisabled(true);
    unigui::im::Button("cannot click");
    unigui::im::EndDisabled();
    unigui::im::BeginDisabled(false); // no-op push, still must be popped
    unigui::im::Button("can click");
    unigui::im::EndDisabled();
    ImGui::End();
}

// ── A6: Combo / ListBox / Selectable ──────────────────────────────────────────

TEST_F(ImTest, BeginCombo_AndSelectable) {
    ImGui::Begin("a6_combo");
    if (unigui::im::BeginCombo("combo", "preview")) {
        unigui::im::Selectable("opt A", true);
        bool sel = false;
        unigui::im::Selectable("opt B", &sel);
        unigui::im::EndCombo();
    }
    if (unigui::im::BeginCombo("empty_preview", {})) {
        unigui::im::EndCombo();
    }
    ImGui::End();
}

TEST_F(ImTest, BeginListBox_DoesNotCrash) {
    ImGui::Begin("a6_listbox");
    if (unigui::im::BeginListBox("list", ImVec2(120, 80))) {
        unigui::im::Selectable("row 1");
        unigui::im::Selectable("row 2", true);
        unigui::im::EndListBox();
    }
    ImGui::End();
}

// ── A6: Trees & headers ───────────────────────────────────────────────────────

TEST_F(ImTest, TreeAndHeader_DoNotCrash) {
    ImGui::Begin("a6_tree");
    unigui::im::SetNextItemOpen(true);
    if (unigui::im::TreeNode("root")) {
        unigui::im::Text("child");
        unigui::im::TreePop();
    }
    if (unigui::im::TreeNodeEx("root2", ImGuiTreeNodeFlags_DefaultOpen)) {
        unigui::im::TreePop();
    }
    unigui::im::CollapsingHeader("section");
    bool visible = true;
    unigui::im::CollapsingHeader("closable", &visible);
    ImGui::End();
}

// ── A6: Tab bars ──────────────────────────────────────────────────────────────

TEST_F(ImTest, TabBar_DoesNotCrash) {
    ImGui::Begin("a6_tabs");
    if (unigui::im::BeginTabBar("tabs")) {
        if (unigui::im::BeginTabItem("Tab 1")) {
            unigui::im::Text("tab 1 body");
            unigui::im::EndTabItem();
        }
        bool open = true;
        if (unigui::im::BeginTabItem("Tab 2", &open)) {
            unigui::im::EndTabItem();
        }
        unigui::im::EndTabBar();
    }
    ImGui::End();
}

// ── A6: Plots & progress ──────────────────────────────────────────────────────

TEST_F(ImTest, PlotsAndProgress_DoNotCrash) {
    ImGui::Begin("a6_plots");
    unigui::im::ProgressBar(0.5f);
    unigui::im::ProgressBar(0.75f, ImVec2(100, 0), "75%");
    const float values[] = {0.f, 1.f, 0.5f, 0.25f, 0.9f};
    unigui::im::PlotLines("lines", values, 5);
    unigui::im::PlotHistogram("hist", values, 5, 0, "hist overlay");
    ImGui::End();
}

// ── A6: Color editors & conversion ────────────────────────────────────────────

TEST_F(ImTest, ColorEditAndPicker_DoNotCrash) {
    ImGui::Begin("a6_color");
    float c3[3] = {1.f, 0.5f, 0.f};
    float c4[4] = {0.2f, 0.4f, 0.6f, 1.f};
    unigui::im::ColorEdit3("edit3", c3);
    unigui::im::ColorEdit4("edit4", c4);
    unigui::im::ColorPicker3("pick3", c3);
    unigui::im::ColorPicker4("pick4", c4);
    ImGui::End();
}

TEST_F(ImTest, ColorConvert_RoundTrips) {
    float h = 0, s = 0, v = 0;
    unigui::im::ColorConvertRGBtoHSV(1.f, 0.f, 0.f, h, s, v);
    EXPECT_FLOAT_EQ(h, 0.f); // pure red → hue 0
    EXPECT_FLOAT_EQ(s, 1.f);
    EXPECT_FLOAT_EQ(v, 1.f);
    float r = 0, g = 0, b = 0;
    unigui::im::ColorConvertHSVtoRGB(h, s, v, r, g, b);
    EXPECT_FLOAT_EQ(r, 1.f);
    EXPECT_FLOAT_EQ(g, 0.f);
    EXPECT_FLOAT_EQ(b, 0.f);

    ImU32 packed = unigui::im::ColorConvertFloat4ToU32(ImVec4(1.f, 0.f, 0.f, 1.f));
    ImVec4 back = unigui::im::ColorConvertU32ToFloat4(packed);
    EXPECT_FLOAT_EQ(back.x, 1.f);
    EXPECT_FLOAT_EQ(back.w, 1.f);
}

// ── A6: Window-state queries ──────────────────────────────────────────────────

TEST_F(ImTest, WindowQueries_DoNotCrash) {
    ImGui::Begin("a6_winq");
    (void) unigui::im::IsWindowAppearing();
    (void) unigui::im::IsWindowCollapsed();
    (void) unigui::im::IsWindowFocused();
    (void) unigui::im::IsWindowHovered();
    ImGui::End();
}

// ── A6: Misc utilities ────────────────────────────────────────────────────────

TEST_F(ImTest, CalcTextSize_PositiveWidth) {
    ImVec2 sz = unigui::im::CalcTextSize("hello");
    EXPECT_GT(sz.x, 0.f);
    EXPECT_GT(sz.y, 0.f);
    // Text hidden after "##" is excluded from the measured width.
    ImVec2 hidden = unigui::im::CalcTextSize("hi##secret-suffix", true);
    ImVec2 shown = unigui::im::CalcTextSize("hi");
    EXPECT_FLOAT_EQ(hidden.x, shown.x);
}

TEST_F(ImTest, MiscUtilities_DoNotCrash) {
    ImGui::Begin("a6_misc");
    std::string buf = "edit me";
    unigui::im::SetKeyboardFocusHere();
    unigui::im::InputText("field", &buf);
    EXPECT_GE(unigui::im::GetTime(), 0.0);
    EXPECT_GE(unigui::im::GetFrameCount(), 1);
    unigui::im::SetMouseCursor(ImGuiMouseCursor_Hand);
    EXPECT_EQ(unigui::im::GetMouseCursor(), ImGuiMouseCursor_Hand);
    ImGui::End();
}

// ── Regression: EnterReturnsTrue must not swallow the typing ────────────────
//
// `EditString` used to write back to the caller's std::string only when the
// widget returned true. With ImGuiInputTextFlags_EnterReturnsTrue that return
// fires *only on the Enter frame*, so every keystroke was discarded and the
// field visibly emptied itself the moment it lost focus — a password box you
// could not type into. Reported 2026-07-27 against a change-password dialog.
//
// The flag is documented as changing the *return value*; nothing at the call
// site hints that it also disables persistence. Hence this test, which types
// like a human does: focus, one character per frame, and never press Enter.
namespace {

/// Runs `body` inside a real frame; returns when the frame is rendered.
void RunFrame(const std::function<void()>& body) {
    ImGui::NewFrame();
    ImGui::Begin("host");
    body();
    ImGui::End();
    ImGui::Render();
}

} // namespace

TEST(ImInputTextPersistence, EnterReturnsTrueStillKeepsTypedText) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(800, 600);
    io.DeltaTime = 1.0f / 60.0f;
    io.Fonts->Build();

    std::string value;
    const ImGuiInputTextFlags flags =
        ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue;

    // Frame 1: focus the field.
    RunFrame([&] {
        ImGui::SetKeyboardFocusHere();
        unigui::im::InputText("##pwd", &value, 128, flags);
    });

    // Frames 2..4: type "abc", one character per frame, Enter never pressed.
    for (char c : {'a', 'b', 'c'}) {
        io.AddInputCharacter(static_cast<unsigned int>(c));
        RunFrame([&] { unigui::im::InputText("##pwd", &value, 128, flags); });
    }

    // Frame 5: the field loses focus. Under the old behaviour `value` was still
    // empty here and the next frame repainted an empty box.
    RunFrame([&] {
        unigui::im::InputText("##pwd", &value, 128, flags);
        ImGui::SetKeyboardFocusHere();
        ImGui::InputText("##elsewhere", nullptr, 0);
    });

    EXPECT_EQ(value, "abc")
        << "typing was discarded: EnterReturnsTrue must not disable write-back";

    ImGui::DestroyContext();
}
