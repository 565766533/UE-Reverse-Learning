# 进阶知识路线图

当你可以熟练定位内存数据、理解指针链之后，就可以进入 UE 逆向的专题学习。本指南与仓库 `02-08` 目录呼应，把“UE 核心概念 → 进程工具 → 调试技巧 → Hook/项目实战”串成可执行的路径。如果要把这些知识迁移到真实单机 UE 游戏，请同步参考 `docs/SinglePlayerPractice.md` 中的流程与限制。

---

## 1. UE 核心结构 & 数据流

| 主题 | 关键点 | 练习指引 |
| ---- | ------ | -------- |
| UObject 系统 | `FUObjectArray`、`GObjects`、`UClass`、`UFunction` | 阅读 `02-UEObjectSystem/UETypes.h`，在笔记中画出对象关系图 |
| 世界数据 | `UGameInstance` → `UWorld` → `ULevel` → `AActor` | 使用 `03-ReverseTools` 中的 `ProcessHelper`，模拟遍历 Actor 链 |
| 名称系统 | `FName`, `FNameEntry`, `GNames` 的存储方式与查找 | 在 Demo 中实现 `FindName(uint32 index)`，熟悉字符串池结构 |
| 容器 | `TArray`, `TMap`, `TSet` 的内存布局 | 对比 `std::vector` 与 `TArray` 的差别，练习写序列化/反序列化代码 |

> 输出要求：任意一个 UE 模拟对象的“字段偏移表 + 访问伪代码”，方便后续 Hook/ESP 快速定位。

---

## 2. 逆向工具与自动化

1. **ProcessHelper / MemoryHelper**  
   - 在 `03-ReverseTools` 中实现或完善：模块基址获取、读写内存、Pattern Scan、Pointer Resolve。  
   - 增强日志与错误处理，确保能在调试时快速发现失效。

2. **自动化偏移更新**  
   - 结合 IDA 脚本或 Ghidra Export，生成包含 `UWorld`、`GNames`、`GObjects` 的偏移文件。  
   - 在 `07-RealWorldProject` 中加载该文件，实现“版本升级时仅需更新 JSON/YAML”。

3. **工具联动**  
   - 写一个小型 CLI：输入 Pattern → 输出所有匹配地址并可导出到 `.ct` 或 `.json`。  
   - 通过 CE 的 Lua 或 x64dbg 的 Python，和你的 CLI 进行交互，形成自定义流水线。

---

## 3. 调试 & Hook 深入

| 项目 | 目标 | 对应模块 |
| ---- | ---- | -------- |
| x64dbg 场景化练习 | 能快速设置读写断点、条件断点、日志断点，复原函数原型 | `05-x64dbg` |
| Hook 策略对比 | 理解 VTable Hook、Import Hook、Inline Hook 的实现要点与风险 | `06-HookingTechniques` |
| 反检测与规避 | 识别常见检测点，如完整性校验、线程扫描、异常监控，并提出规避方案 | `06-HookingTechniques` |
| ESP / 功能模块化 | 把数据采集、世界坐标转换、渲染分成独立层，便于调试与扩展 | `07-RealWorldProject` |

实践建议：
- 先在模拟环境中完成 Hook，确认对象访问无误，再迁移至真实项目。
- 记录每次 Hook 的调用约定（fastcall、thiscall 等）以及寄存器使用情况。
- 为每个 Hook 保存“原始函数指针”，提供开关与恢复逻辑。

---

## 4. 典型任务模板

1. **对象定位模板**
   ```cpp
   uintptr_t world = ResolvePointerChain({ base + offsets::GEngine, 0x80, 0x1A0 });
   auto actors = Read<TArray<AActor*>>(world + offsets::World::ActorArray);
   ```
   - 任务：封装 `ResolvePointerChain`，对输入的地址+偏移列表进行验证和日志输出。

2. **Pattern Scan 模板**
   ```cpp
   auto addr = PatternScan(moduleBase, moduleSize, "48 8B ?? ?? ?? ?? ?? 48 85 C0 74");
   addr = FollowRipRelative(addr);
   ```
   - 任务：支持 `??`、`?x` 等通配，加入缓存避免重复扫描。

3. **Hook 模板**
   ```cpp
   using TickFn = void(*)(AActor*, float);
   TickFn originalTick;

   void HookedTick(AActor* actor, float deltaTime) {
       LogActorState(actor);
       originalTick(actor, deltaTime);
   }
   ```
   - 任务：实现任意 VTable Hook，支持启停、异常捕获、统计调用次数。

4. **ESP 处理链**
   ```
   CollectActors -> Filter (Team, Distance, Visibility) -> ProjectWorldToScreen -> RenderOverlay
   ```
   - 任务：为链路中的每一步写日志/性能计时，并在 `docs/Advanced.md` 中更新结论。

---

## 5. 进阶自检清单

- [ ] 能独立从 IDA/Ghidra 中导出 `UWorld/GNames/GObjects` 偏移，并验证准确性。  
- [ ] 熟练掌握至少一种 Hook 框架或自行实现的 VTable Hook，清楚保存/恢复原函数的全过程。  
- [ ] 清楚 Pattern Scan 的局限（页保护、模块更新、反检测），知道如何加上黑名单/白名单。  
- [ ] 能够把 ESP/功能模块拆成“采集 → 处理 → 渲染”，并针对性能、稳定性提出优化方案。  
- [ ] 有一份针对目标游戏的“偏移更新手册”，描述版本更新时的排查流程。  

若以上条目都能完成，你已经具备将仓库示例迁移到真实 UE 项目的能力。建议继续维护自己的知识库，关注 UE 版本更新、反作弊机制演变，并不断复盘。

---

## 6. 推荐延伸阅读

- Unreal Engine 源码（可在 Epic Launcher 下载），重点阅读：`Object`, `Engine`, `Gameplay`, `Renderer` 模块。
- GitHub 项目参考：`ReClass.NET`、`UE4SS`、`CheatEngine/patterns`。
- 博客/论坛：`unknowncheats.me`、`guidedhacking.com` 上的 UE 专题帖子。
- 安全会议论文：关注关于游戏反作弊、内核监控、Hook 检测的演讲资料。

> 进阶阶段更强调“持续维护自己的 offset/Hook 仓库”。建议将本项目作为脚手架，把真实项目中的偏移、Pattern、Hook 都沉淀成可复用模块。
