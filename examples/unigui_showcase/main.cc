// ─────────────────────────────────────────────────────────────────────────────
// UniGUI Showcase — a real, comprehensive demo of the TeamkillerUniGUI library.
//
// Every line of UI below is written against the **UniGUI public API** only:
//   • retained-mode widgets   (unigui::Button, DataTable<T>, Gauge, …)
//   • the immediate layer      (unigui::im::Text / Button / SliderFloat / …)
//   • RAII scope guards        (unigui::WindowScope / TabBarScope / TabItemScope)
//   • the theme engine         (unigui::ApplyTheme, ThemePreset, SurfaceStyle)
//
// There is intentionally **not a single raw `ImGui::` call** in this file — that
// is the point of the wrapper. Run headless for CI:  ./unigui_showcase --frames 10
//
// All 92 widgets are exercised, grouped into ten tabs by category:
//   Buttons · Inputs · Text & Pickers · Display · Indicators · Data ·
//   Layout · Charts & Trading · Overlays & Dialogs · Utilities
// ─────────────────────────────────────────────────────────────────────────────

#include <unigui/unigui.h> // pulls in most widgets, im::, scope guards, theme, app
// A few widgets are not in the umbrella header — include them explicitly:
#include <unigui/widgets/imagebutton.h>
#include <unigui/widgets/markdown.h>
#include <unigui/widgets/richtext.h>

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

using namespace unigui;
namespace im = unigui::im;

// ── small helpers ────────────────────────────────────────────────────────────
static void Section(const char* title) {
    im::Spacing();
    im::SeparatorText(title);
}

// Notification is a normal widget (queue + Render), not a static singleton like
// Toast — keep one instance the overlays tab pushes to and main renders.
static Notification g_notif("sc_notif");

// Global keyboard shortcuts. ShortcutManager::Process() reads global per-frame
// input state, so it must run EVERY frame from the main loop — not inside a tab
// body (a tab's body only executes while that tab is the active one, so a chord
// registered there would almost never be polled). Register the chords exactly
// once here; main() calls g_shortcuts.Process() per frame.
static ShortcutManager g_shortcuts;
static const bool g_shortcutsInit = [] {
    g_shortcuts.Register(
        ImGuiKey_S, /*ctrl=*/true, [] { Toast::Success("Ctrl+S fired!"); }, "Save");
    return true;
}();

// File-scope row types for the data-grid widgets in the Data tab.
struct PosRow {
    std::string sym;
    double pnl;
    int lots;
};
struct EdgPod {
    std::string sym;
    int mode;
    int lots;
};
struct BasketLeg {
    std::string sym;
    int lots;
};

// ═════════════════════════════════════════════════════════════════════════════
// Tab 1 — Buttons & actions
// ═════════════════════════════════════════════════════════════════════════════
static void TabButtons() {
    Section("Retained Button (fluent)");
    static Button save("sc_save", "Save");
    static Button danger("sc_del", "Delete");
    save.WithPrimary().Render();
    im::SameLine();
    danger.WithDanger().Render();
    if (save.WasClicked())
        Toast::Success("Saved!");

    Section("Immediate buttons (im::) — colour variants");
    if (im::Button("Default"))
        Toast::Info("default");
    im::SameLine();
    if (im::Button("Primary", im::ButtonVariant::Primary))
        Toast::Info("primary");
    im::SameLine();
    if (im::Button("Success", im::ButtonVariant::Success))
        Toast::Success("ok");
    im::SameLine();
    if (im::Button("Warning", im::ButtonVariant::Warning))
        Toast::Warn("warn");
    im::SameLine();
    if (im::Button("Danger", im::ButtonVariant::Danger))
        Toast::Error("danger");

    Section("ToggleButton · SegmentedControl · ButtonGroup");
    static ToggleButton run("sc_run", "Start", "Stop");
    run.WithOnToggle([](bool on) { Toast::Info(on ? "running" : "stopped"); });
    run.Render();

    static SegmentedControl tf("sc_tf", {"1D", "1W", "1M", "1Y"});
    static bool tfInit = [] {
        // Configure ONCE — calling WithSelected() every frame would force the
        // selection back to 0 and the control would never appear to switch.
        tf.WithSelected(0).WithOnChange([](int, const std::string& label) { Toast::Info(label); });
        return true;
    }();
    (void) tfInit;
    tf.Render();

    static ButtonGroup acts("sc_acts");
    static bool actsInit = [] {
        // AddButton appends — do it once, not every frame (else it grows forever).
        acts.AddButton("Edit", [] {})
            .AddTintedButton(
                "Delete", [] { Toast::Error("deleted"); }, theme::Semantic::Danger)
            .WithAlign(ButtonGroup::Align::Right);
        return true;
    }();
    (void) actsInit;
    acts.Render();

    Section("IconButton · Hyperlink · im arrow/small");
    static IconButton gear("sc_gear", "Cfg");
    gear.Render();
    im::SameLine();
    if (im::SmallButton("small"))
        Toast::Info("small");
    im::SameLine();
    im::ArrowButton("sc_arrow", ImGuiDir_Right);
    static Hyperlink link("sc_link", "Open the UniGUI repository");
    link.SetURL("https://github.com/Teamkiller131/TeamkillerUniGUI");
    link.Render();
}

