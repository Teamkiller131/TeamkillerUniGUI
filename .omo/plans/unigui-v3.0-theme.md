# UniGUI v3.0 — UI Beautification

## TL;DR

> **Quick Summary**: 全面美化 UI 骨架库——10 套内置主题 + 缓动动画引擎 + 视觉特效（阴影/发光/毛玻璃/渐变）+ CSS 引擎 50+ 属性 + 组件精细化打磨。多平台支持推迟到 v4+。
>
> **Deliverables**:
> - 10+ 开箱即用主题 (Material, Fluent, Dracula, Nord, Gruvbox, Catppuccin, Solarized, TokyoNight, OneDark, Everforest)
> - 缓动动画引擎 (easing curves: linear, quad, cubic, expo, elastic, bounce, back)
> - 视觉特效模块 (阴影/发光/毛玻璃/渐变背景/涟漪/粒子)
> - CSS Engine v2 (50+ 属性, 动画, 过渡, 伪类)
> - 组件打磨 (渐变按钮/圆角卡片/悬停态/骨架屏/微光)
> - 新增组件 (Card, HeroSection, SkeletonScreen, Shimmer, Badge)
> - 丝滑过渡 (窗口/页面/标签切换动画)
>
> **Estimated Effort**: Large (~40 tasks, 8 waves)
> **Parallel Execution**: YES — 8 waves, 5-6 tasks per wave
> **Critical Path**: EffectScope → CSS Engine v2 → Theme Library → Widget Polish

---

## Context

### Original Request
> "v3 将专注于美化 UI，添加主题，特效等，多平台支持推迟到 v4 或以后"

### Exploration Findings (Current State)

| Capability | Status | Gap |
|------------|--------|-----|
| Theme System (53-color dark/light) | Production | Only 2 presets, no real-time switching |
| Animation (FadeIn, SlideIn, Lerp) | Prototype | No easing, no state management |
| CSS Engine (16 properties) | Alpha | No gradients, shadows, animations |
| Font Manager (multi-font + Nerd Font icons) | Production | No emoji, no gradient text |
| Visual Effects | **Missing** | No shadows, blur, glow, particles |
| Widget Polish | Basic | Colors exist, no hover/active effects |

### Test Infrastructure
- **Framework**: GTest + gtest_discover_tests
- **Infrastructure exists**: YES
- **Strategy**: Tests-after (unit tests for easing/math, integration tests for CSS parsing, manual QA for visual effects)

---

## Work Objectives

### Core Objective
Transform UniGUI from a functional widget library into a polished, visually-rich UI framework with 10+ built-in themes, smooth animations, CSS-powered styling with 50+ properties, and modern visual effects (shadows, glow, blur, gradients).

### Concrete Deliverables
- `include/unigui/theme/presets/` — 10 theme header files
- `include/unigui/fx/easing.h` — 10 easing curves
- `include/unigui/fx/effect_scope.h` — ShadowEffect, GlowEffect, BlurEffect
- `include/unigui/fx/animation.h` — Animation state manager
- `src/fx/effect_scope.cc` — effect implementations
- `src/fx/animation.cc` — animation state manager
- `include/unigui/styling/style_engine.h` — expanded to 50+ properties
- `src/v2/style_engine.cc` — updated parser
- `include/unigui/widgets/card.h`, `skeleton.h`, `shimmer.h`, `herosection.h`
- `src/widgets/card.cc`, `skeleton.cc`, `shimmer.cc`, `herosection.cc`
- `include/unigui/fonts/gradient_text.h` — gradient text rendering

### Definition of Done
- [ ] `ctest --preset windows-msvc-debug` runs 200+ tests (existing + new easing/CSS tests)
- [ ] All 10 themes render correctly in widget_gallery
- [ ] Animation demo shows all 10 easing curves
- [ ] Effect demo shows shadow, glow, blur, glass on panels
- [ ] CSS engine parses 50+ properties without error
- [ ] 3 preset builds pass: recommended, minimal, full

### Must Have
- 10 built-in themes (Material, Fluent, Dracula, Nord, Gruvbox, Catppuccin, Solarized, TokyoNight, OneDark, Everforest)
- Real-time theme switching (no restart)
- Easing curve library (10 curves)
- Shadow/glow/blur effects via EffectScope
- CSS Engine: gradients, box-shadow, border-radius, transition, animation
- Button hover/active gradient effects
- Card component with shadow + rounded corners
- Window/panel transition animations

### Must NOT Have
- Multi-platform support (deferred to v4)
- GPU shader-based effects (too complex for v3)
- Theme designer GUI tool (deferred)
- Custom rendering pipeline (use ImDrawList only)
- Breaking changes to existing widget API
- Any file outside `.omo/plans/`

---

## Verification Strategy

> **ZERO HUMAN INTERVENTION** - ALL verification is agent-executed.

### Test Decision
- **Infrastructure exists**: YES
- **Automated tests**: Tests-after (add tests for new code, not rewrite existing)
- **Framework**: GTest (existing)
- **Agent-Executed QA**: MANDATORY for all tasks

### QA Policy
- **Frontend/UI**: Playwright opens widget_gallery, switches themes, observes effects
- **CLI/Math**: Bash runs unit tests for easing curves, CSS parser
- **API/Backend**: curl validates theme JSON export/import

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (Foundation):
├── Task 1: Easing curve library [quick]
└── Task 2: EffectScope base class [unspecified-high]

Wave 2 (Effects Implementation — MAX PARALLEL):
├── Task 3: ShadowEffect [visual-engineering]
├── Task 4: GlowEffect [visual-engineering]
├── Task 5: BlurEffect [unspecified-high]
└── Task 6: GradientBrush [visual-engineering]

Wave 3 (Animation System):
├── Task 7: AnimationState manager [unspecified-high]
└── Task 8: Transition utils [quick]

Wave 4 (CSS Engine v2 — MAX PARALLEL):
├── Task 9: CSS property registry (50 properties) [unspecified-high]
├── Task 10: CSS gradient parser [unspecified-high]
├── Task 11: CSS animation/transition parser [unspecified-high]
├── Task 12: CSS pseudo-class expansion [quick]
└── Task 13: CSS media query support [quick]

Wave 5 (Theme Library — MAX PARALLEL):
├── Task 14: Theme preset framework [quick]
├── Task 15: Material theme [visual-engineering]
├── Task 16: Fluent theme [visual-engineering]
├── Task 17: Dracula + Nord themes [visual-engineering]
├── Task 18: Gruvbox + Catppuccin [visual-engineering]
├── Task 19: Solarized + TokyoNight [visual-engineering]
├── Task 20: OneDark + Everforest [visual-engineering]
└── Task 21: Live theme switching [quick]

