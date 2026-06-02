# CascadingCombo — 使用指南 / Usage Guide

`unigui::CascadingCombo` 是一个多级（联动）下拉框控件：把若干个 `BeginCombo` 组合成"省 → 市 → 区""地区 → 国家"这类层级选择。每一级都是独立的下拉框，选择变化时会触发回调。

> 头文件：`#include <unigui/widgets/cascadingcombo.h>`

从本版本起，`CascadingCombo` 支持**横向 / 纵向两种排布**，并且可以**统一或逐级调整宽度**，不再被迫竖排、也不再难以控制宽度。

---

## 1. 数据模型：`Level`

每一级用一个 `Level` 描述：

```cpp
struct Level {
    std::string              label;            // 级别标签（显示在下拉框右侧）
    std::vector<std::string> options;          // 该级可选项
    int                      selectedIndex = 0;// 当前选中项下标
    float                    width = 0.f;      // 该级宽度（px，0 = 用全局宽度）
};
```

---

## 2. 基础用法

```cpp
std::vector<unigui::CascadingCombo::Level> levels = {
    {"地区", {"华东", "华南", "华北"}},
    {"城市", {"上海", "杭州", "南京"}},
};

auto cc = std::make_shared<unigui::CascadingCombo>("addr", levels);
cc->SetOnChanged([&](int level, int index) {
    // 当某一级的选择变化时联动更新下一级选项
    if (level == 0) {
        cc->SetOptions(1, CitiesOf(index));
    }
});
cc->Render();
```

常用接口：

```cpp
void SetLevels(std::vector<Level> levels);
void SetOptions(int level, std::vector<std::string> options); // 替换某级选项（自动夹紧下标）
int         GetSelectedIndex(int level) const;
std::string GetSelectedText(int level) const;
void SetOnChanged(std::function<void(int level, int index)> fn);
```

---

## 3. 排布方式：横向 / 纵向

默认是**纵向**（每一级一行，自上而下）。一行排不下、或希望像工具栏那样并排时，切换为**横向**：

```cpp
cc->SetLayout(unigui::CascadingCombo::Layout::Horizontal); // 横向并排
cc->SetLayout(unigui::CascadingCombo::Layout::Vertical);   // 纵向堆叠（默认）
```

横向排布时，可用 `SetSpacing` 控制各下拉框之间的间距：

```cpp
cc->SetSpacing(12.f);  // 相邻下拉框间隔 12px
cc->SetSpacing(-1.f);  // 传负值 = 使用 ImGui 默认间距
```

> `Spacing` 仅在 `Horizontal` 排布下生效；纵向排布的行距由 ImGui 主题样式决定。

---

## 4. 宽度调整：全局 / 逐级

### 4.1 统一宽度（推荐）

一次性把所有下拉框设为同一宽度：

```cpp
cc->SetItemWidth(140.f);  // 每个下拉框宽 140px
cc->SetItemWidth(0.f);    // 传 <=0 = 恢复控件自动宽度（会额外预留三角箭头空间）
```

### 4.2 逐级宽度

某一级需要更宽 / 更窄时，单独覆盖该级宽度（优先级高于全局宽度）：

```cpp
cc->SetItemWidth(0, 200.f); // 第 0 级固定 200px
cc->SetItemWidth(1, 0.f);   // 第 1 级清除覆盖，回退到全局宽度
```

也可以在构造数据时直接填 `Level::width`：

```cpp
std::vector<unigui::CascadingCombo::Level> levels = {
    {"地区", {"华东", "华南"}, 0, 120.f},  // 该级宽 120px
    {"城市", {"上海", "杭州"}},             // 用全局宽度
};
```

宽度生效规则（从高到低）：

| 优先级 | 来源                       |
|--------|----------------------------|
| 1      | `Level::width`（逐级覆盖） |
| 2      | `SetItemWidth(float)`（全局）|
| 3      | ImGui 默认自适应宽度       |

---

## 5. 链式配置（Fluent API）

排布与宽度可以一行写完：

```cpp
cc->WithLayout(unigui::CascadingCombo::Layout::Horizontal)
   .WithItemWidth(140.f)
   .WithSpacing(10.f);
```

---

## 6. 完整示例：横向三级联动

```cpp
#include <unigui/widgets/cascadingcombo.h>

auto cc = std::make_shared<unigui::CascadingCombo>("region");
cc->SetLevels({
    {"省", {"江苏", "浙江"}},
    {"市", {"南京", "苏州"}},
    {"区", {"玄武区", "鼓楼区"}},
});

// 横向并排，统一宽度 120px，间距 8px，省级单独加宽到 150px
cc->WithLayout(unigui::CascadingCombo::Layout::Horizontal)
   .WithItemWidth(120.f)
   .WithSpacing(8.f);
cc->SetItemWidth(0, 150.f);

cc->SetOnChanged([&](int level, int index) {
    if (level == 0) cc->SetOptions(1, CitiesOf(index));
    if (level == 1) cc->SetOptions(2, DistrictsOf(index));
});

// 每帧渲染
cc->Render();

// 读取结果
std::string province = cc->GetSelectedText(0);
std::string city     = cc->GetSelectedText(1);
```

---

## 7. 注意事项

- **联动逻辑在 `OnChanged` 里自行实现**：本控件只负责渲染与选择，下一级选项需要你在回调中通过 `SetOptions` 更新。
- **`SetOptions` 会夹紧下标**：当新选项数量比原来少时，越界的 `selectedIndex` 会自动归零。
- **逐级宽度 / 全局宽度只影响下拉框本体**；级别 `label` 文本绘制在下拉框右侧，会额外占用横向空间，横排时请预留足够宽度。
- **宽度传 `<=0` 即恢复自动宽度**：无论全局还是逐级，传入 0 或负值都会按当前/候选文本自动估宽，并额外预留右侧下拉箭头空间。