// ═════════════════════════════════════════════════════════════════════════════
// Tab 2 — Inputs (booleans + numeric, retained and immediate)
// ═════════════════════════════════════════════════════════════════════════════
static void TabInputs() {
    Section("Booleans");
    static bool enabled = true;
    static CheckBox cb("sc_cb", "Enable feature", &enabled);
    cb.Render();
    im::SameLine();
    static ToggleSwitch sw("sc_sw", "Dark accents", true);
    sw.Render();
    static RadioGroup rg("sc_rg", {"Market", "Limit", "Stop"}, 0);
    rg.Render();

    Section("Numeric — retained");
    static Slider<float> sl("sc_sl", "Gain", 0.5f, 0.f, 1.f);
    sl.Render();
    static SliderBar bar("sc_bar");
    static bool barInit = [] {
        // Set the initial lots ONCE, otherwise every frame resets your drag.
        bar.SetMaxValue(100);
        bar.SetCurrentLots(40);
        return true;
    }();
    (void) barInit;
    bar.Render();
    static SpinBox<int> spin("sc_spin", "Quantity", 10, 0, 100);
    spin.Render();
    static DragInt drag("sc_drag", "Lots", 5, 1.0f, 0, 100);
    drag.Render();
    static DragFloat dfv("sc_dfv", "Drag", 1.f, 0.1f, 0.f, 10.f);
    dfv.Render();
    static InputInt ii("sc_ii", "Int");
    ii.Render();
    static InputFloat iff("sc_iff", "Float");
    iff.Render();

    Section("MultiHandleSlider (two draggable ticks)");
    static MultiHandleSlider mh("sc_mh");
    static bool mhInit = [] {
        mh.SetRange(0.f, 100.f);
        mh.SetTicks({{0, 20.f}, {1, 80.f}});
        return true;
    }();
    (void) mhInit;
    mh.Render();

    Section("Numeric — immediate (im::)");
    static float imGain = 0.3f;
    im::SliderFloat("im SliderFloat", &imGain, 0.f, 1.f);
    static int imN = 4;
    im::SliderInt("im SliderInt", &imN, 0, 10);
    static float price = 12.5f;
    im::InputFloat("im InputFloat", &price, 0.1f, 1.0f, "%.2f");
}

// ═════════════════════════════════════════════════════════════════════════════
// Tab 3 — Text & pickers (text inputs, combos, colour, date, file pickers)
// ═════════════════════════════════════════════════════════════════════════════
static void TabTextPickers() {
    Section("Text inputs");
    static LineEdit le("sc_le", "Name", "");
    le.SetPlaceholder("type your name…");
    le.Render();
    static PasswordInput pw("sc_pw", "Password", "");
    pw.Render();
    static MultiLine ml("sc_ml", "multi-line\nnotes…", 6);
    ml.SetEditable(true);
    ml.Render();
    static SearchBox sb("sc_sb", "Search symbols…");
    sb.SetItems({"AAPL", "MSFT", "NVDA", "IF2506", "IC2506"});
    sb.Render();
    static InputText it("sc_it", "Name (retained)");
    it.Render();

    Section("Pickers");
    static ComboBox combo("sc_combo", "Symbol", std::vector<std::string>{"IF", "IC", "IH", "IM"});
    combo.Render();
    static MultiCombo mc("sc_mc", "Asset classes",
                         std::vector<std::string>{"Equities", "Futures", "Options"});
    static bool mcInit = [] {
        mc.SetSelected(0, true);
        mc.SetSelected(1, true);
        return true;
    }();
    (void) mcInit;
    mc.Render();
    static DatePicker dp("sc_dp", "Expiry");
    static bool dpInit = [] {
        dp.SetDate(2026, 6, 25);
        return true;
    }();
    (void) dpInit;
    dp.Render();
    static ColorEdit ce("sc_ce", "Accent", 0.18f, 0.55f, 0.92f, 1.f);
    ce.Render();
    static ColorPicker cpick("sc_cpick", "Pick", std::array<float, 3>{0.2f, 0.5f, 0.9f});
    cpick.Render();
    static Selectable selA("sc_sel", "A selectable row", false);
    selA.Render();

    Section("CascadingCombo (linked dropdowns)");
    // Level 0 = Province, level 1 = City. The City options depend on the chosen
    // Province, so they must be relinked whenever the Province changes — that is
    // what makes this combo "cascading" rather than two independent lists.
    static CascadingCombo cc(
        "sc_cc", {{"Province", {"Jiangsu", "Zhejiang"}}, {"City", {"Nanjing", "Suzhou"}}});
    static const std::vector<std::vector<std::string>> kCitiesByProvince = {
        {"Nanjing", "Suzhou", "Wuxi"},    // Jiangsu
        {"Hangzhou", "Ningbo", "Wenzhou"} // Zhejiang
    };
    // Configure exactly once: register the link callback and seed the City list to
    // match the default Province. Reconfiguring every frame would reset selection.
    static bool ccInit = [] {
        cc.WithLayout(CascadingCombo::Layout::Horizontal);
        cc.SetOnChanged([](int level, int index) {
            if (level == 0 && index >= 0 && index < (int) kCitiesByProvince.size())
                cc.SetOptions(1, kCitiesByProvince[index]);
        });
        cc.SetOptions(1, kCitiesByProvince[0]);
        return true;
    }();
    (void) ccInit;
    cc.Render();

    Section("File / directory pickers");
    static FilePath fp("sc_fp", "Input file");
    fp.Render();
    static DirPath dpp("sc_dpp", "Output dir");
    dpp.Render();
}

