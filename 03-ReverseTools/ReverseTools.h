/*
 * ========================================
 * UE游戏逆向学习 - 第三课：实战工具类
 * ========================================
 * 
 * 这个文件包含实际逆向中常用的工具函数
 */

#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <string>
#include <vector>
#include <iostream>
#include <cwchar>

// ====================================
// 第一部分：进程操作工具
// ====================================

class ProcessUtils {
public:
    
    // 根据进程名获取进程ID
    static DWORD GetProcessIdByName(const wchar_t* processName) {
        DWORD pid = 0;
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        
        if (snapshot != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32W pe;
            pe.dwSize = sizeof(pe);
            
            if (Process32FirstW(snapshot, &pe)) {
                do {
                    if (_wcsicmp(pe.szExeFile, processName) == 0) {
                        pid = pe.th32ProcessID;
                        break;
                    }
                } while (Process32NextW(snapshot, &pe));
            }
            CloseHandle(snapshot);
        }
        
        return pid;
    }
    
    // 获取模块基址
    static uintptr_t GetModuleBase(DWORD pid, const wchar_t* moduleName) {
        uintptr_t moduleBase = 0;
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        
        if (snapshot != INVALID_HANDLE_VALUE) {
            MODULEENTRY32W me;
            me.dwSize = sizeof(me);
            
            if (Module32FirstW(snapshot, &me)) {
                do {
                    if (_wcsicmp(me.szModule, moduleName) == 0) {
                        moduleBase = (uintptr_t)me.modBaseAddr;
                        break;
                    }
                } while (Module32NextW(snapshot, &me));
            }
            CloseHandle(snapshot);
        }
        
        return moduleBase;
    }
};

// ====================================
// 第二部分：内存操作工具
// ====================================

class MemoryUtils {
public:
    
    // 读取内存（外部进程）
    template<typename T>
    static T ReadMemory(HANDLE process, uintptr_t address) {
        T value = {};
        SIZE_T bytesRead;
        ReadProcessMemory(process, (LPCVOID)address, &value, sizeof(T), &bytesRead);
        return value;
    }
    
    // 写入内存（外部进程）
    template<typename T>
    static bool WriteMemory(HANDLE process, uintptr_t address, T value) {
        SIZE_T bytesWritten;
        return ReadProcessMemory(process, (LPVOID)address, &value, sizeof(T), &bytesWritten);
    }
    
    // 读取内存（当前进程）
    template<typename T>
    static T Read(uintptr_t address) {
        return *(T*)address;
    }
    
    // 写入内存（当前进程）
    template<typename T>
    static void Write(uintptr_t address, T value) {
        DWORD oldProtect;
        VirtualProtect((LPVOID)address, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect);
        *(T*)address = value;
        VirtualProtect((LPVOID)address, sizeof(T), oldProtect, &oldProtect);
    }
    
    // 读取字符串
    static std::string ReadString(uintptr_t address, size_t maxLength = 256) {
        char buffer[256] = {0};
        size_t length = min(maxLength, 255);
        
        for (size_t i = 0; i < length; i++) {
            buffer[i] = *(char*)(address + i);
            if (buffer[i] == 0) break;
        }
        
        return std::string(buffer);
    }
};

// ====================================
// 第三部分：特征码扫描工具
// ====================================

class PatternScanner {
public:
    
    // 将特征码字符串转为字节数组
    // 例: "48 8B 05 ?? ?? ?? ??" -> {0x48, 0x8B, 0x05, -1, -1, -1, -1}
    static std::vector<int> ParsePattern(const char* pattern) {
        std::vector<int> bytes;
        const char* current = pattern;
        
        while (*current) {
            if (*current == '?') {
                bytes.push_back(-1);  // 通配符
                current++;
                if (*current == '?') current++;  // 跳过第二个?
            }
            else if (*current == ' ') {
                current++;
            }
            else {
                char byteStr[3] = {current[0], current[1], 0};
                bytes.push_back(strtol(byteStr, nullptr, 16));
                current += 2;
            }
        }
        
        return bytes;
    }
    
    // 在内存中搜索特征码
    static uintptr_t ScanPattern(uintptr_t start, size_t size, const char* pattern) {
        std::vector<int> patternBytes = ParsePattern(pattern);
        
        for (size_t i = 0; i <= size - patternBytes.size(); i++) {
            bool found = true;
            
            for (size_t j = 0; j < patternBytes.size(); j++) {
                if (patternBytes[j] == -1) continue;  // 跳过通配符
                
                unsigned char byte = *(unsigned char*)(start + i + j);
                if (byte != patternBytes[j]) {
                    found = false;
                    break;
                }
            }
            
            if (found) {
                return start + i;
            }
        }
        
        return 0;
    }
    
    // 扫描整个模块
    static uintptr_t ScanModule(const wchar_t* moduleName, const char* pattern) {
        HMODULE module = GetModuleHandleW(moduleName);
        if (!module) return 0;
        
        MODULEINFO moduleInfo;
        if (!GetModuleInformation(GetCurrentProcess(), module, &moduleInfo, sizeof(moduleInfo))) {
            return 0;
        }
        
        return ScanPattern((uintptr_t)module, moduleInfo.SizeOfImage, pattern);
    }
};

// ====================================
// 第四部分：偏移计算工具
// ====================================

class OffsetUtils {
public:
    
    // 解析相对地址（RIP相对寻址）
    // 例如: mov rax, [rip+offset]
    // 指令地址 + 指令长度 + offset = 目标地址
    static uintptr_t GetAbsoluteAddress(uintptr_t instructionAddr, int instructionLength, int offsetPos) {
        // 读取偏移值（通常是int32）
        int32_t offset = MemoryUtils::Read<int32_t>(instructionAddr + offsetPos);
        
        // 计算绝对地址
        return instructionAddr + instructionLength + offset;
    }
    
