/*
 * ========================================
 * 第五课：x64dbg 动态调试实战
 * ========================================
 * 
 * 本程序包含多个调试场景，帮助你学习x64dbg
 */

#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

// ====================================
// 场景1：简单的函数调用
// ====================================

int Add(int a, int b) {
    // 调试任务：在这里设置断点，观察参数
    int result = a + b;
    return result;
}

int Multiply(int a, int b) {
    // 调试任务：观察寄存器 RCX 和 RDX (x64调用约定)
    return a * b;
}

void Scenario_FunctionCall() {
    cout << "\n=== 场景1：函数调用调试 ===" << endl;
    cout << "x64dbg 任务：" << endl;
    cout << "1. 在 Add 函数设置断点" << endl;
    cout << "2. 观察 RCX=5, RDX=3 (x64 fastcall调用约定)" << endl;
    cout << "3. 单步执行，观察 RAX 返回值" << endl;
    
    int sum = Add(5, 3);
    int product = Multiply(4, 6);
    
    cout << "结果: " << sum << ", " << product << endl;
}

// ====================================
// 场景2：字符串操作
// ====================================

void ProcessString(const char* input) {
    // 调试任务：设置内存断点，观察字符串访问
    char buffer[100];
    strcpy_s(buffer, input);
    
    cout << "处理字符串: " << buffer << endl;
}

void Scenario_StringDebug() {
    cout << "\n=== 场景2：字符串调试 ===" << endl;
    cout << "x64dbg 任务：" << endl;
    cout << "1. 找到字符串 \"Hello World\" 在内存中的位置" << endl;
    cout << "2. 设置内存访问断点" << endl;
    cout << "3. 观察 strcpy_s 的实现" << endl;
    
    ProcessString("Hello World");
}

// ====================================
// 场景3：循环和条件
// ====================================

int FindMax(int arr[], int size) {
    // 调试任务：观察循环变量和比较逻辑
    int max = arr[0];
    
    for (int i = 1; i < size; i++) {
        if (arr[i] > max) {
            max = arr[i];  // 在这里设置条件断点
        }
    }
    
    return max;
}

void Scenario_LoopDebug() {
    cout << "\n=== 场景3：循环调试 ===" << endl;
    cout << "x64dbg 任务：" << endl;
    cout << "1. 在 FindMax 函数设置断点" << endl;
    cout << "2. 观察 for 循环的汇编实现" << endl;
    cout << "3. 设置条件断点：当 max 被更新时才中断" << endl;
    
    int numbers[] = {5, 2, 8, 1, 9, 3};
    int max = FindMax(numbers, 6);
    
    cout << "最大值: " << max << endl;
}

// ====================================
// 场景4：指针和内存
// ====================================

struct Player {
    int health;
    int mana;
    float x, y, z;
};

void ModifyPlayer(Player* player) {
    // 调试任务：观察指针解引用
    player->health = 100;
    player->mana = 50;
    player->x = 10.5f;
    player->y = 20.3f;
    player->z = 5.0f;
}

void Scenario_PointerDebug() {
    cout << "\n=== 场景4：指针调试 ===" << endl;
    cout << "x64dbg 任务：" << endl;
    cout << "1. 查看 Player 结构体在栈上的布局" << endl;
    cout << "2. 观察 RCX 寄存器（player指针）" << endl;
    cout << "3. 跟随 [RCX] 查看内存内容" << endl;
    
    Player p = {0};
    cout << "Player地址: 0x" << hex << (uintptr_t)&p << dec << endl;
    
    ModifyPlayer(&p);
    
    cout << "血量: " << p.health << ", 魔法: " << p.mana << endl;
}

// ====================================
// 场景5：异常处理
// ====================================

void TriggerException(bool shouldCrash) {
    if (shouldCrash) {
        // 调试任务：观察异常处理流程
        int* nullPtr = nullptr;
        *nullPtr = 42;  // 访问空指针
    }
}

void Scenario_ExceptionDebug() {
    cout << "\n=== 场景5：异常调试 ===" << endl;
    cout << "x64dbg 任务：" << endl;
    cout << "1. 设置异常断点" << endl;
    cout << "2. 让程序崩溃，观察异常信息" << endl;
    cout << "3. 查看调用栈，找到异常来源" << endl;
    
    __try {
        TriggerException(false);  // 改成 true 来触发异常
        cout << "正常执行" << endl;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        cout << "捕获异常" << endl;
    }
}

// ====================================
// 场景6：虚函数调用
// ====================================

class Animal {
public:
    virtual void Speak() {
        cout << "Animal speaks" << endl;
    }
    
    virtual void Move() {
        cout << "Animal moves" << endl;
    }
};