// ═════════════════════════════════════════════════════════════════════════════
// Tab 4 — Display & feedback (text, badges, status)
// ═════════════════════════════════════════════════════════════════════════════
static void TabDisplay() {
    Section("Text & rich text");
    static Label lbl("sc_lbl", "A retained Label widget.");
    lbl.Render();
    static RichText rt("sc_rt");
    static bool rtInit = [] {
        rt.AddSpan("Daily P&L  ", ImVec4(0.86f, 0.86f, 0.86f, 1.f));
        rt.AddSpan("+12.4%", ImVec4(0.18f, 0.82f, 0.37f, 1.f), /*bold=*/true);
        return true;
    }();
    (void) rtInit;
    rt.Render();
    static Markdown md("sc_md", "### Markdown\n- supports **bold**\n- and lists");
    md.Render();

    Section("HeroSection");
    static HeroSection hero("sc_hero", "UniGUI", "A C++23 Dear ImGui wrapper");
    hero.SetHeight(110.f);
    hero.Render();

    Section("Badges · Tags · Status lamp");
    if (im::Button("Inbox")) {}
    static Badge badge("sc_badge");
    badge.SetVariant(Badge::Count);
    badge.SetCount(3);
    badge.Render(); // draws relative to the previous item
    static Tag beta("sc_tag", "Beta");
    beta.SetRemovable(true);
    beta.Render();
    im::SameLine();
    static StatusLamp lamp("sc_lamp");
    lamp.SetState(StatusLamp::Running);
    lamp.SetGlowEnabled(true);
    lamp.SetCaption("Live");
    lamp.Render();

    Section("PnlText · TagList (immediate trading idioms)");
    unigui::PnlText(12.4, "P&L +12.4%");
    im::SameLine();
    unigui::PnlText(-3.2, 2);
    unigui::TagList({{"涨停", theme::Semantic::Up},
                     {"跌停", theme::Semantic::Down},
                     {"风险", theme::Semantic::Danger}});

    Section("Separator (retained) · StatusBar");
    static Separator sepw("sc_sepw");
    sepw.Render();
    static StatusBar sbar("sc_sbar", "Ready · UniGUI Showcase");
    sbar.Render();

    Section("Tooltip");
    im::Text("Hover me for a tooltip");
    if (im::IsItemHovered())
        Tooltip::Show("This is a unigui::Tooltip");
}

