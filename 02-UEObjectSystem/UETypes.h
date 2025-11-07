/*
 * ========================================
 * UE游戏逆向学习 - 第二课：UE对象系统
 * ========================================
 * 
 * 这个文件模拟UE4/UE5的对象系统
 * 帮助你理解UE内部结构
 */

#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <cstdint>
#include <cmath>

// ====================================
// 第一部分：FName - UE的名称系统
// ====================================

/*
 * 知识点：FName
 * - UE中用于快速比较字符串的系统
 * - 使用索引而不是字符串比较，性能更高
 * - 所有FName存储在GNames全局表中
 */
struct FName {
    int32_t Index;      // 在GNames表中的索引
    int32_t Number;     // 实例编号（如多个同名对象）
    
    // 在实际逆向中，我们需要通过这个函数获取真实名称
    std::string GetName() {
        // 实际游戏中会调用引擎的GetName函数
        // 地址类似: 游戏基址 + 0x12345678
        return "ExampleName_" + std::to_string(Index);
    }
};

// ====================================
// 第二部分：TArray - UE的动态数组
// ====================================

/*
 * 知识点：TArray<T>
 * - UE的动态数组实现
 * - 内存布局：Data指针 + Count + Max
 * - 逆向时经常遇到，如PlayerArray、ActorArray
 */
template<typename T>
struct TArray {
    T* Data;            // +0x00: 数组数据指针
    int32_t Count;      // +0x08: 当前元素数量
    int32_t Max;        // +0x0C: 最大容量
    
    // 访问元素
    T& operator[](int i) {
        return Data[i];
    }
    
    // 获取数量
    int Num() const {
        return Count;
    }
    
    // 检查索引是否有效
    bool IsValidIndex(int i) const {
        return i >= 0 && i < Count;
    }
};

// ====================================
// 第三部分：FVector - 3D向量
// ====================================

/*
 * 知识点：FVector
 * - UE中的3D坐标/方向
 * - 常用于位置、速度、朝向等
 * - 在游戏中非常常见
 */
struct FVector {
    float X;  // +0x00
    float Y;  // +0x04
    float Z;  // +0x08
    
    FVector() : X(0), Y(0), Z(0) {}
    FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}
    
    // 计算距离
    float Distance(const FVector& other) const {
        float dx = X - other.X;
        float dy = Y - other.Y;
        float dz = Z - other.Z;
        return sqrt(dx*dx + dy*dy + dz*dz);
    }
};

struct FVector2D {
    float X;  // +0x00
    float Y;  // +0x04
    
    FVector2D() : X(0), Y(0) {}
    FVector2D(float x, float y) : X(x), Y(y) {}
};

// ====================================
// 第四部分：UObject - 基础对象
// ====================================

/*
 * 知识点：UObject
 * - UE中所有对象的基类
 * - 包含虚函数表、类信息、名称等
 * - 逆向的核心结构
 */
class UClass;  // 前向声明

class UObject {
public:
    void** VTable;          // +0x00: 虚函数表指针（用于找到ProcessEvent等函数）
    int32_t Flags;          // +0x08: 对象标志
    int32_t InternalIndex;  // +0x0C: 内部索引
    UClass* ClassPrivate;   // +0x10: 类型信息
    FName NamePrivate;      // +0x18: 对象名称
    UObject* OuterPrivate;  // +0x20: 外部对象（所有者）
    
    // 获取对象名称
    virtual std::string GetName() {
        return NamePrivate.GetName();
    }
    
    // ProcessEvent - 调用蓝图函数的核心！
    // 在实际游戏中，这个函数的地址需要逆向查找
    // 通常在 VTable[68] 或类似位置
    virtual void ProcessEvent(void* Function, void* Params) {
        // 实际调用：VTable[68](this, Function, Params)
        // 这是调用游戏函数的关键
    }
};

// ====================================
// 第五部分：UClass - 类型信息
// ====================================

/*
 * 知识点：UClass
 * - 描述UObject的类型
 * - 包含类的成员变量、函数等信息
 */
class UClass : public UObject {
public:
    // 这里简化，实际更复杂
    UClass* SuperClass;  // 父类
};

