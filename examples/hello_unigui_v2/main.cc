/// hello_unigui_v2: same demo, using DSL.
#include <unigui/unigui.h>
#include <unigui/dsl/dsl.h>
#include <cstdio>
#include <cstdlib>
using namespace unigui::dsl;

int main(int argc, char** argv) {
    int max_frames = 0;
    for (int i=1;i<argc;i++) if (argv[i]==std::string("--frames")&&i+1<argc) max_frames=std::atoi(argv[++i]);

    unigui::AppConfig cfg; cfg.title="Hello UniGUI DSL";
    if(!unigui::Init(cfg))return 1;
    std::printf("[DSL] Initialized\n");

    // Build UI tree ONCE
    auto ui = Window("UniGUI DSL Demo", VBox({
        Text("Welcome to UniGUI DSL!"),
        Separator(),
        Label("This UI is built with the declarative DSL."),
        Label("No manual ImGui::Begin/End calls needed."),
        Separator(),
        HBox({
            Button("Click Me", []{ unigui::Toast::Info("Clicked!"); }),
            Button("Exit",     []{ unigui::Shutdown(); std::exit(0); })
        }),
        Separator(),
        For(5, [](int i){ return Label("Item #" + std::to_string(i+1)); })
    }));

    int frame=0; bool done=false;
    while(!done&&!unigui::ShouldClose()){
        unigui::NewFrame();
        Render(ui);  // DSL render — just walk the tree
        unigui::Render(); frame++;
        if(max_frames>0&&frame>=max_frames){done=true;}
    }
    unigui::Shutdown(); std::printf("[DSL] Done\n"); return 0;
}
