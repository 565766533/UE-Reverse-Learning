/*
 * ========================================
 * 实战项目：完整的ESP功能实现
 * ========================================
 * 
 * 本程序实现一个完整的ESP（透视）功能
 * 包含内存读取、数据处理、显示等
 */

#include "SimulatedGame.h"
#include <iostream>
#include <iomanip>

using namespace std;

// 全局引擎实例定义
UGameEngine* GEngine = nullptr;

// ====================================
// ESP 数据结构
// ====================================

struct ESPData {
    string name;
    int teamId;
    float health;
    FVector position;
    float distance;
    bool isBot;
    bool isAlive;
};

// ====================================
// 内存读取模块（模拟实际逆向中的读取）
// ====================================

class MemoryReader {
public:
    // 读取GEngine
    static UGameEngine* GetGEngine() {
        // 在真实逆向中，这是一个静态地址
        // 例如: GameBase + 0x12345678
        return GEngine;
    }
    
    // 读取World
    static UWorld* GetWorld() {
        auto engine = GetGEngine();
        if (!engine) return nullptr;
        
        // 偏移: +0x78
        auto viewport = engine->GameViewport;
        if (!viewport) return nullptr;
        
        // 偏移: +0x80
        return viewport->World;
    }
    
    // 读取GameState
    static AGameState* GetGameState() {
        auto world = GetWorld();
        if (!world) return nullptr;
        
        // 偏移: +0x150
        return world->GameState;
    }
    
    // 读取所有角色
    static TArray<AActor*> GetAllActors() {
        auto world = GetWorld();
        if (!world || world->Levels.Num() == 0) {
            return TArray<AActor*>();
        }
        
        return world->Levels[0]->Actors;
    }
    
    // 读取本地玩家
    static ACharacter* GetLocalPlayer() {
        auto actors = GetAllActors();
        if (actors.Num() == 0) return nullptr;
        
        // 假设第一个是本地玩家
        return (ACharacter*)actors[0];
    }
};

// ====================================
// ESP 功能模块
// ====================================

class ESP {
private:
    int localTeamId;
    FVector localPosition;
    
public:
    ESP() : localTeamId(-1) {}
    
    void Update() {
        // 获取本地玩家信息
        auto localPlayer = MemoryReader::GetLocalPlayer();
        if (localPlayer) {
            localTeamId = localPlayer->GetTeamId();
            localPosition = localPlayer->GetActorLocation();
        }
    }
    
    vector<ESPData> GatherESPData() {
        vector<ESPData> espList;
        
        auto actors = MemoryReader::GetAllActors();
        
        for (int i = 0; i < actors.Num(); i++) {
            ACharacter* character = (ACharacter*)actors[i];
            if (!character) continue;
            
            // 跳过本地玩家
            if (i == 0) continue;
            
            // 只显示敌人
            int teamId = character->GetTeamId();
            if (teamId == localTeamId) continue;
            
            // 收集数据
            ESPData data;
            data.name = character->PlayerState ? character->PlayerState->PlayerName : "Unknown";
            data.teamId = teamId;
            data.health = character->HealthComponent ? character->HealthComponent->CurrentHealth : 0;
            data.position = character->GetActorLocation();
            data.distance = localPosition.Distance(data.position);
            data.isBot = character->bIsBot;
            data.isAlive = character->IsAlive();
            
            if (data.isAlive) {
                espList.push_back(data);
            }
        }
        
        return espList;
    }
    
