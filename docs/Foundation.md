# 基础知识路线图

本指南聚焦“打底”的 C/C++ 与 Windows 内存基础，配合仓库的 `01-BasicMemory` 与工具准备环节，帮助你在进入 UE 逆向前补齐必备能力。阅读顺序建议与 `docs/Syllabus.md` 中的阶段 0-1 对应。

---

## 1. C/C++ 与内存布局

| 主题 | 目标 | 练习入口 |
| ---- | ---- | -------- |
| 指针、引用、取址 | 能解释 `int* p = &value; *p = 10;` 的意义与风险 | 修改 `01-BasicMemory/main.cpp` 中的 `Demo_DirectAccess`，尝试新增 `armor`、`shield` 字段并打印地址 |
| 结构体对齐与 `offsetof` | 明白编译器对齐规则，能够手算偏移 | 在 `Demo_PointerOffset` 中输出 `sizeof(Player)`、`offsetof(name)` 等值，观察不同成员排序的差别 |
| 多级指针 | 能画出 2~4 级指针的指向链路 | 仿照 `Demo_MultiLevelPointer`，把 `Player` 改成 `Inventory`，练习 `Item**`、`Item***` 的访问 |
| 字符串/数组内存 | 了解静态数组、`char[32]` 与 `std::string` 的内存差异 | 尝试把 `Player::name` 改为 `std::string`，对比地址打印结果 |

> 提示：多使用 `uintptr_t` + `reinterpret_cast` 打印地址，能更贴近期望的十六进制表现。

---

## 2. Windows 进程与内存模型

1. **虚拟地址空间**：理解用户态 0x0000`0000`0000 ~ 0x0000`7FFF`FFFF 范围的分页映射关系，知道“地址看起来连续但并非物理连续”。
2. **模块（Module）与基址**：`kernel32.dll`、`Game.exe` 等在进程中的加载地址会影响偏移计算；需掌握 `Base + Offset` 的定位方式。
3. **读写内存权限**：了解 `ReadProcessMemory`、`WriteProcessMemory` 需要的访问权限，明白为什么有时必须以管理员身份运行。
4. **异常与保护**：熟悉 `PAGE_GUARD`、`PAGE_EXECUTE_READWRITE` 等标志，便于 CE/x64dbg 调试时判断崩溃原因。

学习建议：
- 结合 MSDN 或《Windows Internals》阅读虚拟内存章节。
- 用 x64dbg 打开一个简单程序，观察 `Memory Map` 中模块基址与区段权限。

---

## 3. 工具入门 Checklist

| 工具 | 必学功能 | 验收方式 |
| ---- | -------- | -------- |
| Cheat Engine | 数值搜索、再次扫描、Pointer Scan、Auto Assembler | 在任何单机游戏或 Demo 里找到一个血量地址并锁定 |
| x64dbg | 下断点、单步、查看寄存器/调用栈、导出模块列表 | 为 Demo 中的一个写入函数下断点，观察寄存器变化 |
| IDA Free / Ghidra | 静态查看函数、字符串、交叉引用 | 对 Demo 可执行文件运行“快速分析”，找到 `main` 并确认伪代码 |

> 把每次练习的截图、关键偏移记录在 `/notes` 或个人笔记软件中，后续阶段要频繁复盘。

---

## 4. 基础实践任务

1. **结构体写真**  
   - 用 `std::byte` 或 `unsigned char*` 遍历 `Player` 的内存，每 4 字节打印一次。  
   - 在笔记中画出字段、偏移与数据类型的对应表。

2. **特征码打靶**  
   - 改写 `Demo_PatternScan`，引入 `??` 通配符与 `mask`，模拟 Cheat Engine 的 AoB 扫描。  
   - 试着用 `rg -n` 搜索你生成的特征码，确认输出格式正确。

3. **指针链更新**  
   - 写一个小函数 `int* ResolveHealth(Player*** triplePtr)`，模拟通过多级指针拿血量。  
   - 把其中一级指针置空，观察崩溃或异常，并记录“指针失效的排查流程”。

4. **工具联动**  
   - 在 Linux 环境编译 Demo，在 Windows 虚拟机用 CE/x64dbg 打开同一程序。  
   - 对比“源码偏移”与“调试器偏移”，理解 Release/Debug、结构体对齐导致的差异。

---

## 5. 基础自检清单

- [ ] 精确描述“指针”和“引用”的差别，以及各自何时会导致悬垂（Dangling）。  
- [ ] 能解释 `Base + Offset` 与 `指针链` 各自适合的场景。  
- [ ] 会写一个最简单的 Pattern Scan 循环（支持通配符）。  
- [ ] 能在 CE 中独立完成一次“从数值搜索 → 指针扫描 → 锁定地址”的流程。  
- [ ] 清楚虚拟地址空间的概念，知道 32 位/64 位进程中指针大小不同。  

若以上皆能完成，即可进入《进阶知识路线图》并开始针对 UE 特性的学习。

---

## 6. 推荐资料

- 《Windows Internals, Part 1/2》：虚拟内存、内核对象、调试机制基础。
- 《Game Hacking: Developing Autonomous Bots for Online Games》：对指针、偏移、Hook 基本思路有直观讲解。
- MSDN 文档：`VirtualQueryEx`、`ReadProcessMemory`、`CreateToolhelp32Snapshot` 等 API。
- Unreal Engine 官方文档：UObject、反射系统、Actor 生命周期的原理。

> 建议尽量结合仓库源码阅读：把文档中的概念映射回 `01-BasicMemory` 里的具体代码，将会更快建立“概念 ↔ 二进制”之间的联系。

