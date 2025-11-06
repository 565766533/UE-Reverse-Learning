/*
 * ========================================
 * 第六课：Hook 技术详解 - VTable Hook
 * ========================================
 * 
 * 本节学习虚函数表Hook，这是游戏逆向最常用的技术
 */

#include <windows.h>
#include <iostream>
#include <vector>

using namespace std;

// ====================================
// 第一部分：理解 VTable
// ====================================

class GameObject {
public:
    virtual void Update() {
        cout << "[原始] GameObject::Update()" << endl;
    }
    
    virtual void Render() {
        cout << "[原始] GameObject::Render()" << endl;
    }
    
    virtual int GetHealth() {
        cout << "[原始] GameObject::GetHealth()" << endl;
        return 100;
    }
    
    virtual ~GameObject() {}
};

// 演示 VTable 结构
void Demo_UnderstandVTable() {
    cout << "\n=== VTable 结构讲解 ===" << endl;
    
    GameObject obj;
    
    // 获取 VTable 指针（对象首8字节）
    uintptr_t* vtable = *(uintptr_t**)&obj;
    
    cout << "对象地址:   0x" << hex << (uintptr_t)&obj << dec << endl;
    cout << "VTable地址: 0x" << hex << (uintptr_t)vtable << dec << endl;
    
    cout << "\nVTable 内容（虚函数地址）：" << endl;
    cout << "vtable[0] (析构函数):   0x" << hex << vtable[0] << dec << endl;
    cout << "vtable[1] (Update):     0x" << hex << vtable[1] << dec << endl;
    cout << "vtable[2] (Render):     0x" << hex << vtable[2] << dec << endl;
    cout << "vtable[3] (GetHealth):  0x" << hex << vtable[3] << dec << endl;
    
    cout << "\n调用虚函数的过程：" << endl;
    cout << "1. 读取对象首地址 -> VTable指针" << endl;
    cout << "2. 读取 VTable[index] -> 函数地址" << endl;
    cout << "3. call 函数地址" << endl;
}

// ====================================
// 第二部分：VTable Hook 实现
// ====================================

class VTableHook {
private:
    void* target;           // 目标对象
    uintptr_t* oldVTable;   // 原始VTable
    uintptr_t* newVTable;   // 新VTable
    size_t vtableSize;      // VTable大小
    
public:
    VTableHook() : target(nullptr), oldVTable(nullptr), newVTable(nullptr), vtableSize(0) {}
    
    // 初始化Hook
    bool Initialize(void* obj) {
        target = obj;
        oldVTable = *(uintptr_t**)obj;
        
        // 计算VTable大小
        vtableSize = 0;
        while (oldVTable[vtableSize] != 0) {
            vtableSize++;
            if (vtableSize > 100) break;  // 防止无限循环
        }
        
        cout << "\n[Hook] VTable 大小: " << vtableSize << " 个函数" << endl;
        
        // 复制VTable
        newVTable = new uintptr_t[vtableSize];
        memcpy(newVTable, oldVTable, vtableSize * sizeof(uintptr_t));
        
        // 替换对象的VTable指针
        *(uintptr_t**)obj = newVTable;
        
        cout << "[Hook] 已替换 VTable" << endl;
        return true;
    }
    
    // Hook某个虚函数
    void HookFunction(size_t index, void* hookFunc) {
        if (index >= vtableSize) {
            cout << "[错误] 索引超出范围" << endl;
            return;
        }
        
        cout << "[Hook] 替换函数[" << index << "]" << endl;
        cout << "       原地址: 0x" << hex << oldVTable[index] << dec << endl;
        cout << "       新地址: 0x" << hex << (uintptr_t)hookFunc << dec << endl;
        
        newVTable[index] = (uintptr_t)hookFunc;
    }
    
    // 获取原始函数
    template<typename T>
    T GetOriginal(size_t index) {
        return (T)oldVTable[index];
    }
    
    // 恢复Hook
    void Unhook() {
        if (target && oldVTable) {
            *(uintptr_t**)target = oldVTable;
            delete[] newVTable;
            cout << "[Hook] 已恢复原始 VTable" << endl;
        }
    }
    
    ~VTableHook() {
        Unhook();
    }
};

// ====================================
// 第三部分：Hook 示例
// ====================================

// 全局Hook对象
VTableHook g_Hook;