// ═════════════════════════════════════════════════════════════════════════════
// Tab 5 — Indicators (gauges, bars, loading states, media, animation)
// ═════════════════════════════════════════════════════════════════════════════
static void TabIndicators() {
    Section("Gauges · sparkline · progress");
    static Gauge cpu("sc_cpu");
    cpu.WithRange(0.f, 100.f).WithValue(63.f).WithSweepDegrees(270.f).WithCenterLabel("CPU");
    cpu.Render();
    im::SameLine();
    static Sparkline spark("sc_spark");
    spark.WithSize(120.f, 40.f)
        .WithColorByTrend()
        .WithShowLastDot()
        .WithData({11.2f, 11.5f, 11.1f, 11.8f, 12.0f, 11.7f, 12.3f, 12.1f});
    spark.Render();
    static ProgressBar pbar("sc_pbar", 0.65f);
    pbar.Render();
    static RiskBar risk("sc_risk");
    risk.SetRatio(0.55);
    risk.SetDisplayText("230万 / 450万");
    risk.Render();
    static FuturesRiskBar frisk("sc_frisk");
    frisk.SetAccountName("Account A");
    frisk.SetActualRatio(0.72);
    frisk.SetEstimatedRatio(0.80);
    frisk.Render();

    Section("MetricCard · loading states");
    static MetricCard card("sc_metric");
    card.WithTitle("Account A")
        .WithStatusDot(theme::Semantic::Success)
        .WithValue("1,234,567")
        .WithDelta(1.2, "+1.20%")
        .WithSubtext("Available 500k");
    card.Render();

    static LoadingIndicator spinner("sc_spin2");
    spinner.Render();
    im::SameLine();
    static Shimmer shimmer;
    static bool shimInit = [] {
        shimmer.AddBlock(220, 14);
        shimmer.AddBlock(160, 14, 0, 22);
        shimmer.Start();
        return true;
    }();
    (void) shimInit;
    shimmer.Render();

    Section("SkeletonScreen");
    static SkeletonScreen sk = SkeletonScreen::FromSize(260, 80, 4);
    static bool skInit = [] {
        sk.SetShimmer(true);
        return true;
    }();
    (void) skInit;
    sk.Render();

    Section("Image · ImageButton (placeholder — no texture loaded)");
    static Image img("sc_img");
    img.SetTexture(nullptr, 64, 64);
    img.Render();
    im::SameLine();
    static ImageButton ibtn("sc_ibtn", "icon");
    ibtn.SetImage((ImTextureID) 0, 48, 48);
    ibtn.Render();

    Section("Animate (eased value driving a wrapped widget — no raw ImGui)");
    float fade = Animate::FadeIn(0.6f); // eases 0→1 over 0.6s
    im::Text("Animate::FadeIn → ProgressBar:");
    im::ProgressBar(fade, ImVec2(220, 0));
}

// ═════════════════════════════════════════════════════════════════════════════
// Tab 6 — Data (tables, trees, lists, editable grids)
// ═════════════════════════════════════════════════════════════════════════════
static void TabData() {
    Section("DataTable<T> — sign-coloured P&L");
    static std::vector<PosRow> rows{
        {"IF2506", 12500.0, 2}, {"IC2506", -3400.0, 1}, {"IH2506", 880.0, 3}, {"IM2506", 0.0, 0}};
    static DataTable<PosRow> dt("sc_dt", {{"Symbol", 110}, {"P&L", 110}, {"Lots", 70}});
    static bool dtInit = [] {
        dt.SetDataSource(&rows);
        dt.SetCellFormatter([](int, int col, const PosRow& r) -> std::string {
            if (col == 0)
                return r.sym;
            if (col == 1)
                return std::to_string((long long) r.pnl);
            return std::to_string(r.lots);
        });
        dt.SetCellSignColor(1, [](int, const PosRow& r) { return r.pnl; });
        return true;
    }();
    (void) dtInit;
    dt.Render();

    Section("Table — quick rows");
    static Table tbl("sc_tbl", {"Name", "Role"});
    static bool tblInit = [] {
        tbl.AddRow({"Alice", "Trader"});
        tbl.AddRow({"Bob", "Quant"});
        return true;
    }();
    (void) tblInit;
    tbl.Render();

    Section("TreeView · ListView · ListBox");
    static TreeView tv("sc_tv");
    static bool tvInit = [] {
        TreeNode root{"Accounts", {{"Group A", {{"Acct1", {}}, {"Acct2", {}}}}, {"Group B", {}}}};
        tv.SetRoot(std::move(root));
        tv.SetHideRoot(false);
        return true;
    }();
    (void) tvInit;
    tv.Render();

    static ListView lv("sc_lv");
    lv.SetItems({"Order #1001", "Order #1002", "Order #1003"});
    lv.Render();

    static ListBox lb("sc_lb", "Side", std::vector<std::string>{"Long", "Short", "Flat"}, 0);
    lb.Render();

    Section("PropertyGrid · VirtualList (100k rows)");
    static PropertyGrid grid("sc_pg");
    static bool pgInit = [] {
        grid.AddProperty(
            {.name = "hedge", .label = "Auto-hedge", .type = PropType::Bool, .value = true});
        grid.AddProperty({.name = "lots",
                          .label = "Max lots",
                          .type = PropType::Int,
                          .value = 7,
                          .minVal = 0,
                          .maxVal = 100});
        return true;
    }();
    (void) pgInit;
    grid.Render();

    static VirtualList vlist("sc_vl", 100000);
    vlist.SetItemGetter([](int i) { return "Tick #" + std::to_string(i); });
    vlist.Render();

    Section("EditableDataGrid<T> (combo + int cell editors)");
    static std::vector<EdgPod> pods{{"IF2506", 0, 2}, {"IC2506", 1, 1}};
    static EditableDataGrid<EdgPod> edg("sc_edg", {{"Sym", 90}, {"Mode", 90}, {"Lots", 70}});
    static bool edgInit = [] {
        edg.SetDataSource(&pods);
        edg.SetCellFormatter([](int, int col, const EdgPod& p) -> std::string {
            if (col == 0)
                return p.sym;
            if (col == 1)
                return p.mode == 0 ? "Open" : "Close";
            return std::to_string(p.lots);
        });
        edg.SetComboColumn(
            1, [](int, const EdgPod&) { return std::vector<std::string>{"Open", "Close"}; },
            [](int, const EdgPod& p) { return p.mode; }, [](int r, int v) { pods[r].mode = v; });
        edg.SetIntColumn(
            2, [](int, const EdgPod& p) { return p.lots; }, [](int r, int v) { pods[r].lots = v; });
        return true;
    }();
    (void) edgInit;
    edg.Render();

    Section("BasketTicket<T> (Add / Remove toolbar over an editable grid)");
    static BasketTicket<BasketLeg> ticket("sc_basket", {{"Symbol", 110}, {"Lots", 70}});
    static bool ticketInit = [] {
        ticket.SetRowFactory([] { return BasketLeg{"NEW", 1}; })
            .SetValidator([](const BasketLeg& l) { return !l.sym.empty() && l.lots > 0; });
        ticket.AddRow({"IF2506", 2});
        ticket.Grid().SetCellFormatter([](int, int col, const BasketLeg& l) -> std::string {
            return col == 0 ? l.sym : std::to_string(l.lots);
        });
        return true;
    }();
    (void) ticketInit;
    ticket.Render();
}

