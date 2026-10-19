#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// UniGUI design tokens — v0 semantic tokens（语义 token，值可换肤）
//
// v0 语义 token：这里的每个名字描述的是「用途/语义」，不是一个固定的外观值。
// 数值只是 v0 默认皮肤（深色，与 theme.cc 的 Dark 基色一致）；后续换肤/主题包
// 只允许替换这些值，不允许让组件重新硬编码颜色、间距、字号或时长。
//
// 约定 / units:
//   * 颜色是 sRGB，分量 0..1，实心色 alpha = 1；用 ImVec4 因为它是 ImGui 的
//     原生颜色类型，可零成本传给任何 ImGui API。
//   * 间距 / 圆角 / 字号 / 控件高度都是「逻辑像素」，按 96 DPI 设计；实际渲染
//     时由 DPI scale（theme::DetectDPIScale / ThemeConfig::dpi_scale）统一缩放。
//     字体像素尺寸请用 ThemeConfig::font_size * dpi_scale 后再乘字号比例。
//   * 动效时长单位是毫秒（int）；ImGui 的动画 API 要秒，用 SecondsFromMillis()。
//
// 使用方（installer shell / suite / basket / ChartForge 等套件）只引用本头文件
// 里的 token，不要复制数值。命名规则、最小对比度要求和安装器壳页面映射见
// docs/design-tokens.md。
// ─────────────────────────────────────────────────────────────────────────────

#include <imgui.h>

#include <cstdint>

