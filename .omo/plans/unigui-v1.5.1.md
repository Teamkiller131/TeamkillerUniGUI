# UniGUI v1.5.1 — System Tray + Widget Docs

## Context
User feedback v1.5.0 fixed 4 blocking items. Remaining experience improvements:
- #5: System tray icon (TrayIcon widget)
- #6: Widget documentation/examples

## TODOs

- [ ] 1. **TrayIcon Widget**
  File: `include/unigui/widgets/trayicon.h` + `src/widgets/trayicon.cc`
  API: `TrayIcon(title, icon)`, `SetMenu(items)`, `Show()`, `Hide()`, `ShowNotification(title,msg)`
  Implementation: Windows uses `Shell_NotifyIcon`, other platforms stub.
  On Windows: `NOTIFYICONDATA`, `WM_TRAYICON` callback, popup menu.
  On Linux/macOS: stub (log warning).

- [ ] 2. **hello_unigui widget showcase improvements**
  Expand each panel with concise API usage examples.
  Add LanguageSwitch combo inline demo.

- [ ] 3. **README widget quick-reference table**
  Add a table: Widget | API | Description for all 55 widgets.

- [ ] 4. **Version bump 1.5.0→1.5.1 + CHANGELOG**
- [ ] 5. **Build + test: 200/200 pass**
