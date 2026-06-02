# TreeView — 使用指南 / Usage Guide

`unigui::TreeView` 是一个分层树形控件，支持多选、节点图标、行内进度条，以及**完全自定义的复合行（composite item）**。每一个节点都可以是"账户名 + 进度条（仓位轻重）+ 标签"这类复合组件。

> 头文件：`#include <unigui/widgets/treeview.h>`

---

## 1. 数据模型：`TreeNode`

树的内容由一棵 `TreeNode` 描述。每个节点自带一组可选的展示字段，开箱即用即可拼出复合行：

```cpp
struct TreeNode {
    std::string            label;            // 主文本（如账户名）
    std::vector<TreeNode>  children;         // 子节点
    bool                   expanded = false; // 预留：默认展开状态

    // ── 行内展示字段（无需自定义渲染即可使用）─────────────
    std::string icon;             // label 前的图标（Nerd Font / Emoji 字符）
    std::string suffix;           // label 后的文本（如百分比）
    ImU32       labelColor = 0;   // 文字颜色（0 = 跟随主题）
    ImU32       bgColor    = 0;   // 整行背景色（0 = 无）
    float       progress   = -1;  // 行内进度条 0~1（<0 = 不显示）
    ImU32       progressColor = 0;// 进度条颜色（0 = 默认）
};
```

---

## 2. 基础用法

```cpp
unigui::TreeNode root;
root.label = "投资组合";
root.children.push_back({"账户 A", {}});
root.children.push_back({"账户 B", {}});

auto tv = std::make_shared<unigui::TreeView>("portfolio");
tv->SetRoot(root);
tv->SetHideRoot(false);   // 设为 true 可隐藏根节点，直接显示其子节点
tv->Render();
```

常用接口：

```cpp
void SetRoot(TreeNode root);
const TreeNode& GetRoot() const;
void SetHideRoot(bool on);             // 隐藏根节点
void SetMultiSelect(bool on);          // 允许多选
std::vector<int> GetSelectedNodes() const;
```

---

## 3. 复合行方式一：内置字段（推荐，零代码）

“账户组展开后看到 *账户名 + 仓位进度条*” 这种最常见的复合行，**直接填 `TreeNode` 字段**即可，
`TreeView` 会自动在 `icon + label` 之后绘制 `suffix` 和 `ProgressBar`：

```cpp
unigui::TreeNode group;
group.label = "股票账户组";
group.icon  = "\uf0d6";          // 钱袋图标（Nerd Font）

auto makeAccount = [](const char* name, float weight) {
    unigui::TreeNode n;
    n.label         = name;
    n.progress      = weight;                       // 仓位轻重 0~1
    n.suffix        = "";                           // 也可放 "75%"
    n.progressColor = weight > 0.8f
                    ? IM_COL32(0xE5, 0x3E, 0x3E, 0xFF)  // 重仓→红
                    : IM_COL32(0x2E, 0xD1, 0x5E, 0xFF); // 轻仓→绿
    return n;
};

group.children.push_back(makeAccount("账户 A · 主仓", 0.92f));
group.children.push_back(makeAccount("账户 B · 网格", 0.35f));
group.children.push_back(makeAccount("账户 C · 套利", 0.60f));

auto tv = std::make_shared<unigui::TreeView>("accounts");
tv->SetRoot(std::move(group));
tv->Render();
```

字段对照：

| 复合元素      | 对应字段                       |
|---------------|--------------------------------|
| 账户名        | `label`                        |
| 图标          | `icon`                         |
| 仓位进度条    | `progress` + `progressColor`   |
| 百分比/备注   | `suffix`                       |
| 文字 / 行底色 | `labelColor` / `bgColor`       |

> 注意：内置的 `suffix` / `progress` 仅在节点展开（`open`）时绘制。叶子节点始终视为展开，
> 因此账户行的进度条总会显示；若某个**父分组节点自身**也需要常驻进度条，请用下面的自定义行渲染。

---

## 4. 复合行方式二：`SetRowRenderer`（完全自定义）

