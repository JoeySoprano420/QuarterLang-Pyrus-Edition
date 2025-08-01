#include <unordered_map>
#include <string>

std::unordered_map<std::string, uint8_t> dgOpcodeMap = {
    // 🔹 Core Move and Memory
    {"dg_01", 0x8B}, // mov rax, [rbx]
    {"dg_02", 0x89}, // mov [rcx], rdx
    {"dg_03", 0x01}, // add rax, rbx
    {"dg_04", 0x29}, // sub rdx, rcx

    // 🔹 Control Flow
    {"dg_05", 0xEB}, // jmp short
    {"dg_06", 0x39}, // cmp r/m, reg
    {"dg_07", 0x75}, // jne short
    {"dg_08", 0xE8}, // call rel32
    {"dg_09", 0xC3}, // ret

    // 🔹 Stack Control
    {"dg_0A", 0x50}, // push rax
    {"dg_0B", 0x58}, // pop rax

    // 🔹 Extended Arithmetic / Logic
    {"dg_0C", 0x31}, // xor r/m, reg
    {"dg_0D", 0x21}, // and r/m, reg
    {"dg_0E", 0x09}, // or  r/m, reg
    {"dg_0F", 0xF7}, // mul/div (depends on ModR/M)
    
    // 🔹 Immediate Ops
    {"dg_10", 0xB8}, // mov rax, imm32/64
    {"dg_11", 0xC7}, // mov r/m32, imm32
    {"dg_12", 0x83}, // add/sub/cmp r/m, imm8 (with ModR/M)
    
    // 🔹 Shifts and Rotates
    {"dg_13", 0xD1}, // shl/shr r/m, 1
    {"dg_14", 0xD3}, // shl/shr r/m, cl
    {"dg_15", 0xC1}, // shl/shr r/m, imm8

    // 🔹 Conditional Jumps
    {"dg_16", 0x74}, // je short
    {"dg_17", 0x7C}, // jl short
    {"dg_18", 0x7F}, // jg short
    {"dg_19", 0x7E}, // jle short
    {"dg_1A", 0x7D}, // jge short

    // 🔹 Floating-point (placeholder opcodes; actual encoding varies)
    {"dg_1B", 0xD9}, // fld (float load)
    {"dg_1C", 0xDD}, // fstp (float store/pop)
    
    // 🔹 System/Meta Instructions
    {"dg_1D", 0x0F}, // prefix for extended opcodes (e.g., SSE)
    {"dg_1E", 0xCC}, // int3 (breakpoint)
    {"dg_1F", 0x90}, // nop
};