    void RenderESP(const vector<ESPData>& espList) {
        cout << "\n╔═══════════════════════════════════════════════════════════╗" << endl;
        cout << "║                    ESP - 敌人透视                         ║" << endl;
        cout << "╚═══════════════════════════════════════════════════════════╝" << endl;
        
        if (espList.empty()) {
            cout << "  没有发现敌人" << endl;
            return;
        }
        
        cout << "\n敌人数量: " << espList.size() << endl;
        cout << "\n" << left 
             << setw(15) << "名称"
             << setw(8) << "类型"
             << setw(10) << "血量"
             << setw(12) << "距离"
             << "位置" << endl;
        cout << string(70, '-') << endl;
        
        for (const auto& data : espList) {
            cout << left
                 << setw(15) << data.name
                 << setw(8) << (data.isBot ? "[AI]" : "[玩家]")
                 << setw(10) << (to_string((int)data.health) + " HP")
                 << setw(12) << (to_string((int)data.distance) + "m")
                 << "(" << (int)data.position.X << ", " 
                 << (int)data.position.Y << ", "
                 << (int)data.position.Z << ")"
                 << endl;
        }
        
        cout << string(70, '-') << endl;
    }
    
    void RenderRadar(const vector<ESPData>& espList) {
        cout << "\n╔═══════════════════════════════════════════╗" << endl;
        cout << "║              雷达视图                     ║" << endl;
        cout << "╚═══════════════════════════════════════════╝" << endl;
        
        // 简化的2D雷达（11x11网格）
        const int SIZE = 11;
        const int CENTER = SIZE / 2;
        const float SCALE = 20.0f;  // 每格代表20米
        
        char radar[SIZE][SIZE];
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                radar[i][j] = '.';
            }
        }
        
        // 本地玩家（中心）
        radar[CENTER][CENTER] = '@';
        
        // 敌人
        for (const auto& data : espList) {
            float relX = data.position.X - localPosition.X;
            float relY = data.position.Y - localPosition.Y;
            
            int gridX = CENTER + (int)(relX / SCALE);
            int gridY = CENTER - (int)(relY / SCALE);  // Y轴反向
            
            if (gridX >= 0 && gridX < SIZE && gridY >= 0 && gridY < SIZE) {
                char symbol = data.isBot ? 'A' : 'E';  // A=AI, E=Enemy
                radar[gridY][gridX] = symbol;
            }
        }
        
        // 显示雷达
        cout << "\n  ";
        for (int i = 0; i < SIZE; i++) cout << "-";
        cout << endl;
        
        for (int i = 0; i < SIZE; i++) {
            cout << " |";
            for (int j = 0; j < SIZE; j++) {
                cout << radar[i][j];
            }
            cout << "|" << endl;
        }
        
        cout << "  ";
        for (int i = 0; i < SIZE; i++) cout << "-";
        cout << endl;
        
        cout << "\n  @ = 你    E = 敌人玩家    A = 敌人AI    . = 空" << endl;
        cout << "  每格 = " << SCALE << "米" << endl;
    }
};

// ====================================
// 偏移查找演示
// ====================================

void DemonstrateOffsetFinding() {
    cout << "\n╔═══════════════════════════════════════════╗" << endl;
    cout << "║        内存偏移查找演示                   ║" << endl;
    cout << "╚═══════════════════════════════════════════╝" << endl;
    
    cout << "\n【关键地址】" << endl;
    cout << "GEngine:                0x" << hex << (uintptr_t)GEngine << dec << endl;
    
    if (GEngine) {
        cout << "GEngine->GameViewport:  0x" << hex << (uintptr_t)GEngine->GameViewport << dec << endl;
        
        uintptr_t gameViewportOffset = (uintptr_t)&GEngine->GameViewport - (uintptr_t)GEngine;
        cout << "  偏移: +0x" << hex << gameViewportOffset << dec << endl;
        
        if (GEngine->GameViewport) {
            cout << "GameViewport->World:    0x" << hex << (uintptr_t)GEngine->GameViewport->World << dec << endl;
            
            uintptr_t worldOffset = (uintptr_t)&GEngine->GameViewport->World - (uintptr_t)GEngine->GameViewport;
            cout << "  偏移: +0x" << hex << worldOffset << dec << endl;
            
            if (GEngine->GameViewport->World) {
                cout << "World->GameState:       0x" << hex << (uintptr_t)GEngine->GameViewport->World->GameState << dec << endl;
                
                uintptr_t gameStateOffset = (uintptr_t)&GEngine->GameViewport->World->GameState - (uintptr_t)GEngine->GameViewport->World;
                cout << "  偏移: +0x" << hex << gameStateOffset << dec << endl;
            }
        }
    }
    
    cout << "\n【访问路径】" << endl;
    cout << "GEngine [静态地址]" << endl;
    cout << "  -> GameViewport [+0x78]" << endl;
    cout << "     -> World [+0x80]" << endl;
    cout << "        -> GameState [+0x150]" << endl;
    cout << "           -> PlayerArray [+0x2A8]" << endl;
    cout << "        -> Levels [+0x148]" << endl;
    cout << "           -> Actors" << endl;
    
    cout << "\n【如何在真实游戏中找到这些偏移】" << endl;
    cout << "1. 用CE找到玩家对象地址" << endl;
    cout << "2. 用 'Find out what accesses this address' 找到访问代码" << endl;
    cout << "3. 在x64dbg中分析访问代码，找到偏移值" << endl;
    cout << "4. 向上追溯，找到GEngine的引用" << endl;
    cout << "5. 使用特征码定位GEngine的静态地址" << endl;
    cout << "==========================================\n" << endl;
}