// Hook后的函数
void __fastcall Hooked_Update(GameObject* thisPtr) {
    cout << "[Hook] Hooked_Update() 被调用！" << endl;
    cout << "       this指针: 0x" << hex << (uintptr_t)thisPtr << dec << endl;
    
    // 可以在这里添加自定义逻辑
    cout << "       [自定义] 执行额外功能..." << endl;
    
    // 调用原始函数
    typedef void(__fastcall* OriginalFunc)(GameObject*);
    OriginalFunc original = g_Hook.GetOriginal<OriginalFunc>(1);
    original(thisPtr);
    
    cout << "       [自定义] 原始函数执行完毕" << endl;
}

void __fastcall Hooked_Render(GameObject* thisPtr) {
    cout << "[Hook] Hooked_Render() - 完全替换！" << endl;
    // 不调用原函数，完全替换行为
}

int __fastcall Hooked_GetHealth(GameObject* thisPtr) {
    cout << "[Hook] Hooked_GetHealth() - 返回修改值" << endl;
    
    // 调用原函数获取真实值
    typedef int(__fastcall* OriginalFunc)(GameObject*);
    OriginalFunc original = g_Hook.GetOriginal<OriginalFunc>(3);
    int realHealth = original(thisPtr);
    
    cout << "       真实血量: " << realHealth << endl;
    cout << "       返回血量: 999 (修改)" << endl;
    
    return 999;  // 返回假血量
}

void Demo_VTableHook() {
    cout << "\n=== VTable Hook 演示 ===" << endl;
    
    GameObject obj;
    
    cout << "\n【Hook前】调用虚函数：" << endl;
    obj.Update();
    obj.Render();
    int health = obj.GetHealth();
    cout << "返回值: " << health << endl;
    
    // 初始化Hook
    g_Hook.Initialize(&obj);
    
    // Hook函数
    g_Hook.HookFunction(1, (void*)Hooked_Update);   // Update
    g_Hook.HookFunction(2, (void*)Hooked_Render);   // Render
    g_Hook.HookFunction(3, (void*)Hooked_GetHealth); // GetHealth
    
    cout << "\n【Hook后】调用虚函数：" << endl;
    obj.Update();
    obj.Render();
    health = obj.GetHealth();
    cout << "返回值: " << health << endl;
}

// ====================================
// 第四部分：实战应用 - 游戏对象Hook
// ====================================

class Player {
public:
    int health;
    int level;
    
    virtual void TakeDamage(int damage) {
        health -= damage;
        cout << "[游戏] 受到 " << damage << " 点伤害，剩余血量: " << health << endl;
    }
    
    virtual void GainExp(int exp) {
        cout << "[游戏] 获得 " << exp << " 点经验" << endl;
    }
    
    virtual bool IsAlive() {
        return health > 0;
    }
    
    Player() : health(100), level(1) {}
};

VTableHook g_PlayerHook;

// Hook TakeDamage - 实现无敌
void __fastcall Hooked_TakeDamage(Player* player, int damage) {
    cout << "[作弊] 无敌模式：伤害被忽略！" << endl;
    cout << "       原本要受到 " << damage << " 点伤害" << endl;
    // 不调用原函数，直接忽略伤害
}

// Hook GainExp - 经验倍增
void __fastcall Hooked_GainExp(Player* player, int exp) {
    int multipliedExp = exp * 10;  // 10倍经验
    
    cout << "[作弊] 经验倍增：" << exp << " -> " << multipliedExp << endl;
    
    // 调用原函数，但传入修改后的值
    typedef void(__fastcall* OriginalFunc)(Player*, int);
    OriginalFunc original = g_PlayerHook.GetOriginal<OriginalFunc>(1);
    original(player, multipliedExp);
}

void Demo_GameHook() {
    cout << "\n=== 游戏Hook实战 ===" << endl;
    
    Player player;
    
    cout << "\n【正常游戏】" << endl;
    player.TakeDamage(30);
    player.GainExp(10);
    cout << "当前血量: " << player.health << endl;
    
    // 启用作弊
    cout << "\n【启用作弊】" << endl;
    g_PlayerHook.Initialize(&player);
    g_PlayerHook.HookFunction(0, (void*)Hooked_TakeDamage);
    g_PlayerHook.HookFunction(1, (void*)Hooked_GainExp);
    
    cout << "\n【作弊后】" << endl;
    player.TakeDamage(50);  // 伤害被忽略
    player.GainExp(10);     // 经验翻倍
    cout << "当前血量: " << player.health << endl;
}

