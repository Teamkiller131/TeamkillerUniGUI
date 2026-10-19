# Design tokens (v0)

`include/unigui/theme/tokens.h` — namespace `unigui::theme::tokens`.

> **v0 语义 token，值可换肤。** v0 把安装器壳（installer shell）已经用到的
> 颜色、间距、圆角、字号、控件高度和动效时长固定成**有语义的名字**。名字描述
> 用途，不描述外观；`tokens.h` 里的数值只是 v0 默认深色皮肤。换肤/主题包**只
> 允许替换这些值**，组件不得重新硬编码颜色或尺寸。

The header is constexpr-only: values are usable in `static_assert`, constexpr
tables and `ImGui` calls alike. `tokens::kColors[]` is the machine-readable
name → value → usage table used by tests and (later) by the skin generator.

## Naming rules

| Group | Naming | Examples |
|---|---|---|
| Colour roles | lowerCamelCase, semantic role, no material/lightness words | `bg`, `surfaceAlt`, `textMuted`, `accentHover`, `danger` |
| Spacing | `s<px>` on the 4 px scale | `s4`, `s8`, `s12`, `s16`, `s24` |
| Radius | `radius<px>` | `radius4` (controls), `radius6` (cards/panels) |
| Typography | role + base size in px @ 96 DPI | `body16`, `caption13`, `title20`, `hint12` |
| Control height | `control<px>` | `control32` (default), `control36` (primary action bar) |
| Motion | `duration<ms>` | `duration120` (micro-interaction), `duration180` (page/progress) |

Rules:

- A token name must still make sense after a re-skin: never `blue`, `gray900`,
  `roundedLg`; always `accent`, `surfaceAlt`, `radius6`.
- One role per token — no component-specific tokens in v0 (`installerFooterBg` is
  not a token; it is `surface` or `surfaceAlt`).
- Adding/removing/renaming a token is an API change: update this file,
  `docs/THEMING.md`, `docs/API_INDEX.md` and `tests/theme/tokens_test.cc`
  together.

## Units

- **Colours**: sRGB, components `0..1`, solid roles have `alpha = 1` so they can
  be layered over any surface without unexpected blending. Pass as `ImVec4`
  (ImGui native) or convert with `tokens::ColorToU32()`.
- **Sizes** (spacing, radius, font size, control height): **logical pixels at
  96 DPI**. Render with the app's DPI scale, e.g.
  `px = tokens::s16 * dpi_scale`; font pixel size =
  `ThemeConfig::font_size * dpi_scale`. Never bake monitor DPI into a token.
- **Motion**: **milliseconds** (`int`). ImGui animation APIs take seconds — use
  `tokens::SecondsFromMillis(tokens::duration180)`.

## Minimum contrast (WCAG 2.2 AA)

| Content | Minimum ratio | Applies to |
|---|---|---|
| Body text | **4.5 : 1** | `text`, `textMuted` on `bg`/`surface`/`surfaceAlt` |
| Large text (≥ 24 px, or ≥ 19 px bold) | 3 : 1 | `title20` at large scale, progress percentage |
| UI component & focus indicator | **3 : 1** | `accent` fills, `focus` ring, control boundaries, status icons |
| Decorative separators | exempt | `border` hairlines (must never be the only signal) |
| Disabled text | exempt | disabled controls only |

v0 defaults (measured, sRGB relative luminance):

| Pair | Ratio | Verdict |
|---|---|---|
| `text` on `bg` / `surface` / `surfaceAlt` | 14.6 / 12.7 / 10.9 | AA body text |
| `textMuted` on `bg` / `surface` / `surfaceAlt` | 6.9 / 6.0 / 5.1 | AA body text |
| `accent` on `bg` / `surface` | 5.8 / 5.1 | AA body text + 3:1 UI |
| `focus` on `bg` / `surface` | 8.4 / 7.4 | AA body text + 3:1 focus |
| `danger` on `bg` / `surface` / `surfaceAlt` | 5.0 / 4.3 / 3.7 | AA body on bg; ≥3:1 UI on all |
| `warning` on `bg` / `surface` | 9.3 / 8.1 | AA body text |
| `success` on `bg` / `surface` | 8.1 / 7.0 | AA body text |
| `border` on `bg` | 2.2 | decorative only — do not use as a text/only-state signal |

