#include <unigui/unigui.h>
#include <implot.h>
#include <cstdio>
#include <cstdlib>
#include <string_view>
#include <vector>

int main(int argc, char** argv) {
    int max_frames = 0;
    for (int i = 1; i < argc; i++) {
        std::string_view a = argv[i];
        if (a == "--frames" && i+1 < argc) max_frames = std::atoi(argv[++i]);
    }

    unigui::AppConfig cfg;
    cfg.width = 1024; cfg.height = 768;
    cfg.title = "UniGUI — Plot Demo (ImPlot)";
    if (!unigui::Init(cfg)) { std::fprintf(stderr, "Init failed\n"); return 1; }

    // Sample data
    std::vector<float> xs = {1,2,3,4,5,6,7,8,9,10};
    std::vector<float> ys = {1,4,2,7,3,9,5,11,6,13};
    std::vector<float> ys2 = {2,3,1,5,2,6,4,7,3,9};
    static float scatterX[20], scatterY[20];
    static bool first = true;
    if (first) {
        for (int i=0;i<20;i++) { scatterX[i]=i*0.5f; scatterY[i]=(float)(i%7)*1.5f+(float)i*0.2f; }
        first = false;
    }

    int frame = 0;
    while (!unigui::ShouldClose()) {
        unigui::NewFrame();

        // Line plot
        {
            ImGui::SetNextWindowSize(ImVec2(500, 350), ImGuiCond_FirstUseEver);
            ImGui::Begin("Line Plot");
            if (ImPlot::BeginPlot("##line")) {
                ImPlot::SetupAxes("X","Y");
                ImPlot::PlotLine("Series 1", xs.data(), ys.data(), (int)xs.size());
                ImPlot::PlotLine("Series 2", xs.data(), ys2.data(), (int)xs.size());
                ImPlot::EndPlot();
            }
            ImGui::End();
        }

        // Bar plot
        {
            ImGui::SetNextWindowSize(ImVec2(500, 350), ImGuiCond_FirstUseEver);
            ImGui::Begin("Bar Plot");
            if (ImPlot::BeginPlot("##bar")) {
                ImPlot::SetupAxes("Category","Value");
                ImPlot::PlotBars("Bars", ys.data(), (int)ys.size(), 0.5f);
                ImPlot::EndPlot();
            }
            ImGui::End();
        }

        // Scatter plot
        {
            ImGui::SetNextWindowSize(ImVec2(500, 350), ImGuiCond_FirstUseEver);
            ImGui::Begin("Scatter Plot");
            if (ImPlot::BeginPlot("##scatter")) {
                ImPlot::SetupAxes("X","Y");
                ImPlot::PlotScatter("Random", scatterX, scatterY, 20);
                ImPlot::EndPlot();
            }
            ImGui::End();
        }

        unigui::Render();
        frame++;
        if (max_frames > 0 && frame >= max_frames) break;
    }

    unigui::Shutdown();
    std::printf("[plot_demo] Done. %d frames.\n", frame);
    return 0;
}