// ====================================
// 第五部分：Hook的优缺点分析
// ====================================

void AnalyzeHookMethods() {
    cout << "\n╔═══════════════════════════════════════════╗" << endl;
    cout << "║        Hook 技术对比分析                  ║" << endl;
    cout << "╚═══════════════════════════════════════════╝" << endl;
    
    cout << "\n【VTable Hook】" << endl;
    cout << "优点：" << endl;
    cout << "  ✓ 实现简单，容易理解" << endl;
    cout << "  ✓ 只影响特定对象" << endl;
    cout << "  ✓ 不修改原代码，容易恢复" << endl;
    cout << "  ✓ 稳定性高" << endl;
    
    cout << "\n缺点：" << endl;
    cout << "  ✗ 只能Hook虚函数" << endl;
    cout << "  ✗ 需要找到对象实例" << endl;
    cout << "  ✗ 每个对象都要单独Hook" << endl;
    
    cout << "\n适用场景：" << endl;
    cout << "  • UE游戏的对象Hook（大部分函数都是虚函数）" << endl;
    cout << "  • 需要针对特定对象的Hook" << endl;
    cout << "  • 渲染Hook（DirectX对象）" << endl;
    
    cout << "\n\n【Inline Hook】（下节课讲解）" << endl;
    cout << "优点：" << endl;
    cout << "  ✓ 可以Hook任何函数" << endl;
    cout << "  ✓ 全局生效" << endl;
    
    cout << "\n缺点：" << endl;
    cout << "  ✗ 实现复杂" << endl;
    cout << "  ✗ 容易被反作弊检测" << endl;
    cout << "  ✗ 可能不稳定" << endl;
    
    cout << "\n\n【Import Hook】（下节课讲解）" << endl;
    cout << "优点：" << endl;
    cout << "  ✓ 实现简单" << endl;
    cout << "  ✓ 稳定" << endl;
    
    cout << "\n缺点：" << endl;
    cout << "  ✗ 只能Hook导入函数" << endl;
    cout << "  ✗ 容易被检测" << endl;
    
    cout << "\n==========================================\n" << endl;
}

// ====================================
// 主程序
// ====================================

int main() {
    SetConsoleOutputCP(CP_UTF8);
    
    cout << "╔═══════════════════════════════════════════╗" << endl;
    cout << "║    UE逆向学习 - VTable Hook 技术         ║" << endl;
    cout << "╚═══════════════════════════════════════════╝" << endl;
    
    // 演示1：理解VTable
    Demo_UnderstandVTable();
    
    cout << "\n按任意键继续..." << endl;
    system("pause");
    
    // 演示2：基础Hook
    Demo_VTableHook();
    
    cout << "\n按任意键继续..." << endl;
    system("pause");
    
    // 演示3：游戏应用
    Demo_GameHook();
    
    cout << "\n按任意键继续..." << endl;
    system("pause");
    
    // 分析对比
    AnalyzeHookMethods();
    
    cout << "\n程序结束！" << endl;
    system("pause");
    return 0;
}

/*
 * ═══════════════════════════════════════
 * 课后作业
 * ═══════════════════════════════════════
 * 
 * 1. 用x64dbg分析VTable的汇编实现
 * 2. 尝试Hook更多的虚函数
 * 3. 实现一个通用的VTableHook类
 * 4. 对比DeltaForce-Glow项目中的Hook实现
 * 5. 思考：如何检测VTable被Hook？
 * 
 * ═══════════════════════════════════════
 * 实战挑战
 * ═══════════════════════════════════════
 * 
 * 1. Hook DirectX的Present函数实现绘制
 * 2. Hook UE的PostRender函数
 * 3. 实现多对象批量Hook
 * 4. 添加Hook保护机制
 * 
 * ═══════════════════════════════════════
 * 下一课预告
 * ═══════════════════════════════════════
 * 
 * 第七课：Inline Hook 和 Detour
 * - MinHook 库使用
 * - 手动实现Inline Hook
 * - Trampoline 技术
 * - Hook保护和检测
 */
