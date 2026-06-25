# API Index

Master lookup for the **entire public surface** of TeamkillerUniGUI 3.8.12.

| Category | Count | Primary doc |
|----------|------:|-------------|
| Retained widgets & helpers | **93** | [WIDGET_API.md](WIDGET_API.md), [WIDGET_EXAMPLES.md](WIDGET_EXAMPLES.md) |
| `unigui::im` functions | **201** | [WIDGET_API §2](WIDGET_API.md#immediate-mode-uniguiim-vs-retained-mode) |
| `unigui::dsl` builders | **18** | [WIDGET_API — DSL](WIDGET_API.md#declarative-dsl-uniguidsl) |
| App / theme / core | **15+** | Below + [GETTING_STARTED.md](GETTING_STARTED.md) |

> README badge **「93 widgets」** counts retained components under `include/unigui/widgets/` (81 headers + layout RAII). It does **not** include `im::`, DSL, theme, or optional modules — those add **40+** additional APIs.

---

## Application lifecycle

| API | Header | Summary |
|-----|--------|---------|
| `Init` / `Shutdown` | `app/app.h` | Create/destroy GLFW + ImGui context |
| `NewFrame` / `Render` | `app/app.h` | Per-frame pump |
| `Run` / `RunApp` | `app/app.h` | Built-in loop + user callback |
| `ShouldClose` | `app/app.h` | Exit polling |
| `AppConfig` | `app/app.h` | Size, title, theme, backend flags |
| `ProcessMainThreadTasks` | `core/main_thread.h` | Drain `InvokeOnMainThread` queue |
| `InvokeOnMainThread` | `core/main_thread.h` | Thread-safe UI dispatch |
| `ApplyTheme` | `theme/theme.h` | Colors, fonts, DPI, surfaces |
| `ThemeRegistry` | `theme/presets/registry.h` | Named presets (`RegisterAllThemes`) |

---

## Immediate mode (`unigui::im`)

Header: `im/im.h`. Complements retained widgets; same frame, no `shared_ptr`.

| Function | Summary |
|----------|---------|
| `Button`, `Button(variant)`, `SmallButton` | Clickable buttons |
| `Text`, `TextWrapped`, `TextDisabled`, `TextColored` | Labels |
| `BulletText`, `LabelText` | Bulleted / label-value text |
| `Checkbox`, `RadioButton` (×2 overloads) | Booleans / radio groups |
| `SliderFloat`, `SliderFloat2/3/4` | Horizontal float sliders (scalar and vector) |
| `SliderAngle` | Angle slider (radians storage, degrees display) |
| `SliderInt`, `SliderInt2/3/4` | Horizontal int sliders (scalar and vector) |
| `VSliderFloat`, `VSliderInt` | Vertical sliders |
| `DragFloat`, `DragFloat2/3/4`, `DragFloatRange2` | Drag float fields (scalar, vector, range) |
| `DragInt`, `DragInt2/3/4`, `DragIntRange2` | Drag int fields (scalar, vector, range) |
| `InputInt`, `InputInt2/3/4` | Typed int inputs |
| `InputFloat`, `InputFloat2/3/4` | Typed float inputs |
| `InputDouble` | Typed double input |
| `InputText`, `InputTextWithHint`, `InputTextMultiline` | `std::string` binding (with optional hint) |
| `Combo` | Index into `vector<string>` |
| `SameLine`, `NewLine`, `Spacing`, `Separator`, `SeparatorText` | Layout |
| `Dummy`, `Indent`, `Unindent`, `Bullet` | Spacing / indent |
| `PushItemWidth`, `PopItemWidth`, `SetNextItemWidth`, `CalcItemWidth` | Item-width stack |
| `BeginGroup`, `EndGroup` | Horizontal group bounding box |
| `BeginChild` (string-id + numeric-id), `EndChild` | Scrollable sub-region |
| `SetNextWindowPos`, `SetNextWindowSize`, `SetNextWindowSizeConstraints` | Pre-Begin window hints |
| `SetNextWindowContentSize`, `SetNextWindowCollapsed`, `SetNextWindowFocus` | Pre-Begin window hints (cont.) |
| `SetNextWindowScroll`, `SetNextWindowBgAlpha` | Pre-Begin window hints (cont.) |
| `GetScrollX/Y`, `GetScrollMaxX/Y`, `SetScrollX/Y` | Scroll get/set |
| `SetScrollHereX/Y`, `SetScrollFromPosX/Y` | Scroll-to-position helpers |
| `GetCursorScreenPos`, `SetCursorScreenPos` | Absolute cursor position (best-friend API) |
| `GetCursorPos`, `GetCursorPosX/Y`, `SetCursorPos`, `SetCursorPosX/Y` | Window-local cursor |
| `GetCursorStartPos`, `GetContentRegionAvail` | Window geometry (best-friend pair) |
| `GetWindowPos`, `GetWindowSize`, `GetWindowWidth`, `GetWindowHeight` | Current window metrics |
| `PushClipRect`, `PopClipRect` | Scissor / clip rect |
| `AlignTextToFramePadding`, `SetItemDefaultFocus` | Alignment / focus |
| `GetTextLineHeight`, `GetTextLineHeightWithSpacing` | Line metrics |
| `GetFrameHeight`, `GetFrameHeightWithSpacing` | Frame metrics |
| `IsItemHovered`, `IsItemActive`, `IsItemFocused`, `IsItemClicked` | Item state queries |
| `IsItemVisible`, `IsItemEdited`, `IsItemActivated`, `IsItemDeactivated` | Item lifecycle queries |
| `IsItemDeactivatedAfterEdit`, `IsItemToggledOpen` | Item lifecycle queries (cont.) |
| `IsAnyItemHovered`, `IsAnyItemActive`, `IsAnyItemFocused` | Aggregate item queries |
| `GetItemRectMin`, `GetItemRectMax`, `GetItemRectSize` | Item bounding rect |
| `IsKeyDown`, `IsKeyPressed`, `IsKeyReleased` | Keyboard queries |
| `IsMouseDown`, `IsMouseClicked`, `IsMouseReleased`, `IsMouseDoubleClicked` | Mouse button queries |
| `IsMouseDragging`, `IsMouseHoveringRect` | Mouse position queries |
| `GetMousePos`, `GetMouseDragDelta`, `ResetMouseDragDelta` | Mouse position/delta |
| `InvisibleButton`, `ArrowButton` | Hit-test and directional buttons |
| `CheckboxFlags` (int + unsigned int) | Bit-flag checkboxes |
| `ColorButton` | Clickable color swatch |
| `ShowDemoWindow`, `ShowMetricsWindow`, `ShowStyleEditor` | Built-in debug/tool windows |
| `GetWindowDrawList`, `GetBackgroundDrawList`, `GetForegroundDrawList` | `ImDrawList*` access |
| `OpenPopup` (string + numeric ID), `OpenPopupOnItemClick` | Open a popup |
| `BeginPopup`, `BeginPopupModal`, `EndPopup` | Render popup / modal |
| `CloseCurrentPopup`, `IsPopupOpen` | Popup state |
| `BeginPopupContextItem`, `BeginPopupContextWindow`, `BeginPopupContextVoid` | Context popups |
| `BeginMenuBar`, `EndMenuBar`, `BeginMainMenuBar`, `EndMainMenuBar` | Menu bars |
| `BeginMenu`, `EndMenu`, `MenuItem` (×2 overloads) | Menu entries |
| `TextUnformatted`, `TextLink`, `TextLinkOpenURL` | Raw text & clickable hyperlinks |
| `BeginTooltip`, `EndTooltip`, `SetTooltip` | Tooltips (scoped + one-shot) |
| `BeginItemTooltip`, `SetItemTooltip` | Hover-gated tooltips for the last item |
| `BeginDisabled`, `EndDisabled` | Disabled (greyed/inert) scope |
| `BeginCombo`, `EndCombo` | Custom combo (emit `Selectable` items) |
| `BeginListBox`, `EndListBox` | Custom scrolling list region |
| `Selectable` (value + `bool*` overloads) | Selectable rows |
| `TreeNode`, `TreeNodeEx`, `TreePop`, `SetNextItemOpen` | Tree nodes |
| `CollapsingHeader` (×2 overloads) | Collapsible header (optional close ✕) |
| `BeginTabBar`, `EndTabBar`, `BeginTabItem`, `EndTabItem` | Tab bars |
| `ProgressBar`, `PlotLines`, `PlotHistogram` | Progress bar & inline plots |
| `ColorEdit3/4`, `ColorPicker3/4` | Color editors & pickers |
| `ColorConvertRGBtoHSV`, `ColorConvertHSVtoRGB` | Color-space conversion |
| `ColorConvertFloat4ToU32`, `ColorConvertU32ToFloat4` | Packed-color conversion |
| `IsWindowAppearing`, `IsWindowCollapsed`, `IsWindowFocused`, `IsWindowHovered` | Window-state queries |
| `CalcTextSize`, `SetKeyboardFocusHere` | Text measurement & keyboard focus |
| `GetTime`, `GetFrameCount` | Frame/time counters |
| `SetMouseCursor`, `GetMouseCursor` | Mouse cursor shape |

> **Coverage:** `unigui::im` wraps **100% of Dear ImGui's practical public
> surface** (201 functions). The remainder of ImGui's API is intentionally out
> of the immediate layer (context/IO/backend plumbing, docking & viewports,
> `va_list` `*V` overloads, generic `*Scalar` forms, the style/ID/font *stacks*
> — use the RAII scopes in `core/scope.h` — and functions with a richer
> retained-mode widget equivalent). Tracked by `scripts/coverage_vs_imgui.py`.

---

## Declarative DSL (`unigui::dsl`, optional)

Header: `dsl/dsl.h`. CMake: `UNIGUI_MODULE_DSL=ON`.

| Builder | Summary |
|---------|---------|
| `Window`, `VBox`, `HBox` | Containers |
| `Label`, `Text`, `TextWrapped`, `TextDisabled`, `BulletText` | Text |
| `Button` (×2), `CheckBox` (×2), `SliderFloat` (×2), `InputText` (×2) | Controls |
| `Separator`, `Spacing` | Spacers |
| `If`, `IfElse`, `For` | Control flow |
| `Render` | Draw tree once per frame |

---

## RAII, factories, layout

| API | Header | Summary |
|-----|--------|---------|
| `WindowScope`, `IDScope`, `DisabledScope`, … | `core/scope.h` | Auto `End` / `Pop` |
| `Make`, `MakeNamed` | `core/make.h` | `shared_ptr` widget helpers |
| `Observable<T>`, `Subscription`, `Bind` | `core/observable.h` | Reactive value + RAII subscriptions |
| `Layout::HBox`, `Layout::VBox`, `BeginHSplit` | `widgets/layout.h` | Inline layout functions |
| `HBox`, `VBox` (RAII) | `widgets/layout.h` | Scoped horizontal/vertical groups |

---

## FX & non-Widget utilities

| Type | Header | Summary | Detail |
|------|--------|---------|--------|
| `unigui::Animate` | `animate.h` | FadeIn, SlideIn, Lerp | [§15](WIDGET_API.md#animate-uniguianimate) |
| `Badge` | `badge.h` | Dot/count overlay (not `Widget`) | [§4](WIDGET_API.md#badge--tag) |
| `SkeletonScreen` | `skeleton.h` | Loading placeholder | [§14](WIDGET_API.md#skeleton-skeletonscreen) |
| `Shimmer` | `shimmer.h` | Shimmer bar | [§14](WIDGET_API.md#shimmer) |
| `Clipboard` | `clipboard.h` | OS clipboard | [§15](WIDGET_API.md#clipboard-uniguiclipboard) |
| `DragDrop` | `dragdrop.h` | Drag source/target | [§15](WIDGET_API.md#dragdrop-uniguidragdrop) |
| `Shortcut` | `shortcut.h` | Global hotkeys | [§15](WIDGET_API.md#shortcut) |
| `Toast` | `toast.h` | Transient toasts | [§11](WIDGET_API.md#toast) |
| `Notification` | `notification.h` | Notification center | [§11](WIDGET_API.md#notification) |
| `ConfirmDialog` | `confirmdialog.h` | Modal confirm | [§11](WIDGET_API.md#confirmdialog) |
| `TrayIcon` | `trayicon.h` | System tray | [§11](WIDGET_API.md#trayicon) |

---

## Widgets (A–Z)

| Widget | Header | One-line | API | Example |
|--------|--------|----------|-----|---------|
| AlertBar | `alertbar.h` | Banner message | [§11](WIDGET_API.md#alertbar) | [§1](WIDGET_EXAMPLES.md#1-alertbar) |
| Animate | `animate.h` | Fade/slide helpers | [§15](WIDGET_API.md#animate-uniguianimate) | [§2](WIDGET_EXAMPLES.md#2-animate) |
| Badge | `badge.h` | Count/dot badge | [§4](WIDGET_API.md#badge--tag) | [§3](WIDGET_EXAMPLES.md#3-badge) |
| BasketTicket<T> | `basketticket.h` | Basket/program editor | [§9](WIDGET_API.md#basketticket-t) | [§88](WIDGET_EXAMPLES.md#88-basketticket) |
| Breadcrumb | `breadcrumb.h` | Path navigation | [§10](WIDGET_API.md#breadcrumb) | [§4](WIDGET_EXAMPLES.md#4-breadcrumb) |
| Button | `button.h` | Primary actions | [§5](WIDGET_API.md#button) | [§5](WIDGET_EXAMPLES.md#5-button) |
| ButtonGroup | `buttongroup.h` | Aligned button cluster | [§5](WIDGET_API.md#buttongroup) | [§93](WIDGET_EXAMPLES.md#93-buttongroup) |
| Card | `card.h` | Elevated surface | [§3](WIDGET_API.md#card) | [§6](WIDGET_EXAMPLES.md#6-card) |
| CascadingCombo | `cascadingcombo.h` | Multi-level combo | [§9](WIDGET_API.md#cascadingcombo) | [§7](WIDGET_EXAMPLES.md#7-cascadingcombo) |
| CheckBox | `checkbox.h` | Boolean toggle | [§5](WIDGET_API.md#checkbox) | [§8](WIDGET_EXAMPLES.md#8-checkbox) |
| Clipboard | `clipboard.h` | Copy/paste | [§15](WIDGET_API.md#clipboard-uniguiclipboard) | [§9](WIDGET_EXAMPLES.md#9-clipboard) |
| CollapsingHeader | `collapsingheader.h` | Expandable section | [§3](WIDGET_API.md#collapsingheader) | [§10](WIDGET_EXAMPLES.md#10-collapsingheader) |
| ColorEdit | `coloredit.h` | RGBA editor | [§8](WIDGET_API.md#coloredit--colorpicker) | [§11](WIDGET_EXAMPLES.md#11-coloredit) |
| ColorPicker | `colorpicker.h` | Color dialog | [§8](WIDGET_API.md#coloredit--colorpicker) | [§12](WIDGET_EXAMPLES.md#12-colorpicker) |
| ComboBox | `combobox.h` | Dropdown list | [§8](WIDGET_API.md#combobox) | [§13](WIDGET_EXAMPLES.md#13-combobox) |
| ConfirmDialog | `confirmdialog.h` | Yes/no modal | [§11](WIDGET_API.md#confirmdialog) | [§14](WIDGET_EXAMPLES.md#14-confirmdialog) |
| ConnectionStatusBar | `connection_status.h` | Link-health strip | [§13](WIDGET_API.md#connectionstatusbar) | [§91](WIDGET_EXAMPLES.md#91-connectionstatusbar) |
| ContextMenu | `contextmenu.h` | Right-click menu | [§11](WIDGET_API.md#tooltip--contextmenu) | [§15](WIDGET_EXAMPLES.md#15-contextmenu) |
| DataTable\<T\> | `datatable.h` | Virtual table | [§9](WIDGET_API.md#datatablet) | [§16](WIDGET_EXAMPLES.md#16-datatable) |
| DatePicker | `datepicker.h` | Date selection | [§8](WIDGET_API.md#datepicker) | [§17](WIDGET_EXAMPLES.md#17-datepicker) |
| Dialog | `dialog.h` | Modal window | [§11](WIDGET_API.md#dialog) | [§18](WIDGET_EXAMPLES.md#18-dialog) |
| DirPath | `dirpath.h` | Folder picker | [§8](WIDGET_API.md#filepath--dirpath) | [§19](WIDGET_EXAMPLES.md#19-dirpath) |
| DragDrop | `dragdrop.h` | Drag source/target | [§15](WIDGET_API.md#dragdrop-uniguidragdrop) | [§20](WIDGET_EXAMPLES.md#20-dragdrop) |
| DragFloat / DragInt | `dragfloat.h`, `dragint.h` | Drag numeric | [§7](WIDGET_API.md#dragfloatt--dragintt) | [§21–22](WIDGET_EXAMPLES.md#21-dragfloat) |
| EditableDataGrid<T> | `editabledatagrid.h` | Typed cell editors | [§9](WIDGET_API.md#editabledatagridt) | [§87](WIDGET_EXAMPLES.md#87-editabledatagrid) |
| FilePath | `filepath.h` | File picker | [§8](WIDGET_API.md#filepath--dirpath) | [§23](WIDGET_EXAMPLES.md#23-filepath) |
| Form | `form.h` | Validated form | [§12](WIDGET_API.md#form) | [§24](WIDGET_EXAMPLES.md#24-form) |
| FuturesRiskBar | `futuresriskbar.h` | Futures margin bar | [§13](WIDGET_API.md#futuresriskbar) | [§25](WIDGET_EXAMPLES.md#25-futuresriskbar) |
| Gauge | `gauge.h` | Radial progress dial | [§4](WIDGET_API.md#gauge) | [§83](WIDGET_EXAMPLES.md#83-gauge) |
| GroupBox | `groupbox.h` | Titled group | [§3](WIDGET_API.md#groupbox) | [§26](WIDGET_EXAMPLES.md#26-groupbox) |
| GroupedRiskTree | `groupedrisktree.h` | Risk rollup tree | [§9](WIDGET_API.md#groupedrisktree) | [§89](WIDGET_EXAMPLES.md#89-groupedrisktree) |
| HeroSection | `herosection.h` | Banner hero | [§13](WIDGET_API.md#herosection) | [§27](WIDGET_EXAMPLES.md#27-herosection) |
| Hyperlink | `hyperlink.h` | Clickable link | [§5](WIDGET_API.md#hyperlink) | [§28](WIDGET_EXAMPLES.md#28-hyperlink) |
| IconButton | `iconbutton.h` | Icon-only button | [§5](WIDGET_API.md#iconbutton) | [§29](WIDGET_EXAMPLES.md#29-iconbutton) |
| Image | `image.h` | Texture display | [§4](WIDGET_API.md#image) | [§30](WIDGET_EXAMPLES.md#30-image) |
| ImageButton | `imagebutton.h` | Image button | [§5](WIDGET_API.md#imagebutton) | [§31](WIDGET_EXAMPLES.md#31-imagebutton) |
| InputFloat / InputInt | `inputfloat.h`, `inputint.h` | Typed input | [§7](WIDGET_API.md#inputint--inputfloat) | [§32–33](WIDGET_EXAMPLES.md#32-inputfloat) |
| InputText | `inputtext.h` | String field | [§6](WIDGET_API.md#inputtext) | [§34](WIDGET_EXAMPLES.md#34-inputtext) |
| Label | `label.h` | Static text | [§4](WIDGET_API.md#label) | [§35](WIDGET_EXAMPLES.md#35-label) |
| HBox / VBox | `layout.h` | RAII layout | [§3](WIDGET_API.md#hbox--vbox-layouth--raii-helpers) | [§36](WIDGET_EXAMPLES.md#36-layout-hboxvbox) |
| LineEdit | `lineedit.h` | Single-line + undo | [§6](WIDGET_API.md#lineedit) | [§37](WIDGET_EXAMPLES.md#37-lineedit) |
| ListBox | `listbox.h` | Selectable list | [§9](WIDGET_API.md#listbox) | [§38](WIDGET_EXAMPLES.md#38-listbox) |
| ListView | `listview.h` | Simple list | [§9](WIDGET_API.md#listview) | [§39](WIDGET_EXAMPLES.md#39-listview) |
| LoadingIndicator | `loadingindicator.h` | Spinner | [§14](WIDGET_API.md#loadingindicator) | [§40](WIDGET_EXAMPLES.md#40-loadingindicator) |
| Markdown | `markdown.h` | Markdown render | [§4](WIDGET_API.md#markdown) | [§41](WIDGET_EXAMPLES.md#41-markdown) |
| MenuBar | `menubar.h` | Top menu | [§10](WIDGET_API.md#menubar) | [§42](WIDGET_EXAMPLES.md#42-menubar) |
| MetricCard | `metriccard.h` | KPI/pod tile | [§13](WIDGET_API.md#metriccard) | [§90](WIDGET_EXAMPLES.md#90-metriccard) |
| MultiCombo | `multicombo.h` | Multi-select combo | [§8](WIDGET_API.md#multicombo) | [§43](WIDGET_EXAMPLES.md#43-multicombo) |
| MultiHandleSlider | `multihandleslider.h` | Multi-thumb slider | [§7](WIDGET_API.md#multihandleslider) | [§44](WIDGET_EXAMPLES.md#44-multihandleslider) |
| MultiLine | `multiline.h` | Multi-line text | [§6](WIDGET_API.md#multiline) | [§45](WIDGET_EXAMPLES.md#45-multiline) |
| MultiSplitter | `multisplitter.h` | N-panel split | [§3](WIDGET_API.md#multisplitter) | [§46](WIDGET_EXAMPLES.md#46-multisplitter) |
| Notification | `notification.h` | Notification center | [§11](WIDGET_API.md#notification) | [§47](WIDGET_EXAMPLES.md#47-notification) |
| Panel | `panel.h` | Dock panel | [§3](WIDGET_API.md#panel) | [§48](WIDGET_EXAMPLES.md#48-panel) |
| PanelBox | `panelbox.h` | Titled dark panel | [§3](WIDGET_API.md#panelbox) | [§49](WIDGET_EXAMPLES.md#49-panelbox) |
| PasswordInput | `passwordinput.h` | Password + strength | [§6](WIDGET_API.md#passwordinput) | [§50](WIDGET_EXAMPLES.md#50-passwordinput) |
| PriceTicker | `priceticker.h` | Scrolling price marquee | [§13](WIDGET_API.md#priceticker) | [§84](WIDGET_EXAMPLES.md#84-priceticker) |
| ProgressBar | `progressbar.h` | Progress fraction | [§4](WIDGET_API.md#progressbar) | [§51](WIDGET_EXAMPLES.md#51-progressbar) |
| PropertyGrid | `propertygrid.h` | Key/value grid | [§12](WIDGET_API.md#propertygrid) | [§52](WIDGET_EXAMPLES.md#52-propertygrid) |
| RadioGroup | `radiogroup.h` | Radio set | [§5](WIDGET_API.md#radiogroup) | [§53](WIDGET_EXAMPLES.md#53-radiogroup) |
| RichText | `richtext.h` | Colored spans | [§4](WIDGET_API.md#richtext) | [§54](WIDGET_EXAMPLES.md#54-richtext) |
| RiskBar | `riskbar.h` | Usage ratio bar | [§13](WIDGET_API.md#riskbar) | [§55](WIDGET_EXAMPLES.md#55-riskbar) |
| ScrollArea | `scrollarea.h` | Scroll region | [§3](WIDGET_API.md#scrollarea) | [§56](WIDGET_EXAMPLES.md#56-scrollarea) |
| SearchBox | `searchbox.h` | Filterable search | [§6](WIDGET_API.md#searchbox) | [§57](WIDGET_EXAMPLES.md#57-searchbox) |
| SegmentedControl | `segmentedcontrol.h` | Single-select button group | [§5](WIDGET_API.md#segmentedcontrol) | [§85](WIDGET_EXAMPLES.md#85-segmentedcontrol) |
| Selectable | `selectable.h` | Selectable row | [§8](WIDGET_API.md#selectable) | [§58](WIDGET_EXAMPLES.md#58-selectable) |
| Separator | `separator.h` | Divider | [§3](WIDGET_API.md#separator--space) | [§59](WIDGET_EXAMPLES.md#59-separator) |
| Shimmer | `shimmer.h` | Loading shimmer | [§14](WIDGET_API.md#shimmer) | [§60](WIDGET_EXAMPLES.md#60-shimmer) |
| Shortcut | `shortcut.h` | Hotkeys | [§15](WIDGET_API.md#shortcut) | [§61](WIDGET_EXAMPLES.md#61-shortcut) |
| SkeletonScreen | `skeleton.h` | Placeholder layout | [§14](WIDGET_API.md#skeleton-skeletonscreen) | [§62](WIDGET_EXAMPLES.md#62-skeletonscreen) |
| Slider\<T\> | `slider.h` | Value slider | [§7](WIDGET_API.md#slidert) | [§63](WIDGET_EXAMPLES.md#63-slider) |
| SliderBar | `sliderbar.h` | Price/lot bar | [§13](WIDGET_API.md#sliderbar) | [§64](WIDGET_EXAMPLES.md#64-sliderbar) |
| Space / DockSpace | `space.h` | Spacing / dock | [§3](WIDGET_API.md#dockspace-spaceh) | [§65](WIDGET_EXAMPLES.md#65-space--dockspace) |
| Sparkline | `sparkline.h` | Inline trend chart | [§13](WIDGET_API.md#sparkline) | [§86](WIDGET_EXAMPLES.md#86-sparkline) |
| SpinBox\<T\> | `spinbox.h` | Spin control | [§7](WIDGET_API.md#spinboxt) | [§66](WIDGET_EXAMPLES.md#66-spinbox) |
| Splitter | `splitter.h` | Two-pane split | [§3](WIDGET_API.md#splitter) | [§67](WIDGET_EXAMPLES.md#67-splitter) |
| StatusBar | `statusbar.h` | Bottom status | [§4](WIDGET_API.md#statusbar) | [§68](WIDGET_EXAMPLES.md#68-statusbar) |
| StatusLamp | `statuslamp.h` | LED state | [§4](WIDGET_API.md#statuslamp) | [§69](WIDGET_EXAMPLES.md#69-statuslamp) |
| Table | `table.h` | Sortable table | [§9](WIDGET_API.md#table) | [§70](WIDGET_EXAMPLES.md#70-table) |
| TabWidget | `tabwidget.h` | Tab pages | [§3](WIDGET_API.md#tabwidget) | [§71](WIDGET_EXAMPLES.md#71-tabwidget) |
| Tag | `tag.h` | Removable tag | [§4](WIDGET_API.md#badge--tag) | [§72](WIDGET_EXAMPLES.md#72-tag) |
| TimeSeriesChart | `timeseries_chart.h` | implot chart | [§13](WIDGET_API.md#timeserieschart) | [§73](WIDGET_EXAMPLES.md#73-timeserieschart) |
| Toast | `toast.h` | Popup toast | [§11](WIDGET_API.md#toast) | [§74](WIDGET_EXAMPLES.md#74-toast) |
| ToggleButton | `togglebutton.h` | Bistate run/stop | [§5](WIDGET_API.md#togglebutton) | [§92](WIDGET_EXAMPLES.md#92-togglebutton) |
| ToggleSwitch | `toggleswitch.h` | iOS-style switch | [§5](WIDGET_API.md#toggleswitch) | [§75](WIDGET_EXAMPLES.md#75-toggleswitch) |
| ToolBar | `toolbar.h` | Toolbar buttons | [§5](WIDGET_API.md#toolbar) | [§76](WIDGET_EXAMPLES.md#76-toolbar) |
| Tooltip | `tooltip.h` | Hover tip | [§11](WIDGET_API.md#tooltip--contextmenu) | [§77](WIDGET_EXAMPLES.md#77-tooltip) |
| TreeView | `treeview.h` | Hierarchical tree | [§9](WIDGET_API.md#treeview) | [§78](WIDGET_EXAMPLES.md#78-treeview) |
| TrayIcon | `trayicon.h` | System tray | [§11](WIDGET_API.md#trayicon) | [§79](WIDGET_EXAMPLES.md#79-trayicon) |
| VirtualList | `virtuallist.h` | 100k+ virtual list | [§9](WIDGET_API.md#virtuallist) | [§80](WIDGET_EXAMPLES.md#80-virtuallist) |
| Window | `window.h` | Top-level window | [§3](WIDGET_API.md#window) | [§81](WIDGET_EXAMPLES.md#81-window) |
| Wizard | `wizard.h` | Step wizard | [§10](WIDGET_API.md#wizard) | [§82](WIDGET_EXAMPLES.md#82-wizard) |

---

## Base classes

| Type | Header | Summary |
|------|--------|---------|
| `Widget` | `widget_base.h` | `Render`, visibility, `With*` |
| `FluentWidget<T>` | `widget_base.h` | CRTP fluent chain (`Button`, …) |
| `ValueWidget<T>` | `value_widget.h` | `GetValue` / `SetOnChange` |

---

## Optional frameworks

| Module | Doc |
|--------|-----|
| `unigui::dsl` | [MODULES.md](MODULES.md), [WIDGET_API — DSL](WIDGET_API.md#declarative-dsl-uniguidsl) |
| `unigui::events::Bus` | [README — EventBus](../README.md#eventbus) |
| `unigui::styling::Engine` | [README — CSS](../README.md#css-styling) |
| `unigui::plugin::Manager` | [README — Plugins](../README.md#plugin-system) |