Wave 6 (Widget Polish — MAX PARALLEL):
├── Task 22: Button polish (gradient hover) [visual-engineering]
├── Task 23: Panel polish (rounded corners + shadow) [visual-engineering]
├── Task 24: Toast polish (eased slide + stack) [visual-engineering]
├── Task 25: ProgressBar polish (gradient fill) [visual-engineering]
├── Task 26: ToggleSwitch polish (eased transition) [visual-engineering]
└── Task 27: TabWidget transition animation [visual-engineering]

Wave 7 (New Widgets — MAX PARALLEL):
├── Task 28: Card component [visual-engineering]
├── Task 29: HeroSection component [visual-engineering]
├── Task 30: SkeletonScreen component [unspecified-high]
├── Task 31: Shimmer component [visual-engineering]
├── Task 32: Badge component [quick]
└── Task 33: GradientText component [quick]

Wave 8 (Integration):
├── Task 34: Update widget_gallery with all demos [visual-engineering]
├── Task 35: Wire effects into existing widgets [visual-engineering]
├── Task 36: Theme demo example [quick]
└── Task 37: Performance benchmarks [quick]

Wave FINAL (Review):
├── Task F1: Plan compliance audit (oracle)
├── Task F2: Code quality review (unspecified-high)
├── Task F3: Real manual QA (unspecified-high + playwright)
└── Task F4: Scope fidelity check (deep)

Critical Path: T1 → T2 → T3 → T7 → T14 → T22 → T34 → F1-F4
Max Concurrent: 8 (Waves 5 & 6)

### Dependency Matrix

| Task | Depends On | Blocks |
|------|-----------|--------|
| 1-2 | - | 3-8 |
| 3  | 2 | 22, 23, 28, 34 |
| 4  | 2 | 34 |
| 5  | 2 | 23, 34 |
| 6  | - | 10, 22, 25, 29, 31, 33, 34 |
| 7  | 1 | 8, 22, 26, 27, 31, 34 |
| 8  | 7 | 24, 27, 34 |
| 9  | 3, 4, 6 | 10-13, 34 |
| 10 | 6, 9 | 34 |
| 11-13 | 9 | 34 |
| 14 | - | 15-21, 34 |
| 15-20 | 14 | 34 |
| 21 | 14 | 34 |
| 22 | 6, 7 | 34, 35 |
| 23 | 3 | 34, 35 |
| 24 | 8 | 34 |
| 25 | 6 | 34 |
| 26 | 7 | 34 |
| 27 | 7, 8 | 34 |
| 28 | 3 | 34 |
| 29 | 6 | 34 |
| 30 | - | 31, 34 |
| 31 | 6, 30 | 34 |
| 32-33 | 6 | 34 |
| 34 | 1-33 | 35-37 |
| 35 | 22-27 | F1-F4 |
| 36-37 | 14-21, 3-6 | F1-F4 |

### Agent Dispatch Summary

- **Wave 1**: 2 tasks — T1→`quick`, T2→`unspecified-high`
- **Wave 2**: 4 tasks — T3,T4,T6→`visual-engineering`, T5→`unspecified-high`
- **Wave 3**: 2 tasks — T7→`unspecified-high`, T8→`quick`
- **Wave 4**: 5 tasks — T9,T10,T11→`unspecified-high`, T12,T13→`quick`
- **Wave 5**: 8 tasks — T14,T21→`quick`, T15-T20→`visual-engineering`
- **Wave 6**: 6 tasks — T22-T27→`visual-engineering`
- **Wave 7**: 6 tasks — T28,T29,T31→`visual-engineering`, T30→`unspecified-high`, T32,T33→`quick`
- **Wave 8**: 4 tasks — T34,T35→`visual-engineering`, T36,T37→`quick`
- **FINAL**: 4 tasks — F1→`oracle`, F2→`unspecified-high`, F3→`unspecified-high`+`playwright`, F4→`deep`
```

---

## TODOs

### Wave 1 — Foundation

- [ ] 1. Easing curve library (`include/unigui/fx/easing.h`)

  **What to do**:
  - Create header-only `include/unigui/fx/easing.h` under `namespace unigui::fx`
  - Implement 10 easing functions: `linear`, `quadIn`, `quadOut`, `quadInOut`, `cubicIn`, `cubicOut`, `expoIn`, `expoOut`, `elasticOut`, `bounceOut`
  - Each takes `float t` (0..1) returns `float` (0..1)
  - Add `EasingCurve` enum for string-based selection: `Ease("bounceOut", 0.3f)`

  **Must NOT do**:
  - No external dependencies (use math.h only)
  - No per-frame state management (that's Task 7)

  **Recommended Agent Profile**:
  - **Category**: `quick` — pure math, single header, no ImGui dependency

  **Parallelization**: Wave 1 — parallel with Task 2
  **Blocked By**: None

  **QA Scenarios**:
  ```
  Scenario: All 10 curves return [0,1] range
    Tool: Bash (compile test_easing.cc with GTest)
    Steps: 1. Compile test 2. Run: EXPECT_NEAR(easing::linear(0.0f), 0.0f)
           3. Run: EXPECT_NEAR(easing::linear(0.5f), 0.5f)
           4. Run: EXPECT_NEAR(easing::bounceOut(1.0f), 1.0f, 0.01f)
    Expected: All 10 curves validated, edge cases (0, 1) correct
    Evidence: .omo/evidence/task-1-easing.txt

  Scenario: Ease() string lookup works
    Tool: Bash
    Steps: 1. EXPECT_NEAR(Ease("quadInOut", 0.5f), 0.5f, 0.01f)
           2. EXPECT_EQ(Ease("invalid", 0.5f), 0.5f) — fallback to linear
    Expected: String lookup matches enum, unknown falls back to linear
    Evidence: .omo/evidence/task-1-easing-lookup.txt
  ```

  **Commit**: YES (groups with T2)
  - Message: `feat(fx): add easing curve library (10 curves)`
  - Files: `include/unigui/fx/easing.h`, `tests/fx/easing_test.cc`

- [ ] 2. EffectScope base class + render layer

  **What to do**:
  - Create `include/unigui/fx/effect_scope.h` and `src/fx/effect_scope.cc`
  - Define `EffectScope` RAII base class in `namespace unigui::fx`
  - Provides `Push(ImDrawList* dl)` / `Pop()` virtual methods
  - Create `Effects` singleton: `Effects::Shadow(radius, offset, color)`, `Effects::Glow(radius, color)`
  - Create `Effects::Blur(radius)` placeholder (render-to-texture for later)
  - Each returns an `EffectScope` that manipulates ImDrawList during its lifetime

  **Must NOT do**:
  - No full blur implementation (requires render target — placeholder only)
  - No particle system (that's later)

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high` — needs ImDrawList understanding

  **Parallelization**: Wave 1 — parallel with Task 1
  **Blocked By**: None

  **QA Scenarios**:
  ```
  Scenario: EffectScope Push/Pop lifecycle
    Tool: Bash (compile + GTest)
    Steps: 1. Create Effects::Shadow(5.f, {2,2}, 0x40000000)
           2. Verify Push stores ImDrawList pointer
           3. Verify Pop restores original state
    Expected: RAII pattern works, no crash on double-pop
    Evidence: .omo/evidence/task-2-effectscope.txt
  ```

  **Commit**: YES (groups with T1)
  - Message: `feat(fx): EffectScope base class + render layer`
  - Files: `include/unigui/fx/effect_scope.h`, `src/fx/effect_scope.cc`, `tests/fx/effect_scope_test.cc`

