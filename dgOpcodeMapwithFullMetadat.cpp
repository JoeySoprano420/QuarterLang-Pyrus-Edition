std::unordered_map<std::string, DodecagramOpcode> dgOpcodeMap = {
    // 🔹 Core Move and Memory
    {"dg_01", {0x8B, true, false, 0, true,  "dg_01"}}, : mov rax, [rbx]
    {"dg_02", {0x89, true, false, 0, true,  "dg_02"}}, : mov [rcx], rdx
    {"dg_03", {0x01, true, false, 0, true,  "dg_03"}}, : add rax, rbx
    {"dg_04", {0x29, true, false, 0, true,  "dg_04"}}, : sub rdx, rcx

    // 🔹 Control Flow
    {"dg_05", {0xEB, false, false, 1, false, "dg_05"}}, : jmp short
    {"dg_06", {0x39, true, false, 0, true,  "dg_06"}}, : cmp r/m, reg
    {"dg_07", {0x75, false, false, 1, false, "dg_07"}}, : jne short
    {"dg_08", {0xE8, false, false, 4, false, "dg_08"}}, : call rel32
    {"dg_09", {0xC3, false, false, 0, false, "dg_09"}}, : ret

    // 🔹 Stack Control
    {"dg_0A", {0x50, false, false, 0, true,  "dg_0A"}}, : push rax
    {"dg_0B", {0x58, false, false, 0, true,  "dg_0B"}}, : pop rax

    // 🔹 Extended Arithmetic / Logic
    {"dg_0C", {0x31, true, false, 0, true,  "dg_0C"}}, : xor r/m, reg
    {"dg_0D", {0x21, true, false, 0, true,  "dg_0D"}}, : and r/m, reg
    {"dg_0E", {0x09, true, false, 0, true,  "dg_0E"}}, : or r/m, reg
    {"dg_0F", {0xF7, true, false, 0, true,  "dg_0F"}}, : mul/div

    // 🔹 Immediate Ops
    {"dg_10", {0xB8, false, false, 4, true,  "dg_10"}}, : mov rax, imm
    {"dg_11", {0xC7, true, false, 4, true,  "dg_11"}}, : mov r/m32, imm32
    {"dg_12", {0x83, true, false, 1, true,  "dg_12"}}, : add/sub/cmp r/m, imm8

    // 🔹 Shifts and Rotates
    {"dg_13", {0xD1, true, false, 0, true,  "dg_13"}}, : shift by 1
    {"dg_14", {0xD3, true, false, 0, true,  "dg_14"}}, : shift by CL
    {"dg_15", {0xC1, true, false, 1, true,  "dg_15"}}, : shift by imm8

    // 🔹 Conditional Jumps
    {"dg_16", {0x74, false, false, 1, false, "dg_16"}}, : je
    {"dg_17", {0x7C, false, false, 1, false, "dg_17"}}, : jl
    {"dg_18", {0x7F, false, false, 1, false, "dg_18"}}, : jg
    {"dg_19", {0x7E, false, false, 1, false, "dg_19"}}, : jle
    {"dg_1A", {0x7D, false, false, 1, false, "dg_1A"}}, : jge

    // 🔹 Floating-point
    {"dg_1B", {0xD9, true, false, 0, false, "dg_1B"}}, : fld
    {"dg_1C", {0xDD, true, false, 0, false, "dg_1C"}}, : fstp

    // 🔹 System/Meta Instructions
    {"dg_1D", {0x0F, false, false, 0, false, "dg_1D"}}, : prefix
    {"dg_1E", {0xCC, false, false, 0, false, "dg_1E"}}, : int3
    {"dg_1F", {0x90, false, false, 0, false, "dg_1F"}}, : nop
};
