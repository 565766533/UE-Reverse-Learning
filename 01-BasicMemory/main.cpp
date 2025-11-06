/*
 * ========================================
 * UE游戏逆向学习 - 第一课：基础内存操作
 * ========================================
 * 
 * 学习目标：
 * 1. 理解进程内存读写
 * 2. 学习指针的概念
 * 3. 掌握基本的内存搜索
 */

#include <windows.h>
#include <iostream>
#include <iomanip>

using namespace std;

// ====================================
// 第一部分：内存读写基础
// ====================================

namespace MemoryBasics {
    
    // 模拟游戏数据结构
    struct Player {
        int health;         // 血量
        int maxHealth;      // 最大血量
        float posX;         // X坐标
        float posY;         // Y坐标
        float posZ;         // Z坐标
        char name[32];      // 玩家名称
    };

    // 演示1：直接内存访问
    void Demo_DirectAccess() {
        cout << "\n=== 演示1：直接内存访问 ===" << endl;
        
        Player player;
        player.health = 100;
        player.maxHealth = 100;
        player.posX = 1000.0f;
        player.posY = 2000.0f;
        player.posZ = 500.0f;
        strcpy_s(player.name, "TestPlayer");

        cout << "玩家地址: 0x" << hex << (uintptr_t)&player << dec << endl;
        cout << "血量: " << player.health << endl;
        cout << "坐标: (" << player.posX << ", " << player.posY << ", " << player.posZ << ")" << endl;
        
        // 知识点1：通过地址访问内存
        int* healthPtr = &player.health;
        cout << "\n通过指针修改血量..." << endl;
        *healthPtr = 50;
        cout << "新血量: " << player.health << endl;
    }

    // 演示2：指针偏移
    void Demo_PointerOffset() {
        cout << "\n=== 演示2：指针偏移（UE中很常见） ===" << endl;
        
        Player player = {100, 100, 0, 0, 0, "Player1"};
        
        // 获取结构体起始地址
        uintptr_t baseAddr = (uintptr_t)&player;
        
        // 计算各成员偏移
        cout << "结构体基址: 0x" << hex << baseAddr << dec << endl;
        cout << "health偏移: 0x" << hex << offsetof(Player, health) << dec << endl;
        cout << "posX偏移: 0x" << hex << offsetof(Player, posX) << dec << endl;
        cout << "name偏移: 0x" << hex << offsetof(Player, name) << dec << endl;
        
        // 通过基址+偏移访问（这就是游戏逆向的核心！）
        int* healthPtr = (int*)(baseAddr + offsetof(Player, health));
        float* posXPtr = (float*)(baseAddr + offsetof(Player, posX));
        
        cout << "\n通过基址+偏移读取数据：" << endl;
        cout << "血量: " << *healthPtr << endl;
        cout << "X坐标: " << *posXPtr << endl;
    }

    // 演示3：多级指针（UE中非常重要）
    void Demo_MultiLevelPointer() {
        cout << "\n=== 演示3：多级指针链 ===" << endl;
        
        Player player = {100, 100, 1000, 2000, 500, "Hero"};
        Player* playerPtr = &player;
        Player** playerPtrPtr = &playerPtr;
        Player*** playerPtrPtrPtr = &playerPtrPtr;
        
        cout << "Level 0 (对象): " << hex << (uintptr_t)&player << dec << endl;
        cout << "Level 1 (指针): " << hex << (uintptr_t)playerPtr << dec << endl;
        cout << "Level 2 (指针的指针): " << hex << (uintptr_t)*playerPtrPtr << dec << endl;
        cout << "Level 3 (指针的指针的指针): " << hex << (uintptr_t)**playerPtrPtrPtr << dec << endl;
        
        // 通过三级指针访问血量
        int health = (***playerPtrPtrPtr).health;
        cout << "\n通过三级指针读取血量: " << health << endl;
        
        cout << "\n在UE游戏中，通常是这样的结构：" << endl;
        cout << "GEngine -> UWorld -> GameState -> PlayerArray -> PlayerController -> Pawn -> Health" << endl;
    }
}

// ====================================
// 第二部分：内存搜索基础
// ====================================

namespace MemorySearch {
    
