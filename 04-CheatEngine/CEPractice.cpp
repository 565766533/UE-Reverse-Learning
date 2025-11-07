/*
 * ========================================
 * 第四课：Cheat Engine 实战练习
 * ========================================
 * 
 * 本节模拟真实游戏环境，让你练习CE技巧
 */

#include <windows.h>
#include <iostream>
#include <thread>
#include <random>
#include <cstring>
#include <cstdlib>

using namespace std;

// ====================================
// 模拟游戏数据
// ====================================

class GamePlayer {
public:
    int health;              // +0x00: 当前血量
    int maxHealth;           // +0x04: 最大血量
    int mana;                // +0x08: 魔法值
    int level;               // +0x0C: 等级
    int experience;          // +0x10: 经验值
    float posX, posY, posZ;  // +0x14: 位置
    int gold;                // +0x20: 金币
    bool isAlive;            // +0x24: 是否存活
    char name[32];           // +0x28: 玩家名称
    
    void Initialize() {
        health = 100;
        maxHealth = 100;
        mana = 50;
        level = 1;
        experience = 0;
        posX = 0; posY = 0; posZ = 0;
        gold = 100;
        isAlive = true;
        strcpy_s(name, "Player1");
    }
    
    void Update() {
        // 模拟受伤
        if (rand() % 100 < 5) {
            health -= 10;
            if (health < 0) health = 0;
        }
        
        // 模拟获得经验
        if (rand() % 100 < 10) {
            experience += 5;
            if (experience >= level * 100) {
                level++;
                experience = 0;
                cout << "升级了！当前等级: " << level << endl;
            }
        }
        
        // 模拟移动
        posX += (rand() % 3 - 1) * 0.1f;
        posY += (rand() % 3 - 1) * 0.1f;
    }
    
    void PrintInfo() {
        cout << "========== 玩家信息 ==========" << endl;
        cout << "名称: " << name << endl;
        cout << "地址: 0x" << hex << (uintptr_t)this << dec << endl;
        cout << "血量: " << health << "/" << maxHealth << endl;
        cout << "魔法: " << mana << endl;
        cout << "等级: " << level << " (经验: " << experience << ")" << endl;
        cout << "金币: " << gold << endl;
        cout << "位置: (" << posX << ", " << posY << ", " << posZ << ")" << endl;
        cout << "=============================" << endl << endl;
    }
};

// 多级指针结构（模拟游戏引擎）
class GameEngine {
public:
    GamePlayer* player;
    
    GameEngine() {
        player = new GamePlayer();
        player->Initialize();
    }
    
    ~GameEngine() {
        delete player;
    }
};

GameEngine* g_Engine = nullptr;  // 全局引擎指针

// ====================================
// CE 练习任务
// ====================================

