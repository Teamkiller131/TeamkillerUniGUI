# API Index

Quick lookup for **82 widgets** and core APIs. Full signatures and examples: [WIDGET_API.md](WIDGET_API.md).

## Application lifecycle

| API | Header | Summary |
|-----|--------|---------|
| `Init` / `Shutdown` | `app/app.h` | Create/destroy context |
| `NewFrame` / `Render` | `app/app.h` | Frame loop |
| `Run` / `RunApp` | `app/app.h` | Built-in loop + callback |
| `ShouldClose` | `app/app.h` | Exit polling |
| `AppConfig` | `app/app.h` | Size, title, theme, backend |
| `InvokeOnMainThread` | `core/main_thread.h` | Thread-safe UI dispatch |
| `ApplyTheme` | `theme/theme.h` | Colors, fonts, DPI |
| `ThemeRegistry` | `theme/presets/registry.h` | Named presets |

## Immediate mode (`unigui::im`)

| Function | Summary |
|----------|---------|
| `Button`, `SmallButton` | Clickable buttons + variants |
| `Text`, `TextWrapped`, `TextColored`, … | Labels |
| `Checkbox`, `RadioButton` | Booleans |
| `SliderFloat`, `SliderInt`, `DragFloat`, `DragInt` | Numeric |
| `InputText`, `InputTextMultiline` | String binding |
| `Combo` | Dropdown |
| `SameLine`, `Separator`, `Spacing`, … | Layout |

