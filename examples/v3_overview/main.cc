// v3_overview — UniGUI v3.2.7 full feature showcase
#include <unigui/unigui.h>
#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv) {
    int max_frames = 0;
    for (int i = 1; i < argc; i++)
        if (std::string(argv[i]) == "--frames" && i+1 < argc) max_frames = std::atoi(argv[++i]);

    unigui::AppConfig cfg; cfg.width = 1300; cfg.height = 800;
    cfg.title = "UniGUI v3.2.7 Widget Showcase";
    if (!unigui::Init(cfg)) return 1;
    std::printf("[v3] Init OK\n"); fflush(stdout);

    auto win = std::make_shared<unigui::Window>("main", "UniGUI v3.2.7");
    win->SetSize(1260, 760);

    // P1: DataTable — group rows + cell color + cell bold
    auto pDT = std::make_shared<unigui::Panel>("datatable",
        "DataTable<T> — Cell Color (red/green), Cell Bold (today's positions), Group Rows");
    pDT->SetContentCallback([]() {
        struct Pos { std::string sym; int lv, sv; double pnl, cost; bool today; };
        static std::vector<Pos> data;

        // Populate once
        if (data.empty()) {
            data.push_back({"第一组-A001",100,0,8000.0,5000.0,false});
            data.push_back({"第一组-A002",200,0,1200.0,5100.0,true});
            data.push_back({"第一组-A003",0,150,-300.0,4900.0,false});
            data.push_back({"第一组-A004",50,0,600.0,5050.0,true});
            data.push_back({"第一组-A005",0,0,0.0,5000.0,false});
            data.push_back({"第二组-B001",0,300,-500.0,4800.0,false});
            data.push_back({"第二组-B002",150,0,900.0,5100.0,true});
            data.push_back({"第二组-B003",200,100,400.0,5000.0,false});
            data.push_back({"第二组-B004",0,0,0.0,5000.0,false});
            data.push_back({"独立行-C001",300,0,1500.0,5200.0,true});
            data.push_back({"独立行-C002",100,200,-200.0,4900.0,false});
        }

        static char filter[64] = "";
        ImGui::InputTextWithHint("##f", "Filter...", filter, sizeof(filter));

        static unigui::DataTable<Pos> t("持仓",{{"属性1",110},{"属性2",60},{"属性3",60},{"属性4",80},{"属性5",80}});
        t.SetDataSource(&data);
        t.SetMultiSelect(true); t.SetColumnAutoWidth(0, true);
        t.SetCellFormatter([](int,int c,const Pos& p)->std::string {
            switch(c){case 0:return p.sym;case 1:return p.lv>0?std::to_string(p.lv):"-";
            case 2:return p.sv>0?std::to_string(p.sv):"-";
            case 3:{char b[32];snprintf(b,32,"%.0f",p.pnl);return b;}
            case 4:{char b[32];snprintf(b,32,"%.0f",p.cost);return b;}}return "-";
        });
        // Cell-level color: green for positive, red for negative
        t.SetCellColor([](int,int col,const Pos& p)->ImU32 {
            if (col==3) return p.pnl>=0 ? IM_COL32(0,220,100,255) : IM_COL32(255,80,80,255);
            if (col==1) return p.lv>0 ? IM_COL32(255,80,80,255) : IM_COL32(180,180,180,255);
            if (col==2) return p.sv>0 ? IM_COL32(0,220,100,255) : IM_COL32(180,180,180,255);
            return IM_COL32(200,200,200,255);
        });
        // Bold: highlight today's positions
        t.SetCellBold([](int,int,const Pos& p)->bool { return p.today; });
        t.SetRowColor([](int,const Pos& p){return p.pnl>=0?IM_COL32(0,80,40,40):IM_COL32(80,20,20,40);});
        if(filter[0]) t.SetFilterText(filter); else t.SetFilterText("");

        static std::vector<unigui::DataTable<Pos>::GroupInfo> groups;
        if (groups.empty()) {
            groups = {{"第一组",0,5},{.label="第二组",.startRow=5,.endRow=9}};
            t.SetGroups(groups);
        }
        t.SetStickyHeader(true);
        t.Render();
    });
    win->AddPanel(pDT);

    // TreeView + Buttons + Badge (compact)
    auto pTV = std::make_shared<unigui::Panel>("treeview", "TreeView + Buttons + Badge");
    pTV->SetContentCallback([]() {
        static unigui::TreeNode root;
        static bool b=false;
        if(!b){root.label="Groups";
        unigui::TreeNode g1{"Group A",{},false};g1.children.push_back({"A-001 | 65%",{},false});g1.children.push_back({"A-002 | 42%",{},false});
        root.children.push_back(g1);b=true;}
        static unigui::TreeView tv("tree");tv.SetRoot(root);
        static std::vector<float> us={0.65f,0.42f};static int ui=0;
        tv.SetNodeRenderer([](int,int d,const unigui::TreeNode&){if(d>0&&ui<(int)us.size()){ImGui::SameLine(ImGui::GetWindowWidth()-180);ImGui::ProgressBar(us[ui],ImVec2(120,14),"");ImGui::SameLine();ImGui::Text("%.0f%%",us[ui]*100);ui++;}});
        ui=0;tv.Render();
        ImGui::Separator();
        static unigui::Button b1("b1","Primary");b1.SetColorVariant(unigui::Button::Primary);b1.Render();ImGui::SameLine();
        static unigui::Button b2("b2","Danger");b2.SetColorVariant(unigui::Button::Danger);b2.Render();ImGui::SameLine();
        static unigui::Button b3("b3","Success");b3.SetColorVariant(unigui::Button::Success);b3.Render();
        if(b1.WasClicked())unigui::Toast::Info("Primary");if(b2.WasClicked())unigui::Toast::Error("Danger");if(b3.WasClicked())unigui::Toast::Success("Success");
        // Font size toggle
        ImGui::SameLine();static unigui::Button bFont("bfont","Font 24px");
        bFont.Render();static bool bigFont=false;
        if(bFont.WasClicked()){bigFont=!bigFont;unigui::ThemeConfig tc;tc.font_size=bigFont?24.f:16.f;unigui::ApplyTheme(tc);}
        ImGui::SameLine(400);
        static unigui::Badge dot("");dot.SetVariant(unigui::Badge::Dot);dot.Render();ImGui::SameLine(20);
        static unigui::Badge cnt("");cnt.SetCount(12);cnt.SetColor(IM_COL32(233,69,96,255));cnt.Render();ImGui::SameLine(60);
        static unigui::Badge lbl("NEW");lbl.SetColor(IM_COL32(0,180,100,255));lbl.Render();
    });
    win->AddPanel(pTV);

    int frame=0; bool first=true;
    while(!unigui::ShouldClose()){unigui::NewFrame();if(first){win->RestoreLayout(R"({"x":20,"y":20,"w":1260,"h":760})");first=false;}win->Render();unigui::Render();frame++;if(max_frames>0&&frame>=max_frames)break;}
    unigui::Shutdown();std::printf("[v3] Done %d frames\n",frame);fflush(stdout);return 0;
}