// ═════════════════════════════════════════════════════════════════════════════
// Tab 7 — Layout & containers
// ═════════════════════════════════════════════════════════════════════════════
static void TabLayout() {
    Section("Card · GroupBox · CollapsingHeader");
    static Card card("Cards");
    card.SetContent([] {
        im::Text("Cards group related content");
        im::TextDisabled("with elevation + padding.");
    });
    card.Render();

    static GroupBox gb("sc_gb", "Connection");
    gb.SetContentCallback([] {
        im::Text("Host: 192.168.1.240");
        im::Text("Status: connected");
    });
    gb.Render();

    static CollapsingHeader hdr("sc_hdr", "Advanced settings", true);
    hdr.SetContentCallback([] {
        static bool a = false, b = true;
        im::Checkbox("Aggressive fills", &a);
        im::Checkbox("Confirm orders", &b);
    });
    hdr.Render();

    Section("Panel · PanelBox");
    static Panel pnl("sc_pnl", "Panel");
    pnl.SetContentCallback([] { im::Text("A Panel hosts content via a callback."); });
    pnl.Render();
    static PanelBox pbx("sc_pbx", "Account");
    pbx.SetContentCallback([] { im::Text("PanelBox — a titled, bordered box."); });
    pbx.Render();

    Section("ScrollArea");
    static ScrollArea sca("sc_sca");
    sca.SetContentCallback([] {
        for (int i = 0; i < 60; ++i)
            im::Text("Scrollable line " + std::to_string(i));
    });
    sca.Render();

    Section("Splitter (drag the divider)");
    static Splitter split("sc_split", Splitter::Horizontal, 0.4f);
    split.SetContentA([] { im::TextWrapped("Left pane — instrument list."); });
    split.SetContentB([] { im::TextWrapped("Right pane — order book / chart."); });
    split.Render();

    Section("MultiSplitter (3 panes)");
    static MultiSplitter msp("sc_msp", MultiSplitter::Horizontal);
    static bool mspInit = [] {
        msp.AddPanel(0.34f, [] { im::Text("Pane 1"); });
        msp.AddPanel(0.33f, [] { im::Text("Pane 2"); });
        msp.AddPanel(0.33f, [] { im::Text("Pane 3"); });
        return true;
    }();
    (void) mspInit;
    msp.Render();

    Section("Breadcrumb · Toolbar · nested TabWidget");
    static Breadcrumb bc("sc_bc");
    bc.SetItems({"Home", "Trading", "Futures"});
    bc.Render();

    static ToolBar tb("sc_toolbar");
    static bool tbInit = [] {
        tb.SetItems({{"New", [] {}}, {"Open", [] {}}, {"Save", [] {}}});
        return true;
    }();
    (void) tbInit;
    tb.Render();

    static TabWidget tabs("sc_inner_tabs");
    static bool tabsInit = [] {
        tabs.AddTab({"o", "Orders", [] { im::Text("Working orders…"); }});
        tabs.AddTab({"p", "Positions", [] { im::Text("Open positions…"); }});
        return true;
    }();
    (void) tabsInit;
    tabs.Render();

    Section("Layout::HBox helper");
    Layout::HBox({
        [] { im::Button("HBox A"); },
        [] { im::Button("HBox B"); },
        [] { im::Button("HBox C"); },
    });

    Section("Form (validated fields)");
    static Form form("sc_form", "Login");
    static bool formInit = [] {
        form.AddTextField("user", "Username", /*required=*/true);
        form.AddTextField("pwd", "Password", true);
        form.AddCheckbox("remember", "Remember me");
        return true;
    }();
    (void) formInit;
    form.Render();

    Section("Wizard (multi-step)");
    static Wizard wiz("sc_wiz");
    static bool wizInit = [] {
        wiz.AddStep("s1", "Account", [] { im::Text("Step 1 — account details"); });
        wiz.AddStep("s2", "Confirm", [] { im::Text("Step 2 — confirm"); });
        return true;
    }();
    (void) wizInit;
    wiz.Render();

    Section("Root-level primitives (Window · MenuBar · DockSpace)");
    static Window demoWin("sc_win", "Demo Window (floats)");
    static bool winInit = [] {
        auto body = std::make_shared<Panel>("sc_winbody", "Body");
        body->SetContentCallback(
            [] { im::Text("A retained Window hosts Panels, floating separately."); });
        demoWin.AddPanel(body);
        return true;
    }();
    (void) winInit;
    demoWin.Render(); // shows as its own floating window

    static MenuBar topbar("sc_topbar");
    static bool topbarInit = [] {
        topbar.SetMenus({{"Demo", {{"Action", [] { Toast::Info("menu action"); }}}}});
        return true;
    }();
    (void) topbarInit;
    topbar.Render(); // renders as a global main menu bar at the top of the viewport

    static DockSpace dock("sc_dock");
    (void) dock; // a DockSpaceOverViewport root layout — instantiated here, but it
                 // belongs at the app root, not inside a tab (it covers the viewport).
    im::TextDisabled(
        "Window floats separately, MenuBar is the top bar, DockSpace is app-root only.");
}