### Wave 2 — Effects Implementation

- [ ] 3. ShadowEffect implementation

  **What to do**:
  - Implement `ShadowEffect` inheriting `EffectScope`
  - On `Push()`: draws blurred rects behind current widget using ImDrawList::AddShadowRect (ImGui 1.92.8 API) or manual multi-pass alpha rects
  - Configurable: `radius`, `offset_x`, `offset_y`, `color` (RGBA)
  - Widget integration: `ShadowEffect shadow(8.f, {4,4}, IM_COL32(0,0,0,100)); shadow.Push(dl); /* draw widget */ shadow.Pop();`
  - Test in widget_gallery: panel with drop shadow

  **Must NOT do**:
  - No GPU compute shaders
  - No per-pixel accurate shadow (approximate with multi-layer rects)

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering` — visual output, needs rendering expertise
  - **Skills**: [`playwright`] — verify shadow renders visually

  **Parallelization**: Wave 2 — parallel with T4, T5, T6
  **Blocked By**: T2

  **QA Scenarios**:
  ```
  Scenario: Shadow renders with correct offset
    Tool: Playwright
    Preconditions: widget_gallery running, ShadowEffect enabled on a panel
    Steps: 1. Navigate to shadow demo panel
           2. Screenshot the panel
           3. Verify dark region exists below+right of panel bounds
           4. Measure offset pixels ≈ configured offset
    Expected: Visible drop shadow below panel
    Evidence: .omo/evidence/task-3-shadow.png

  Scenario: Shadow disabled has no visual artifact
    Tool: Playwright
    Steps: 1. Toggle shadow off 2. Screenshot 3. Verify no residual rects
    Expected: Clean panel, no ghosting
    Evidence: .omo/evidence/task-3-no-shadow.png
  ```

  **Commit**: YES (groups with T4, T5, T6)
  - Message: `feat(fx): ShadowEffect`

- [ ] 4. GlowEffect implementation

  **What to do**:
  - Implement `GlowEffect` inheriting `EffectScope`
  - On `Push()`: draws expanding glow rects around widget using `ImDrawList::AddCircleFilled` or `AddRectFilled` with decreasing alpha
  - Configurable: `radius` (spread), `color` (typically bright color)
  - Use case: button hover glow, selected item highlight

  **Must NOT do**:
  - No custom shaders

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`

  **Parallelization**: Wave 2 — parallel with T3, T5, T6
  **Blocked By**: T2

  **QA Scenarios**:
  ```
  Scenario: Glow renders around button
    Tool: Playwright
    Steps: 1. Navigate to glow demo 2. Hover button
           3. Screenshot 4. Verify colored halation around button
    Expected: Visible glow ring
    Evidence: .omo/evidence/task-4-glow.png
  ```

  **Commit**: YES (groups with T3, T5, T6)

- [ ] 5. BlurEffect placeholder + glass morphism

  **What to do**:
  - Implement `BlurEffect` with two modes:
    a. **Approximate blur** using multi-pass semi-transparent rects with offset (fast, no render target)
    b. **Glass morphism preset**: semi-transparent background + blur approximation + subtle border
  - Add `Effects::GlassPanel()` convenience: combines BlurEffect + white 20% alpha bg + 1px white border
  - Configurable: `blur_radius`, `bg_alpha`, `border_color`

  **Must NOT do**:
  - No render-to-texture blur (requires GPU pipeline — v4)
  - Approximate blur is acceptable for v3

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`

  **Parallelization**: Wave 2
  **Blocked By**: T2

  **QA Scenarios**:
  ```
  Scenario: Glass panel has transparent bg
    Tool: Playwright
    Steps: 1. Place glass panel over colorful background
           2. Screenshot 3. Verify bg color bleeds through panel
    Expected: Translucent panel, not 100% opaque
    Evidence: .omo/evidence/task-5-glass.png
  ```

  **Commit**: YES (groups with T3, T4, T6)

- [ ] 6. GradientBrush implementation

  **What to do**:
  - Implement `GradientBrush` utility (not EffectScope — standalone)
  - `GradientBrush::Horizontal(startColor, endColor)` — fills ImDrawList rect
  - `GradientBrush::Vertical(startColor, endColor)` — fills ImDrawList rect
  - `GradientBrush::MultiStop(colors, stops)` — multiple color stops
  - Used by: Button backgrounds, ProgressBar fill, Panel headers
  - Add `ImDrawList` helper: `AddRectFilledMultiColor()` for gradient

  **Must NOT do**:
  - No gradient text (that's Task 33)
  - No radial gradient (edge case for v3)

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`

  **Parallelization**: Wave 2
  **Blocked By**: None (uses ImDrawList directly)

  **QA Scenarios**:
  ```
  Scenario: Horizontal gradient renders 2 colors
    Tool: Playwright
    Steps: 1. Draw rect with GradientBrush::Horizontal(red, blue)
           2. Screenshot 3. Verify left side red, right side blue
    Expected: Smooth color transition visible
    Evidence: .omo/evidence/task-6-gradient.png
  ```

  **Commit**: YES (groups with T3, T4, T5)

### Wave 3 — Animation System

