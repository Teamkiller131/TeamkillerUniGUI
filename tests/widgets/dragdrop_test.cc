// Drag-and-drop wrapper round-trip test.
//
// Regression guard for the bug where unigui::AcceptDragDrop<T> read the payload
// pointer AFTER ImGui::EndDragDropTarget() — which calls ClearDragDrop() on the
// delivery frame and nulls/frees the payload buffer — so a completed drop never
// delivered its value (the receiver kept its sentinel). The fix copies the value
// out before EndDragDropTarget(). This test scripts a full press → drag → hover →
// release gesture over pinned item rects and asserts the value actually arrives.

#include <unigui/widgets/dragdrop.h>

#include <imgui.h>

#include <gtest/gtest.h>

namespace {

class DragDropTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ctx_ = ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(800, 600);
        io.Fonts->Build();
        io.MouseDragThreshold = 1.0f; // make the synthetic drag trip immediately
    }
    void TearDown() override { ImGui::DestroyContext(ctx_); }

    // Submit a pinned drag source ("Drag me") and drop target ("Drop here"), run
    // the unigui wrappers, and return the value delivered to the target this frame
    // (or `miss` if nothing was delivered). Mouse state is scripted via the event
    // queue before NewFrame.
    static int RunFrame(float mx, float my, bool down, int payload, int miss) {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(mx, my);
        io.AddMouseButtonEvent(0, down);
        ImGui::NewFrame();
        int delivered = miss;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(800, 600));
        ImGui::Begin("dnd", nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoTitleBar);
        ImGui::SetCursorScreenPos(ImVec2(100, 100));
        ImGui::Button("Drag me", ImVec2(80, 30)); // rect center ~ (140, 115)
        unigui::BeginDragSource("dd_int", payload);
        ImGui::SetCursorScreenPos(ImVec2(400, 100));
        ImGui::Button("Drop here", ImVec2(80, 30)); // rect center ~ (440, 115)
        if (const int* p = unigui::AcceptDragDrop<int>("dd_int"))
            delivered = *p;
        ImGui::End();
        ImGui::Render();
        return delivered;
    }

    ImGuiContext* ctx_ = nullptr;
};

// A completed drag from the source onto the target must deliver the payload VALUE
// to the target. This returned -1 before the EndDragDropTarget()-ordering fix.
TEST_F(DragDropTest, DeliversPayloadValueOnDrop) {
    constexpr int kPayload = 7;
    constexpr int kMiss = -1;
    constexpr float kSrcX = 140, kSrcY = 115, kDstX = 440, kDstY = 115;

    RunFrame(kSrcX, kSrcY, false, kPayload, kMiss); // warm up: window appears
    RunFrame(kSrcX, kSrcY, false, kPayload, kMiss); // hover the source settles (hov=1)
    RunFrame(kSrcX, kSrcY, true, kPayload, kMiss);  // press on the already-hovered source
    RunFrame(kSrcX, kSrcY, true, kPayload, kMiss);  // hold in place → activation sticks
    RunFrame(250, kSrcY, true, kPayload, kMiss);    // drag past the threshold (source arms)
    RunFrame(kDstX, kDstY, true, kPayload, kMiss);  // hover the target (registers accept)
    RunFrame(kDstX, kDstY, true, kPayload, kMiss);  // hold over target one more frame
    int got = RunFrame(kDstX, kDstY, false, kPayload, kMiss); // release over target → deliver

    EXPECT_EQ(got, kPayload);
}

// With no gesture at all the target must not deliver a (garbage) value.
TEST_F(DragDropTest, NoDeliveryWithoutDrag) {
    int got = RunFrame(440, 115, false, 99, -1);
    EXPECT_EQ(got, -1);
}

} // namespace