`tests/theme/tokens_test.cc` enforces the first four rows so a re-skin cannot
silently drop below AA. When a re-skin changes values, recompute this table.

## Installer shell page mapping

The installer shell consumes tokens; it must not introduce literals.

| Page / element | Tokens |
|---|---|
| App frame / backdrop | `bg`; content card `surface` with `radius6` |
| Header / page title | `title20`, `text`, page padding `s24`, title→body gap `s12` |
| Body / description | `body16`, `text`; spacing after title `s12`, between blocks `s16` |
| Secondary text (version, path, size) | `caption13` / `hint12`, `textMuted` |
| Primary button (Next/Install/Finish) | `accent` fill, `bg` label (5.7:1 on accent — re-check if a skin changes `accent`), `control36`, `radius4`, hover `accentHover`; transition `duration120` |
| Secondary button (Back/Cancel) | `surfaceAlt` fill, `text`, `control32`, `radius4`, hover: lightened `surfaceAlt` + `border` outline |
| Keyboard focus | `focus` ring, 2 px, `radius4`, `duration120` |
| Welcome page | `surface` card, `title20` greeting, `body16` explanation, primary action `accent`; `s24` outer / `s16` inner |
| Progress page | `surface` content, progress bar fill `accent`, track `surfaceAlt`, percent text `caption13`/`textMuted`, indeterminate pulse `duration180`; status line `body16`/`text` |
| Done page | `success` status icon + checkmark, `title20`/`text` headline, summary rows `surfaceAlt` with `radius4`, `caption13`/`textMuted` labels |
| Fail page | `danger` status icon + headline accent, body `text`, details `caption13`/`textMuted`, primary retry `accent`, secondary "open log" `surfaceAlt` |
| Restart-required page | `warning` status icon + headline, `body16`/`text` explanation, primary "Restart" `accent` (`control36`), secondary "Later" `surfaceAlt` (`control32`) |
| Footer action bar | `surface` or `surfaceAlt`, `s16` padding, button gap `s8`, divider `border` |

## Reuse across suite / basket / ChartForge

The token set is the shared design-language contract, not installer-only:

- **installer shell** is the first consumer; every page above maps to the same
  colour/spacing/typography tokens as the rest of the app.
- **suite** (settings, update, maintenance pages) reuses `bg`/`surface`/
  `text`/`spacing`/`control` unchanged; only its status semantics
  (`success`/`warning`/`danger`) appear in different places.
- **basket** (cart/checkout-style flows) reuses the same control heights and
  `accent`/`accentHover`; its totals row may use `surfaceAlt` + `caption13`.
- **ChartForge** reuses `surface`/`surfaceAlt` for chart backgrounds, `text`/
  `textMuted` for axes/labels, `accent`/`success`/`warning`/`danger` for series
  and thresholds. Chart colours that must remain distinguishable beyond the
  semantic four are a **palette** concern (theme package), not new tokens.

Relationship to existing theming layers:

- `tokens.h` values match the Dark base palette in `src/theme/theme.cc`
  (`bg` = `ImGuiCol_WindowBg`, `textMuted` ≈ `ImGuiCol_TextDisabled`, `accent` =
  default accent) and the semantic values of `theme::ColorTokens`.
- Geometry tokens (`theme::StyleTokens`) remain the ImGui-style mapping layer:
  e.g. `radius4` → `FrameRounding`, `radius6` → `WindowRounding`, `s8` →
  `FramePadding`/`ItemSpacing`, `s12` → `WindowPadding`. `ApplyStyleTokens()`
  stays authoritative for the raw `ImGuiStyle`; design tokens are the semantic
  source of truth for app-level layout.
- A skin/theme package may replace token values and re-derive `ColorTokens`;
  it must keep the contrast requirements above and must not change token names.

## Scope / non-goals (v0)

- No component tokens, no elevation/shadow scale, no icon size tokens yet.
- No automatic `ImGuiStyle` application from `tokens.h` (colors are applied by
  the palette layer; see `ApplyTheme`/`ApplyColorTokens`). Apps read tokens
  directly for layout and pass colours to widgets.
- Light palette values are out of scope for v0 (semantic names are palette
  neutral so a light skin only swaps values).