    // 示例：解析 "48 8B 05 ?? ?? ?? ??" (mov rax, [rip+offset])
    static uintptr_t ParseMovInstruction(uintptr_t instructionAddr) {
        return GetAbsoluteAddress(instructionAddr, 7, 3);
    }
};

// ====================================
// 第五部分：实战示例
// ====================================

namespace ReverseExamples {
    
    // 示例1：查找GEngine
    void Example_FindGEngine() {
        std::cout << "\n=== 示例：查找GEngine ===" << std::endl;
        
        // 步骤1：搜索特征码
        // 通常GEngine的引用会在特定函数中
        // 例如某个初始化函数会 mov rax, [GEngine]
        
        const char* pattern = "48 8B 05 ?? ?? ?? ?? 48 85 C0 74";  // 示例特征码
        uintptr_t found = PatternScanner::ScanModule(nullptr, pattern);
        
        if (found) {
            std::cout << "找到特征码地址: 0x" << std::hex << found << std::dec << std::endl;
            
            // 步骤2：解析相对地址
            uintptr_t gEngineAddr = OffsetUtils::ParseMovInstruction(found);
            std::cout << "GEngine地址: 0x" << std::hex << gEngineAddr << std::dec << std::endl;
            
            // 步骤3：读取GEngine指针
            uintptr_t gEngine = MemoryUtils::Read<uintptr_t>(gEngineAddr);
            std::cout << "GEngine实例: 0x" << std::hex << gEngine << std::dec << std::endl;
        }
        else {
            std::cout << "未找到特征码" << std::endl;
        }
    }
    
    // 示例2：遍历PlayerArray
    void Example_EnumeratePlayers() {
        std::cout << "\n=== 示例：遍历玩家列表 ===" << std::endl;
        
        /*
         * 访问路径：
         * GEngine -> GameViewport -> World -> GameState -> PlayerArray
         * 
         * 每一步都需要：
         * 1. 找到偏移（通过IDA/Ghidra分析）
         * 2. 读取指针
         * 3. 继续下一步
         */
        
        // 假设已知偏移
        const int OFFSET_GameViewport = 0x78;
        const int OFFSET_World = 0x80;
        const int OFFSET_GameState = 0x150;
        const int OFFSET_PlayerArray = 0x2A8;
        
        // 假设已找到GEngine
        uintptr_t gEngine = 0x12345678;  // 示例地址
        
        std::cout << "访问路径演示：" << std::endl;
        std::cout << "1. GEngine = 0x" << std::hex << gEngine << std::dec << std::endl;
        
        // 注意：这里只是演示，实际使用时取消注释
        /*
        uintptr_t gameViewport = MemoryUtils::Read<uintptr_t>(gEngine + OFFSET_GameViewport);
        std::cout << "2. GameViewport = 0x" << std::hex << gameViewport << std::dec << std::endl;
        
        uintptr_t world = MemoryUtils::Read<uintptr_t>(gameViewport + OFFSET_World);
        std::cout << "3. World = 0x" << std::hex << world << std::dec << std::endl;
        
        uintptr_t gameState = MemoryUtils::Read<uintptr_t>(world + OFFSET_GameState);
        std::cout << "4. GameState = 0x" << std::hex << gameState << std::dec << std::endl;
        
        // 读取TArray结构
        struct {
            uintptr_t Data;
            int Count;
            int Max;
        } playerArray;
        
        playerArray.Data = MemoryUtils::Read<uintptr_t>(gameState + OFFSET_PlayerArray);
        playerArray.Count = MemoryUtils::Read<int>(gameState + OFFSET_PlayerArray + 8);
        
        std::cout << "5. PlayerArray.Count = " << playerArray.Count << std::endl;
        
        for (int i = 0; i < playerArray.Count; i++) {
            uintptr_t player = MemoryUtils::Read<uintptr_t>(playerArray.Data + i * 8);
            std::cout << "   Player[" << i << "] = 0x" << std::hex << player << std::dec << std::endl;
        }
        */
    }
    
    // 示例3：调用游戏函数（ProcessEvent）
    void Example_CallGameFunction() {
        std::cout << "\n=== 示例：调用游戏函数 ===" << std::endl;
        
        /*
         * ProcessEvent 是UE中调用蓝图函数的核心
         * 
         * 函数签名：
         * void ProcessEvent(UObject* Object, UFunction* Function, void* Params)
         * 
         * 步骤：
         * 1. 找到ProcessEvent地址（通常在UObject的VTable中）
         * 2. 找到要调用的函数对象（通过StaticFindObject）
         * 3. 准备参数结构
         * 4. 调用
         */
        
        std::cout << "调用游戏函数的步骤：" << std::endl;
        std::cout << "1. 找到ProcessEvent地址" << std::endl;
        std::cout << "   - 通过分析UObject的VTable" << std::endl;
        std::cout << "   - 通常在 VTable[68] 或附近" << std::endl;
        
        std::cout << "\n2. 使用StaticFindObject查找函数" << std::endl;
        std::cout << "   - 例: FindObject(\"Function EnginePackage.ClassName.FunctionName\")" << std::endl;
        
        std::cout << "\n3. 准备参数结构体" << std::endl;
        std::cout << "   struct Params {" << std::endl;
        std::cout << "       int Param1;" << std::endl;
        std::cout << "       float Param2;" << std::endl;
        std::cout << "       void* ReturnValue;" << std::endl;
        std::cout << "   };" << std::endl;
        
        std::cout << "\n4. 调用ProcessEvent" << std::endl;
        std::cout << "   ProcessEvent(objectPtr, functionPtr, &params);" << std::endl;
    }
}