- [ ] 7. AnimationState manager

  **What to do**:
  - Create `include/unigui/fx/animation.h` and `src/fx/animation.cc`
  - `AnimationState` struct: `float progress`, `EasingCurve curve`, `float duration`, `float elapsed`, `bool playing`
  - `AnimationManager` singleton: `Play(id, curve, duration)`, `Stop(id)`, `GetProgress(id)`, `Update(dt)`
  - Widget integration: each widget gets `AnimationState` member, calls `anim.Update(dt)` each frame
  - Restart support: `Play()` resets `elapsed=0` and sets `playing=true`
  - Completion callback: `OnComplete(std::function<void()>)`

  **Must NOT do**:
  - No keyframe system (v4)
  - No animation sequencing/chaining

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high` — state management, threading considerations

  **Parallelization**: Wave 3 — parallel with T8
  **Blocked By**: T1 (easing curves)

  **QA Scenarios**:
  ```
  Scenario: Animation plays to completion
    Tool: Bash (GTest)
    Steps: 1. Create AnimationState(0.5f, EasingCurve::linear)
           2. Call Play() 3. Update(dt=0.25f) 4. Verify progress=0.5
           5. Update(dt=0.25f) 6. Verify progress=1.0, playing=false
    Expected: Progress goes 0→1 over duration, stops at 1
    Evidence: .omo/evidence/task-7-animation.txt

  Scenario: Restart resets state
    Tool: Bash (GTest)
    Steps: 1. Play, update to 50%, 2. Play again
           3. Verify elapsed=0, progress=0
    Expected: Full reset
    Evidence: .omo/evidence/task-7-restart.txt
  ```

  **Commit**: YES (groups with T8)
  - Message: `feat(fx): AnimationState manager`
  - Files: `include/unigui/fx/animation.h`, `src/fx/animation.cc`, `tests/fx/animation_test.cc`

- [ ] 8. Transition utilities

  **What to do**:
  - Create `include/unigui/fx/transition.h` (header-only)
  - `Transition::Fade(widget, alpha, duration, curve)` — animates widget alpha
  - `Transition::SlideIn(widget, from_x, from_y, duration, curve)` — slides widget from position
  - `Transition::Scale(widget, from_scale, to_scale, duration, curve)` — scales widget
  - `Transition::CrossFade(fromWidget, toWidget, duration, curve)` — cross-fade between two widgets
  - All use `AnimationState` internally, auto-manage lifecycle
  - `Transition::OnPageSwitch(from, to)` — convenience for page transitions

  **Must NOT do**:
  - No 3D transforms (perspective, rotation-3d)
  - No spring physics (just easing curves)

  **Recommended Agent Profile**:
  - **Category**: `quick` — header-only wrappers

  **Parallelization**: Wave 3
  **Blocked By**: T7

  **QA Scenarios**:
  ```
  Scenario: Fade transition works
    Tool: Bash (GTest)
    Steps: 1. Transition::Fade(widget, 1.0f, 0.5f, EasingCurve::easeOut)
           2. Update for 0.5s 3. Verify widget alpha goes 0→1
    Expected: Smooth alpha transition
    Evidence: .omo/evidence/task-8-transition.txt
  ```

  **Commit**: YES (groups with T7)

### Wave 4 — CSS Engine v2

- [ ] 9. CSS property registry expansion (16 → 50+)

  **What to do**:
  - Expand `style_engine.h` property map to 50+ properties:
    **Colors**: `text`, `bg`, `border-color`, `accent`, `text-secondary`, `bg-secondary`, `bg-tertiary`, `text-disabled`, `border-hover`, `bg-hover`, `bg-active`, `title-bg`, `title-text`, `header-bg`, `header-text`
    **Sizing**: `rounding`, `padding`, `padding-x`, `padding-y`, `margin`, `margin-x`, `margin-y`, `spacing`, `scrollbar-size`, `border-width`, `min-width`, `min-height`, `max-width`, `max-height`
    **Typography**: `font-size`, `font-family`, `font-weight`, `text-align`, `line-height`, `letter-spacing`
    **Effects**: `box-shadow`, `opacity`, `transition`, `animation`, `blur`
    **Decorations**: `border-radius`, `border-radius-top-left`, `border-radius-top-right`, `border-radius-bottom-left`, `border-radius-bottom-right`
    **Layout**: `display`, `flex-direction`, `align-items`, `justify-content`, `gap`
    **Background**: `bg-gradient`, `bg-gradient-direction`
  - Add `CSSPropertyType` enum with type-safe parsing
  - Validate `PropertyParser::Parse(prop, value)` returns error for unknown props

  **Must NOT do**:
  - No runtime property validation (parse-time only)
  - No inherited properties (all explicit)

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high` — large config surface, needs careful enum design

  **Parallelization**: Wave 4 — parallel with T10, T11, T12, T13
  **Blocked By**: T3, T4, T6 (shadow/glow/gradient must exist before CSS can reference them)

  **QA Scenarios**:
  ```
  Scenario: All 50 properties parse without error
    Tool: Bash (GTest)
    Steps: 1. Parse CSS with all 50 properties
           2. Verify parsed count = property count
           3. Verify each property value type matches expected type
    Expected: 50/50 accepted, type-safe parsing
    Evidence: .omo/evidence/task-9-css-props.txt

  Scenario: Unknown property returns error
    Tool: Bash
    Steps: 1. Parse "Button { unknown-prop: value; }"
           2. Verify engine reports unknown property
    Expected: Error logged, rule skipped, rest parse
    Evidence: .omo/evidence/task-9-unknown-prop.txt
  ```

  **Commit**: YES (groups with T10-T13)

