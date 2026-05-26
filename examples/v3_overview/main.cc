/// v3_overview — comprehensive demo of all v3.0 features
#include <unigui/unigui.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>

int main(int argc, char** argv) {
    int max_frames = 0;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--frames" && i+1 < argc)
            max_frames = std::atoi(argv[++i]);
    }

    unigui::AppConfig cfg;
    cfg.width = 1400; cfg.height = 900;
    cfg.title = "UniGUI v3.0 — Overview Demo";
    if (!unigui::Init(cfg)) return 1;
    std::printf("[v3_overview] Init OK\n");

    int frame = 0;
    while (!unigui::ShouldClose()) {
        unigui::NewFrame();

        // ── Main window ──────────────────────────────────────────────────
        ImGui::SetNextWindowSize(ImVec2(1360, 860), ImGuiCond_FirstUseEver);
        ImGui::Begin("UniGUI v3.0 Features", nullptr,
                     ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse);

        // ══════════════════════════════════════════════════════════════════
        // Menu: Theme Switcher
        // ══════════════════════════════════════════════════════════════════
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Theme")) {
                auto names = unigui::theme::ThemeRegistry::Instance().List();
                for (auto& n : names) {
                    if (ImGui::MenuItem(n.c_str()))
                        unigui::theme::ThemeRegistry::Instance().Apply(n);
                }
                ImGui::EndMenu();
            }
            ImGui::Text(" | Frame: %d", frame);
            ImGui::EndMenuBar();
        }

        // ══════════════════════════════════════════════════════════════════
        // Tab: Easing Curves
        // ══════════════════════════════════════════════════════════════════
        if (ImGui::CollapsingHeader("Easing Curves (10 curves)", ImGuiTreeNodeFlags_DefaultOpen)) {
            static float t = 0.f;
            ImGui::SliderFloat("Time", &t, 0.f, 1.f, "%.3f");
            if (ImGui::Button("Animate")) t = 0.f;

            static std::pair<unigui::fx::EasingCurve, const char*> curves[] = {
                {unigui::fx::EasingCurve::Linear,    "Linear"},
                {unigui::fx::EasingCurve::QuadIn,    "QuadIn"},
                {unigui::fx::EasingCurve::QuadOut,   "QuadOut"},
                {unigui::fx::EasingCurve::QuadInOut, "QuadInOut"},
                {unigui::fx::EasingCurve::CubicIn,   "CubicIn"},
                {unigui::fx::EasingCurve::CubicOut,  "CubicOut/ease"},
                {unigui::fx::EasingCurve::ExpoIn,    "ExpoIn"},
                {unigui::fx::EasingCurve::ExpoOut,   "ExpoOut"},
                {unigui::fx::EasingCurve::ElasticOut,"ElasticOut"},
                {unigui::fx::EasingCurve::BounceOut, "BounceOut"},
            };

            auto* dl = ImGui::GetWindowDrawList();
            ImVec2 orig = ImGui::GetCursorScreenPos();
            float graphW = ImGui::GetContentRegionAvail().x - 20;
            float graphH = 40.f;

            for (int i = 0; i < 10; i++) {
                ImVec2 off(orig.x + 120, orig.y + i * 48.f + 8);
                ImGui::SetCursorScreenPos(ImVec2(orig.x, off.y));
                ImGui::Text("%s", curves[i].second);

                // Draw curve preview
                float y0 = off.y + graphH;
                for (float x = 0; x <= graphW; x += 2.f) {
                    float px = x / graphW;
                    float py = unigui::fx::Ease(px, curves[i].first);
                    dl->AddRectFilled(ImVec2(off.x + x, y0 - py * graphH),
                                     ImVec2(off.x + x + 2.f, y0 - py * graphH + 2.f),
                                     IM_COL32(40, 49, 237, 200));
                }
                // Current t marker
                float mx = t * graphW;
                float my = unigui::fx::Ease(t, curves[i].first) * graphH;
                dl->AddCircleFilled(ImVec2(off.x + mx, y0 - my), 5.f, IM_COL32(255, 255, 255, 255));

                ImGui::SetCursorScreenPos(ImVec2(off.x, off.y + graphH + 4));
                ImGui::Dummy(ImVec2(graphW, 0));
            }
            t += ImGui::GetIO().DeltaTime * 0.4f;
            if (t > 1.f) t -= 1.f;
        }

        // ══════════════════════════════════════════════════════════════════
        // Tab: Visual Effects
        // ══════════════════════════════════════════════════════════════════
        if (ImGui::CollapsingHeader("Visual Effects (Shadow, Glow, Blur, Gradient)")) {
            ImVec2 colW = ImGui::GetContentRegionAvail();
            float panelW = (colW.x - 30) / 2.f;

            // Shadow
            ImGui::BeginChild("shadow", ImVec2(panelW, 120), ImGuiChildFlags_Borders);
            {
                auto* dl = ImGui::GetWindowDrawList();
                ImVec2 c = ImGui::GetCursorScreenPos();
                ImVec2 size(panelW - 20, 80);
                unigui::fx::ShadowEffect sh(8.f, 3.f, 3.f, IM_COL32(0,0,0,80), 3);
                sh.SetRect(c, ImVec2(c.x + size.x, c.y + size.y));
                sh.Push(dl);
                dl->AddRectFilled(c, ImVec2(c.x + size.x, c.y + size.y),
                                 IM_COL32(45,45,55,255), 6.f);
                sh.Pop();
                dl->AddText(ImVec2(c.x + 10, c.y + 10), IM_COL32_WHITE, "ShadowEffect");
            }
            ImGui::EndChild();
            ImGui::SameLine();

            // Glow
            ImGui::BeginChild("glow", ImVec2(panelW, 120), ImGuiChildFlags_Borders);
            {
                auto* dl = ImGui::GetWindowDrawList();
                ImVec2 c = ImGui::GetCursorScreenPos();
                ImVec2 size(panelW - 20, 80);
                unigui::fx::GlowEffect gw(10.f, IM_COL32(100,149,237,100), 4);
                gw.SetRect(c, ImVec2(c.x + size.x, c.y + size.y));
                gw.Push(dl);
                dl->AddRectFilled(c, ImVec2(c.x + size.x, c.y + size.y),
                                 IM_COL32(45,45,55,255), 6.f);
                gw.Pop();
                dl->AddText(ImVec2(c.x + 10, c.y + 10), IM_COL32_WHITE, "GlowEffect");
            }
            ImGui::EndChild();

            // Gradient
            ImGui::Spacing();
            ImGui::BeginChild("gradient", ImVec2(colW.x - 15, 40));
            {
                auto* dl = ImGui::GetWindowDrawList();
                ImVec2 c = ImGui::GetCursorScreenPos();
                ImVec2 size(colW.x - 30, 30);
                unigui::fx::GradientBrush::Horizontal(dl, c,
                    ImVec2(c.x + size.x, c.y + size.y),
                    IM_COL32(40,49,237,255), IM_COL32(233,69,96,255));
                dl->AddText(ImVec2(c.x + 10, c.y + 8), IM_COL32_WHITE, "GradientBrush::Horizontal");
            }
            ImGui::EndChild();

            // Glass
            ImGui::Spacing();
            ImGui::BeginChild("glass-bg", ImVec2(colW.x - 15, 80));
            {
                // Colorful bg to show through
                auto* dl = ImGui::GetWindowDrawList();
                ImVec2 c = ImGui::GetCursorScreenPos();
                ImVec2 size(colW.x - 30, 60);
                dl->AddRectFilled(c, ImVec2(c.x + size.x, c.y + size.y),
                                 IM_COL32(40,49,237,180), 8.f);
                // Glass panel on top
                unigui::fx::BlurEffect g(12.f, 0.18f, IM_COL32(255,255,255,40));
                g.SetRect(ImVec2(c.x + 15, c.y + 5),
                         ImVec2(c.x + size.x - 15, c.y + size.y - 5));
                g.Push(dl);
                g.Pop();
                dl->AddText(ImVec2(c.x + 25, c.y + 20), IM_COL32(255,255,255,200), "BlurEffect / GlassPanel");
            }
            ImGui::EndChild();
        }

        // ══════════════════════════════════════════════════════════════════
        // Tab: Card + Shimmer + Badge
        // ══════════════════════════════════════════════════════════════════
        if (ImGui::CollapsingHeader("New Widgets (Card, Shimmer, Badge, Skeleton)")) {
            ImVec2 colW = ImGui::GetContentRegionAvail();
            float w = (colW.x - 20) / 3.f;

            // Card
            ImGui::BeginChild("card-demo", ImVec2(w, 180), ImGuiChildFlags_Borders);
            {
                static unigui::Card card("Feature Card");
                card.SetShadowRadius(6.f);
                card.SetContent([]() {
                    ImGui::TextWrapped("Cards support titles, content regions, and footers with drop shadows.");
                    static int n = 0;
                    if (ImGui::Button("Count")) n++;
                    ImGui::SameLine(); ImGui::Text("%d clicks", n);
                });
                card.SetFooter([]() { ImGui::TextDisabled("v3.0 Card"); });
                card.Render();
            }
            ImGui::EndChild();
            ImGui::SameLine();

            // Shimmer + Skeleton
            ImGui::BeginChild("shimmer-demo", ImVec2(w, 180), ImGuiChildFlags_Borders);
            {
                ImGui::Text("Shimmer Loading:");
                static unigui::Shimmer sh;
                static bool init = true;
                if (init) {
                    sh.AddBlock(200, 14, 0, 0);
                    sh.AddBlock(160, 14, 0, 20);
                    sh.AddBlock(120, 14, 0, 40);
                    sh.AddCircle(30, 0, 60);
                    sh.AddBlock(180, 30, 38, 60);
                    sh.SetSpeed(1.2f); sh.Start();
                    init = false;
                }
                if (ImGui::Button(sh.IsPlaying() ? "Stop" : "Start")) {
                    sh.IsPlaying() ? sh.Stop() : sh.Start();
                }
                ImGui::Dummy(ImVec2(0,4));
                sh.Render();
            }
            ImGui::EndChild();
            ImGui::SameLine();

            // Badge
            ImGui::BeginChild("badge-demo", ImVec2(w, 180), ImGuiChildFlags_Borders);
            {
                ImGui::Text("Badges:");
                static unigui::Badge dot(""); dot.SetVariant(unigui::Badge::Dot);
                static unigui::Badge cnt(""); cnt.SetCount(12);
                cnt.SetColor(IM_COL32(233,69,96,255));
                static unigui::Badge lbl("NEW"); lbl.SetColor(IM_COL32(0,180,100,255));
                static unigui::Badge beta("BETA"); beta.SetColor(IM_COL32(100,149,237,255));

                ImGui::Text("Dot:"); ImGui::SameLine(); dot.Render();
                ImGui::SameLine(70); ImGui::Text("Count:"); ImGui::SameLine(); cnt.Render();
                ImGui::Text("Label:"); ImGui::SameLine(); lbl.Render();
                ImGui::SameLine(100); beta.Render();

                // Skeleton
                ImGui::Spacing();
                ImGui::Text("Skeleton:");
                static unigui::SkeletonScreen sk = unigui::SkeletonScreen::FromSize(180, 100, 3);
                sk.Render();
            }
            ImGui::EndChild();
        }

        // ══════════════════════════════════════════════════════════════════
        // Tab: Gradient Text + Animated Button + Toast
        // ══════════════════════════════════════════════════════════════════
        if (ImGui::CollapsingHeader("Typography + Buttons + Toast")) {
            ImVec2 colW = ImGui::GetContentRegionAvail();

            // Gradient Text
            ImGui::Text("Gradient Text:");
            unigui::GradientText::RenderHex("UniGUI v3.0 — Beautiful UI Framework",
                                            40, 49, 237, 233, 69, 96);

            // Animated Button
            ImGui::Spacing();
            ImGui::Text("Animated Buttons (hover to see smooth transition):");
            static unigui::Button btn1("b1", "Primary"); btn1.SetColorVariant(unigui::Button::Primary);
            static unigui::Button btn2("b2", "Danger");  btn2.SetColorVariant(unigui::Button::Danger);
            static unigui::Button btn3("b3", "Success"); btn3.SetColorVariant(unigui::Button::Success);
            static unigui::Button btn4("b4", "Default");

            btn1.Render(); ImGui::SameLine();
            btn2.Render(); ImGui::SameLine();
            btn3.Render(); ImGui::SameLine();
            btn4.Render();

            if (btn1.WasClicked()) unigui::Toast::Info("Primary button clicked!");
            if (btn2.WasClicked()) unigui::Toast::Error("Danger button clicked!");
            if (btn3.WasClicked()) unigui::Toast::Success("Success button clicked!");
            if (btn4.WasClicked()) unigui::Toast::Warn("Default button clicked!");

            // Toast trigger
            ImGui::Spacing();
            ImGui::Text("Toast Notifications:");
            if (ImGui::Button("Toast Info"))    unigui::Toast::Info("Info toast — animated fade-in");
            ImGui::SameLine();
            if (ImGui::Button("Toast Success")) unigui::Toast::Success("Success! Operation completed.");
            ImGui::SameLine();
            if (ImGui::Button("Toast Warn"))    unigui::Toast::Warn("Warning! Check your input.");
            ImGui::SameLine();
            if (ImGui::Button("Toast Error"))   unigui::Toast::Error("Error! Something went wrong.");
        }

        // ══════════════════════════════════════════════════════════════════
        // Tab: Animation Demo
        // ══════════════════════════════════════════════════════════════════
        if (ImGui::CollapsingHeader("Animation State + Transition")) {
            static unigui::fx::AnimationState fadeAnim;
            static unigui::fx::AnimationState slideAnim;
            static unigui::fx::AnimationState bounceAnim;

            ImVec2 colW = ImGui::GetContentRegionAvail();

            if (ImGui::Button("Fade In"))  fadeAnim.Play(0.5f, unigui::fx::EasingCurve::CubicOut);
            ImGui::SameLine();
            if (ImGui::Button("Slide In")) slideAnim.Play(0.5f, unigui::fx::EasingCurve::ExpoOut);
            ImGui::SameLine();
            if (ImGui::Button("Bounce"))   bounceAnim.Play(0.8f, unigui::fx::EasingCurve::BounceOut);

            float dt = ImGui::GetIO().DeltaTime;

            // Fade
            float a = fadeAnim.Update(dt);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, a);
            ImGui::BeginChild("fade-box", ImVec2(colW.x - 15, 30));
            ImGui::TextColored(ImVec4(1,1,0,1), "Fading text — alpha: %.2f", a);
            ImGui::EndChild();
            ImGui::PopStyleVar();

            // Slide
            float s = unigui::fx::Transition::SlideIn(slideAnim, -100.f, 0.3f,
                                                       unigui::fx::EasingCurve::ExpoOut, dt);
            ImGui::BeginChild("slide-box", ImVec2(colW.x - 15, 25));
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + s);
            ImGui::TextColored(ImVec4(0.5f,1,0.5f,1), "Sliding text — offset: %.0f", s);
            ImGui::EndChild();

            // Bounce scale
            float sc = unigui::fx::Transition::Scale(bounceAnim, 0.3f, 1.0f, 0.8f,
                                                      unigui::fx::EasingCurve::BounceOut, dt);
            ImGui::BeginChild("bounce-box", ImVec2(colW.x - 15, 30));
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40);
            auto* dl = ImGui::GetWindowDrawList();
            ImVec2 c = ImGui::GetCursorScreenPos();
            dl->AddCircleFilled(ImVec2(c.x + 10*sc, c.y + 12), 10.f * sc,
                               IM_COL32(233,69,96,200));
            ImGui::Dummy(ImVec2(60, 24));
            ImGui::SameLine();
            ImGui::Text("Bounce scale: %.2f", sc);
            ImGui::EndChild();
        }

        ImGui::End(); // main window

        unigui::Render(); frame++;
        if (max_frames > 0 && frame >= max_frames) break;
    }

    unigui::Shutdown();
    std::printf("[v3_overview] Done — %d frames\n", frame);
    return 0;
}
