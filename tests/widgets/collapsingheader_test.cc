#include <unigui/unigui.h>
#include <unigui/widgets/collapsingheader.h>

#include <imgui.h>

#include <gtest/gtest.h>
class CollapsingHeaderTest : public ::testing::Test {
protected:
    void SetUp() override {
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
TEST_F(CollapsingHeaderTest, DefaultsToClosed) {
    unigui::CollapsingHeader ch("ch", "Header");
    EXPECT_FALSE(ch.IsOpen());
}
TEST_F(CollapsingHeaderTest, DefaultOpen_Works) {
    unigui::CollapsingHeader ch("ch", "Header", true);
    EXPECT_TRUE(ch.IsOpen());
}
TEST_F(CollapsingHeaderTest, SetOpen_Works) {
    unigui::CollapsingHeader ch("ch", "Header");
    ch.SetOpen(true);
    EXPECT_TRUE(ch.IsOpen());
    ch.SetOpen(false);
    EXPECT_FALSE(ch.IsOpen());
}
TEST_F(CollapsingHeaderTest, GetLabel_ReturnsLabel) {
    unigui::CollapsingHeader ch("ch", "MyHeader");
    EXPECT_EQ(ch.GetLabel(), "MyHeader");
}
TEST_F(CollapsingHeaderTest, Render_DoesNotCrash) {
    unigui::CollapsingHeader ch("ch", "Header");
    ch.Render();
}
TEST_F(CollapsingHeaderTest, Render_WithCallback_DoesNotCrash) {
    unigui::CollapsingHeader ch("ch", "Header", true);
    ch.SetContentCallback([]() { ImGui::Text("content"); });
    ch.Render();
}
TEST_F(CollapsingHeaderTest, Callback_FiresWhenOpen) {
    unigui::CollapsingHeader ch("ch", "Header", true);
    bool called = false;
    ch.SetContentCallback([&]() { called = true; });
    ch.Render();
    EXPECT_TRUE(called);
}
TEST_F(CollapsingHeaderTest, Callback_DoesNotFireWhenClosed) {
    unigui::CollapsingHeader ch("ch", "Header", false);
    bool called = false;
    ch.SetContentCallback([&]() { called = true; });
    ch.Render();
    EXPECT_FALSE(called);
}

// Regression: the old code passed &open_ as CollapsingHeader's p_visible (the
// close-'X' / don't-render flag), discarding the RETURN value that carries the real
// expand state. A closed header must still render its own row — with the p_visible
// misuse, open_=false submitted no item at all.
TEST_F(CollapsingHeaderTest, Header_StillRenders_WhenClosed) {
    unigui::CollapsingHeader ch("ch", "Header", false);
    // Probe by layout-cursor advance: a submitted header row moves the cursor by
    // its frame height. Under the p_visible misuse, open_=false early-outed
    // before ItemAdd — nothing was submitted, cursor unchanged.
    const float y0 = ImGui::GetCursorPosY();
    ch.Render();
    EXPECT_GT(ImGui::GetCursorPosY(), y0); // collapsed, not invisible
    EXPECT_FALSE(ch.IsOpen());
}

// Regression: SetOpen(false) must actually collapse on the next render pass and
// suppress the content callback (open state now flows through SetNextItemOpen +
// the return value, so programmatic state leads).
TEST_F(CollapsingHeaderTest, SetOpen_False_SuppressesCallback_AfterRenderPass) {
    unigui::CollapsingHeader ch("ch", "Header", true);
    int calls = 0;
    ch.SetContentCallback([&]() { ++calls; });
    ch.Render();
    EXPECT_EQ(calls, 1);
    EXPECT_TRUE(ch.IsOpen());
    // New frame, programmatically collapse: the callback must stop firing, but
    // the header row itself must still be submitted (cursor advances — under the
    // p_visible misuse SetOpen(false) made the whole header vanish).
    ImGui::Render();
    ImGui::NewFrame();
    ch.SetOpen(false);
    const float y0 = ImGui::GetCursorPosY();
    ch.Render();
    EXPECT_EQ(calls, 1);
    EXPECT_GT(ImGui::GetCursorPosY(), y0);
    EXPECT_FALSE(ch.IsOpen());
}
