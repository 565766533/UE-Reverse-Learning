/*
 * ========================================
 * 实战项目：模拟UE游戏环境
 * ========================================
 * 
 * 这个文件模拟一个真实的UE游戏环境
 * 包含完整的对象系统和内存结构
 */

#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <cstdint>
#include <random>

// ====================================
// UE 基础类型定义
// ====================================

struct FVector {
    float X, Y, Z;
    
    FVector() : X(0), Y(0), Z(0) {}
    FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}
    
    float Distance(const FVector& other) const {
        float dx = X - other.X;
        float dy = Y - other.Y;
        float dz = Z - other.Z;
        return sqrt(dx*dx + dy*dy + dz*dz);
    }
};

struct FRotator {
    float Pitch, Yaw, Roll;
    
    FRotator() : Pitch(0), Yaw(0), Roll(0) {}
    FRotator(float p, float y, float r) : Pitch(p), Yaw(y), Roll(r) {}
};

template<typename T>
struct TArray {
    T* Data;
    int32_t Count;
    int32_t Max;
    
    TArray() : Data(nullptr), Count(0), Max(0) {}
    
    T& operator[](int i) { return Data[i]; }
    int Num() const { return Count; }
    
    void Add(const T& item) {
        if (Count >= Max) {
            Max = Max ? Max * 2 : 4;
            T* newData = new T[Max];
            if (Data) {
                memcpy(newData, Data, Count * sizeof(T));
                delete[] Data;
            }
            Data = newData;
        }
        Data[Count++] = item;
    }
};

// ====================================
// UE 对象系统
// ====================================

class UObject {
public:
    void** VTable;                  // +0x00
    uint32_t Flags;                 // +0x08
    uint32_t Index;                 // +0x0C
    void* ClassPrivate;             // +0x10
    char NameData[16];              // +0x18
    
    UObject() {
        static void* dummyVTable[10] = {0};
        VTable = dummyVTable;
        Flags = 0;
        Index = 0;
        ClassPrivate = nullptr;
        memset(NameData, 0, sizeof(NameData));
    }
    
    virtual ~UObject() {}
    virtual void Update() {}
};

class USceneComponent : public UObject {
public:
    FVector RelativeLocation;       // +0x28
    FRotator RelativeRotation;      // +0x34
    FVector RelativeScale3D;        // +0x40
    
    FVector GetWorldLocation() const {
        return RelativeLocation;
    }
};

class AActor : public UObject {
public:
    USceneComponent* RootComponent; // +0x130 (模拟偏移)
    uint8_t Padding1[0x130 - sizeof(UObject)];
    
    AActor() : RootComponent(nullptr) {
        memset(Padding1, 0, sizeof(Padding1));
    }
    
    FVector GetActorLocation() {
        if (RootComponent) {
            return RootComponent->GetWorldLocation();
        }
        return FVector();
    }
};

// ====================================
// 游戏角色类
// ====================================

class APlayerState : public AActor {
public:
    char PlayerName[32];            // +0x2A0
    int32_t PlayerId;               // +0x2C0
    int32_t TeamId;                 // +0x2C4
    
    APlayerState() : PlayerId(0), TeamId(0) {
        memset(PlayerName, 0, sizeof(PlayerName));
    }
};

class UHealthComponent : public UObject {
public:
    float CurrentHealth;            // +0x28
    float MaxHealth;                // +0x2C
    
    UHealthComponent() : CurrentHealth(100.0f), MaxHealth(100.0f) {}
    
    bool IsAlive() const { return CurrentHealth > 0; }
    float GetHealthPercent() const { return CurrentHealth / MaxHealth; }
};

class APawn : public AActor {
public:
    APlayerState* PlayerState;      // +0x2B0
    uint8_t Padding2[0x2B0 - 0x130 - sizeof(void*)];
    
    APawn() : PlayerState(nullptr) {
        memset(Padding2, 0, sizeof(Padding2));
    }
};

class ACharacter : public APawn {
public:
    UHealthComponent* HealthComponent; // +0x3A0
    bool bIsBot;                       // +0x3A8
    uint8_t Padding3[0x3A0 - 0x2B0 - sizeof(void*)];
    
    ACharacter() : HealthComponent(nullptr), bIsBot(false) {
        memset(Padding3, 0, sizeof(Padding3));
        HealthComponent = new UHealthComponent();
    }
    
    ~ACharacter() {
        delete HealthComponent;
    }
    
    bool IsAlive() {
        return HealthComponent && HealthComponent->IsAlive();
    }
    
    int GetTeamId() {
        if (PlayerState) {
            return PlayerState->TeamId;
        }
        return -1;
    }
};

// ====================================
// 游戏世界
// ====================================

class AGameState : public AActor {
public:
    TArray<APlayerState*> PlayerArray;  // +0x2A8
    uint8_t Padding4[0x2A8 - sizeof(AActor)];
    
    AGameState() {
        memset(Padding4, 0, sizeof(Padding4));
    }
};

class ULevel : public UObject {
public:
    TArray<AActor*> Actors;            // +0x98
    uint8_t Padding5[0x98 - sizeof(UObject)];
    
    ULevel() {
        memset(Padding5, 0, sizeof(Padding5));
    }
};

class UWorld : public UObject {
public:
    TArray<ULevel*> Levels;            // +0x148
    AGameState* GameState;             // +0x150
    uint8_t Padding6[0x148 - sizeof(UObject)];
    
    UWorld() : GameState(nullptr) {
        memset(Padding6, 0, sizeof(Padding6));
        GameState = new AGameState();
    }
    
