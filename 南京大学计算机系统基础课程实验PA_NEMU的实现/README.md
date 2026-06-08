# 南京大学计算机系统基础 — PA 实验

## 📘 课程信息

- **课程名称**：计算机系统基础（Introduction to Computer System, ICS）
- **开课学校**：南京大学计算机科学与技术系
- **实验平台**：PA（Programming Assignment）2025
- **实验指南**：https://nju-projectn.github.io/ics-pa-gitbook/ics2025/

## 📂 目录结构

```
ics2025（PA1-PA3)/
├── nemu/                  # NEMU —— RISC-V 全系统模拟器（核心）
├── abstract-machine/      # Abstract Machine —— 抽象机器层
├── nanos-lite/            # Nanos-lite —— 轻量级操作系统
├── navy-apps/             # Navy Apps —— 用户态应用程序
├── am-kernels/            # AM Kernels —— 基于 AM 的内核示例
├── fceux-am/              # FCEUX-AM —— FC 模拟器移植

├── Makefile               # 顶层 Makefile
├── init.sh                # 初始化脚本
└── README.md              # 原始 README（来自 NJU ProjectN）
```

## 🧪 实验内容

### PA1 —— 简易调试器与表达式求值
- 实现寄存器和内存查看
- 实现表达式求值引擎
- 实现监视点（watchpoint）

### PA2 —— RISC-V 指令模拟
- 实现 RISC-V RV32IM 指令集
- 实现特权级架构（M-mode）
- 通过 CPU 测试集验证

### PA3 —— 存储体系与程序加载
- 实现分页机制与 MMU
- 实现程序加载器（ELF 解析）
- 支持从操作系统（Nanos-lite）启动用户程序

## 🔧 构建与运行

参考原始指南进行环境配置：
```bash
# 初始化（需在 Linux 环境下）
bash init.sh nemu
bash init.sh abstract-machine
bash init.sh nanos-lite
bash init.sh navy-apps

# 编译并运行 NEMU
make -C nemu run
```

> 详细步骤请参考 [ICS PA 指南](https://nju-projectn.github.io/ics-pa-gitbook/ics2025/) 及原始 [README](ics2025（PA1-PA3)/README.md)。

## 📝 相关资源

- [NJU ProjectN 官方仓库](https://github.com/NJU-ProjectN)
- [ICS PA 实验指南](https://nju-projectn.github.io/ics-pa-gitbook/ics2025/)
- [RISC-V 指令集手册](https://riscv.org/technical/specifications/)
