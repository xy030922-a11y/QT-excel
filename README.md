# QtExcel

基于 **Qt5 / C++17** 开发的轻量级桌面电子表格应用，支持单元格格式化、CSV 读写与行列动态管理。

---

## 功能特性

### 表格核心
- 默认 50 × 50 网格，加载 CSV 时自动扩展
- 列头采用 Excel 风格标签（A、B … Z、AA、AB …）
- 行头采用从 1 开始的数字标签
- 支持连续多单元格选中

### 单元格编辑与格式化
- 通过 `QLineEdit` 委托进行行内编辑
- 每个单元格独立设置：字体族、字号、粗体、斜体、下划线
- 字体颜色与背景颜色（颜色选择对话框）
- 水平对齐方式：左对齐 / 居中 / 右对齐
- 工具栏实时反映当前选中单元格的格式状态

### 剪贴板
- 复制 / 剪切 / 粘贴，使用制表符分隔值
- 支持从外部程序（如 Excel、LibreOffice）粘贴内容

### 行列管理（右键上下文菜单）
- 在上方 / 下方插入若干行（弹窗输入数量）
- 在左侧 / 右侧插入若干列（弹窗输入数量）
- 删除选中的行或列
- 交互式调整行高 / 列宽

### 文件读写
- **保存 / 打开 CSV** — 符合 RFC-4180 规范，UTF-8 with BOM（Excel 可正确识别中文）
- **新建** — 弹窗确认后清空所有数据
- **另存为** — 若文件名无 `.csv` 后缀则自动补全
- 标题栏显示当前文件名

---

## 项目结构

```
QtExcel/
├── main.cpp                  # 程序入口
├── mainwindow.h / .cpp       # 主窗口、菜单栏、右键菜单、剪贴板
├── spreadsheetmodel.h / .cpp # QAbstractTableModel — 数据、格式化、CSV、插入/删除
├── celldelegate.h / .cpp     # QStyledItemDelegate — 自定义绘制与编辑器
├── celldata.h                # CellData / CellFormat 数据结构
├── toolbarManger.h / .cpp    # 工具栏构建与信号定义
├── commands.h / .cpp         # （预留 — 撤销/重做）
├── formulaengine.h / .cpp    # （预留 — 公式计算）
└── mainwindow.ui             # Qt Designer UI 文件
```

---

## 架构概览

```
MainWindow
├── QTableView  (m_view)
│   ├── SpreadsheetModel        ← QAbstractTableModel
│   └── CellDelegate            ← QStyledItemDelegate
└── ToolbarManger (tb)
    └── 信号 ──→ MainWindow 槽 ──→ SpreadsheetModel 格式化方法
```

**数据流：**

1. 用户编辑单元格 → `CellDelegate::setModelData` → `SpreadsheetModel::setData`
2. 工具栏触发信号 → `MainWindow` lambda → `SpreadsheetModel` 格式化方法
3. `SpreadsheetModel` 发出 `dataChanged` → 视图通过 `CellDelegate::paint` 重绘

---

## 构建环境

| 依赖项 | 版本要求 |
|--------|----------|
| Qt     | 5.x      |
| 编译器 | MinGW / MSVC，需支持 C++17 |
| Qt Creator | 任意近期版本 |

### 构建步骤

```bash
# Qt Creator
# 1. 打开 excel.pro
# 2. 配置 Kit（Qt 5 + MinGW）
# 3. 构建并运行（Ctrl+R）

# 命令行
qmake excel.pro
make -j4
```

---

## 关键实现说明

### SpreadsheetModel
- 数据存储为 `QVector<QVector<CellData>>`，每个单元格包含原始公式字符串 `formula`、计算后的值 `value` 以及格式 `CellFormat`。
- `data()` 对 `DisplayRole`（显示值）和 `EditRole`（原始公式）分别返回不同内容。
- `notifyChanged()` 计算索引列表的边界矩形，仅发出一次 `dataChanged`，减少不必要的重绘。
- 插入/删除行列均使用 `beginInsertRows` / `endInsertRows` 等接口，保证视图与数据同步。

### CellDelegate
- `paint()` 完全手动绘制背景、选中边框与文本，不调用 `QStyledItemDelegate::paint`，从而获得完整的外观控制权。
- `createEditor()` 返回普通 `QLineEdit`；`setEditorData` 加载原始公式字符串。

### CSV 读写
- `escapeCsvField` 对含有 `,`、`"` 或换行符的字段用双引号包裹，内部双引号转义为 `""`，符合 RFC-4180。
- `parseCsvLine` 正确处理带引号字段与字段内嵌逗号。
- 写入时添加 UTF-8 BOM，确保 Excel 正确识别中文内容。

### 工具栏
- `ToolbarManger` 持有所有工具栏控件，并发出类型明确的信号（`boldToggled(bool)`、`fontColorChanged(QColor)` 等）。
- `MainWindow::onCurrentChanged` 使用 `QSignalBlocker` 阻塞工具栏控件的信号，在更新工具栏显示时不触发对 Model 的二次修改。

---

## 待实现 / 进行中

- [ ] 撤销 / 重做（`commands.h` — `QUndoStack`）
- [ ] 公式引擎（`formulaengine.h` — 四则运算 + 单元格引用）
- [ ] 多 Sheet 支持
- [ ] 数字格式化（货币、百分比、日期）

---
