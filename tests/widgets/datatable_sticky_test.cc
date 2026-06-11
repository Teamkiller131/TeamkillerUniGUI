#include <unigui/widgets/datatable.h>

#include <imgui.h>

#include <gtest/gtest.h>
struct Row {
    std::string n;
    int v;
};
TEST(DataTableSticky, Render) {
    ImGui::CreateContext();
    ImGui::GetIO().Fonts->AddFontDefault();
    ImGui::GetIO().DisplaySize = ImVec2(800, 600);
    std::vector<Row> d;
    for (int i = 0; i < 20; i++)
        d.push_back({"R" + std::to_string(i), i});
    unigui::DataTable<Row> t("st", {{"A", 100}, {"B", 60}});
    t.SetDataSource(&d);
    t.SetStickyHeader(true);
    t.SetVirtualScroll(true);
    t.SetCellFormatter([](int, int c, const Row& r) -> std::string {
        if (c == 0)
            return r.n;
        return std::to_string(r.v);
    });
    ImGui::NewFrame();
    ImGui::Begin("T");
    t.Render();
    ImGui::End();
    ImGui::Render();
    ImGui::DestroyContext();
    SUCCEED();
}