// ====================================
// 第六部分：AActor - 游戏对象
// ====================================

/*
 * 知识点：AActor
 * - 所有可以放置在世界中的对象
 * - 包含位置、旋转、组件等
 */
class USceneComponent;

class AActor : public UObject {
public:
    // 注意：实际偏移需要通过逆向工具找到
    // 这里只是示例
    
    USceneComponent* RootComponent;  // 根组件（包含Transform信息）
    
    // 在DeltaForce中，你需要找到这些偏移：
    // RootComponent的偏移可能是 +0x130
};

// ====================================
// 第七部分：APawn - 可控制角色
// ====================================

/*
 * 知识点：APawn
 * - 可被玩家或AI控制的Actor
 * - 包含控制器、状态等
 */
class APlayerState;
class AController;

class APawn : public AActor {
public:
    APlayerState* PlayerState;    // 玩家状态
    AController* Controller;      // 控制器
};

// ====================================
// 第八部分：USceneComponent - 场景组件
// ====================================

/*
 * 知识点：USceneComponent
 * - 包含Transform信息（位置、旋转、缩放）
 */
struct FRotator {
    float Pitch;  // 俯仰
    float Yaw;    // 偏航
    float Roll;   // 翻滚
};

struct FTransform {
    FRotator Rotation;
    FVector Translation;  // 位置
    FVector Scale3D;      // 缩放
};

class USceneComponent : public UObject {
public:
    FVector RelativeLocation;     // 相对位置
    FRotator RelativeRotation;    // 相对旋转
    FVector RelativeScale3D;      // 相对缩放
    
    // 获取世界坐标（需要调用游戏函数）
    FVector GetWorldLocation() {
        // 实际需要调用引擎函数
        return RelativeLocation;
    }
};

// ====================================
// 第九部分：UWorld - 游戏世界
// ====================================

/*
 * 知识点：UWorld
 * - 代表整个游戏世界
 * - 包含所有关卡、Actor等
 */
class ULevel;
class AGameStateBase;

class UWorld : public UObject {
public:
    TArray<ULevel*> Levels;           // 关卡数组
    AGameStateBase* GameState;        // 游戏状态
    
    // 在实际逆向中的偏移示例：
    // Levels: +0x148
    // GameState: +0x150
};

// ====================================
// 使用示例和讲解
// ====================================

namespace UEExample {
    
    // 演示：如何访问玩家位置
    void Demo_GetPlayerLocation() {
        /*
         * 实际游戏中的访问路径：
         * 
         * 1. 找到 GEngine (静态地址，通过CE扫描)
         *    地址示例: GameBase + 0x12345678
         * 
         * 2. GEngine -> GameViewport
         *    偏移: +0x78 (需要逆向查找)
         * 
         * 3. GameViewport -> World
         *    偏移: +0x80
         * 
         * 4. World -> GameState
         *    偏移: +0x150
         * 
         * 5. GameState -> PlayerArray
         *    偏移: +0x2A8
         * 
         * 6. PlayerArray[0] -> Pawn
         *    偏移: +0x2B0
         * 
         * 7. Pawn -> RootComponent
         *    偏移: +0x130
         * 
         * 8. RootComponent -> RelativeLocation
         *    偏移: +0x120
         */
        
        // 伪代码：
        /*
        uintptr_t GEngine = *(uintptr_t*)(GameBase + 0x12345678);
        uintptr_t GameViewport = *(uintptr_t*)(GEngine + 0x78);
        uintptr_t World = *(uintptr_t*)(GameViewport + 0x80);
        uintptr_t GameState = *(uintptr_t*)(World + 0x150);
        uintptr_t PlayerArray = *(uintptr_t*)(GameState + 0x2A8);
        uintptr_t PlayerState = *(uintptr_t*)(PlayerArray + 0x00);  // 第一个玩家
        uintptr_t Pawn = *(uintptr_t*)(PlayerState + 0x2B0);
        uintptr_t RootComponent = *(uintptr_t*)(Pawn + 0x130);
        FVector Location = *(FVector*)(RootComponent + 0x120);
        */
    }
}