    ~UWorld() {
        // 清理Levels
        for (int i = 0; i < Levels.Num(); i++) {
            delete Levels[i];
        }
        delete GameState;
    }
};

// ====================================
// 游戏引擎
// ====================================

class UGameViewportClient : public UObject {
public:
    UWorld* World;                     // +0x80
    uint8_t Padding7[0x80 - sizeof(UObject)];
    
    UGameViewportClient() : World(nullptr) {
        memset(Padding7, 0, sizeof(Padding7));
        World = new UWorld();
    }
    
    ~UGameViewportClient() {
        delete World;
    }
};

class UGameEngine : public UObject {
public:
    UGameViewportClient* GameViewport; // +0x78
    uint8_t Padding8[0x78 - sizeof(UObject)];
    
    UGameEngine() : GameViewport(nullptr) {
        memset(Padding8, 0, sizeof(Padding8));
        GameViewport = new UGameViewportClient();
    }
    
    ~UGameEngine() {
        delete GameViewport;
    }
};

// ====================================
// 全局引擎实例（模拟GEngine）
// ====================================

extern UGameEngine* GEngine;

// ====================================
// 游戏模拟器
// ====================================

class GameSimulator {
private:
    std::vector<ACharacter*> characters;
    std::mt19937 rng;
    
public:
    GameSimulator() : rng(std::random_device{}()) {
        InitializeGame();
    }
    
    ~GameSimulator() {
        for (auto* character : characters) {
            delete character;
        }
    }
    
    void InitializeGame() {
        // 创建关卡
        ULevel* level = new ULevel();
        GEngine->GameViewport->World->Levels.Add(level);
        
        // 创建玩家（本地玩家，队伍0）
        ACharacter* localPlayer = CreateCharacter("LocalPlayer", 0, 0, false);
        localPlayer->RootComponent->RelativeLocation = FVector(0, 0, 0);
        
        // 创建队友（队伍0）
        for (int i = 0; i < 3; i++) {
            char name[32];
            sprintf_s(name, "Teammate%d", i + 1);
            ACharacter* teammate = CreateCharacter(name, i + 1, 0, false);
            
            // 随机位置
            float angle = (i + 1) * 3.14f / 2;
            teammate->RootComponent->RelativeLocation = FVector(
                cos(angle) * 50, sin(angle) * 50, 0
            );
        }
        
        // 创建敌人（队伍1）
        for (int i = 0; i < 5; i++) {
            char name[32];
            sprintf_s(name, "Enemy%d", i + 1);
            ACharacter* enemy = CreateCharacter(name, i + 10, 1, i < 2); // 前2个是AI
            
            // 随机位置
            std::uniform_real_distribution<float> dist(-100, 100);
            enemy->RootComponent->RelativeLocation = FVector(
                dist(rng), dist(rng), 0
            );
        }
    }
    
    ACharacter* CreateCharacter(const char* name, int playerId, int teamId, bool isBot) {
        ACharacter* character = new ACharacter();
        character->bIsBot = isBot;
        
        // 设置RootComponent
        character->RootComponent = new USceneComponent();
        
        // 设置PlayerState
        character->PlayerState = new APlayerState();
        strcpy_s(character->PlayerState->PlayerName, name);
        character->PlayerState->PlayerId = playerId;
        character->PlayerState->TeamId = teamId;
        
        // 添加到世界
        ULevel* level = GEngine->GameViewport->World->Levels[0];
        level->Actors.Add(character);
        
        // 添加到PlayerArray
        GEngine->GameViewport->World->GameState->PlayerArray.Add(character->PlayerState);
        
        characters.push_back(character);
        return character;
    }
    
    void Update() {
        // 模拟角色移动
        for (auto* character : characters) {
            if (!character->IsAlive()) continue;
            
            std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
            FVector& pos = character->RootComponent->RelativeLocation;
            pos.X += dist(rng);
            pos.Y += dist(rng);
            
            // 随机受伤
            if (rand() % 100 < 5) {
                character->HealthComponent->CurrentHealth -= 10;
                if (character->HealthComponent->CurrentHealth < 0) {
                    character->HealthComponent->CurrentHealth = 0;
                }
            }
        }
    }
    
    void PrintGameState() {
        printf("\n========== 游戏状态 ==========\n");
        printf("GEngine:        0x%p\n", (void*)GEngine);
        printf("GameViewport:   0x%p\n", (void*)GEngine->GameViewport);
        printf("World:          0x%p\n", (void*)GEngine->GameViewport->World);
        printf("GameState:      0x%p\n", (void*)GEngine->GameViewport->World->GameState);
        
        ULevel* level = GEngine->GameViewport->World->Levels[0];
        printf("\n角色数量: %d\n", level->Actors.Num());
        
        for (int i = 0; i < level->Actors.Num(); i++) {
            ACharacter* character = (ACharacter*)level->Actors[i];
            if (!character) continue;
            
            FVector pos = character->GetActorLocation();
            const char* name = character->PlayerState ? character->PlayerState->PlayerName : "Unknown";
            int teamId = character->GetTeamId();
            float health = character->HealthComponent->CurrentHealth;
            const char* type = character->bIsBot ? "AI" : "Player";
            
            printf("[%s][Team %d][%.0f HP] %s at (%.1f, %.1f, %.1f)\n",
                type, teamId, health, name, pos.X, pos.Y, pos.Z);
        }
        
        printf("==============================\n\n");
    }
};
