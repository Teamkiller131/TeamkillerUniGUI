#!/usr/bin/env python3
"""Generate docs/WIDGET_EXAMPLES.md from widget catalog."""
from pathlib import Path

W = [
    ("AlertBar", "alertbar.h", "AlertBar",
     'auto bar = std::make_shared<unigui::AlertBar>("net", "Relay disconnected");\n'
     "bar->WithSeverity(unigui::AlertBar::Severity::Warning).Render();", "alertbar"),
    ("Animate", "animate.h", "Animate",
     "float a = unigui::Animate::FadeIn(0.25f);\n"
     "ImGui::PushStyleVar(ImGuiStyleVar_Alpha, a);\n"
     'ImGui::Text("Fading in");\n'
     "ImGui::PopStyleVar();", "animate-uniguianimate"),
    ("Badge", "badge.h", "Badge",
     'unigui::Badge badge("new");\n'
     "badge.SetVariant(unigui::Badge::Count).SetCount(3);\n"
     'ImGui::Button("Inbox");\n'
     "badge.Render();  // after parent", "badge--tag"),
    ("Breadcrumb", "breadcrumb.h", "Breadcrumb",
     'auto bc = std::make_shared<unigui::Breadcrumb>("nav");\n'
     'bc->SetItems({{"Home", []{}}, {"Settings", []{}}});\n'
     "bc->Render();", "breadcrumb"),
    ("Button", "button.h", "Button",
     'auto btn = std::make_shared<unigui::Button>("save", "Save");\n'
     "btn->WithPrimary().Render();\n"
     "if (btn->WasClicked()) { /* ... */ }", "button"),
    ("Card", "card.h", "Card",
     'auto card = std::make_shared<unigui::Card>("card");\n'
     'card->SetChild([]{ ImGui::Text("Content"); });\n'
     "card->Render();", "card"),
    ("CascadingCombo", "cascadingcombo.h", "CascadingCombo",
     'auto cc = std::make_shared<unigui::CascadingCombo>("region");\n'
     'cc->SetLevels({{"省", {"江苏"}}, {"市", {"南京"}}});\n'
     "cc->WithLayout(unigui::CascadingCombo::Layout::Horizontal).Render();", "cascadingcombo"),
    ("CheckBox", "checkbox.h", "CheckBox",
     "static bool on = true;\n"
     'auto cb = std::make_shared<unigui::CheckBox>("cb", "Enable", &on);\n'
     "cb->Render();", "checkbox"),
    ("Clipboard", "clipboard.h", "Clipboard",
     'unigui::Clipboard::SetText("copy me");\n'
     "std::string s = unigui::Clipboard::GetText();", "clipboard-uniguiclipboard"),
    ("CollapsingHeader", "collapsingheader.h", "CollapsingHeader",
     'auto h = std::make_shared<unigui::CollapsingHeader>("adv", "Advanced");\n'
     'h->SetChild([]{ ImGui::Text("Details"); });\n'
     "h->Render();", "collapsingheader"),
    ("ColorEdit", "coloredit.h", "ColorEdit",
     "static ImVec4 col{1, 0, 0, 1};\n"
     'auto ce = std::make_shared<unigui::ColorEdit>("c", &col);\n'
     "ce->Render();", "coloredit--colorpicker"),
    ("ColorPicker", "colorpicker.h", "ColorPicker",
     "static ImVec4 col{0.2f, 0.5f, 0.9f, 1};\n"
     'auto cp = std::make_shared<unigui::ColorPicker>("pick", &col);\n'
     "cp->Render();", "coloredit--colorpicker"),
    ("ComboBox", "combobox.h", "ComboBox",
     'auto box = std::make_shared<unigui::ComboBox>("sym", std::vector<std::string>{"IF", "IC"});\n'
     "box->Render();", "combobox"),
    ("ConfirmDialog", "confirmdialog.h", "ConfirmDialog",
     'unigui::ConfirmDialog::Show("Delete?", "Cannot undo", []{ erase(); });', "confirmdialog"),
    ("ContextMenu", "contextmenu.h", "ContextMenu",
     'static unigui::ContextMenu menu("ctx");\n'
     'menu.AddItem("Copy", []{ /* ... */ });\n'
     "menu.Render();", "tooltip--contextmenu"),
    ("DataTable", "datatable.h", "DataTable",
     "struct Row { std::string n; double v; };\n"
     'static std::vector<Row> rows{{"A", 1.0}};\n'
     'static unigui::DataTable<Row> t("t", {{"Name", 120}});\n'
     "t.SetDataSource(&rows);\n"
     "t.Render();", "datatablet"),
    ("DatePicker", "datepicker.h", "DatePicker",
     "static int y = 2026, m = 6, d = 4;\n"
     'auto dp = std::make_shared<unigui::DatePicker>("d", &y, &m, &d);\n'
     "dp->Render();", "datepicker"),
    ("Dialog", "dialog.h", "Dialog",
     'static unigui::Dialog dlg("dlg", "Title");\n'
     'dlg.SetContent([]{ ImGui::Text("Body"); });\n'
     "dlg.SetOpen(true);\n"
     "dlg.Render();", "dialog"),
    ("DirPath", "dirpath.h", "DirPath",
     "static std::string path;\n"
     'auto p = std::make_shared<unigui::DirPath>("out", &path);\n'
     "p->Render();", "filepath--dirpath"),
    ("DragDrop", "dragdrop.h", "DragDrop",
     'unigui::DragDrop::Source("payload", []{ return std::vector<uint8_t>{1, 2, 3}; });\n'
     'unigui::DragDrop::Target("payload", [](auto& bytes){ /* ... */ });',
     "dragdrop-uniguidragdrop"),
    ("DragFloat", "dragfloat.h", "DragFloat",
     "static float v = 1.f;\n"
     'auto d = std::make_shared<unigui::DragFloat>("df", &v, 0.1f, 0.f, 10.f);\n'
     "d->Render();", "dragfloatt--dragintt"),
    ("DragInt", "dragint.h", "DragInt",
     "static int v = 5;\n"
     'auto d = std::make_shared<unigui::DragInt>("di", &v, 1, 0, 100);\n'
     "d->Render();", "dragfloatt--dragintt"),
    ("FilePath", "filepath.h", "FilePath",
     "static std::string file;\n"
     'auto fp = std::make_shared<unigui::FilePath>("in", &file);\n'
     "fp->Render();", "filepath--dirpath"),
    ("Form", "form.h", "Form",
     'static unigui::Form form("login");\n'
     'form.AddField("user", []{ /* InputText */ });\n'
     "form.Render();", "form"),
    ("FuturesRiskBar", "futuresriskbar.h", "FuturesRiskBar",
     'auto bar = std::make_shared<unigui::FuturesRiskBar>("risk");\n'
     "bar->SetRatio(0.72f).Render();", "futuresriskbar"),
    ("GroupBox", "groupbox.h", "GroupBox",
     'auto g = std::make_shared<unigui::GroupBox>("g", "Group");\n'
     'g->SetChild([]{ ImGui::Text("Inside"); });\n'
     "g->Render();", "groupbox"),
    ("HeroSection", "herosection.h", "HeroSection",
     'auto hero = std::make_shared<unigui::HeroSection>("hero");\n'
     'hero->SetTitle("UniGUI").SetSubtitle("C++23 UI").Render();', "herosection"),
    ("Hyperlink", "hyperlink.h", "Hyperlink",
     'auto link = std::make_shared<unigui::Hyperlink>("docs", "Documentation");\n'
     'link->SetURL("https://github.com/Teamkiller131/TeamkillerUniGUI").Render();', "hyperlink"),
    ("IconButton", "iconbutton.h", "IconButton",
     'auto ib = std::make_shared<unigui::IconButton>("gear", "\\ue001");\n'
     "ib->Render();", "iconbutton"),
    ("Image", "image.h", "Image",
     'auto img = std::make_shared<unigui::Image>("logo");\n'
     "img->SetTexture(myTexId, ImVec2(128, 128)).Render();", "image"),
    ("ImageButton", "imagebutton.h", "ImageButton",
     'auto ib = std::make_shared<unigui::ImageButton>("ib", myTexId, ImVec2(32, 32));\n'
     "ib->Render();", "imagebutton"),
    ("InputFloat", "inputfloat.h", "InputFloat",
     "static float v = 0;\n"
     'auto in = std::make_shared<unigui::InputFloat>("f", &v);\n'
     "in->Render();", "inputint--inputfloat"),
    ("InputInt", "inputint.h", "InputInt",
     "static int v = 0;\n"
     'auto in = std::make_shared<unigui::InputInt>("i", &v);\n'
     "in->Render();", "inputint--inputfloat"),
    ("InputText", "inputtext.h", "InputText",
     "static std::string s;\n"
     'auto t = std::make_shared<unigui::InputText>("name", &s);\n'
     "t->Render();", "inputtext"),
    ("Label", "label.h", "Label",
     'auto lbl = std::make_shared<unigui::Label>("lbl", "Hello");\n'
     "lbl->Render();", "label"),
    ("Layout (HBox/VBox)", "layout.h", "HBox",
     "{ unigui::HBox row(8.f);\n"
     '  std::make_shared<unigui::Button>("a", "A")->Render();\n'
     '  std::make_shared<unigui::Button>("b", "B")->Render();\n'
     "}", "hbox--vbox-layouth--raii-helpers"),
    ("LineEdit", "lineedit.h", "LineEdit",
     "static std::string s;\n"
     'auto le = std::make_shared<unigui::LineEdit>("le", &s);\n'
     "le->WithUndoRedo().Render();", "lineedit"),
    ("ListBox", "listbox.h", "ListBox",
     "static int sel = 0;\n"
     'auto lb = std::make_shared<unigui::ListBox>("lb", std::vector<std::string>{"A", "B"}, &sel);\n'
     "lb->Render();", "listbox"),
    ("ListView", "listview.h", "ListView",
     'auto lv = std::make_shared<unigui::ListView>("lv");\n'
     'lv->SetItems({"Row1", "Row2"});\n'
     "lv->Render();", "listview"),
    ("LoadingIndicator", "loadingindicator.h", "LoadingIndicator",
     'auto sp = std::make_shared<unigui::LoadingIndicator>("load");\n'
     "sp->Render();", "loadingindicator"),
    ("Markdown", "markdown.h", "Markdown",
     'auto md = std::make_shared<unigui::Markdown>("md");\n'
     'md->SetText("# Title\\n- item").Render();', "markdown"),
    ("MenuBar", "menubar.h", "MenuBar",
     'static unigui::MenuBar bar("mb");\n'
     'bar.AddMenu("File", {{"Quit", []{ std::exit(0); }}});\n'
     "bar.Render();", "menubar"),
    ("MultiCombo", "multicombo.h", "MultiCombo",
     "static std::vector<bool> sel{true, false};\n"
     'auto mc = std::make_shared<unigui::MultiCombo>("mc", std::vector<std::string>{"A", "B"}, &sel);\n'
     "mc->Render();", "multicombo"),
    ("MultiHandleSlider", "multihandleslider.h", "MultiHandleSlider",
     "static std::vector<float> pts{0.2f, 0.8f};\n"
     'auto mh = std::make_shared<unigui::MultiHandleSlider>("mh", &pts, 0.f, 1.f);\n'
     "mh->Render();", "multihandleslider"),
    ("MultiLine", "multiline.h", "MultiLine",
     "static std::string text;\n"
     'auto ml = std::make_shared<unigui::MultiLine>("ml", &text);\n'
     "ml->Render();", "multiline"),
    ("MultiSplitter", "multisplitter.h", "MultiSplitter",
     'static unigui::MultiSplitter sp("sp", unigui::MultiSplitter::Horizontal);\n'
     'sp.AddPanel(0.3f, []{ ImGui::Text("L"); });\n'
     'sp.AddPanel(0.7f, []{ ImGui::Text("R"); });\n'
     "sp.Render();", "multisplitter"),
    ("Notification", "notification.h", "Notification",
     'unigui::Notification::Push("Info", "Connected");\n'
     "unigui::Notification::RenderAll();", "notification"),
    ("Panel", "panel.h", "Panel",
     'auto p = std::make_shared<unigui::Panel>("dock");\n'
     'p->SetContent([]{ ImGui::Text("Panel"); });\n'
     "p->Render();", "panel"),
    ("PanelBox", "panelbox.h", "PanelBox",
     'auto pb = std::make_shared<unigui::PanelBox>("pb", "Account");\n'
     'pb->SetChild([]{ ImGui::Text("Body"); });\n'
     "pb->Render();", "panelbox"),
    ("PasswordInput", "passwordinput.h", "PasswordInput",
     "static std::string pwd;\n"
     'auto pw = std::make_shared<unigui::PasswordInput>("pw", &pwd);\n'
     "pw->Render();", "passwordinput"),
    ("ProgressBar", "progressbar.h", "ProgressBar",
     'auto pb = std::make_shared<unigui::ProgressBar>("p", 0.65f);\n'
     "pb->Render();", "progressbar"),
    ("PropertyGrid", "propertygrid.h", "PropertyGrid",
     'static unigui::PropertyGrid grid("props");\n'
     'grid.AddBool("Enabled", &flag);\n'
     "grid.Render();", "propertygrid"),
    ("RadioGroup", "radiogroup.h", "RadioGroup",
     "static int choice = 0;\n"
     'auto rg = std::make_shared<unigui::RadioGroup>("rg", std::vector<std::string>{"A", "B"}, &choice);\n'
     "rg->Render();", "radiogroup"),
    ("RichText", "richtext.h", "RichText",
     'auto rt = std::make_shared<unigui::RichText>("rt");\n'
     "rt->AddSpan(\"Profit \", IM_COL32(255, 255, 255, 255));\n"
     "rt->AddSpan(\"+12%\", IM_COL32(46, 209, 94, 255));\n"
     "rt->Render();", "richtext"),
    ("RiskBar", "riskbar.h", "RiskBar",
     'auto rb = std::make_shared<unigui::RiskBar>("rb");\n'
     "rb->SetUsage(0.55f).Render();", "riskbar"),
    ("ScrollArea", "scrollarea.h", "ScrollArea",
     'auto sa = std::make_shared<unigui::ScrollArea>("scroll");\n'
     "sa->SetChild([]{ for (int i = 0; i < 100; ++i) ImGui::Text(\"Line %d\", i); });\n"
     "sa->Render();", "scrollarea"),
    ("SearchBox", "searchbox.h", "SearchBox",
     "static std::string q;\n"
     'auto sb = std::make_shared<unigui::SearchBox>("q", &q);\n'
     "sb->Render();", "searchbox"),
    ("Selectable", "selectable.h", "Selectable",
     "static bool sel = false;\n"
     'auto s = std::make_shared<unigui::Selectable>("row", "Item", &sel);\n'
     "s->Render();", "selectable"),
    ("Separator", "separator.h", "Separator",
     'std::make_shared<unigui::Separator>("sep")->Render();', "separator--space"),
    ("Shimmer", "shimmer.h", "Shimmer",
     "unigui::Shimmer shim;\n"
     "shim.SetSize(ImVec2(200, 24));\n"
     "shim.Render();", "shimmer"),
    ("Shortcut", "shortcut.h", "Shortcut",
     'unigui::Shortcut::Register("Save", ImGuiMod_Ctrl | ImGuiKey_S, []{ save(); });\n'
     "unigui::Shortcut::Process();", "shortcut"),
    ("SkeletonScreen", "skeleton.h", "SkeletonScreen",
     "auto sk = unigui::SkeletonScreen::FromSize(240, 120, 4);\n"
     "sk.SetShimmer(true).Render();", "skeleton-skeletonscreen"),
    ("Slider", "slider.h", "Slider",
     "static float v = 0.5f;\n"
     'auto sl = std::make_shared<unigui::Slider<float>>("sl", &v, 0.f, 1.f);\n'
     "sl->Render();", "slidert"),
    ("SliderBar", "sliderbar.h", "SliderBar",
     'auto sb = std::make_shared<unigui::SliderBar>("sb");\n'
     "sb->SetRange(100, 200).SetValue(150).Render();", "sliderbar"),
    ("Space / DockSpace", "space.h", "Space",
     'std::make_shared<unigui::Space>("gap")->SetHeight(12.f)->Render();\n'
     "// or: unigui::DockSpace::RenderMainDockSpace();", "dockspace-spaceh"),
    ("SpinBox", "spinbox.h", "SpinBox",
     "static int v = 10;\n"
     'auto sp = std::make_shared<unigui::SpinBox<int>>("sp", &v, 0, 100);\n'
     "sp->Render();", "spinboxt"),
    ("Splitter", "splitter.h", "Splitter",
     'static unigui::Splitter split("split");\n'
     'split.SetLeft([]{ ImGui::Text("L"); });\n'
     'split.SetRight([]{ ImGui::Text("R"); });\n'
     "split.Render();", "splitter"),
    ("StatusBar", "statusbar.h", "StatusBar",
     'static unigui::StatusBar bar("status");\n'
     'bar.SetLeft("Ready").SetRight("v3.5").Render();', "statusbar"),
    ("StatusLamp", "statuslamp.h", "StatusLamp",
     'auto lamp = std::make_shared<unigui::StatusLamp>("conn");\n'
     "lamp->SetState(unigui::StatusLamp::State::Ok).SetGlowEnabled(true).Render();", "statuslamp"),
    ("Table", "table.h", "Table",
     'static unigui::Table tbl("tbl");\n'
     'tbl.AddColumn("Name", 120);\n'
     'tbl.AddRow({"Alice"});\n'
     "tbl.Render();", "table"),
    ("TabWidget", "tabwidget.h", "TabWidget",
     'static unigui::TabWidget tabs("tabs");\n'
     'tabs.AddTab({"a", "Tab A", []{ ImGui::Text("A"); }});\n'
     "tabs.Render();", "tabwidget"),
    ("Tag", "tag.h", "Tag",
     'auto tag = std::make_shared<unigui::Tag>("t", "Beta");\n'
     "tag->SetRemovable(true).Render();", "badge--tag"),
    ("TimeSeriesChart", "timeseries_chart.h", "TimeSeriesChart",
     'static unigui::TimeSeriesChart chart("live");\n'
     'static int sid = chart.AddSeries({.label = "PnL"});\n'
     "chart.AppendPoint(sid, 1.23, ImGui::GetTime());\n"
     "chart.Render();", "timeserieschart"),
    ("Toast", "toast.h", "Toast",
     'unigui::Toast::Success("Saved");\n'
     "unigui::Toast::Instance().Render();", "toast"),
    ("ToggleSwitch", "toggleswitch.h", "ToggleSwitch",
     "static bool on = true;\n"
     'auto sw = std::make_shared<unigui::ToggleSwitch>("sw", &on);\n'
     "sw->Render();", "toggleswitch"),
    ("ToolBar", "toolbar.h", "ToolBar",
     'static unigui::ToolBar tb("tb");\n'
     'tb.AddButton("save", "Save", []{});\n'
     "tb.Render();", "toolbar"),
    ("Tooltip", "tooltip.h", "Tooltip",
     'ImGui::Text("Hover me");\n'
     'if (ImGui::IsItemHovered())\n    unigui::Tooltip::Show("Hint text");',
     "tooltip--contextmenu"),
    ("TreeView", "treeview.h", "TreeView",
     'unigui::TreeNode root{"Root", {{"Child", {}}}};\n'
     'auto tv = std::make_shared<unigui::TreeView>("tree");\n'
     "tv->SetRoot(std::move(root));\n"
     "tv->SetHideRoot(true).Render();", "treeview"),
    ("TrayIcon", "trayicon.h", "TrayIcon",
     'unigui::TrayIcon::Show("UniGUI", iconPath);\n'
     "unigui::TrayIcon::SetOnClick([]{ /* restore window */ });", "trayicon"),
    ("VirtualList", "virtuallist.h", "VirtualList",
     'static unigui::VirtualList vl("vl");\n'
     "vl.SetItemCount(100000);\n"
     'vl.SetItemRenderer([](int i){ ImGui::Text("Row %d", i); });\n'
     "vl.Render();", "virtuallist"),
    ("Window", "window.h", "Window",
     'auto win = std::make_shared<unigui::Window>("w", "My Window");\n'
     'win->SetContent([]{ ImGui::Text("Content"); });\n'
     "win->Render();", "window"),
    ("Wizard", "wizard.h", "Wizard",
     'static unigui::Wizard wiz("wiz");\n'
     'wiz.AddStep("Step 1", []{ ImGui::Text("One"); });\n'
     "wiz.Render();", "wizard"),
]