class Dog : public Animal {
public:
    void Speak() override {
        cout << "Woof!" << endl;
    }
    
    void Move() override {
        cout << "Dog runs" << endl;
    }
};

void Scenario_VTableDebug() {
    cout << "\n=== 场景6：虚函数表调试 ===" << endl;
    cout << "x64dbg 任务：" << endl;
    cout << "1. 查看对象的 VTable 指针（对象首8字节）" << endl;
    cout << "2. 跟随 VTable，查看虚函数地址" << endl;
    cout << "3. 观察虚函数调用的汇编代码" << endl;
    
    Dog dog;
    Animal* animal = &dog;
    
    cout << "Dog对象地址: 0x" << hex << (uintptr_t)&dog << dec << endl;
    cout << "VTable地址: 0x" << hex << *(uintptr_t*)&dog << dec << endl;
    
    animal->Speak();  // 在这里设置断点观察虚函数调用
    animal->Move();
}

// ====================================
// x64dbg 调试技巧
// ====================================

void PrintDebuggingTips() {
    cout << "\n╔═══════════════════════════════════════════╗" << endl;
    cout << "║        x64dbg 调试技巧大全                ║" << endl;
    cout << "╚═══════════════════════════════════════════╝" << endl;
    
    cout << "\n【断点类型】" << endl;
    cout << "1. 软件断点 (F2)：" << endl;
    cout << "   - 在代码地址设置" << endl;
    cout << "   - 快捷键：F2" << endl;
    
    cout << "\n2. 硬件断点：" << endl;
    cout << "   - 右键 -> Breakpoint -> Hardware, Execute" << endl;
    cout << "   - 最多4个" << endl;
    cout << "   - 不修改代码，难以检测" << endl;
    
    cout << "\n3. 内存断点：" << endl;
    cout << "   - 右键内存 -> Breakpoint -> Memory, Access/Write" << endl;
    cout << "   - 监控内存访问" << endl;
    
    cout << "\n4. 条件断点：" << endl;
    cout << "   - 右键断点 -> Edit" << endl;
    cout << "   - 添加条件：RCX==5" << endl;
    
    cout << "\n【执行控制】" << endl;
    cout << "F7  - 单步步入（进入函数）" << endl;
    cout << "F8  - 单步步过（跳过函数）" << endl;
    cout << "F9  - 运行" << endl;
    cout << "Ctrl+F9 - 运行到返回" << endl;
    cout << "Ctrl+F7 - 运行到选中行" << endl;
    
    cout << "\n【寄存器查看】" << endl;
    cout << "RAX - 返回值" << endl;
    cout << "RCX - 第1个参数 (x64 fastcall)" << endl;
    cout << "RDX - 第2个参数" << endl;
    cout << "R8  - 第3个参数" << endl;
    cout << "R9  - 第4个参数" << endl;
    cout << "RSP - 栈指针" << endl;
    cout << "RBP - 基址指针" << endl;
    
    cout << "\n【内存查看】" << endl;
    cout << "Ctrl+G - 跳转到地址" << endl;
    cout << "Ctrl+B - 二进制搜索" << endl;
    cout << "右键 -> Follow in Dump - 跟随到内存窗口" << endl;
    cout << "右键 -> Follow in Disassembler - 跟随到代码" << endl;
    
    cout << "\n【调用栈】" << endl;
    cout << "查看 Call Stack 窗口" << endl;
    cout << "双击可跳转到调用位置" << endl;
    cout << "观察函数调用链" << endl;
    
    cout << "\n【常用技巧】" << endl;
    cout << "1. 搜索字符串引用：" << endl;
    cout << "   右键字符串 -> Find references" << endl;
    
    cout << "\n2. 搜索函数调用：" << endl;
    cout << "   右键函数名 -> Find references" << endl;
    
    cout << "\n3. 设置注释：" << endl;
    cout << "   ; 键添加注释" << endl;
    cout << "   : 键添加标签" << endl;
    
    cout << "\n4. 修改内存：" << endl;
    cout << "   选中字节 -> 直接输入16进制" << endl;
    cout << "   Ctrl+E -> 编辑" << endl;
    
    cout << "\n5. Patch 程序：" << endl;
    cout << "   右键 -> Patches -> Patch file" << endl;
    cout << "   保存修改后的程序" << endl;
    
    cout << "==========================================\n" << endl;
}

// ====================================
// 汇编指令速查
// ====================================

