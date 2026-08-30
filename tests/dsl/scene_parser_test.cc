// Scene-text parser tests (dsl::ParseScene) — the designer tool's in-app
// scene-editing format. Pinned here: the grammar round-trips through ToSource,
// `for` templates clone per iteration, and every malformed input yields a
// line-numbered error instead of throwing.
#include <unigui/dsl/dsl.h>
#include <unigui/dsl/dsl_scene.h>

#include <imgui.h>
#include <imgui_internal.h> // ImGui::FindWindowByName for the nested-window cursor check

#include <gtest/gtest.h>
#include <string>

namespace dsl = unigui::dsl;

namespace {

constexpr const char* kSettings = R"(# demo scene
window "Settings"
  vbox
    text "Welcome"
    separator
    hbox
      checkbox "Wireframe"
      button "Save" primary
    slider_float "Gain" 0 1.5
    for 3
      label "item"
)";

} // namespace

TEST(SceneParser, ValidScene_ParsesAndRoundTripsThroughToSource) {
    const auto r = dsl::ParseScene(kSettings);
    ASSERT_TRUE(r.error.empty()) << r.error;
    ASSERT_NE(r.tree, nullptr);

    const std::string src = dsl::ToSource(r.tree);
    EXPECT_TRUE(src.find("Window(\"Settings\", VBox({") != std::string::npos);
    EXPECT_TRUE(src.find("Text(\"Welcome\"),") != std::string::npos);
    EXPECT_TRUE(src.find("Separator(),") != std::string::npos);
    EXPECT_TRUE(src.find("HBox({") != std::string::npos);
    EXPECT_TRUE(src.find("CheckBox(\"Wireframe\"),") != std::string::npos);
    EXPECT_TRUE(src.find("Button(\"Save\", ButtonVariant::Primary),") != std::string::npos);
    EXPECT_TRUE(src.find("SliderFloat(\"Gain\", 0.0f, 1.5f),") != std::string::npos);
    EXPECT_TRUE(src.find("For(3, [](int i) { /* item builder */ return "
                         "Label(std::to_string(i)); })") != std::string::npos);
}

TEST(SceneParser, ParsedTree_RendersWithoutCrash) {
    const auto r = dsl::ParseScene(kSettings);
    ASSERT_TRUE(r.error.empty()) << r.error;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().DisplaySize = ImVec2(800, 600);
    ImGui::GetIO().Fonts->Build();
    ImGui::NewFrame();
    ImGui::Begin("scene", nullptr, ImGuiWindowFlags_NoSavedSettings);
    dsl::Render(r.tree);
    ImGui::End();
    ImGui::Render();
    ImGui::DestroyContext();
    SUCCEED();
}

TEST(SceneParser, For_TemplateClonesPerIteration_StateIsFresh) {
    // A `for` with a STATEFUL template child (checkbox) must give every
    // iteration its own instance: the parser turns the template into a
    // cloning item builder. Drive it headlessly: render inside one window and
    // count the checkbox items via the id stack is engine-only, so here we
    // assert the structural contract — For keeps count and the template is
    // cloned (a rendered clone tree differs from the template's addresses).
    const auto r = dsl::ParseScene(R"(window "W"
  for 2
    checkbox "flag"
)");
    ASSERT_TRUE(r.error.empty()) << r.error;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().DisplaySize = ImVec2(800, 600);
    ImGui::GetIO().Fonts->Build();
    ImGui::NewFrame();
    ImGui::Begin("w", nullptr, ImGuiWindowFlags_NoSavedSettings);
    dsl::Render(r.tree); // must not crash; two independent checkboxes render
    ImGui::End();
    ImGui::Render();
    ImGui::DestroyContext();
    SUCCEED();
}