    // 演示4：简单的内存搜索
    void Demo_SimpleSearch() {
        cout << "\n=== 演示4：内存搜索（Cheat Engine原理） ===" << endl;
        
        // 创建一个内存区域用于搜索
        int testData[100];
        for (int i = 0; i < 100; i++) {
            testData[i] = i * 10;
        }
        
        // 搜索值为500的地址
        int searchValue = 500;
        cout << "搜索值: " << searchValue << endl;
        
        uintptr_t startAddr = (uintptr_t)testData;
        uintptr_t endAddr = startAddr + sizeof(testData);
        
        cout << "搜索范围: 0x" << hex << startAddr << " - 0x" << endAddr << dec << endl;
        cout << "\n找到的地址：" << endl;
        
        for (uintptr_t addr = startAddr; addr < endAddr; addr += sizeof(int)) {
            int* ptr = (int*)addr;
            if (*ptr == searchValue) {
                size_t offset = addr - startAddr;
                cout << "  0x" << hex << addr << dec 
                     << " (偏移: 0x" << hex << offset << dec << ")" 
                     << " = " << *ptr << endl;
            }
        }
    }

    // 演示5：特征码扫描（Pattern Scan）
    void Demo_PatternScan() {
        cout << "\n=== 演示5：特征码扫描 ===" << endl;
        
        // 模拟一段内存
        unsigned char memory[] = {
            0x48, 0x8B, 0x05, 0x12, 0x34, 0x56, 0x78,  // mov rax, [rip+offset]
            0x48, 0x85, 0xC0,                          // test rax, rax
            0x74, 0x08,                                // je +8
            0x90, 0x90, 0x90                           // nop nop nop
        };
        
        // 特征码：48 8B 05 ?? ?? ?? ?? (??表示通配符)
        unsigned char pattern[] = {0x48, 0x8B, 0x05};
        
        cout << "内存内容: ";
        for (int i = 0; i < sizeof(memory); i++) {
            cout << hex << setw(2) << setfill('0') << (int)memory[i] << " ";
        }
        cout << dec << endl;
        
        cout << "搜索特征码: 48 8B 05" << endl;
        
        // 扫描
        for (size_t i = 0; i <= sizeof(memory) - sizeof(pattern); i++) {
            bool found = true;
            for (size_t j = 0; j < sizeof(pattern); j++) {
                if (memory[i + j] != pattern[j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                cout << "找到特征码位置: 0x" << hex << i << dec << endl;
            }
        }
    }
}

// ====================================
// 第三部分：UE引擎基础概念
// ====================================

namespace UEConcepts {
    
    void Demo_UEStructures() {
        cout << "\n=== UE引擎基础结构讲解 ===" << endl;
        
        cout << "\n1. UObject - UE中所有对象的基类" << endl;
        cout << "   ├─ VTable (虚函数表指针)  +0x00" << endl;
        cout << "   ├─ ClassPrivate            +0x08" << endl;
        cout << "   ├─ NamePrivate             +0x18" << endl;
        cout << "   └─ OuterPrivate            +0x20" << endl;
        
        cout << "\n2. AActor - 所有可放置在世界中的对象" << endl;
        cout << "   ├─ UObject 基类" << endl;
        cout << "   ├─ RootComponent           +偏移（动态查找）" << endl;
        cout << "   └─ 其他成员..." << endl;
        
        cout << "\n3. APawn - 可被控制的Actor" << endl;
        cout << "   ├─ AActor 基类" << endl;
        cout << "   ├─ PlayerState" << endl;
        cout << "   └─ Controller" << endl;
        
        cout << "\n4. 重要全局变量：" << endl;
        cout << "   ├─ GEngine  (引擎实例)" << endl;
        cout << "   ├─ GWorld   (当前世界)" << endl;
        cout << "   └─ GNames   (名称表)" << endl;
        
        cout << "\n5. 内存访问流程示例：" << endl;
        cout << "   GEngine(静态地址) -> GameViewport -> World -> GameState -> PlayerArray[0]" << endl;
        cout << "   -> PlayerController -> Pawn -> HealthComponent -> CurrentHealth" << endl;
    }
}

// ====================================
// 主程序
// ====================================

int main() {
    SetConsoleOutputCP(CP_UTF8);  // 支持中文输出
    
    cout << "========================================" << endl;
    cout << "  UE游戏逆向学习 - 第一课：基础知识" << endl;
    cout << "========================================" << endl;
    
    // 第一部分：内存基础
    MemoryBasics::Demo_DirectAccess();
    MemoryBasics::Demo_PointerOffset();
    MemoryBasics::Demo_MultiLevelPointer();
    
    // 第二部分：内存搜索
    MemorySearch::Demo_SimpleSearch();
    MemorySearch::Demo_PatternScan();
    
    // 第三部分：UE概念
    UEConcepts::Demo_UEStructures();
    
    cout << "\n========================================" << endl;
    cout << "第一课完成！" << endl;
    cout << "\n课后作业：" << endl;
    cout << "1. 理解指针和内存地址的关系" << endl;
    cout << "2. 尝试修改 Demo 代码，搜索不同的值" << endl;
    cout << "3. 思考：为什么游戏更新后地址会变？" << endl;
    cout << "========================================" << endl;
    
    system("pause");
    return 0;
}