- [ ] 10. CSS gradient parser

  **What to do**:
  - Extend CSS parser to handle gradient values:
    `bg-gradient: linear-gradient(90deg, #ff0000, #0000ff)`
    `bg-gradient: linear-gradient(to right, #e94560, #0f3460)`
  - Parse direction: angle (90deg), keywords (to right, to bottom, to top left)
  - Parse color stops: 2+ colors, optional percentage positions
  - Store parsed gradient in `StyleRule` as `GradientDesc` struct
  - Wire to `GradientBrush` (Task 6) when applying rule

  **Must NOT do**:
  - No radial gradient `radial-gradient()` syntax
  - No conic gradient

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high` — complex string parsing

  **Parallelization**: Wave 4
  **Blocked By**: T6, T9

  **QA Scenarios**:
  ```
  Scenario: Linear gradient parses with angle
    Tool: Bash (GTest)
    Steps: 1. Parse "Button { bg-gradient: linear-gradient(90deg, #f00, #00f); }"
           2. Verify direction=90deg, stops=[#ff0000, #0000ff]
    Expected: Correct direction + color stops extracted
    Evidence: .omo/evidence/task-10-gradient-parse.txt
  ```

  **Commit**: YES (groups with T9, T11-T13)

- [ ] 11. CSS animation/transition parser

  **What to do**:
  - Parse `transition` shorthand: `transition: opacity 0.3s ease-in-out`
  - Parse `animation` shorthand: `animation: fadeIn 0.5s ease-out`
  - Parse individual properties: `transition-duration`, `transition-timing-function`, `transition-property`
  - Store `TransitionDesc` and `AnimationDesc` in `StyleRule`
  - Accepted timing functions: linear, ease, ease-in, ease-out, ease-in-out, plus all 10 easing curve names
  - Accepted properties for transition: opacity, color, bg, rounding, padding, font-size, shadow

  **Must NOT do**:
  - No keyframe `@keyframes` syntax (v4)
  - No `animation-delay` or `animation-iteration-count`

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`

  **Parallelization**: Wave 4
  **Blocked By**: T9

  **QA Scenarios**:
  ```
  Scenario: Transition shorthand parses correctly
    Tool: Bash (GTest)
    Steps: 1. Parse "Button { transition: opacity 0.3s ease-out; }"
           2. Verify property=opacity, duration=0.3s, timing=ease-out
    Expected: Correct decomposition
    Evidence: .omo/evidence/task-11-transition-parse.txt
  ```

  **Commit**: YES (groups with T9, T10, T12, T13)

- [ ] 12. CSS pseudo-class expansion

  **What to do**:
  - Currently only `:hover` is supported. Add:
    `:active`, `:focus`, `:disabled`, `:checked`, `:first-child`, `:last-child`, `:nth-child(n)`
  - Implement `MatchPseudoClass(rule, widgetState)` that checks:
    - Widget's current hover/active/focus/disabled state
    - Widget's index in parent (for first/last/nth)
  - Update `Apply()` to accept widget state + index params

  **Must NOT do**:
  - No `:before`/`:after` pseudo-elements (ImGui limitation)
  - No `:not()` negation

  **Recommended Agent Profile**:
  - **Category**: `quick` — logic extension, no rendering

  **Parallelization**: Wave 4
  **Blocked By**: T9

  **QA Scenarios**:
  ```
  Scenario: nth-child selector matches correctly
    Tool: Bash (GTest)
    Steps: 1. Parse "Label:nth-child(2) { text: #ff0000; }"
           2. Apply to Label at index 2
           3. Verify rule matches 4. Apply to Label at index 0, verify no match
    Expected: Only 2nd child styled
    Evidence: .omo/evidence/task-12-pseudo.txt
  ```

  **Commit**: YES (groups with T9-T11, T13)

- [ ] 13. CSS media query support

  **What to do**:
  - Parse `@media` blocks: `@media (min-width: 800px) { Window { rounding: 12; } }`
  - Supported queries: `min-width`, `max-width`, `min-height`, `max-height`, `prefers-color-scheme` (dark/light)
  - `MediaQuery::Evaluate(currentWidth, currentHeight, colorScheme)` returns bool
  - Store active media queries, evaluate on window resize
  - Apply matched media query rules on top of base rules

  **Must NOT do**:
  - No `and`/`or` combinators (single query per block)
  - No `orientation` or `resolution` queries

  **Recommended Agent Profile**:
  - **Category**: `quick` — single block parser

  **Parallelization**: Wave 4
  **Blocked By**: T9

  **QA Scenarios**:
  ```
  Scenario: min-width media query activates
    Tool: Bash (GTest)
    Steps: 1. Parse "@media (min-width: 800px) { Window { rounding: 12; } }"
           2. Evaluate at width=1024: verify match
           3. Evaluate at width=640: verify no match
    Expected: Only applies at >= 800px
    Evidence: .omo/evidence/task-13-media-query.txt
  ```

  **Commit**: YES (groups with T9-T12)

### Wave 5 — Theme Library

- [ ] 14. Theme preset framework

  **What to do**:
  - Create `include/unigui/theme/presets/preset.h` — base struct for all themes
  - `ThemePreset` struct: `name`, `description`, `colors[53]` (matching ImGui color enum), `style_overrides` (rounding, spacing, etc.)
  - `ThemeRegistry` singleton: `Register(preset)`, `Get(name)`, `ListNames()`
  - `ApplyPreset(name)` — replaces current `ApplyTheme()` with preset data
  - `ExportPreset(name) -> JSON`, `ImportPreset(json) -> preset`
  - Real-time switching: `ApplyPreset()` re-applies all 53 colors + style settings

  **Must NOT do**:
  - No GUI theme editor
  - No hot-reload from file (v4)

  **Recommended Agent Profile**:
  - **Category**: `quick` — data structure + registry pattern

  **Parallelization**: Wave 5 — parallel with T15-T21
  **Blocked By**: None

  **QA Scenarios**:
  ```
  Scenario: Apply preset changes all 53 colors
    Tool: Bash (GTest)
    Steps: 1. Register preset with specific WindowBg color
           2. ApplyPreset("test")
           3. Verify ImGui::GetStyle().Colors[ImGuiCol_WindowBg] matches
    Expected: All 53 colors updated, no partial application
    Evidence: .omo/evidence/task-14-preset-apply.txt

  Scenario: JSON round-trip preserves colors
    Tool: Bash (GTest)
    Steps: 1. Export preset to JSON 2. Import JSON to new preset
           3. Verify all 53 colors match original
    Expected: No data loss
    Evidence: .omo/evidence/task-14-preset-json.txt
  ```

  **Commit**: YES (groups with T15-T21)
  - Message: `feat(theme): preset framework + registry`

- [ ] 15. Material Design theme

  **What to do**:
  - Create `include/unigui/theme/presets/material.h`
  - Google Material Design 3 color palette (M3)
  - Color scheme: Primary (#6750A4), Surface (#FFFBFE), SurfaceVariant (#E7E0EC), dark variants
  - Both `MaterialLight` and `MaterialDark` presets
  - Key visual: elevated surfaces, colored primary buttons, rounded corners (12px), subtle shadows
  - Register in ThemeRegistry

  **Must NOT do**:
  - No Material ripple effect (separate task)

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering` — color palette design

  **Parallelization**: Wave 5
  **Blocked By**: T14

  **QA Scenarios**:
  ```
  Scenario: Material theme renders
    Tool: Playwright
    Steps: 1. Launch widget_gallery with Material Dark
           2. Screenshot 3. Verify purple-tinted primary color
    Expected: M3 palette visible (purple accent)
    Evidence: .omo/evidence/task-15-material.png
  ```

  **Commit**: YES (groups with T14, T16-T21)

- [ ] 16. Fluent Design theme

  **What to do**:
  - Create `include/unigui/theme/presets/fluent.h`
  - Microsoft Fluent 2 palette
  - Color scheme: Accent (#0078D4), Surface light (#FFFFFF), Surface dark (#1F1F1F)
  - Both `FluentLight` and `FluentDark`
  - Key visual: acrylic-style transparency hints, blue accent, clean geometric shapes
  - Register in ThemeRegistry

  **Must NOT do**:
  - No acrylic blur (use approximate BlurEffect from T5)

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`

  **Parallelization**: Wave 5
  **Blocked By**: T14

  **QA Scenarios**:
  ```
  Scenario: Fluent theme renders
    Tool: Playwright
    Steps: 1. Launch with Fluent Dark 2. Screenshot
           3. Verify blue accent color present
    Expected: Windows 11 aesthetic
    Evidence: .omo/evidence/task-16-fluent.png
  ```

  **Commit**: YES (groups with T14, T15, T17-T21)

- [ ] 17. Dracula + Nord themes

  **What to do**:
  - Create `dracula.h` and `nord.h`
  - **Dracula**: Base #282a36, accent #bd93f9 (purple), green #50fa7b, pink #ff79c6 — dark-only
  - **Nord**: Base #2e3440, accent #88c0d0 (frost blue), snow #eceff4 — dark-only
  - Both register as dark-only presets

  **Must NOT do**:
  - No light variants for these (community dark themes)

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`

  **Parallelization**: Wave 5
  **Blocked By**: T14

  **QA Scenarios**:
  ```
  Scenario: Dracula renders purple accent
    Tool: Playwright
    Steps: 1. Apply Dracula 2. Screenshot
           3. Verify purple tint on buttons/headers
    Evidence: .omo/evidence/task-17-dracula.png
  ```

  **Commit**: YES (groups with T14-T16, T18-T21)

- [ ] 18. Gruvbox + Catppuccin themes

  **What to do**:
  - Create `gruvbox.h` and `catppuccin.h`
  - **Gruvbox**: Base #282828, accent #d79921 (yellow), red #cc241d — dark (also soft version #32302f)
  - **Catppuccin Mocha**: Base #1e1e2e, accent #cba6f7 (lavender), rosewater #f5e0dc — dark
  - Both register as dark-only

  **Must NOT do**:
  - No Catppuccin Latte/Frappe/Macchiato variants (too many)

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`

  **Parallelization**: Wave 5
  **Blocked By**: T14

  **Commit**: YES (groups with T14-T17, T19-T21)

- [ ] 19. Solarized + TokyoNight themes

  **What to do**:
  - Create `solarized.h` and `tokyonight.h`
  - **Solarized**: Both Dark (#002b36, accent #268bd2) and Light (#fdf6e3, accent #268bd2)
  - **TokyoNight**: Dark-only, base #1a1b26, accent #7aa2f7 (blue), border #565f89
  - Register all 3 presets (SolarizedDark, SolarizedLight, TokyoNight)

  **Must NOT do**:
  - No TokyoNight Storm/Moon variants

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`

  **Parallelization**: Wave 5
  **Blocked By**: T14

  **Commit**: YES (groups with T14-T18, T20, T21)

- [ ] 20. OneDark + Everforest themes

  **What to do**:
  - Create `onedark.h` and `everforest.h`
  - **OneDark**: Base #282c34, accent #61afef (blue), green #98c379 — dark
  - **Everforest**: Base #2d353b, accent #a7c080 (green), aqua #83c092 — dark
  - Both dark-only

  **Must NOT do**:
  - No OneLight variant

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`

  **Parallelization**: Wave 5
  **Blocked By**: T14

  **Commit**: YES (groups with T14-T19, T21)

- [ ] 21. Live theme switching UI

  **What to do**:
  - Add `ThemeSwitcher` widget: dropdown listing all registered themes
  - `OnChange` callback: calls `ApplyPreset(selected)` immediately
  - Show theme preview colors in dropdown (small color swatches)
  - Add to `AppConfig`: `default_preset` field
  - Wire into `widget_gallery` menu bar
  - Smoke test: switch between all 10 themes rapidly, verify no crash/memory leak

  **Must NOT do**:
  - No animation during theme switch (instant is fine for v3)

  **Recommended Agent Profile**:
  - **Category**: `quick` — simple combo box + callback

  **Parallelization**: Wave 5 — depends on T14-T20
  **Blocked By**: T14 (presets must exist)

  **QA Scenarios**:
  ```
  Scenario: Theme switch updates immediately
    Tool: Playwright
    Steps: 1. Launch widget_gallery with Dark default
           2. Select "Dracula" from switcher
           3. Screenshot 4. Verify colors changed (purple accent visible)
           5. Switch to "Material Light" 6. Screenshot
           7. Verify light background visible
    Expected: Seamless switching, no flicker
    Evidence: .omo/evidence/task-21-switch-1.png, task-21-switch-2.png
  ```

  **Commit**: YES (groups with T14-T20)

### Wave 6 — Widget Polish

- [ ] 22. Button polish (gradients, hover, active, ripple)

  **What to do**:
  - Update `widgets/button.cc`:
    - Default background: solid color (from theme)
    - Hover state: GradientBrush horizontal from bg to bg-hover
    - Active state: gradient reverse direction + slight scale
    - Disabled state: reduced opacity (0.5), grayscale effect
    - RippleEffect on click: expanding circle from click point (using AnimationState)
  - CSS-driven: respect `transition` property if set on Button
  - No breaking changes to `Button` API

  **Must NOT do**:
  - No change to `WasClicked()` / `SetEnabled()` API

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`
  - **Skills**: [`playwright`]

  **Parallelization**: Wave 6 — parallel with T23-T27
  **Blocked By**: T6 (GradientBrush), T7 (AnimationState)

  **QA Scenarios**:
  ```
  Scenario: Button shows gradient on hover
    Tool: Playwright
    Steps: 1. Launch widget_gallery 2. Hover "Primary" button
           3. Screenshot 4. Verify color transition (not instant)
           5. Move mouse away 6. Verify returns to solid
    Expected: Smooth hover gradient transition
    Evidence: .omo/evidence/task-22-button-hover.png

  Scenario: Ripple effect on click
    Tool: Playwright
    Steps: 1. Click button 2. Capture frames during animation
           3. Verify expanding circle from click point
    Expected: Ripple fades out over ~0.3s
    Evidence: .omo/evidence/task-22-ripple.mp4
  ```

  **Commit**: YES (groups with T23-T27)

- [ ] 23. Panel polish (rounded corners, shadow)

  **What to do**:
  - Update `widgets/panel.cc`:
    - Add `SetShadow(bool)` / `SetShadowRadius(float)` API
    - Default: subtle 4px shadow with 2px offset
    - Rounded corners: use `ImDrawFlags_RoundCornersAll` with theme's rounding
    - Optional: glass style via `SetGlass(bool)` using BlurEffect
  - CSS-driven: apply `box-shadow` and `border-radius` from CSS rules

  **Must NOT do**:
  - No breaking API changes (additive only)

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`
  - **Skills**: [`playwright`]

  **Parallelization**: Wave 6
  **Blocked By**: T3 (ShadowEffect)

  **QA Scenarios**:
  ```
  Scenario: Panel drops shadow
    Tool: Playwright
    Steps: 1. Render panel with SetShadow(true)
           2. Screenshot 3. Verify visible shadow below+right of panel
    Expected: Soft drop shadow
    Evidence: .omo/evidence/task-23-panel-shadow.png
  ```

  **Commit**: YES (groups with T22, T24-T27)

- [ ] 24. Toast polish (eased slide-in + stack animation)

  **What to do**:
  - Update `widgets/toast.cc`:
    - Replace instant show with `SlideIn(from_y=50px, duration=0.3s, curve=easeOut)`
    - Stack management: new toasts push existing ones up by toast height
    - Dismiss: `FadeOut(duration=0.2s, curve=easeIn)` then remove
    - Add `SetAnimation(bool)` toggle
  - CSS-driven: respect `transition` property

  **Must NOT do**:
  - No change to Toast::Info/Success/Warn/Error static API

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`
  - **Skills**: [`playwright`]

  **Parallelization**: Wave 6
  **Blocked By**: T8 (Transition utils)

  **QA Scenarios**:
  ```
  Scenario: Toast slides in from bottom
    Tool: Playwright
    Steps: 1. Trigger Toast::Info("test") 2. Capture frames
           3. Verify toast position animates from below to final position
    Expected: Smooth slide animation
    Evidence: .omo/evidence/task-24-toast-slide.mp4
  ```

  **Commit**: YES (groups with T22, T23, T25-T27)

- [ ] 25. ProgressBar polish (gradient fill, animated)

  **What to do**:
  - Update `widgets/progressbar.cc`:
    - Fill: GradientBrush horizontal (accent → accent-light) instead of solid
    - Animation: smooth fill transition when `SetFraction()` changes
    - Indeterminate mode: shimmer/scanning effect (gradient sweeps left→right)
    - Add `SetAnimated(bool)` and `SetGradient(bool)` API

  **Must NOT do**:
  - No breaking API

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`

  **Parallelization**: Wave 6
  **Blocked By**: T6 (GradientBrush)

  **QA Scenarios**:
  ```
  Scenario: Progress bar fills with gradient
    Tool: Playwright
    Steps: 1. Set progress bar to 75% 2. Screenshot
           3. Verify fill uses gradient (left color ≠ right color)
    Expected: Gradient fill visible
    Evidence: .omo/evidence/task-25-progress.png
  ```

  **Commit**: YES (groups with T22-T24, T26, T27)

- [ ] 26. ToggleSwitch polish (eased knob transition)

  **What to do**:
  - Update `widgets/toggleswitch.cc`:
    - Animate knob position: eased slide from OFF→ON position
    - Animate track color: smooth transition bg→accent
    - Use `AnimationState` with 0.2s duration, easeOut curve
  - Add `SetAnimated(bool)` toggle (default ON)

  **Must NOT do**:
  - No breaking API

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`

  **Parallelization**: Wave 6
  **Blocked By**: T7 (AnimationState)

- [ ] 27. TabWidget transition animation

  **What to do**:
  - Update `widgets/tabwidget.cc`:
    - Tab switch: `CrossFade` between old and new tab content (0.2s)
    - Active tab indicator: slide from old position to new position
    - Use `AnimationState` for indicator position lerp
  - Add `SetAnimated(bool)` toggle (default ON)

  **Must NOT do**:
  - No breaking API

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`

  **Parallelization**: Wave 6
  **Blocked By**: T7, T8

  **Commit**: YES (groups with T22-T26)

### Wave 7 — New Widgets

- [ ] 28. Card component

  **What to do**:
  - Create `include/unigui/widgets/card.h` and `src/widgets/card.cc`
  - `Card` widget: `SetTitle(string)`, `SetContent(fn)`, `SetImage(textureID)`, `SetFooter(fn)`
  - Visual: rounded corners (8px), drop shadow, padding (16px), optional header image
  - Variants: `Card::Elevated` (shadow), `Card::Outlined` (border only), `Card::Filled` (bg color, no shadow)
  - CSS: respects `box-shadow`, `border-radius`, `bg`

  **Must NOT do**:
  - No card carousel/grid (use VBox/HBox for layout)

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`
  - **Skills**: [`playwright`]

  **Parallelization**: Wave 7 — parallel with T29-T33
  **Blocked By**: T3 (ShadowEffect)

  **QA Scenarios**:
  ```
  Scenario: Card renders with shadow
    Tool: Playwright
    Steps: 1. Render Card::Elevated with title + content
           2. Screenshot 3. Verify shadow, rounded corners, title visible
    Expected: Card looks like a raised surface
    Evidence: .omo/evidence/task-28-card.png
  ```

  **Commit**: YES (groups with T29-T33)

- [ ] 29. HeroSection component

  **What to do**:
  - Create `include/unigui/widgets/herosection.h` and impl
  - `HeroSection` widget: tall banner with `SetTitle(string)`, `SetSubtitle(string)`, `SetBackground(textureID or gradient)`, `SetActionButton(label, callback)`
  - Visual: full-width, gradient or image background, centered text, large title font
  - Optional: subtle parallax effect on scroll (offset shifts slightly)
  - CSS: respects `bg-gradient`, `min-height`

  **Must NOT do**:
  - No video background
  - No carousel (multiple hero slides)

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`

  **Parallelization**: Wave 7
  **Blocked By**: T6 (GradientBrush)

  **Commit**: YES (groups with T28, T30-T33)

- [ ] 30. SkeletonScreen component

  **What to do**:
  - Create `include/unigui/widgets/skeleton.h` and impl
  - `SkeletonScreen` widget: placeholder loading UI
  - `AddBlock(width, height, x, y)` — adds a skeleton block (rounded rect)
  - `AddLine(width, x, y)` — adds a skeleton text line
  - `AddCircle(radius, x, y)` — adds a circle (avatar placeholder)
  - Auto-generates from content dimensions: `SkeletonScreen::From(panel)`
  - CSS: respects `bg`, `rounding`

  **Must NOT do**:
  - No shimmer effect (separate Task 31)

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high` — layout-based generation

  **Parallelization**: Wave 7
  **Blocked By**: None (uses basic ImDrawList rects)

  **Commit**: YES (groups with T28, T29, T31-T33)

- [ ] 31. Shimmer component

  **What to do**:
  - Create `include/unigui/widgets/shimmer.h` and impl
  - `Shimmer` widget: animated gradient sweep over skeleton
  - Wraps `SkeletonScreen`, adds `Start()`, `Stop()`, `SetSpeed(float)`
  - Visual: gradient band sweeps left→right over skeleton blocks
  - Use `AnimationState` for sweep position
  - CSS: respects `bg`, `bg-gradient`

  **Must NOT do**:
  - No per-block shimmer speed (uniform)

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`

  **Parallelization**: Wave 7
  **Blocked By**: T6, T30

  **Commit**: YES (groups with T28-T30, T32, T33)

- [ ] 32. Badge component

  **What to do**:
  - Create `include/unigui/widgets/badge.h` and impl
  - `Badge` widget: small colored label with `SetText(string)`, `SetColor(color)`, `SetVariant(dot/count/label)`
  - Visual: small rounded rect (12px height), centered text, positioned top-right of parent
  - CSS: respects `bg`, `rounding`, `font-size`

  **Must NOT do**:
  - No auto-position to parent (manual positioning)

  **Recommended Agent Profile**:
  - **Category**: `quick` — small widget, simple draw

  **Parallelization**: Wave 7
  **Blocked By**: None

  **Commit**: YES (groups with T28-T31, T33)

- [ ] 33. GradientText component

  **What to do**:
  - Create `include/unigui/fonts/gradient_text.h` (header-only)
  - `GradientText::Render(text, startColor, endColor, direction)` — renders text with gradient
  - Uses ImFont::RenderText with per-character color interpolation
  - Horizontal, vertical, diagonal directions
  - CSS: `text { text-gradient: linear-gradient(90deg, #f00, #00f); }`

  **Must NOT do**:
  - No per-character gradients (uniform word-level)

  **Recommended Agent Profile**:
  - **Category**: `quick`

  **Parallelization**: Wave 7
  **Blocked By**: T6

  **Commit**: YES (groups with T28-T32)

### Wave 8 — Integration & Demo

- [ ] 34. Update widget_gallery with v3.0 demos

  **What to do**:
  - Rewrite `examples/widget_gallery/main.cc` with organized tab structure:
    - Tab "Themes": ThemeSwitcher + visual comparison of all 10 themes
    - Tab "Effects": ShadowEffect/GlowEffect/BlurEffect/GlassPanel demos
    - Tab "Animations": all 10 easing curve demos with real-time sliders
    - Tab "Widgets": existing widget gallery + new Card/HeroSection/Skeleton/Shimmer/Badge
    - Tab "CSS": CSS editor (text input) + live preview panel
    - Tab "Transitions": page transition demo (fade/slide/scale between panels)
  - Add menu bar: File > Theme > Effects > Exit
  - Responsive layout: use HBox/VBox/Splitter for flexible layout

  **Must NOT do**:
  - No standalone theme editor app (just demo tab)

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`
  - **Skills**: [`playwright`]

  **Parallelization**: Wave 8 — parallel with T35-T37
  **Blocked By**: T14-T33 (all features must exist)

  **QA Scenarios**:
  ```
  Scenario: Gallery renders all tabs
    Tool: Playwright
    Steps: 1. Launch widget_gallery 2. Click each tab
           3. Screenshot each 4. Verify no crash, all widgets visible
    Expected: 6 tabs render correctly
    Evidence: .omo/evidence/task-34-gallery-*.png (6 screenshots)
  ```

  **Commit**: YES
  - Message: `feat(demo): widget_gallery v3.0 with all effect/theme demos`

- [ ] 35. Wire effects into existing widget API

  **What to do**:
  - Add `SetShadow(bool/float)` to `WidgetBase` — opt-in for all widgets
  - Add `SetGlass(bool)` to `Panel` and `Window`
  - Add `SetAnimated(bool)` to `Button`, `ProgressBar`, `ToggleSwitch`, `TabWidget`
  - Update `unigui.h` to include new `fx/` and theme preset headers
  - Ensure all new APIs are `if(UNIGUI_MODULE_WIDGETS)` guarded

  **Must NOT do**:
  - No forced shadow on all widgets (opt-in only)

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`

  **Parallelization**: Wave 8
  **Blocked By**: T22-T27

  **Commit**: YES

- [ ] 36. Theme demo example

  **What to do**:
  - Create `examples/theme_demo/main.cc` — simple app that cycles through all 10 themes
  - Auto-cycle mode: switch theme every 2 seconds (--auto flag)
  - Manual mode: arrow keys to switch themes
  - Frame counter: exit after N frames (--frames flag)
  - Render a Panel + Card + Button to show theme effects on widgets

  **Must NOT do**:
  - No GUI controls (simple CLI demo)

  **Recommended Agent Profile**:
  - **Category**: `quick`

  **Parallelization**: Wave 8
  **Blocked By**: T14-T21

  **Commit**: YES

- [ ] 37. Performance benchmarks for effects

  **What to do**:
  - Add `tests/bench/effect_bench_test.cc`
  - Measure: 100 cards with shadow (@60fps target)
  - Measure: 50 buttons with ripple animation
  - Measure: all 10 easing curves evaluation time (single frame)
  - Measure: CSS engine parse time for 1000-rule stylesheet
  - Output: FPS impact, ms per effect, regression baseline

  **Must NOT do**:
  - No continuous profiling (point-in-time benchmarks)

  **Recommended Agent Profile**:
  - **Category**: `quick`

  **Parallelization**: Wave 8
  **Blocked By**: T3-T6, T9

  **Commit**: YES

---

## Final Verification Wave

- [ ] F1. **Plan Compliance Audit** — `oracle`
  Read plan end-to-end. Verify 10 themes exist, 50+ CSS properties, effects module, new widgets.

- [ ] F2. **Code Quality Review** — `unspecified-high`
  Build + lint + test all configs. Check for `as any`/`@ts-ignore`, empty catches, AI slop.

- [ ] F3. **Real Manual QA** — `unspecified-high` + `playwright`
  Launch widget_gallery. Switch themes, observe animations, verify effects render.

- [ ] F4. **Scope Fidelity Check** — `deep`
  Diff vs plan: everything built, nothing extra.

---

## Commit Strategy

- **Commit 1**: `feat(fx): easing curves + EffectScope base`
- **Commit 2**: `feat(fx): ShadowEffect, GlowEffect, BlurEffect, GradientBrush`
- **Commit 3**: `feat(fx): AnimationState manager + transitions`
- **Commit 4**: `feat(css): property registry 50+ props, gradient parser`
- **Commit 5**: `feat(css): animation/transition parser, pseudo-classes, media queries`
- **Commit 6**: `feat(theme): 10 built-in presets + live switching`
- **Commit 7**: `feat(widgets): Button/Panel/Toast/ProgressBar polish`
- **Commit 8**: `feat(widgets): Card, HeroSection, Skeleton, Shimmer, Badge, GradientText`
- **Commit 9**: `feat(demo): widget_gallery v3.0 + theme demo`

---

## Success Criteria

### Verification Commands
```bash
cmake --preset windows-msvc-debug && cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug  # 200+ tests pass
```

### Final Checklist
- [ ] 10 themes render in widget_gallery
- [ ] Theme switching works without restart
- [ ] Shadow/glow/blur effects visible on panels
- [ ] Button hover gradient animation works
- [ ] CSS engine accepts 50+ property types
- [ ] All existing 200+ tests still pass
- [ ] 3 preset builds pass (recommended/minimal/full)