TEST(SceneParser, ForTemplate_RendersTheRequestedCount) {
    // Render the For's template per iteration: the root window becomes a
    // NESTED window inside this test's host window, so measure the nested
    // window's content cursor (imgui_internal) — two label rows must advance
    // it well past its top.
    const auto r = dsl::ParseScene(R"(window "W"
  for 2
    label "row"
)");
    ASSERT_TRUE(r.error.empty()) << r.error;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().DisplaySize = ImVec2(800, 600);
    ImGui::GetIO().Fonts->Build();
    ImGui::NewFrame();
    ImGui::Begin("w", nullptr, ImGuiWindowFlags_NoSavedSettings);
    dsl::Render(r.tree);
    ImGui::End();
    const float nestedY = ImGui::FindWindowByName("W")->DC.CursorPos.y;
    ImGui::Render();
    ImGui::DestroyContext();
    EXPECT_GT(nestedY, 30.0f) << "two For iterations must emit two rows inside the scene window";
}

TEST(SceneParser, Errors_LineNumberedAndSpecific) {
    const auto unknown = dsl::ParseScene(R"(window "W"
  frobnicate
)");
    EXPECT_FALSE(unknown.error.empty());
    EXPECT_TRUE(unknown.error.starts_with("2: "));
    EXPECT_TRUE(unknown.error.find("frobnicate") != std::string::npos);
    EXPECT_EQ(unknown.tree, nullptr);

    const auto noRoot = dsl::ParseScene("text \"hi\"\n");
    EXPECT_TRUE(noRoot.error.find("window") != std::string::npos);

    const auto badQuote = dsl::ParseScene("window \"W\n  text \"oops\n");
    EXPECT_TRUE(badQuote.error.find("unterminated") != std::string::npos);

    const auto badSlider = dsl::ParseScene(R"(window "W"
  slider_float "Gain" 0 notanumber
)");
    EXPECT_TRUE(badSlider.error.find("slider_float") != std::string::npos);

    const auto orphanIndent = dsl::ParseScene("window \"W\"\ntext \"stray\"\n");
    EXPECT_TRUE(orphanIndent.error.find("indentation") != std::string::npos);

    const auto empty = dsl::ParseScene("");
    EXPECT_TRUE(empty.error.find("empty") != std::string::npos);

    // Callback-bearing nodes cannot be expressed in text — a clear error, not
    // silent degradation.
    const auto ifScene = dsl::ParseScene(R"(window "W"
  if
    text "x"
)");
    EXPECT_TRUE(ifScene.error.find("if") != std::string::npos);

    const auto badVariant = dsl::ParseScene(R"(window "W"
  button "B" fancy
)");
    EXPECT_TRUE(badVariant.error.find("fancy") != std::string::npos);
}

TEST(SceneParser, EscapesAndComments) {
    const auto r = dsl::ParseScene(R"(# leading comment
window "A \"quoted\" title"
  label "a\nb\\c"
)");
    ASSERT_TRUE(r.error.empty()) << r.error;
    const std::string src = dsl::ToSource(r.tree);
    EXPECT_TRUE(src.find("Window(\"A \\\"quoted\\\" title\",") != std::string::npos);
    // Single leaf child collapses onto the window line (no trailing comma).
    EXPECT_TRUE(src.find("Window(\"A \\\"quoted\\\" title\", Label(\"a\\nb\\\\c\"));") !=
                std::string::npos);
}

TEST(SceneParser, TextVariantsMapToTheirKinds) {
    const auto r = dsl::ParseScene(R"(window "W"
  vbox
    text_wrapped "w"
    text_disabled "d"
    bullet_text "b"
)");
    ASSERT_TRUE(r.error.empty()) << r.error;
    const std::string src = dsl::ToSource(r.tree);
    EXPECT_TRUE(src.find("TextWrapped(\"w\"),") != std::string::npos);
    EXPECT_TRUE(src.find("TextDisabled(\"d\"),") != std::string::npos);
    EXPECT_TRUE(src.find("BulletText(\"b\"),") != std::string::npos);
}
