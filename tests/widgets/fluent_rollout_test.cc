// Fluent With* rollout verification (roadmap "fluent With* rollout").
//
// Every retained widget that used to derive Widget directly now derives
// FluentWidget<X>, so the base chainers (WithTooltip/WithEnabled/…) return X&
// instead of Widget& and no longer break a chain that continues with a
// derived-specific With* (the CascadingCombo-style mid-chain break). The
// static_asserts below are the mechanical proof for the whole layer: the
// expression only types as X& when the class actually derives FluentWidget<X> —
// with plain Widget as the base it types as Widget& and the assert fails.

#include <unigui/widgets/alertbar.h>
#include <unigui/widgets/basketticket.h>
#include <unigui/widgets/breadcrumb.h>
#include <unigui/widgets/button.h>
#include <unigui/widgets/cascadingcombo.h>
#include <unigui/widgets/collapsingheader.h>
#include <unigui/widgets/coloredit.h>
#include <unigui/widgets/colorpicker.h>
#include <unigui/widgets/combobox.h>
#include <unigui/widgets/commandpalette.h>
#include <unigui/widgets/confirmdialog.h>
#include <unigui/widgets/datatable.h>
#include <unigui/widgets/datepicker.h>
#include <unigui/widgets/dialog.h>
#include <unigui/widgets/dirpath.h>
#include <unigui/widgets/filedialog.h>
#include <unigui/widgets/filepath.h>
#include <unigui/widgets/form.h>
#include <unigui/widgets/futuresriskbar.h>
#include <unigui/widgets/groupbox.h>
#include <unigui/widgets/groupedrisktree.h>
#include <unigui/widgets/herosection.h>
#include <unigui/widgets/hyperlink.h>
#include <unigui/widgets/iconbutton.h>
#include <unigui/widgets/image.h>
#include <unigui/widgets/imagebutton.h>
#include <unigui/widgets/label.h>
#include <unigui/widgets/listbox.h>
#include <unigui/widgets/listview.h>
#include <unigui/widgets/loadingindicator.h>
#include <unigui/widgets/markdown.h>
#include <unigui/widgets/menubar.h>
#include <unigui/widgets/multicombo.h>
#include <unigui/widgets/multihandleslider.h>
#include <unigui/widgets/multiline.h>
#include <unigui/widgets/multisplitter.h>
#include <unigui/widgets/notification.h>
#include <unigui/widgets/panel.h>
#include <unigui/widgets/panelbox.h>
#include <unigui/widgets/progressbar.h>
#include <unigui/widgets/propertygrid.h>
#include <unigui/widgets/radiogroup.h>
#include <unigui/widgets/richtext.h>
#include <unigui/widgets/riskbar.h>
#include <unigui/widgets/scrollarea.h>
#include <unigui/widgets/searchbox.h>
#include <unigui/widgets/selectable.h>
#include <unigui/widgets/separator.h>
#include <unigui/widgets/sliderbar.h>
#include <unigui/widgets/space.h>
#include <unigui/widgets/splitter.h>
#include <unigui/widgets/statusbar.h>
#include <unigui/widgets/statuslamp.h>
#include <unigui/widgets/table.h>
#include <unigui/widgets/tabwidget.h>
#include <unigui/widgets/tag.h>
#include <unigui/widgets/timeseries_chart.h>
#include <unigui/widgets/toast.h>
#include <unigui/widgets/toolbar.h>
#include <unigui/widgets/trayicon.h>
#include <unigui/widgets/treeview.h>
#include <unigui/widgets/virtuallist.h>
#include <unigui/widgets/window.h>
#include <unigui/widgets/wizard.h>

#include <gtest/gtest.h>
#include <string>
#include <type_traits>
#include <utility>