void PrintCETasks() {
    cout << "\n╔════════════════════════════════════════╗" << endl;
    cout << "║   Cheat Engine 练习任务               ║" << endl;
    cout << "╚════════════════════════════════════════╝" << endl;
    
    cout << "\n【任务1：基础数值搜索】" << endl;
    cout << "1. 打开Cheat Engine，附加到此进程" << endl;
    cout << "2. 首次扫描：搜索当前血量值" << endl;
    cout << "3. 等待血量变化" << endl;
    cout << "4. 再次扫描：搜索新的血量值" << endl;
    cout << "5. 重复3-4步直到只剩1-2个地址" << endl;
    cout << "6. 修改血量为999" << endl;
    
    cout << "\n【任务2：指针扫描】" << endl;
    cout << "1. 找到血量地址后，右键 -> 指针扫描" << endl;
    cout << "2. 重启程序，使用指针扫描找到的地址" << endl;
    cout << "3. 验证指针是否有效（重启后仍然正确）" << endl;
    
    cout << "\n【任务3：查找访问代码】" << endl;
    cout << "1. 右键血量地址 -> 查找访问此地址的代码" << endl;
    cout << "2. 观察哪些指令在读写血量" << endl;
    cout << "3. 找到修改血量的函数" << endl;
    
    cout << "\n【任务4：结构体分析】" << endl;
    cout << "1. 找到玩家对象基址：0x" << hex << (uintptr_t)g_Engine->player << dec << endl;
    cout << "2. 在内存查看器中查看该地址" << endl;
    cout << "3. 分析结构体布局：" << endl;
    cout << "   +0x00: 血量 (int)" << endl;
    cout << "   +0x04: 最大血量 (int)" << endl;
    cout << "   +0x08: 魔法值 (int)" << endl;
    cout << "   +0x0C: 等级 (int)" << endl;
    cout << "   +0x10: 经验值 (int)" << endl;
    cout << "   +0x14: X坐标 (float)" << endl;
    cout << "   +0x18: Y坐标 (float)" << endl;
    cout << "   +0x1C: Z坐标 (float)" << endl;
    cout << "   +0x20: 金币 (int)" << endl;
    
    cout << "\n【任务5：多级指针】" << endl;
    cout << "1. g_Engine地址：0x" << hex << (uintptr_t)&g_Engine << dec << endl;
    cout << "2. 访问路径：" << endl;
    cout << "   g_Engine -> player -> health" << endl;
    cout << "3. 手动计算：" << endl;
    cout << "   Step1: 读取 g_Engine 的值" << endl;
    cout << "   Step2: 读取 [g_Engine + 0x00] (player指针)" << endl;
    cout << "   Step3: 读取 [player + 0x00] (血量)" << endl;
    
    cout << "\n【任务6：代码注入】" << endl;
    cout << "1. 找到修改血量的指令" << endl;
    cout << "2. 使用 Auto Assemble 注入代码" << endl;
    cout << "3. 让血量永远不减少（无敌）" << endl;
    
    cout << "\n【提示】" << endl;
    cout << "- 扫描类型选择：4字节（int）、单精度浮点（float）" << endl;
    cout << "- 如果找不到：尝试 \"未知初始值\" 扫描" << endl;
    cout << "- 多级指针扫描时，最大偏移设置为 0x2000" << endl;
    cout << "==========================================\n" << endl;
}

// ====================================
// 主程序
// ====================================

int main() {
    SetConsoleOutputCP(CP_UTF8);
    srand((unsigned)time(nullptr));
    
    cout << "╔═══════════════════════════════════════════╗" << endl;
    cout << "║  UE逆向学习 - Cheat Engine 实战练习      ║" << endl;
    cout << "╚═══════════════════════════════════════════╝" << endl;
    
    // 创建游戏引擎
    g_Engine = new GameEngine();
    
    PrintCETasks();
    
    cout << "程序进程ID: " << GetCurrentProcessId() << endl;
    cout << "按任意键开始游戏循环...\n" << endl;
    system("pause");
    
    // 游戏主循环
    int tick = 0;
    while (true) {
        // 更新游戏
        g_Engine->player->Update();
        
        // 每5秒显示一次信息
        if (tick % 50 == 0) {
            g_Engine->player->PrintInfo();
            
            // 显示关键地址
            cout << "【关键地址】" << endl;
            cout << "g_Engine:        0x" << hex << (uintptr_t)&g_Engine << dec << endl;
            cout << "g_Engine->player: 0x" << hex << (uintptr_t)g_Engine->player << dec << endl;
            cout << "Player.health:   0x" << hex << (uintptr_t)&g_Engine->player->health << dec << endl;
            cout << "Player.gold:     0x" << hex << (uintptr_t)&g_Engine->player->gold << dec << endl;
            cout << endl;
        }
        
        tick++;
        Sleep(100);  // 100ms 一次更新
    }
    
    delete g_Engine;
    return 0;
}

/*
 * ═══════════════════════════════════════
 * 课后作业
 * ═══════════════════════════════════════
 * 
 * 1. 完成所有6个CE练习任务
 * 2. 尝试修改其他数值（金币、等级等）
 * 3. 使用指针扫描找到稳定的指针路径
 * 4. 练习查找访问代码功能
 * 5. 尝试使用Lua脚本自动化操作
 * 
 * ═══════════════════════════════════════
 * 下一课预告
 * ═══════════════════════════════════════
 * 
 * 第五课：x64dbg 动态调试
 * - 断点设置和条件断点
 * - 内存断点和硬件断点
 * - 调用栈分析
 * - 寄存器查看
 * - 单步调试技巧
 */
