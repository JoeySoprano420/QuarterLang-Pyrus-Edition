#include <unordered_map>
#include <string>
std::unordered_map<std::string, uint8_t> dgOpcodeMap = {
    {"dg_01", 0x8B}, // mov rax, [rbx]
    {"dg_02", 0x89}, // mov [rcx], rdx
    {"dg_03", 0x01}, // add rax, rbx
    {"dg_04", 0x29}, // sub rdx, rcx
    {"dg_05", 0xEB}, // jmp
    {"dg_06", 0x39}, // cmp
    {"dg_07", 0x75}, // jne
    {"dg_08", 0xE8}, // call
    {"dg_09", 0xC3}, // ret
    {"dg_0A", 0x50}, // push rax
    {"dg_0B", 0x58}, // pop rax
};
