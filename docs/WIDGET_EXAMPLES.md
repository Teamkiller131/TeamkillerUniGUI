# Widget Examples Catalog

> **95** catalog entries (94 widget headers + layout RAII). Construct once, `Render()` each frame.
> Snippets are verified against the current headers (constructors, methods, enums).

- **Cookbook** (composition, DSL, themes): [EXAMPLES.md](EXAMPLES.md)
- **API signatures**: [WIDGET_API.md](WIDGET_API.md)
- **Lookup table**: [API_INDEX.md](API_INDEX.md)

---

## 1. AlertBar

**Header:** `#include <unigui/widgets/alertbar.h>` · **API:** [WIDGET_API.md#alertbar](WIDGET_API.md#alertbar)

```cpp
#include <unigui/widgets/alertbar.h>
auto bar = std::make_shared<unigui::AlertBar>("net");
bar->Show("Relay disconnected");
bar->Render();
```

---

## 2. Animate

**Header:** `#include <unigui/widgets/animate.h>` · **API:** [WIDGET_API.md#animate-uniguianimate](WIDGET_API.md#animate-uniguianimate)

```cpp
#include <unigui/widgets/animate.h>
float a = unigui::Animate::FadeIn(0.25f);
ImGui::PushStyleVar(ImGuiStyleVar_Alpha, a);
ImGui::Text("Fading in");
ImGui::PopStyleVar();
```

---

## 3. Badge

**Header:** `#include <unigui/widgets/badge.h>` · **API:** [WIDGET_API.md#badge--tag](WIDGET_API.md#badge--tag)

```cpp
#include <unigui/widgets/badge.h>
unigui::Badge badge("new");
badge.SetVariant(unigui::Badge::Count);
badge.SetCount(3);
ImGui::Button("Inbox");
badge.Render();  // after parent
```

---

## 4. Breadcrumb

**Header:** `#include <unigui/widgets/breadcrumb.h>` · **API:** [WIDGET_API.md#breadcrumb](WIDGET_API.md#breadcrumb)

```cpp
#include <unigui/widgets/breadcrumb.h>
auto bc = std::make_shared<unigui::Breadcrumb>("nav");
bc->SetItems({"Home", "Settings"});
bc->SetOnSelect([](int i){ /* navigate */ });
bc->Render();
```

---

## 5. Button

**Header:** `#include <unigui/widgets/button.h>` · **API:** [WIDGET_API.md#button](WIDGET_API.md#button)

```cpp
#include <unigui/widgets/button.h>
auto btn = std::make_shared<unigui::Button>("save", "Save");
btn->WithPrimary().Render();
if (btn->WasClicked()) { /* ... */ }
```

---

## 6. Card

**Header:** `#include <unigui/widgets/card.h>` · **API:** [WIDGET_API.md#card](WIDGET_API.md#card)

```cpp
#include <unigui/widgets/card.h>
auto card = std::make_shared<unigui::Card>("card");
card->SetContent([]{ ImGui::Text("Content"); });
card->Render();
```

---

## 7. CascadingCombo

**Header:** `#include <unigui/widgets/cascadingcombo.h>` · **API:** [WIDGET_API.md#cascadingcombo](WIDGET_API.md#cascadingcombo)

```cpp
#include <unigui/widgets/cascadingcombo.h>
auto cc = std::make_shared<unigui::CascadingCombo>("region");
cc->SetLevels({{"省", {"江苏"}}, {"市", {"南京"}}});
cc->WithLayout(unigui::CascadingCombo::Layout::Horizontal).Render();
```

---

## 8. CheckBox

**Header:** `#include <unigui/widgets/checkbox.h>` · **API:** [WIDGET_API.md#checkbox](WIDGET_API.md#checkbox)

```cpp
#include <unigui/widgets/checkbox.h>
static bool on = true;
auto cb = std::make_shared<unigui::CheckBox>("cb", "Enable", &on);
cb->Render();
```

---

## 9. Clipboard

**Header:** `#include <unigui/widgets/clipboard.h>` · **API:** [WIDGET_API.md#clipboard-uniguiclipboard](WIDGET_API.md#clipboard-uniguiclipboard)

```cpp
#include <unigui/widgets/clipboard.h>
unigui::Clipboard::Copy("copy me");
std::string s = unigui::Clipboard::Paste();
```

---

## 10. CollapsingHeader

**Header:** `#include <unigui/widgets/collapsingheader.h>` · **API:** [WIDGET_API.md#collapsingheader](WIDGET_API.md#collapsingheader)

```cpp
#include <unigui/widgets/collapsingheader.h>
auto h = std::make_shared<unigui::CollapsingHeader>("adv", "Advanced");
h->SetContentCallback([]{ ImGui::Text("Details"); });
h->Render();
```

---

## 11. ColorEdit

**Header:** `#include <unigui/widgets/coloredit.h>` · **API:** [WIDGET_API.md#coloredit--colorpicker](WIDGET_API.md#coloredit--colorpicker)

```cpp
#include <unigui/widgets/coloredit.h>
auto ce = std::make_shared<unigui::ColorEdit>("c", "Color", 1.0f, 0.0f, 0.0f, 1.0f);
ce->Render();
ImVec4 col = ce->GetColor();
```

---

## 12. ColorPicker

**Header:** `#include <unigui/widgets/colorpicker.h>` · **API:** [WIDGET_API.md#coloredit--colorpicker](WIDGET_API.md#coloredit--colorpicker)

```cpp
#include <unigui/widgets/colorpicker.h>
auto cp = std::make_shared<unigui::ColorPicker>("pick", "Pick", std::array<float, 3>{0.2f, 0.5f, 0.9f});
cp->Render();
std::array<float, 3> col = cp->GetColor();
```

---

## 13. ComboBox

**Header:** `#include <unigui/widgets/combobox.h>` · **API:** [WIDGET_API.md#combobox](WIDGET_API.md#combobox)

```cpp
#include <unigui/widgets/combobox.h>
auto box = std::make_shared<unigui::ComboBox>("sym", "Symbol", std::vector<std::string>{"IF", "IC"});
box->Render();
```

---

## 14. ConfirmDialog

**Header:** `#include <unigui/widgets/confirmdialog.h>` · **API:** [WIDGET_API.md#confirmdialog](WIDGET_API.md#confirmdialog)

```cpp
#include <unigui/widgets/confirmdialog.h>
static unigui::ConfirmDialog dlg("confirm");
dlg.SetTitle("Delete?");
dlg.SetMessage("Cannot undo");
dlg.Open([]{ erase(); });
dlg.Render();
```

---

## 15. ContextMenu

**Header:** `#include <unigui/widgets/contextmenu.h>` · **API:** [WIDGET_API.md#tooltip--contextmenu](WIDGET_API.md#tooltip--contextmenu)

```cpp
#include <unigui/widgets/contextmenu.h>
unigui::ContextMenu::Show("ctx", {
    {"Copy", []{ /* ... */ }},
});
```

---

## 16. DataTable

**Header:** `#include <unigui/widgets/datatable.h>` · **API:** [WIDGET_API.md#datatablet](WIDGET_API.md#datatablet)

```cpp
#include <unigui/widgets/datatable.h>
struct Row { std::string n; double v; };
static std::vector<Row> rows{{"A", 1.0}};
static unigui::DataTable<Row> t("t", {{"Name", 120}});
t.SetDataSource(&rows);
t.Render();
```

---

## 17. DatePicker

**Header:** `#include <unigui/widgets/datepicker.h>` · **API:** [WIDGET_API.md#datepicker](WIDGET_API.md#datepicker)

```cpp
#include <unigui/widgets/datepicker.h>
auto dp = std::make_shared<unigui::DatePicker>("d", "Date");
dp->SetDate(2026, 6, 4);
dp->Render();
```

---

## 18. Dialog

**Header:** `#include <unigui/widgets/dialog.h>` · **API:** [WIDGET_API.md#dialog](WIDGET_API.md#dialog)

```cpp
#include <unigui/widgets/dialog.h>
static unigui::Dialog dlg("dlg", "Title", "Body");
dlg.SetButtons("OK", "Cancel");
dlg.Open();
dlg.Render();
```

---

## 19. DirPath

**Header:** `#include <unigui/widgets/dirpath.h>` · **API:** [WIDGET_API.md#filepath--dirpath](WIDGET_API.md#filepath--dirpath)

```cpp
#include <unigui/widgets/dirpath.h>
auto p = std::make_shared<unigui::DirPath>("out", "Output");
p->Render();
```

---

## 20. DragDrop

**Header:** `#include <unigui/widgets/dragdrop.h>` · **API:** [WIDGET_API.md#dragdrop-uniguidragdrop](WIDGET_API.md#dragdrop-uniguidragdrop)

```cpp
#include <unigui/widgets/dragdrop.h>
int payload = 42;
if (unigui::BeginDragSource("payload", payload)) { /* dragging */ }
if (const int* p = unigui::AcceptDragDrop<int>("payload")) { /* dropped *p */ }
```

---

## 21. DragFloat

**Header:** `#include <unigui/widgets/dragfloat.h>` · **API:** [WIDGET_API.md#dragfloatt--dragintt](WIDGET_API.md#dragfloatt--dragintt)

```cpp
#include <unigui/widgets/dragfloat.h>
auto d = std::make_shared<unigui::DragFloat>("df", "Value", 1.f, 0.1f, 0.f, 10.f);
d->Render();
```

---

## 22. DragInt

**Header:** `#include <unigui/widgets/dragint.h>` · **API:** [WIDGET_API.md#dragfloatt--dragintt](WIDGET_API.md#dragfloatt--dragintt)

```cpp
#include <unigui/widgets/dragint.h>
auto d = std::make_shared<unigui::DragInt>("di", "Count", 5, 1.f, 0, 100);
d->Render();
```

---

## 23. FilePath

**Header:** `#include <unigui/widgets/filepath.h>` · **API:** [WIDGET_API.md#filepath--dirpath](WIDGET_API.md#filepath--dirpath)

```cpp
#include <unigui/widgets/filepath.h>
auto fp = std::make_shared<unigui::FilePath>("in", "Input");
fp->Render();
```

---

## 24. Form

**Header:** `#include <unigui/widgets/form.h>` · **API:** [WIDGET_API.md#form](WIDGET_API.md#form)

```cpp
#include <unigui/widgets/form.h>
static unigui::Form form("login", "Login");
form.AddTextField("user", "User", /*required=*/true);
form.Render();
```

---

## 25. FuturesRiskBar

**Header:** `#include <unigui/widgets/futuresriskbar.h>` · **API:** [WIDGET_API.md#futuresriskbar](WIDGET_API.md#futuresriskbar)

```cpp
#include <unigui/widgets/futuresriskbar.h>
auto bar = std::make_shared<unigui::FuturesRiskBar>("risk");
bar->SetActualRatio(0.72);
bar->Render();
```

---

## 26. GroupBox

**Header:** `#include <unigui/widgets/groupbox.h>` · **API:** [WIDGET_API.md#groupbox](WIDGET_API.md#groupbox)

```cpp
#include <unigui/widgets/groupbox.h>
auto g = std::make_shared<unigui::GroupBox>("g", "Group");
g->SetContentCallback([]{ ImGui::Text("Inside"); });
g->Render();
```

---

## 27. HeroSection

**Header:** `#include <unigui/widgets/herosection.h>` · **API:** [WIDGET_API.md#herosection](WIDGET_API.md#herosection)

```cpp
#include <unigui/widgets/herosection.h>
auto hero = std::make_shared<unigui::HeroSection>("hero");
hero->SetTitle("UniGUI");
hero->SetSubtitle("C++23 UI");
hero->Render();
```

---

## 28. Hyperlink

**Header:** `#include <unigui/widgets/hyperlink.h>` · **API:** [WIDGET_API.md#hyperlink](WIDGET_API.md#hyperlink)

```cpp
#include <unigui/widgets/hyperlink.h>
auto link = std::make_shared<unigui::Hyperlink>("docs", "Documentation");
link->SetURL("https://github.com/Teamkiller131/TeamkillerUniGUI");
link->Render();
```

---

## 29. IconButton

**Header:** `#include <unigui/widgets/iconbutton.h>` · **API:** [WIDGET_API.md#iconbutton](WIDGET_API.md#iconbutton)

```cpp
#include <unigui/widgets/iconbutton.h>
auto ib = std::make_shared<unigui::IconButton>("gear", "\ue001");
ib->Render();
```

---

## 30. Image

**Header:** `#include <unigui/widgets/image.h>` · **API:** [WIDGET_API.md#image](WIDGET_API.md#image)

```cpp
#include <unigui/widgets/image.h>
void* myTex = nullptr;
auto img = std::make_shared<unigui::Image>("logo");
img->SetTexture(myTex, 128, 128);
img->Render();
```

---

## 31. ImageButton

**Header:** `#include <unigui/widgets/imagebutton.h>` · **API:** [WIDGET_API.md#imagebutton](WIDGET_API.md#imagebutton)

```cpp
#include <unigui/widgets/imagebutton.h>
ImTextureID myTex = (ImTextureID) 0;
auto ib = std::make_shared<unigui::ImageButton>("ib");
ib->SetImage(myTex, 32, 32);
ib->Render();
```

---

## 32. InputFloat

**Header:** `#include <unigui/widgets/inputfloat.h>` · **API:** [WIDGET_API.md#inputint--inputfloat](WIDGET_API.md#inputint--inputfloat)

```cpp
#include <unigui/widgets/inputfloat.h>
auto in = std::make_shared<unigui::InputFloat>("f", "Float");
in->Render();
```

---

## 33. InputInt

**Header:** `#include <unigui/widgets/inputint.h>` · **API:** [WIDGET_API.md#inputint--inputfloat](WIDGET_API.md#inputint--inputfloat)

```cpp
#include <unigui/widgets/inputint.h>
auto in = std::make_shared<unigui::InputInt>("i", "Int");
in->Render();
```

---

## 34. InputText

**Header:** `#include <unigui/widgets/inputtext.h>` · **API:** [WIDGET_API.md#inputtext](WIDGET_API.md#inputtext)

```cpp
#include <unigui/widgets/inputtext.h>
auto t = std::make_shared<unigui::InputText>("name", "Name");
t->Render();
```

---

## 35. Label

**Header:** `#include <unigui/widgets/label.h>` · **API:** [WIDGET_API.md#label](WIDGET_API.md#label)

```cpp
#include <unigui/widgets/label.h>
auto lbl = std::make_shared<unigui::Label>("lbl", "Hello");
lbl->Render();
```

---

## 36. Layout (HBox/VBox)

**Header:** `#include <unigui/widgets/layout.h>` · **API:** [WIDGET_API.md#hbox--vbox-layouth--raii-helpers](WIDGET_API.md#hbox--vbox-layouth--raii-helpers)

```cpp
#include <unigui/widgets/layout.h>
{ unigui::HBox row(8.f);
  std::make_shared<unigui::Button>("a", "A")->Render();
  std::make_shared<unigui::Button>("b", "B")->Render();
}
```

---

## 37. LineEdit

**Header:** `#include <unigui/widgets/lineedit.h>` · **API:** [WIDGET_API.md#lineedit](WIDGET_API.md#lineedit)

```cpp
#include <unigui/widgets/lineedit.h>
auto le = std::make_shared<unigui::LineEdit>("le", "Name", "hello");
le->Render(); // undo/redo is built-in (le->Undo()/le->Redo())
```

---

## 38. ListBox

**Header:** `#include <unigui/widgets/listbox.h>` · **API:** [WIDGET_API.md#listbox](WIDGET_API.md#listbox)

```cpp
#include <unigui/widgets/listbox.h>
auto lb = std::make_shared<unigui::ListBox>("lb", "Items", std::vector<std::string>{"A", "B"}, 0);
lb->Render();
```

---

## 39. ListView

**Header:** `#include <unigui/widgets/listview.h>` · **API:** [WIDGET_API.md#listview](WIDGET_API.md#listview)

```cpp
#include <unigui/widgets/listview.h>
auto lv = std::make_shared<unigui::ListView>("lv");
lv->SetItems({"Row1", "Row2"});
lv->Render();
```

---

## 40. LoadingIndicator

**Header:** `#include <unigui/widgets/loadingindicator.h>` · **API:** [WIDGET_API.md#loadingindicator](WIDGET_API.md#loadingindicator)

```cpp
#include <unigui/widgets/loadingindicator.h>
auto sp = std::make_shared<unigui::LoadingIndicator>("load");
sp->Render();
```

---

## 41. Markdown

**Header:** `#include <unigui/widgets/markdown.h>` · **API:** [WIDGET_API.md#markdown](WIDGET_API.md#markdown)

```cpp
#include <unigui/widgets/markdown.h>
auto md = std::make_shared<unigui::Markdown>("md");
md->SetMarkdown("# Title\n- item");
md->Render();
```

---

## 42. MenuBar

**Header:** `#include <unigui/widgets/menubar.h>` · **API:** [WIDGET_API.md#menubar](WIDGET_API.md#menubar)

```cpp
#include <unigui/widgets/menubar.h>
static unigui::MenuBar bar("mb");
bar.SetMenus({{"File", {{"Quit", []{ std::exit(0); }}}}});
bar.Render();
```

---

## 43. MultiCombo

**Header:** `#include <unigui/widgets/multicombo.h>` · **API:** [WIDGET_API.md#multicombo](WIDGET_API.md#multicombo)

```cpp
#include <unigui/widgets/multicombo.h>
auto mc = std::make_shared<unigui::MultiCombo>("mc", "Items", std::vector<std::string>{"A", "B"});
mc->SetSelected(0, true);
mc->Render();
```

---

## 44. MultiHandleSlider

**Header:** `#include <unigui/widgets/multihandleslider.h>` · **API:** [WIDGET_API.md#multihandleslider](WIDGET_API.md#multihandleslider)

```cpp
#include <unigui/widgets/multihandleslider.h>
auto mh = std::make_shared<unigui::MultiHandleSlider>("mh");
mh->SetRange(0.f, 1.f);
mh->SetTicks({{0, 0.2f}, {1, 0.8f}});
mh->Render();
```

---

## 45. MultiLine

**Header:** `#include <unigui/widgets/multiline.h>` · **API:** [WIDGET_API.md#multiline](WIDGET_API.md#multiline)

```cpp
#include <unigui/widgets/multiline.h>
auto ml = std::make_shared<unigui::MultiLine>("ml", "initial text");
ml->Render(); // ml->GetText() retrieves the edited value
```

---

## 46. MultiSplitter

**Header:** `#include <unigui/widgets/multisplitter.h>` · **API:** [WIDGET_API.md#multisplitter](WIDGET_API.md#multisplitter)

```cpp
#include <unigui/widgets/multisplitter.h>
static unigui::MultiSplitter sp("sp", unigui::MultiSplitter::Horizontal);
sp.AddPanel(0.3f, []{ ImGui::Text("L"); });
sp.AddPanel(0.7f, []{ ImGui::Text("R"); });
sp.Render();
```

---

## 47. Notification

**Header:** `#include <unigui/widgets/notification.h>` · **API:** [WIDGET_API.md#notification](WIDGET_API.md#notification)

```cpp
#include <unigui/widgets/notification.h>
static unigui::Notification notif("notif");
notif.Show("Info", "Connected");
notif.Render();
```

---

## 48. Panel

**Header:** `#include <unigui/widgets/panel.h>` · **API:** [WIDGET_API.md#panel](WIDGET_API.md#panel)

```cpp
#include <unigui/widgets/panel.h>
auto p = std::make_shared<unigui::Panel>("dock", "Panel");
p->SetContentCallback([]{ ImGui::Text("Panel"); });
p->Render();
```

---

## 49. PanelBox

**Header:** `#include <unigui/widgets/panelbox.h>` · **API:** [WIDGET_API.md#panelbox](WIDGET_API.md#panelbox)

```cpp
#include <unigui/widgets/panelbox.h>
auto pb = std::make_shared<unigui::PanelBox>("pb", "Account");
pb->SetContentCallback([]{ ImGui::Text("Body"); });
pb->Render();
```

---

## 50. PasswordInput

**Header:** `#include <unigui/widgets/passwordinput.h>` · **API:** [WIDGET_API.md#passwordinput](WIDGET_API.md#passwordinput)

```cpp
#include <unigui/widgets/passwordinput.h>
auto pw = std::make_shared<unigui::PasswordInput>("pw", "Password", "");
pw->Render();
```

---

## 51. ProgressBar

**Header:** `#include <unigui/widgets/progressbar.h>` · **API:** [WIDGET_API.md#progressbar](WIDGET_API.md#progressbar)

```cpp
#include <unigui/widgets/progressbar.h>
auto pb = std::make_shared<unigui::ProgressBar>("p", 0.65f);
pb->Render();
```

---

## 52. PropertyGrid

**Header:** `#include <unigui/widgets/propertygrid.h>` · **API:** [WIDGET_API.md#propertygrid](WIDGET_API.md#propertygrid)

```cpp
#include <unigui/widgets/propertygrid.h>
static unigui::PropertyGrid grid("props");
grid.AddProperty({.name = "enabled", .label = "Enabled", .type = unigui::PropType::Bool, .value = true});
grid.Render();
```

---

## 53. RadioGroup

**Header:** `#include <unigui/widgets/radiogroup.h>` · **API:** [WIDGET_API.md#radiogroup](WIDGET_API.md#radiogroup)

```cpp
#include <unigui/widgets/radiogroup.h>
auto rg = std::make_shared<unigui::RadioGroup>("rg", std::vector<std::string>{"A", "B"}, 0);
rg->Render();
```

---

## 54. RichText

**Header:** `#include <unigui/widgets/richtext.h>` · **API:** [WIDGET_API.md#richtext](WIDGET_API.md#richtext)

```cpp
#include <unigui/widgets/richtext.h>
auto rt = std::make_shared<unigui::RichText>("rt");
rt->AddSpan("Profit ", ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
rt->AddSpan("+12%", ImVec4(0.18f, 0.82f, 0.37f, 1.0f));
rt->Render();
```

---

## 55. RiskBar

**Header:** `#include <unigui/widgets/riskbar.h>` · **API:** [WIDGET_API.md#riskbar](WIDGET_API.md#riskbar)

```cpp
#include <unigui/widgets/riskbar.h>
auto rb = std::make_shared<unigui::RiskBar>("rb");
rb->SetRatio(0.55);
rb->Render();
```

---

## 56. ScrollArea

**Header:** `#include <unigui/widgets/scrollarea.h>` · **API:** [WIDGET_API.md#scrollarea](WIDGET_API.md#scrollarea)

```cpp
#include <unigui/widgets/scrollarea.h>
auto sa = std::make_shared<unigui::ScrollArea>("scroll");
sa->SetContentCallback([]{ for (int i = 0; i < 100; ++i) ImGui::Text("Line %d", i); });
sa->Render();
```

---

## 57. SearchBox

**Header:** `#include <unigui/widgets/searchbox.h>` · **API:** [WIDGET_API.md#searchbox](WIDGET_API.md#searchbox)

```cpp
#include <unigui/widgets/searchbox.h>
auto sb = std::make_shared<unigui::SearchBox>("q", "Search...");
sb->Render();
const std::string& q = sb->GetQuery();
```

---

## 58. Selectable

**Header:** `#include <unigui/widgets/selectable.h>` · **API:** [WIDGET_API.md#selectable](WIDGET_API.md#selectable)

```cpp
#include <unigui/widgets/selectable.h>
auto s = std::make_shared<unigui::Selectable>("row", "Item", false);
s->Render();
```

---

## 59. Separator

**Header:** `#include <unigui/widgets/separator.h>` · **API:** [WIDGET_API.md#separator--space](WIDGET_API.md#separator--space)

```cpp
#include <unigui/widgets/separator.h>
std::make_shared<unigui::Separator>("sep")->Render();
```

---

## 60. Shimmer

**Header:** `#include <unigui/widgets/shimmer.h>` · **API:** [WIDGET_API.md#shimmer](WIDGET_API.md#shimmer)

```cpp
#include <unigui/widgets/shimmer.h>
unigui::Shimmer shim;
shim.AddBlock(200.0f, 24.0f);
shim.Start();
shim.Render();
```

---

## 61. Shortcut

**Header:** `#include <unigui/widgets/shortcut.h>` · **API:** [WIDGET_API.md#shortcut](WIDGET_API.md#shortcut)

```cpp
#include <unigui/widgets/shortcut.h>
static unigui::ShortcutManager mgr;
mgr.Register(ImGuiKey_S, /*ctrl=*/true, []{ /* save() */ }, "Save");
mgr.Process();
```

---

## 62. SkeletonScreen

**Header:** `#include <unigui/widgets/skeleton.h>` · **API:** [WIDGET_API.md#skeleton-skeletonscreen](WIDGET_API.md#skeleton-skeletonscreen)

```cpp
#include <unigui/widgets/skeleton.h>
auto sk = unigui::SkeletonScreen::FromSize(240, 120, 4);
sk.SetShimmer(true);
sk.Render();
```

---

## 63. Slider

**Header:** `#include <unigui/widgets/slider.h>` · **API:** [WIDGET_API.md#slidert](WIDGET_API.md#slidert)

```cpp
#include <unigui/widgets/slider.h>
auto sl = std::make_shared<unigui::Slider<float>>("sl", "Value", 0.5f, 0.f, 1.f);
sl->Render();
```

---

## 64. SliderBar

**Header:** `#include <unigui/widgets/sliderbar.h>` · **API:** [WIDGET_API.md#sliderbar](WIDGET_API.md#sliderbar)

```cpp
#include <unigui/widgets/sliderbar.h>
auto sb = std::make_shared<unigui::SliderBar>("sb");
sb->SetMaxValue(200);
sb->SetCurrentLots(150);
sb->Render();
```

---

## 65. Space / DockSpace

**Header:** `#include <unigui/widgets/space.h>` · **API:** [WIDGET_API.md#dockspace-spaceh](WIDGET_API.md#dockspace-spaceh)

```cpp
#include <unigui/widgets/space.h>
auto dock = std::make_shared<unigui::DockSpace>("gap");
dock->Render();
// DockSpace::RenderMainDockSpace() does not exist.
```

---

## 66. SpinBox

**Header:** `#include <unigui/widgets/spinbox.h>` · **API:** [WIDGET_API.md#spinboxt](WIDGET_API.md#spinboxt)

```cpp
#include <unigui/widgets/spinbox.h>
auto sp = std::make_shared<unigui::SpinBox<int>>("sp", "Count", 10, 0, 100);
sp->Render();
```

---

## 67. Splitter

**Header:** `#include <unigui/widgets/splitter.h>` · **API:** [WIDGET_API.md#splitter](WIDGET_API.md#splitter)

```cpp
#include <unigui/widgets/splitter.h>
static unigui::Splitter split("split");
split.SetContentA([]{ ImGui::Text("L"); });
split.SetContentB([]{ ImGui::Text("R"); });
split.Render();
```

---

## 68. StatusBar

**Header:** `#include <unigui/widgets/statusbar.h>` · **API:** [WIDGET_API.md#statusbar](WIDGET_API.md#statusbar)

```cpp
#include <unigui/widgets/statusbar.h>
static unigui::StatusBar bar("status");
bar.SetText("Ready");
bar.Render();
```

---

## 69. StatusLamp

**Header:** `#include <unigui/widgets/statuslamp.h>` · **API:** [WIDGET_API.md#statuslamp](WIDGET_API.md#statuslamp)

```cpp
#include <unigui/widgets/statuslamp.h>
auto lamp = std::make_shared<unigui::StatusLamp>("conn");
lamp->SetState(unigui::StatusLamp::State::Running);
lamp->SetGlowEnabled(true);
lamp->Render();
```

---

## 70. Table

**Header:** `#include <unigui/widgets/table.h>` · **API:** [WIDGET_API.md#table](WIDGET_API.md#table)

```cpp
#include <unigui/widgets/table.h>
static unigui::Table tbl("tbl", {"Name"});
tbl.AddRow({"Alice"});
tbl.Render();
```

---

## 71. TabWidget

**Header:** `#include <unigui/widgets/tabwidget.h>` · **API:** [WIDGET_API.md#tabwidget](WIDGET_API.md#tabwidget)

```cpp
#include <unigui/widgets/tabwidget.h>
static unigui::TabWidget tabs("tabs");
tabs.AddTab({"a", "Tab A", []{ ImGui::Text("A"); }});
tabs.Render();
```

---

## 72. Tag

**Header:** `#include <unigui/widgets/tag.h>` · **API:** [WIDGET_API.md#badge--tag](WIDGET_API.md#badge--tag)

```cpp
#include <unigui/widgets/tag.h>
auto tag = std::make_shared<unigui::Tag>("t", "Beta");
tag->SetRemovable(true);
tag->Render();
```

---

## 73. TimeSeriesChart

**Header:** `#include <unigui/widgets/timeseries_chart.h>` · **API:** [WIDGET_API.md#timeserieschart](WIDGET_API.md#timeserieschart)

```cpp
#include <unigui/widgets/timeseries_chart.h>
static unigui::TimeSeriesChart chart("live");
static int sid = chart.AddSeries({.label = "PnL"});
chart.AppendPoint(sid, 1.23, ImGui::GetTime());
chart.Render();
```

---

## 74. Toast

**Header:** `#include <unigui/widgets/toast.h>` · **API:** [WIDGET_API.md#toast](WIDGET_API.md#toast)

```cpp
#include <unigui/widgets/toast.h>
unigui::Toast::Success("Saved");
unigui::Toast::Instance().Render();
```

---

## 75. ToggleSwitch

**Header:** `#include <unigui/widgets/toggleswitch.h>` · **API:** [WIDGET_API.md#toggleswitch](WIDGET_API.md#toggleswitch)

```cpp
#include <unigui/widgets/toggleswitch.h>
auto sw = std::make_shared<unigui::ToggleSwitch>("sw", "Enabled", true);
sw->Render();
bool on = sw->IsOn();
```

---

## 76. ToolBar

**Header:** `#include <unigui/widgets/toolbar.h>` · **API:** [WIDGET_API.md#toolbar](WIDGET_API.md#toolbar)

```cpp
#include <unigui/widgets/toolbar.h>
static unigui::ToolBar tb("tb");
tb.SetItems({{"Save", []{}}});
tb.Render();
```

---

## 77. Tooltip

**Header:** `#include <unigui/widgets/tooltip.h>` · **API:** [WIDGET_API.md#tooltip--contextmenu](WIDGET_API.md#tooltip--contextmenu)

```cpp
#include <unigui/widgets/tooltip.h>
ImGui::Text("Hover me");
if (ImGui::IsItemHovered())
    unigui::Tooltip::Show("Hint text");
```

---

## 78. TreeView

**Header:** `#include <unigui/widgets/treeview.h>` · **API:** [WIDGET_API.md#treeview](WIDGET_API.md#treeview)

```cpp
#include <unigui/widgets/treeview.h>
unigui::TreeNode root{"Root", {{"Child", {}}}};
auto tv = std::make_shared<unigui::TreeView>("tree");
tv->SetRoot(std::move(root));
tv->SetHideRoot(true);
tv->Render();
```

---

## 79. TrayIcon

**Header:** `#include <unigui/widgets/trayicon.h>` · **API:** [WIDGET_API.md#trayicon](WIDGET_API.md#trayicon)

```cpp
#include <unigui/widgets/trayicon.h>
unigui::TrayIcon tray("tray", "UniGUI");
tray.Show();
tray.SetOnExit([]{ /* restore window */ });
```

---

## 80. VirtualList

**Header:** `#include <unigui/widgets/virtuallist.h>` · **API:** [WIDGET_API.md#virtuallist](WIDGET_API.md#virtuallist)

```cpp
#include <unigui/widgets/virtuallist.h>
static unigui::VirtualList vl("vl");
vl.SetItemCount(100000);
vl.SetItemGetter([](int i){ return "Row " + std::to_string(i); });
vl.Render();
```

---

## 81. Window

**Header:** `#include <unigui/widgets/window.h>` · **API:** [WIDGET_API.md#window](WIDGET_API.md#window)

```cpp
#include <unigui/widgets/window.h>
auto win = std::make_shared<unigui::Window>("w", "My Window");
auto body = std::make_shared<unigui::Panel>("body", "Body");
body->SetContentCallback([]{ ImGui::Text("Content"); });
win->AddPanel(body);
win->Render();
```

---

## 82. Wizard

**Header:** `#include <unigui/widgets/wizard.h>` · **API:** [WIDGET_API.md#wizard](WIDGET_API.md#wizard)

```cpp
#include <unigui/widgets/wizard.h>
static unigui::Wizard wiz("wiz");
wiz.AddStep("s1", "Step 1", []{ ImGui::Text("One"); });
wiz.Render();
```

---

<!-- New in v3.6.0 — appended to keep existing entry numbers stable. -->

## 83. Gauge

**Header:** `#include <unigui/widgets/gauge.h>` · **API:** [WIDGET_API.md#gauge](WIDGET_API.md#gauge)

```cpp
#include <unigui/widgets/gauge.h>
static unigui::Gauge cpu("cpu");
cpu.WithRange(0.f, 100.f).WithValue(63.f).WithSweepDegrees(270.f).WithCenterLabel("CPU");
cpu.Render();
```

---

## 84. PriceTicker

**Header:** `#include <unigui/widgets/priceticker.h>` · **API:** [WIDGET_API.md#priceticker](WIDGET_API.md#priceticker)

```cpp
#include <unigui/widgets/priceticker.h>
static unigui::PriceTicker tape("tape", {
    {"AAPL", "192.30", +1.2f}, {"MSFT", "410.10", -0.8f}, {"BTC", "64,200", +3.1f}});
tape.WithSpeed(60.f).Render();   // scrolls left, green/red ▲▼ by sign
```

---

## 85. SegmentedControl

**Header:** `#include <unigui/widgets/segmentedcontrol.h>` · **API:** [WIDGET_API.md#segmentedcontrol](WIDGET_API.md#segmentedcontrol)

```cpp
#include <unigui/widgets/segmentedcontrol.h>
static unigui::SegmentedControl tf("timeframe", {"1D", "1W", "1M", "1Y"});
tf.WithSelected(0).WithOnChange([](int i, const std::string& label){ /* reload */ });
tf.Render();
```

---

## 86. Sparkline

**Header:** `#include <unigui/widgets/sparkline.h>` · **API:** [WIDGET_API.md#sparkline](WIDGET_API.md#sparkline)

```cpp
#include <unigui/widgets/sparkline.h>
static unigui::Sparkline spark("px");
spark.WithSize(80.f, 20.f).WithColorByTrend().WithShowLastDot()
     .WithData({11.2f, 11.5f, 11.1f, 11.8f, 12.0f, 11.7f});
spark.Render();        // green if last >= first, else red
```

---

<!-- New in v3.7.0 — appended to keep existing entry numbers stable. -->

## 87. EditableDataGrid

**Header:** `#include <unigui/widgets/editabledatagrid.h>` · **API:** [WIDGET_API.md#editabledatagridt](WIDGET_API.md#editabledatagridt)

```cpp
#include <unigui/widgets/editabledatagrid.h>
struct Pod { std::string sym; int mode; int lots; bool running; };
static std::vector<Pod> rows{{"IF2506", 0, 2, false}};
static unigui::EditableDataGrid<Pod> grid("pods", {{"Sym", 90}, {"Mode", 90}, {"Lots", 70}});
grid.SetDataSource(&rows);
grid.SetComboColumn(1, [](int,const Pod&){ return std::vector<std::string>{"Open","Close"}; },
                    [](int,const Pod& p){ return p.mode; }, [&](int r,int v){ rows[r].mode = v; })
    .SetRowReadOnly([](int,const Pod& p){ return p.running; });  // frozen while running
grid.Render();
```

---

## 88. BasketTicket

**Header:** `#include <unigui/widgets/basketticket.h>` · **API:** [WIDGET_API.md#basketticktett](WIDGET_API.md#basketticket-t)

```cpp
#include <unigui/widgets/basketticket.h>
struct Leg { std::string sym; int lots; };
static unigui::BasketTicket<Leg> ticket("basket", {{"Symbol", 100}, {"Lots", 70}});
ticket.SetRowFactory([]{ return Leg{"", 1}; })
      .SetValidator([](const Leg& l){ return !l.sym.empty() && l.lots > 0; })
      .SetOnImportRequested([]{ /* host opens file dialog → ticket.SetRows(parsed) */ });
ticket.Render();
```

---

## 89. GroupedRiskTree

**Header:** `#include <unigui/widgets/groupedrisktree.h>` · **API:** [WIDGET_API.md#groupedrisktree](WIDGET_API.md#groupedrisktree)

```cpp
#include <unigui/widgets/groupedrisktree.h>
static unigui::GroupedRiskTree risk("risk");
risk.SetThresholds(0.7, 0.85);
risk.SetData({"Accounts", 0.0, {{"GroupA", 0.0, {{"Acct1", 0.65, {}}, {"Acct2", 0.92, {}}}}}});
risk.Render();   // parent rows roll children up via Worst (default)
```

---

## 90. MetricCard

**Header:** `#include <unigui/widgets/metriccard.h>` · **API:** [WIDGET_API.md#metriccard](WIDGET_API.md#metriccard)

```cpp
#include <unigui/widgets/metriccard.h>
static unigui::MetricCard card("acct");
card.WithTitle("账户A").WithStatusDot(unigui::theme::Semantic::Success)
    .WithValue("1,234,567").WithDelta(1.2, "+1.20%").WithSubtext("可用 50万");
card.Render();
```

---

## 91. ConnectionStatusBar

**Header:** `#include <unigui/widgets/connection_status.h>` · **API:** [WIDGET_API.md#connectionstatusbar](WIDGET_API.md#connectionstatusbar)

```cpp
#include <unigui/widgets/connection_status.h>
static unigui::ConnectionStatusBar bar("link");
bar.PushLatencySample(rttUs);   // each frame
bar.WithConnected(true).WithCaption("Relay 192.168.1.240")
   .WithLatencyUs(rttUs, avgUs).WithFps(60.f).WithSparkline(true);
bar.Render();
```

---

## 92. ToggleButton

**Header:** `#include <unigui/widgets/togglebutton.h>` · **API:** [WIDGET_API.md#togglebutton](WIDGET_API.md#togglebutton)

```cpp
#include <unigui/widgets/togglebutton.h>
static unigui::ToggleButton run("pod_run", "Start", "Stop");
run.WithOnToggle([](bool on){ if (on) startPod(); else stopPod(); });
run.Render();
```

---

## 93. ButtonGroup

**Header:** `#include <unigui/widgets/buttongroup.h>` · **API:** [WIDGET_API.md#buttongroup](WIDGET_API.md#buttongroup)

```cpp
#include <unigui/widgets/buttongroup.h>
static unigui::ButtonGroup acts("acts");
acts.AddButton("Edit", []{}).AddTintedButton("Delete", []{}, unigui::theme::Semantic::Danger)
    .WithAlign(unigui::ButtonGroup::Align::Right);
acts.Render();
```

---

<!-- New in v3.8.5 -->

## 94. CommandPalette

**Header:** `#include <unigui/widgets/commandpalette.h>` · **API:** [WIDGET_API.md#commandpalette](WIDGET_API.md#commandpalette)

```cpp
#include <unigui/widgets/commandpalette.h>
static unigui::CommandPalette palette;
palette.AddCommand("file.open", "Open File", []{ openFile(); });
palette.AddCommand("file.save", "Save File", []{ saveFile(); });
palette.AddCommand({"view.theme", "Toggle Theme", "View", "Ctrl+T", []{ toggleTheme(); }});

// Bind a hotkey somewhere in your frame:
if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_P)) palette.Open();
palette.Render();   // filters/ranks as you type; Enter runs, Esc dismisses
```

---

## 95. FileDialog

**Header:** `#include <unigui/widgets/filedialog.h>` · **API:** [WIDGET_API.md#filedialog](WIDGET_API.md#filedialog)

```cpp
#include <unigui/widgets/filedialog.h>
static unigui::FileDialog fd;
fd.SetMode(unigui::FileDialog::Mode::OpenFile)
  .SetFilters({".csv", ".txt"})
  .SetTitle("Import basket");
fd.SetOnConfirm([](const std::string& path){ loadBasket(path); });

if (importClicked) fd.Open();
fd.Render();   // navigate dirs, filter by extension, OK/Cancel
```

---
