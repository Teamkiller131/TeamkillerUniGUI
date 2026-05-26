# UniGUI v1.10 — TrayIcon 完善 + 细节打磨

## Context
v1.9 修复了 TrayIcon 核心功能。v1.10 完善子菜单/分隔线/通知图标 + PasswordInput demo + Toast 关闭回调。

## TODOs

- [ ] 1. TrayIcon::UpdateTooltip — NIM_MODIFY + NIF_TIP 动态更新 tooltip
- [ ] 2. TrayMenuItem 子菜单 — children 字段递归 BuildContextMenu
- [ ] 3. TrayMenuItem 分隔线 — isSeparator 字段 + MF_SEPARATOR
- [ ] 4. ShowNotification 图标类型 — NotificationType enum → NIIF_INFO/WARNING/ERROR
- [ ] 5. PasswordInput demo 加入 widget_gallery — 密码输入 + 强度显示
- [ ] 6. Toast 关闭回调 — Show(msg, type, dur, onDismiss) 过期触发
- [ ] 7. CHANGELOG + 版本号 1.9.0 → 1.10.0
- [ ] 8. Build + test: 202 测试不回归

---

## Scope
INCLUDE: TrayIcon tooltip/子菜单/分隔线/图标, PasswordInput gallery, Toast dismiss
EXCLUDE: TrayIcon 气泡超链接, 动画图标, Toast 队列管理