namespace unigui {
namespace {

// True iff the base chainer preserves the derived type — i.e. X derives
// FluentWidget<X> (Widget-based classes yield Widget& here and fail).
template <class W>
constexpr bool kChainPreserves =
    std::is_same_v<decltype(std::declval<W&>().WithTooltip(std::string{})), W&>;

static_assert(kChainPreserves<AlertBar>);
static_assert(kChainPreserves<BasketTicket<int>>);
static_assert(kChainPreserves<Breadcrumb>);
static_assert(kChainPreserves<Button>); // pre-existing fluent widget still intact
static_assert(kChainPreserves<CascadingCombo>);
static_assert(kChainPreserves<CollapsingHeader>);
static_assert(kChainPreserves<ColorEdit>);
static_assert(kChainPreserves<ColorPicker>);
static_assert(kChainPreserves<ComboBox>);
static_assert(kChainPreserves<CommandPalette>);
static_assert(kChainPreserves<ConfirmDialog>);
static_assert(kChainPreserves<DataTable<int>>);
static_assert(kChainPreserves<DatePicker>);
static_assert(kChainPreserves<Dialog>);
static_assert(kChainPreserves<DirPath>);
static_assert(kChainPreserves<FileDialog>);
static_assert(kChainPreserves<FilePath>);
static_assert(kChainPreserves<Form>);
static_assert(kChainPreserves<FuturesRiskBar>);
static_assert(kChainPreserves<GroupBox>);
static_assert(kChainPreserves<GroupedRiskTree>);
static_assert(kChainPreserves<HeroSection>);
static_assert(kChainPreserves<Hyperlink>);
static_assert(kChainPreserves<IconButton>);
static_assert(kChainPreserves<Image>);
static_assert(kChainPreserves<ImageButton>);
static_assert(kChainPreserves<Label>);
static_assert(kChainPreserves<ListBox>);
static_assert(kChainPreserves<ListView>);
static_assert(kChainPreserves<LoadingIndicator>);
static_assert(kChainPreserves<Markdown>);
static_assert(kChainPreserves<MenuBar>);
static_assert(kChainPreserves<MultiCombo>);
static_assert(kChainPreserves<MultiHandleSlider>);
static_assert(kChainPreserves<MultiLine>);
static_assert(kChainPreserves<MultiSplitter>);
static_assert(kChainPreserves<Notification>);
static_assert(kChainPreserves<Panel>);
static_assert(kChainPreserves<PanelBox>);
static_assert(kChainPreserves<ProgressBar>);
static_assert(kChainPreserves<PropertyGrid>);
static_assert(kChainPreserves<RadioGroup>);
static_assert(kChainPreserves<RichText>);
static_assert(kChainPreserves<RiskBar>);
static_assert(kChainPreserves<ScrollArea>);
static_assert(kChainPreserves<SearchBox>);
static_assert(kChainPreserves<Selectable>);
static_assert(kChainPreserves<Separator>);
static_assert(kChainPreserves<SliderBar>);
static_assert(kChainPreserves<DockSpace>);
static_assert(kChainPreserves<Splitter>);
static_assert(kChainPreserves<StatusBar>);
static_assert(kChainPreserves<StatusLamp>);
static_assert(kChainPreserves<Table>);
static_assert(kChainPreserves<TabWidget>);
static_assert(kChainPreserves<Tag>);
static_assert(kChainPreserves<TimeSeriesChart>);
static_assert(kChainPreserves<Toast>);
static_assert(kChainPreserves<ToolBar>);
static_assert(kChainPreserves<TrayIcon>);
static_assert(kChainPreserves<TreeView>);
static_assert(kChainPreserves<VirtualList>);
static_assert(kChainPreserves<Window>);
static_assert(kChainPreserves<Wizard>);

// The named problem case: a base chainer mid-chain used to degrade the type to
// Widget&, so continuing with a derived With* did not compile. Now the whole
// chain stays CascadingCombo& — and the setters take effect on the same object.
TEST(FluentRollout, CascadingCombo_BaseChainerMidChain_KeepsDerivedType) {
    CascadingCombo combo("fluent-cc");
    CascadingCombo& r = combo
                            .WithTooltip("pick a market") // base chainer FIRST
                            .WithShowLabels(true)         // derived With* still chains
                            .WithSpacing(4.0f)
                            .WithEnabled(false); // and back to a base chainer
    EXPECT_EQ(&r, &combo);                       // one object throughout the chain
    EXPECT_FALSE(combo.IsEnabled());             // the chained state landed
}

// Base chainers on a converted container widget mutate the object they return.
TEST(FluentRollout, Window_BaseChainers_ApplyState) {
    Window w("fluent-win", "Fluent");
    Window& r = w.WithVisible(false).WithEnabled(false);
    EXPECT_EQ(&r, &w);
    EXPECT_FALSE(w.IsVisible());
    EXPECT_FALSE(w.IsEnabled());
}

} // namespace
} // namespace unigui