def main() -> None:
    lines = [
        "# Widget Examples Catalog",
        "",
        "> **82** catalog entries (81 widget headers + layout RAII). "
        "Construct once, `Render()` each frame.",
        "",
        "- **Cookbook** (composition, DSL, themes): [EXAMPLES.md](EXAMPLES.md)",
        "- **API signatures**: [WIDGET_API.md](WIDGET_API.md)",
        "- **Lookup table**: [API_INDEX.md](API_INDEX.md)",
        "",
        "---",
        "",
    ]
    for i, (name, hdr, _cls, body, anchor) in enumerate(W, 1):
        lines += [
            f"## {i}. {name}",
            "",
            f"**Header:** `#include <unigui/widgets/{hdr}>` · "
            f"**API:** [WIDGET_API.md#{anchor}](WIDGET_API.md#{anchor})",
            "",
            "```cpp",
            f"#include <unigui/widgets/{hdr}>",
            body,
            "```",
            "",
            "---",
            "",
        ]
    out = Path(__file__).resolve().parents[1] / "docs" / "WIDGET_EXAMPLES.md"
    out.write_text("\n".join(lines), encoding="utf-8")
    print(f"wrote {out} ({len(W)} widgets, {len(lines)} lines)")


if __name__ == "__main__":
    main()