// ═════════════════════════════════════════════════════════════════════════════
// Tab 8 — Charts & trading
// ═════════════════════════════════════════════════════════════════════════════
static void TabCharts() {
    Section("TimeSeriesChart (live P&L)");
    static TimeSeriesChart chart("sc_chart");
    static int sid = chart.AddSeries({.label = "PnL"});
    // Init-once: configure the chart and seed it with a batch of historical mock
    // points so it renders fully populated on the first frame, then keeps animating
    // as fresh points trickle in one-per-frame below. Done exactly once so it never
    // fights the user's pan/zoom.
    static const double kSeedT0 = im::GetTime();
    static const bool chartInit = [] {
        chart.SetSlidingWindow(240);
        constexpr int kSeed = 200;
        double walk = 10.0;
        for (int i = 0; i < kSeed; ++i) {
            // Sine trend + small deterministic random walk = lifelike P&L history.
            walk += 0.15 * std::sin(i * 0.21) + 0.04 * (((i * 1103515245 + 12345) % 7) - 3);
            // Strictly-positive, monotonic timestamps in (0, kSeedT0): spreading the
            // seed across [0, kSeedT0) keeps every point > 0 (so AppendPoint never
            // trips its `timestamp < 0` -> frameCounter_ fallback) and places the
            // whole history just left of the live tail, however small kSeedT0 is.
            const double ts = kSeedT0 * (double) (i + 1) / (double) (kSeed + 1);
            chart.AppendPoint(sid, (float) walk, ts);
        }
        return true;
    }();
    (void) chartInit;
    // Live animation: one fresh point per frame continuing the wave at the real clock.
    static double phase = 0.0;
    phase += 0.15;
    chart.AppendPoint(sid, (float) (10.0 + 3.0 * std::sin(phase)), im::GetTime());
    chart.Render();

    Section("PriceTicker (scrolling tape)");
    static PriceTicker tape("sc_tape", {{"AAPL", "192.30", +1.2f},
                                        {"MSFT", "410.10", -0.8f},
                                        {"NVDA", "131.20", +2.4f},
                                        {"BTC", "64,200", +3.1f}});
    tape.WithSpeed(60.f).Render();

    Section("ConnectionStatusBar");
    static ConnectionStatusBar conn("sc_conn");
    conn.PushLatencySample(820.0);
    conn.WithConnected(true)
        .WithCaption("Relay 192.168.1.240")
        .WithLatencyUs(820.0, 900.0)
        .WithFps(60.f)
        .WithSparkline(true);
    conn.Render();

    Section("GroupedRiskTree (rolls children up)");
    static GroupedRiskTree gtree("sc_gtree");
    static bool gtreeInit = [] {
        gtree.SetThresholds(0.7, 0.85);
        gtree.SetData({"Accounts",
                       0.0,
                       {{"Group A", 0.0, {{"Acct1", 0.65, {}}, {"Acct2", 0.92, {}}}},
                        {"Group B", 0.0, {{"Acct3", 0.40, {}}}}}});
        return true;
    }();
    (void) gtreeInit;
    gtree.Render();
}