namespace unigui::theme::tokens {

/// v0 语义 token 版本的显式标识（测试/文档/未来迁移用）。
inline constexpr const char* kTokenVersion = "v0";
inline constexpr bool kIsDarkPalette = true;

// ── Colour roles ─────────────────────────────────────────────────────────────
// 12 个语义颜色角色。所有角色都是不透明实心色（alpha = 1）。
// 最小对比度（WCAG 2.2 AA）见 docs/design-tokens.md；v0 默认值实测：
//   text/bg 14.6:1, textMuted/bg 6.9:1, accent/bg 5.8:1, danger/bg 5.0:1,
//   warning/bg 9.3:1, success/bg 8.1:1, focus/bg 8.4:1.
inline constexpr ImVec4 bg{0.100f, 0.100f, 0.120f, 1.000f};        // 应用/窗口背景
inline constexpr ImVec4 surface{0.145f, 0.145f, 0.180f, 1.000f};   // 卡片、面板
inline constexpr ImVec4 surfaceAlt{0.190f, 0.190f, 0.235f, 1.000f};// 输入框、悬停行、选中项
inline constexpr ImVec4 text{0.920f, 0.920f, 0.940f, 1.000f};      // 正文主文字
inline constexpr ImVec4 textMuted{0.620f, 0.640f, 0.680f, 1.000f}; // 说明、次要信息
inline constexpr ImVec4 border{0.300f, 0.320f, 0.380f, 1.000f};    // 分割线、描边（装饰性）
inline constexpr ImVec4 accent{0.400f, 0.580f, 0.930f, 1.000f};    // 主操作、链接、进度填充
inline constexpr ImVec4 accentHover{0.500f, 0.680f, 1.000f, 1.000f}; // 主操作悬停
inline constexpr ImVec4 danger{0.920f, 0.340f, 0.340f, 1.000f};    // 错误、危险操作、失败态
inline constexpr ImVec4 warning{0.950f, 0.700f, 0.200f, 1.000f};   // 警告、需重启态
inline constexpr ImVec4 success{0.300f, 0.780f, 0.450f, 1.000f};   // 成功、完成态
inline constexpr ImVec4 focus{0.550f, 0.710f, 1.000f, 1.000f};     // 键盘焦点环

/// 颜色角色的自描述表：name → value + 用途。测试用它校验「命名映射」，
/// 未来的换肤/设计文档生成也以它为准。
struct NamedColor {
    const char* name;
    const ImVec4* value;
    const char* usage;
};

inline constexpr NamedColor kColors[] = {
    {"bg", &bg, "app / window background"},
    {"surface", &surface, "cards, panels, installer content area"},
    {"surfaceAlt", &surfaceAlt, "inputs, hovered rows, selected list items"},
    {"text", &text, "primary body text"},
    {"textMuted", &textMuted, "captions, hints, secondary metadata"},
    {"border", &border, "hairlines, separators, panel outlines (decorative)"},
    {"accent", &accent, "primary action, links, progress fill"},
    {"accentHover", &accentHover, "primary action hover / pressed emphasis"},
    {"danger", &danger, "errors, destructive actions, failure state"},
    {"warning", &warning, "warnings, restart-required state"},
    {"success", &success, "success state, completion checkmark"},
    {"focus", &focus, "keyboard focus ring / focus outline"},
};
inline constexpr int kColorCount = static_cast<int>(sizeof(kColors) / sizeof(kColors[0]));

// ── Spacing (logical px, 4 px base scale) ────────────────────────────────────
inline constexpr float s4 = 4.0f;   // 图标与文字、紧凑内边距
inline constexpr float s8 = 8.0f;   // 控件内边距、相关元素间距
inline constexpr float s12 = 12.0f; // 分组间距
inline constexpr float s16 = 16.0f; // 页面内容边距
inline constexpr float s24 = 24.0f; // 区块间距、页面级留白

// ── Radius (logical px) ──────────────────────────────────────────────────────
inline constexpr float radius4 = 4.0f; // 输入框、按钮、标签
inline constexpr float radius6 = 6.0f; // 卡片、面板、弹窗

// ── Typography sizes (logical px @ 96 DPI) ───────────────────────────────────
inline constexpr float body16 = 16.0f;    // 正文
inline constexpr float caption13 = 13.0f; // 说明文字/进度百分比
inline constexpr float title20 = 20.0f;   // 页面标题
inline constexpr float hint12 = 12.0f;    // 最小提示/脚注

// ── Control heights (logical px) ─────────────────────────────────────────────
inline constexpr float control32 = 32.0f; // 默认按钮、输入框
inline constexpr float control36 = 36.0f; // 主操作按钮、安装器壳底部操作栏

// ── Motion durations (milliseconds) ──────────────────────────────────────────
inline constexpr int duration120 = 120; // 悬停/按下等微交互
inline constexpr int duration180 = 180; // 页面切换、进度变化

/// Convert a millisecond token into the seconds float ImGui animation APIs take.
constexpr float SecondsFromMillis(int milliseconds) {
    return static_cast<float>(milliseconds) / 1000.0f;
}

/// constexpr equivalent of ImGui::ColorConvertFloat4ToU32() (ImGui's own is not
/// constexpr), so tokens can be used in constexpr tables/asserts.
constexpr ImU32 ColorToU32(const ImVec4& c) {
    const float r = c.x < 0.0f ? 0.0f : (c.x > 1.0f ? 1.0f : c.x);
    const float g = c.y < 0.0f ? 0.0f : (c.y > 1.0f ? 1.0f : c.y);
    const float b = c.z < 0.0f ? 0.0f : (c.z > 1.0f ? 1.0f : c.z);
    const float a = c.w < 0.0f ? 0.0f : (c.w > 1.0f ? 1.0f : c.w);
    return (static_cast<ImU32>(a * 255.0f + 0.5f) << IM_COL32_A_SHIFT) |
           (static_cast<ImU32>(b * 255.0f + 0.5f) << IM_COL32_B_SHIFT) |
           (static_cast<ImU32>(g * 255.0f + 0.5f) << IM_COL32_G_SHIFT) |
           (static_cast<ImU32>(r * 255.0f + 0.5f) << IM_COL32_R_SHIFT);
}

} // namespace unigui::theme::tokens
