// v3_overview — sticky header + font scale demo (safe FontGlobalScale)
#include <unigui/unigui.h>

#include <imgui.h>

#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv) {
    int max_frames = 0;
    for (int i = 1; i < argc; i++)
        if (std::string(argv[i]) == "--frames" && i + 1 < argc)
            max_frames = std::atoi(argv[++i]);

    unigui::AppConfig cfg;
    cfg.width = 1300;
    cfg.height = 800;
    cfg.title = "UniGUI v3.2.7";
    if (!unigui::Init(cfg))
        return 1;
    std::printf("[v3] Init OK\n");
    fflush(stdout);

    struct Row {
        std::string n;
        int a, b;
        double c;
        bool today;
    };
    static std::vector<Row> data;
    if (data.empty())
        for (int i = 0; i < 80; i++)
            data.push_back({"R" + std::to_string(6000 + i), i * 100, i * 50, i * 10.0, i % 7 == 0});

    static unigui::DataTable<Row> t(
        "持仓", {{"合约", 120}, {"多头", 70}, {"空头", 70}, {"盈亏", 90}, {"今仓", 60}});
    t.SetDataSource(&data);
    t.SetCellFormatter([](int, int c, const Row& r) -> std::string {
        if (c == 0)
            return r.n;
        if (c == 1)
            return r.a > 0 ? std::to_string(r.a) : "-";
        if (c == 2)
            return r.b > 0 ? std::to_string(r.b) : "-";
        if (c == 3) {
            char b[32];
            snprintf(b, 32, "%.0f", r.c);
            return b;
        }
        return r.today ? "*" : "";
    });
    t.SetCellColor([](int, int c, const Row& r) -> ImU32 {
        if (c == 3)
            return r.c >= 0 ? IM_COL32(0, 220, 100, 255) : IM_COL32(255, 80, 80, 255);
        if (c == 4)
            return r.today ? IM_COL32(255, 200, 50, 255) : IM_COL32(150, 150, 150, 255);
        return IM_COL32(200, 200, 200, 255);
    });
    t.SetCellBold([](int, int c, const Row& r) -> bool { return c == 0 && r.today; });
    t.SetStickyHeader(true);
    t.SetVirtualScroll(true);

    static std::vector<unigui::DataTable<Row>::GroupInfo> gs;
    if (gs.empty()) {
        gs = {{.label = "第一组", .startRow = 0, .endRow = 30},
              {.label = "第二组", .startRow = 30, .endRow = 60}};
        t.SetGroups(gs);
    }

    float baseFontSize = cfg.theme.font_size;
    bool big = false;
    int frame = 0;
    while (!unigui::ShouldClose()) {
        unigui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::Begin("Ctrl", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Font: %.0fpx", big ? 24.f : 16.f);
        if (ImGui::Button(big ? "Small" : "Big")) {
            big = !big;
            ImGui::GetIO().FontGlobalScale = big ? 24.f / baseFontSize : 1.f;
        }
        ImGui::End();
        ImGui::SetNextWindowPos(ImVec2(10, 90), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(1280, 680), ImGuiCond_FirstUseEver);
        ImGui::Begin("Table", nullptr, ImGuiWindowFlags_NoSavedSettings);
        t.Render();
        ImGui::End();
        unigui::Render();
        frame++;
        if (max_frames > 0 && frame >= max_frames)
            break;
    }
    unigui::Shutdown();
    std::printf("[v3] Done %d frames\n", frame);
    fflush(stdout);
    return 0;
}