// ====================================
// 主程序
// ====================================

int main() {
    SetConsoleOutputCP(CP_UTF8);
    
    cout << "╔═══════════════════════════════════════════╗" << endl;
    cout << "║    实战项目：ESP功能完整实现              ║" << endl;
    cout << "╚═══════════════════════════════════════════╝" << endl;
    
    // 初始化游戏引擎
    GEngine = new UGameEngine();
    
    // 初始化游戏
    GameSimulator game;
    
    // 显示偏移信息
    DemonstrateOffsetFinding();
    
    cout << "\n按任意键开始ESP演示..." << endl;
    system("pause");
    
    // 创建ESP
    ESP esp;
    
    // 主循环
    int tick = 0;
    while (true) {
        // 更新游戏
        game.Update();
        
        // 每30帧显示一次
        if (tick % 30 == 0) {
            system("cls");
            
            cout << "╔═══════════════════════════════════════════╗" << endl;
            cout << "║    ESP演示 - Tick: " << setw(5) << tick << "                ║" << endl;
            cout << "╚═══════════════════════════════════════════╝" << endl;
            
            // 更新ESP
            esp.Update();
            
            // 收集ESP数据
            auto espData = esp.GatherESPData();
            
            // 显示ESP
            esp.RenderESP(espData);
            
            // 显示雷达
            esp.RenderRadar(espData);
            
            // 显示游戏状态
            game.PrintGameState();
            
            cout << "\n按 Ctrl+C 退出" << endl;
        }
        
        tick++;
        Sleep(100);
    }
    
    delete GEngine;
    return 0;
}

/*
 * ═══════════════════════════════════════
 * 学习要点
 * ═══════════════════════════════════════
 * 
 * 1. 内存读取路径：
 *    GEngine -> GameViewport -> World -> GameState -> PlayerArray
 * 
 * 2. 数据收集：
 *    - 遍历所有Actor
 *    - 过滤敌人
 *    - 计算距离
 *    - 收集显示信息
 * 
 * 3. 显示方式：
 *    - 列表显示（详细信息）
 *    - 雷达显示（2D位置）
 *    - 可扩展为3D绘制
 * 
 * ═══════════════════════════════════════
 * 扩展练习
 * ═══════════════════════════════════════
 * 
 * 1. 添加血条显示
 * 2. 实现距离排序
 * 3. 添加骨骼绘制
 * 4. 实现自瞄功能
 * 5. 添加配置选项（只显示玩家/AI等）
 * 
 * ═══════════════════════════════════════
 * 对比真实项目
 * ═══════════════════════════════════════
 * 
 * DeltaForce-Glow 项目中的 Entity.h
 * 实现了类似的功能：
 * - C_Entity::UpData() 对应 ESP::Update()
 * - 遍历 ACharacters 对应 GetAllActors()
 * - PlayOutLineEffect() 对应 RenderESP()
 */