当一行需要放置**任意 ImGui 组件**（多个进度条、按钮、Tag、图标组合等）时，使用
`SetRowRenderer`。`TreeView` 仍负责展开 / 折叠 / 选中，你只需在回调里绘制该行的内容：

```cpp
tv->SetRowRenderer(
    [](int id, int depth, const unigui::TreeNode& node, bool selected) {
        // 账户名
        ImGui::TextUnformatted(node.label.c_str());

        // 仓位进度条
        ImGui::SameLine();
        ImGui::ProgressBar(node.progress < 0 ? 0.f : node.progress,
                           ImVec2(120, 0));

        // 一键平仓按钮
        ImGui::SameLine();
        if (ImGui::SmallButton("平仓")) {
            // ... 处理点击 ...
        }
    });
```

回调签名：

```cpp
void SetRowRenderer(
    std::function<void(int id, int depth, const TreeNode& node, bool selected)> fn);
```

- `id`：节点在本帧的顺序编号（深度优先）。
- `depth`：缩进层级（根为 0）。
- `node`：当前节点（含上面的所有字段，可自行取用）。
- `selected`：当前是否被选中（可据此改变高亮样式）。

> `SetRowRenderer` 优先级高于内置字段渲染：一旦设置，内置的 icon/suffix/progress 不再自动绘制，
> 全部由你的回调负责。
> 该回调会直接绘制在节点所在行，不会再生成额外的空白占位行；同名节点也会自动使用独立 ID。

### 兼容接口：`SetNodeRenderer`

`SetNodeRenderer` 是早期接口，在展开箭头 + 默认 label 之后追加内容，适合只想“补一点东西”的场景。
新代码建议优先使用 `SetRowRenderer` 以获得整行控制权。

```cpp
void SetNodeRenderer(std::function<void(int id, int depth, const TreeNode& node)> fn);
```

---

## 5. 多选

```cpp
tv->SetMultiSelect(true);
// 渲染后读取选中项（按本帧节点序号）
std::vector<int> sel = tv->GetSelectedNodes();
```

---

## 6. 完整示例：账户组 + 仓位进度条

```cpp
#include <unigui/widgets/treeview.h>

auto BuildAccountTree() {
    unigui::TreeNode root;
    root.label = "全部账户";

    auto group = [](const char* title) {
        unigui::TreeNode g; g.label = title; g.icon = "\uf07b"; // 文件夹
        return g;
    };
    auto account = [](const char* name, float weight, const char* pct) {
        unigui::TreeNode n;
        n.label         = name;
        n.suffix        = pct;
        n.progress      = weight;
        n.progressColor = weight > 0.8f ? IM_COL32(0xE5,0x3E,0x3E,0xFF)
                                        : IM_COL32(0x2E,0xD1,0x5E,0xFF);
        return n;
    };

    unigui::TreeNode stock = group("股票账户");
    stock.children.push_back(account("账户 A", 0.92f, "92%"));
    stock.children.push_back(account("账户 B", 0.41f, "41%"));

    unigui::TreeNode futures = group("期货账户");
    futures.children.push_back(account("账户 C", 0.70f, "70%"));

    root.children.push_back(std::move(stock));
    root.children.push_back(std::move(futures));
    return root;
}

// 渲染：
auto tv = std::make_shared<unigui::TreeView>("accounts");
tv->SetRoot(BuildAccountTree());
tv->SetHideRoot(true);   // 直接展示分组
tv->Render();
```

---

## 7. 注意事项

- **节点序号（id）按帧重算**：`GetSelectedNodes()` 返回的是本帧深度优先的顺序编号，
  在树结构发生增删后编号会变化；如需稳定标识，建议在 `SetUserData` / 业务侧自行维护映射。
- **内置进度条宽度**为可用区域的 30%，需要固定宽度或更复杂布局时请改用 `SetRowRenderer`。
- 图标依赖字体字形：内置已合并 Nerd Font 与系统 CJK/Emoji 字体，使用 `\uXXXX` 图标前请确认字形已加载。