// ═════════════════════════════════════════════════════════════════════════════
// Tab 9 — Overlays & dialogs
// ═════════════════════════════════════════════════════════════════════════════
static void TabOverlays() {
    Section("Toasts & notifications");
    if (im::Button("Toast: success"))
        Toast::Success("Order filled");
    im::SameLine();
    if (im::Button("Toast: error"))
        Toast::Error("Rejected by gateway");
    im::SameLine();
    if (im::Button("Notification"))
        g_notif.Show("Relay", "Reconnected");

    Section("AlertBar");
    static AlertBar alert("sc_alert");
    static bool alertInit = [] {
        alert.Show("Market data delayed by 2s");
        return true;
    }();
    (void) alertInit;
    alert.Render();

    Section("Dialog (retained, modal OK/Cancel)");
    static Dialog dlg("sc_dlg", "Confirm order", "Submit basket of 3 legs?");
    static bool dlgInit = [] {
        dlg.SetButtons("Yes", "No");
        dlg.SetOnOk([] { Toast::Success("Submitted"); });
        return true;
    }();
    (void) dlgInit;
    if (im::Button("Open dialog"))
        dlg.Open();
    dlg.Render();

    Section("ConfirmDialog (danger-styled)");
    static ConfirmDialog confirm("sc_confirm");
    static bool confirmInit = [] {
        confirm.SetTitle("Delete order?");
        confirm.SetMessage("This cannot be undone.");
        confirm.SetDangerStyle(true);
        return true;
    }();
    (void) confirmInit;
    if (im::Button("Delete…"))
        confirm.Open([] { Toast::Error("Deleted"); });
    confirm.Render();

    Section("ContextMenu — right-click the text");
    im::Text("Right-click me");
    ContextMenu::Show(
        "sc_ctx", {{"Copy", [] { Toast::Info("copy"); }}, {"Paste", [] { Toast::Info("paste"); }}});

    Section("CommandPalette (add-on widget)");
    static CommandPalette palette;
    static bool palInit = [] {
        palette.AddCommand("file.open", "Open File", [] { Toast::Info("open"); });
        palette.AddCommand("file.save", "Save File", [] { Toast::Success("saved"); });
        palette.AddCommand({"view.theme", "Toggle Theme", "View", "Ctrl+T", [] {}});
        return true;
    }();
    (void) palInit;
    if (im::Button("Open command palette"))
        palette.Open();
    palette.Render();

    Section("FileDialog (add-on widget)");
    static FileDialog fd;
    static std::string picked;
    static bool fdInit = [] {
        fd.SetMode(FileDialog::Mode::OpenFile)
            .SetFilters({".csv", ".txt"})
            .SetTitle("Import basket");
        fd.SetOnConfirm([](const std::string& path) { picked = path; });
        return true;
    }();
    (void) fdInit;
    if (im::Button("Open file dialog…"))
        fd.Open();
    fd.Render();
    if (!picked.empty()) {
        im::SameLine();
        im::TextDisabled("→ " + picked);
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// Tab 10 — Utilities (system integration: clipboard, drag-drop, shortcuts, tray)
// ═════════════════════════════════════════════════════════════════════════════
static void TabUtilities() {
    Section("Clipboard");
    static std::string clip = "(nothing pasted yet)";
    if (im::Button("Copy \"hello\""))
        Clipboard::Copy("hello");
    im::SameLine();
    if (im::Button("Paste"))
        clip = Clipboard::Paste();
    im::SameLine();
    im::TextDisabled(clip);

    Section("DragDrop (drag the source onto the target)");
    static int payload = 7;
    static int dropped = -1;
    im::Button("Drag me");
    if (BeginDragSource("sc_dd", payload)) {}
    im::SameLine();
    im::Button("Drop here");
    if (const int* p = AcceptDragDrop<int>("sc_dd"))
        dropped = *p;
    im::SameLine();
    im::TextDisabled("dropped = " + std::to_string(dropped));

    Section("Shortcut — press Ctrl+S");
    // The chord is registered at file scope (g_shortcuts) and processed every
    // frame from main()'s render callback, so it fires regardless of which tab is
    // active. Processing it here would only work while this tab is visible.
    im::TextDisabled("A ShortcutManager checks registered chords each frame (processed globally).");

    Section("TrayIcon (Windows notification area)");
    static TrayIcon tray("sc_tray", "UniGUI Showcase");
    if (im::Button("Show tray icon"))
        tray.Show();
    im::SameLine();
    if (im::Button("Hide tray icon"))
        tray.Hide();
    im::TextDisabled("Creates a real icon in the system tray.");
}

// ═════════════════════════════════════════════════════════════════════════════
// main
// ═════════════════════════════════════════════════════════════════════════════
int main(int argc, char** argv) {
    int maxFrames = 0;
    for (int i = 1; i < argc; ++i)
        if (std::string(argv[i]) == "--frames" && i + 1 < argc)
            maxFrames = std::atoi(argv[++i]);

    AppConfig cfg;
    cfg.title = "UniGUI Showcase";
    cfg.width = 1400;
    cfg.height = 880;
    if (!Init(cfg)) {
        std::fprintf(stderr, "UniGUI Init failed\n");
        return 1;
    }

    static bool light = false;
    static int surfaceIdx = 0; // 0=Glass 1=Solid 2=Frosted 3=Acrylic 4=Minimal
    static const std::vector<std::string> surfaces = {"Glass", "Solid", "Frosted", "Acrylic",
                                                      "Minimal"};
    auto reapplyTheme = [] {
        ThemeConfig tc;
        tc.preset = light ? ThemePreset::Light : ThemePreset::Dark;
        tc.surface = static_cast<theme::SurfaceStyle>(surfaceIdx);
        ApplyTheme(tc);
    };

    Run(
        [&] {
            im::SetNextWindowSize(ImVec2(1360, 840), ImGuiCond_FirstUseEver);
            im::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
            WindowScope main{"UniGUI Showcase  —  v" UNIGUI_VERSION_STRING, nullptr,
                             ImGuiWindowFlags_MenuBar};
            if (main) {
                // ── menu bar ─────────────────────────────────────────────────
                if (im::BeginMenuBar()) {
                    if (im::BeginMenu("File")) {
                        if (im::MenuItem("Quit"))
                            std::exit(0);
                        im::EndMenu();
                    }
                    if (im::BeginMenu("View")) {
                        if (im::MenuItem("Toggle theme")) {
                            light = !light;
                            reapplyTheme();
                        }
                        im::EndMenu();
                    }
                    im::EndMenuBar();
                }

                // ── theme controls ───────────────────────────────────────────
                if (im::Checkbox("Light theme", &light))
                    reapplyTheme();
                im::SameLine();
                im::SetNextItemWidth(160);
                if (im::Combo("Surface", &surfaceIdx, surfaces))
                    reapplyTheme();
                im::SameLine();
                im::TextDisabled("92 widgets · immediate layer · zero raw ImGui");

                // ── tabs (grouped by category) ───────────────────────────────
                if (TabBarScope bar{"sc_tabs"}) {
                    if (TabItemScope t{"Buttons"})
                        TabButtons();
                    if (TabItemScope t{"Inputs"})
                        TabInputs();
                    if (TabItemScope t{"Text & Pickers"})
                        TabTextPickers();
                    if (TabItemScope t{"Display"})
                        TabDisplay();
                    if (TabItemScope t{"Indicators"})
                        TabIndicators();
                    if (TabItemScope t{"Data"})
                        TabData();
                    if (TabItemScope t{"Layout"})
                        TabLayout();
                    if (TabItemScope t{"Charts & Trading"})
                        TabCharts();
                    if (TabItemScope t{"Overlays & Dialogs"})
                        TabOverlays();
                    if (TabItemScope t{"Utilities"})
                        TabUtilities();
                }
            }

            // ── global keyboard shortcuts (checked every frame, any tab) ─────
            (void) g_shortcutsInit;
            g_shortcuts.Process();

            // ── global overlays (drawn on top, once per frame) ───────────────
            Toast::Instance().Render();
            g_notif.Render();
        },
        maxFrames);

    return 0;
}