Header: `im/im.h` — [WIDGET_API §2](WIDGET_API.md#immediate-mode-uniguiim-vs-retained-mode).

## RAII & factory

| Type | Header | Summary |
|------|--------|---------|
| `WindowScope`, `IDScope`, `DisabledScope`, … | `core/scope.h` | Auto End/Pop |
| `Make`, `MakeNamed` | `core/make.h` | `shared_ptr` helpers |

## Widgets (A–Z)

| Widget | Header | One-line | Detail |
|--------|--------|----------|--------|
| AlertBar | `alertbar.h` | Banner message | [§11](WIDGET_API.md#alertbar) |
| Animate | `animate.h` | Fade/slide helpers | [§15](WIDGET_API.md#animate-uniguianimate) |
| Badge | `badge.h` | Count/dot badge | [§4](WIDGET_API.md#badge--tag) |
| Breadcrumb | `breadcrumb.h` | Path navigation | [§10](WIDGET_API.md#breadcrumb) |
| Button | `button.h` | Primary actions | [§5](WIDGET_API.md#button) |
| Card | `card.h` | Elevated surface | [§3](WIDGET_API.md#card) |
| CascadingCombo | `cascadingcombo.h` | Multi-level combo | [CASCADINGCOMBO.md](CASCADINGCOMBO.md) |
| CheckBox | `checkbox.h` | Boolean toggle | [§5](WIDGET_API.md#checkbox) |
| Clipboard | `clipboard.h` | Copy/paste | [§15](WIDGET_API.md#clipboard-uniguiclipboard) |
| CollapsingHeader | `collapsingheader.h` | Expandable section | [§3](WIDGET_API.md#collapsingheader) |
| ColorEdit | `coloredit.h` | RGBA editor | [§8](WIDGET_API.md#coloredit--colorpicker) |
| ColorPicker | `colorpicker.h` | Color dialog | [§8](WIDGET_API.md#coloredit--colorpicker) |
| ComboBox | `combobox.h` | Dropdown list | [§8](WIDGET_API.md#combobox) |
| ConfirmDialog | `confirmdialog.h` | Yes/no modal | [§11](WIDGET_API.md#confirmdialog) |
| ContextMenu | `contextmenu.h` | Right-click menu | [§11](WIDGET_API.md#tooltip--contextmenu) |
| DataTable\<T\> | `datatable.h` | Virtual table | [§9](WIDGET_API.md#datatablet) |
| DatePicker | `datepicker.h` | Date selection | [§8](WIDGET_API.md#datepicker) |
| Dialog | `dialog.h` | Modal window | [§11](WIDGET_API.md#dialog) |
| DirPath | `dirpath.h` | Folder picker | [§8](WIDGET_API.md#filepath--dirpath) |
| DragDrop | `dragdrop.h` | Drag source/target | [§15](WIDGET_API.md#dragdrop-uniguidragdrop) |
| DragFloat / DragInt | `dragfloat.h`, `dragint.h` | Drag numeric | [§7](WIDGET_API.md#dragfloatt--dragintt) |
| FilePath | `filepath.h` | File picker | [§8](WIDGET_API.md#filepath--dirpath) |
| Form | `form.h` | Validated form | [§12](WIDGET_API.md#form) |
| FuturesRiskBar | `futuresriskbar.h` | Futures margin bar | [§13](WIDGET_API.md#futuresriskbar) |
| GroupBox | `groupbox.h` | Titled group | [§3](WIDGET_API.md#groupbox) |
| HeroSection | `herosection.h` | Banner hero | [§13](WIDGET_API.md#herosection) |
| Hyperlink | `hyperlink.h` | Clickable link | [§5](WIDGET_API.md#hyperlink) |
| IconButton | `iconbutton.h` | Icon-only button | [§5](WIDGET_API.md#iconbutton) |
| Image | `image.h` | Texture display | [§4](WIDGET_API.md#image) |
| ImageButton | `imagebutton.h` | Image button | [§5](WIDGET_API.md#imagebutton) |
| InputFloat / InputInt | `inputfloat.h`, `inputint.h` | Typed input | [§7](WIDGET_API.md#inputint--inputfloat) |
| InputText | `inputtext.h` | String field | [§6](WIDGET_API.md#inputtext) |
| Label | `label.h` | Static text | [§4](WIDGET_API.md#label) |
| LineEdit | `lineedit.h` | Single-line + undo | [§6](WIDGET_API.md#lineedit) |
| ListBox | `listbox.h` | Selectable list | [§9](WIDGET_API.md#listbox) |
| ListView | `listview.h` | Simple list | [§9](WIDGET_API.md#listview) |
| LoadingIndicator | `loadingindicator.h` | Spinner | [§14](WIDGET_API.md#loadingindicator) |
| Markdown | `markdown.h` | Markdown render | [§4](WIDGET_API.md#markdown) |
| MenuBar | `menubar.h` | Top menu | [§10](WIDGET_API.md#menubar) |
| MultiCombo | `multicombo.h` | Multi-select combo | [§8](WIDGET_API.md#multicombo) |
| MultiHandleSlider | `multihandleslider.h` | Multi-thumb slider | [§7](WIDGET_API.md#multihandleslider) |
| MultiLine | `multiline.h` | Multi-line text | [§6](WIDGET_API.md#multiline) |
| MultiSplitter | `multisplitter.h` | N-panel split | [§3](WIDGET_API.md#multisplitter) |
| Notification | `notification.h` | Notification center | [§11](WIDGET_API.md#notification) |
| Panel | `panel.h` | Dock panel | [§3](WIDGET_API.md#panel) |
| PanelBox | `panelbox.h` | Titled dark panel | [§3](WIDGET_API.md#panelbox) |
| PasswordInput | `passwordinput.h` | Password + strength | [§6](WIDGET_API.md#passwordinput) |
| ProgressBar | `progressbar.h` | Progress fraction | [§4](WIDGET_API.md#progressbar) |
| PropertyGrid | `propertygrid.h` | Key/value grid | [§12](WIDGET_API.md#propertygrid) |
| RadioGroup | `radiogroup.h` | Radio set | [§5](WIDGET_API.md#radiogroup) |
| RichText | `richtext.h` | Colored spans | [§4](WIDGET_API.md#richtext) |
| RiskBar | `riskbar.h` | Usage ratio bar | [§13](WIDGET_API.md#riskbar) |
| ScrollArea | `scrollarea.h` | Scroll region | [§3](WIDGET_API.md#scrollarea) |
| SearchBox | `searchbox.h` | Filterable search | [§6](WIDGET_API.md#searchbox) |
| Selectable | `selectable.h` | Selectable row | [§8](WIDGET_API.md#selectable) |
| Separator | `separator.h` | Divider | [§3](WIDGET_API.md#separator--space) |
| Shimmer | `shimmer.h` | Loading shimmer | [§14](WIDGET_API.md#shimmer) |
| SkeletonScreen | `skeleton.h` | Placeholder layout | [§14](WIDGET_API.md#skeleton-skeletonscreen) |
| Slider\<T\> | `slider.h` | Value slider | [§7](WIDGET_API.md#slidert) |
| SliderBar | `sliderbar.h` | Price/lot bar | [§13](WIDGET_API.md#sliderbar) |
| Space | `space.h` | Spacing / DockSpace | [§3](WIDGET_API.md#dockspace-spaceh) |
| SpinBox\<T\> | `spinbox.h` | Spin control | [§7](WIDGET_API.md#spinboxt) |
| Splitter | `splitter.h` | Two-pane split | [§3](WIDGET_API.md#splitter) |
| StatusBar | `statusbar.h` | Bottom status | [§4](WIDGET_API.md#statusbar) |
| StatusLamp | `statuslamp.h` | LED state | [§4](WIDGET_API.md#statuslamp) |
| Table | `table.h` | Sortable table | [§9](WIDGET_API.md#table) |
| TabWidget | `tabwidget.h` | Tab pages | [§3](WIDGET_API.md#tabwidget) |
| Tag | `tag.h` | Removable tag | [§4](WIDGET_API.md#badge--tag) |
| TimeSeriesChart | `timeseries_chart.h` | implot chart | [§13](WIDGET_API.md#timeserieschart) |
| Toast | `toast.h` | Popup toast | [§11](WIDGET_API.md#toast) |
| ToggleSwitch | `toggleswitch.h` | iOS-style switch | [§5](WIDGET_API.md#toggleswitch) |
| ToolBar | `toolbar.h` | Toolbar buttons | [§5](WIDGET_API.md#toolbar) |
| Tooltip | `tooltip.h` | Hover tip | [§11](WIDGET_API.md#tooltip--contextmenu) |
| TreeView | `treeview.h` | Hierarchical tree | [TREEVIEW.md](TREEVIEW.md) |
| TrayIcon | `trayicon.h` | System tray | [§11](WIDGET_API.md#trayicon) |
| VirtualList | `virtuallist.h` | 100k+ virtual list | [§9](WIDGET_API.md#virtuallist) |
| Window | `window.h` | Top-level window | [§3](WIDGET_API.md#window) |
| Wizard | `wizard.h` | Step wizard | [§10](WIDGET_API.md#wizard) |

## Base classes

| Type | Header | Summary |
|------|--------|---------|
| `Widget` | `widget_base.h` | `Render`, `With*`, visibility |
| `FluentWidget<T>` | `widget_base.h` | CRTP fluent chain (`Button`, …) |
| `ValueWidget<T>` | `value_widget.h` | `GetValue` / `SetOnChange` |

## Optional frameworks

| Module | Doc |
|--------|-----|
| `unigui::dsl` | [WIDGET_API — DSL](WIDGET_API.md#declarative-dsl-uniguidsl), [MODULES.md](MODULES.md) |
| `unigui::events::Bus` | [README — EventBus](../README.md#eventbus) |
| `unigui::styling::Engine` | [README — CSS](../README.md#css-styling) |
| `unigui::plugin::Manager` | [README — Plugins](../README.md#plugin-system) |