void PrintAssemblyReference() {
    cout << "\n╔═══════════════════════════════════════════╗" << endl;
    cout << "║      常见汇编指令速查（x64）              ║" << endl;
    cout << "╚═══════════════════════════════════════════╝" << endl;
    
    cout << "\n【数据传送】" << endl;
    cout << "mov rax, rbx    ; rax = rbx" << endl;
    cout << "lea rax, [rbx]  ; rax = &rbx (取地址)" << endl;
    cout << "push rax        ; 压栈" << endl;
    cout << "pop rax         ; 出栈" << endl;
    
    cout << "\n【算术运算】" << endl;
    cout << "add rax, rbx    ; rax += rbx" << endl;
    cout << "sub rax, rbx    ; rax -= rbx" << endl;
    cout << "imul rax, rbx   ; rax *= rbx (有符号)" << endl;
    cout << "idiv rcx        ; rax / rcx, 商->rax, 余数->rdx" << endl;
    
    cout << "\n【逻辑运算】" << endl;
    cout << "and rax, rbx    ; rax &= rbx" << endl;
    cout << "or rax, rbx     ; rax |= rbx" << endl;
    cout << "xor rax, rbx    ; rax ^= rbx" << endl;
    cout << "not rax         ; rax = ~rax" << endl;
    
    cout << "\n【比较和跳转】" << endl;
    cout << "cmp rax, rbx    ; 比较 rax 和 rbx" << endl;
    cout << "test rax, rax   ; 检查 rax 是否为0" << endl;
    cout << "je addr         ; 相等则跳转" << endl;
    cout << "jne addr        ; 不等则跳转" << endl;
    cout << "jg addr         ; 大于则跳转" << endl;
    cout << "jl addr         ; 小于则跳转" << endl;
    cout << "jmp addr        ; 无条件跳转" << endl;
    
    cout << "\n【函数调用】" << endl;
    cout << "call addr       ; 调用函数" << endl;
    cout << "ret             ; 返回" << endl;
    
    cout << "\n【内存访问】" << endl;
    cout << "mov rax, [rbx]      ; rax = *rbx (读取)" << endl;
    cout << "mov [rbx], rax      ; *rbx = rax (写入)" << endl;
    cout << "mov rax, [rbx+8]    ; rax = *(rbx+8)" << endl;
    cout << "mov rax, [rbx+rcx*4]; 数组访问" << endl;
    
    cout << "\n【特殊指令】" << endl;
    cout << "nop             ; 空操作（常用于对齐）" << endl;
    cout << "int3            ; 软件断点" << endl;
    cout << "xchg rax, rbx   ; 交换寄存器" << endl;
    
    cout << "==========================================\n" << endl;
}

// ====================================
// 主程序
// ====================================

int main() {
    SetConsoleOutputCP(CP_UTF8);
    
    cout << "╔═══════════════════════════════════════════╗" << endl;
    cout << "║    UE逆向学习 - x64dbg 调试实战          ║" << endl;
    cout << "╚═══════════════════════════════════════════╝" << endl;
    
    PrintDebuggingTips();
    PrintAssemblyReference();
    
    cout << "\n选择调试场景：" << endl;
    cout << "1. 函数调用调试" << endl;
    cout << "2. 字符串调试" << endl;
    cout << "3. 循环调试" << endl;
    cout << "4. 指针调试" << endl;
    cout << "5. 异常调试" << endl;
    cout << "6. 虚函数表调试" << endl;
    cout << "0. 全部执行" << endl;
    
    int choice;
    cout << "\n输入选择: ";
    cin >> choice;
    
    switch (choice) {
        case 1: Scenario_FunctionCall(); break;
        case 2: Scenario_StringDebug(); break;
        case 3: Scenario_LoopDebug(); break;
        case 4: Scenario_PointerDebug(); break;
        case 5: Scenario_ExceptionDebug(); break;
        case 6: Scenario_VTableDebug(); break;
        case 0:
            Scenario_FunctionCall();
            Scenario_StringDebug();
            Scenario_LoopDebug();
            Scenario_PointerDebug();
            Scenario_ExceptionDebug();
            Scenario_VTableDebug();
            break;
    }
    
    cout << "\n程序结束！" << endl;
    system("pause");
    return 0;
}

/*
 * ═══════════════════════════════════════
 * 实战练习
 * ═══════════════════════════════════════
 * 
 * 1. 用 x64dbg 附加到程序
 * 2. 完成所有6个调试场景
 * 3. 熟悉所有快捷键
 * 4. 尝试修改程序行为
 * 5. 导出修改后的程序
 * 
 * ═══════════════════════════════════════
 * 进阶挑战
 * ═══════════════════════════════════════
 * 
 * 1. 找到 cout 的实现
 * 2. Hook cout 函数，修改输出
 * 3. 分析虚函数调用的完整流程
 * 4. 使用脚本自动化调试
 */
