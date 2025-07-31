#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <stdexcept>

struct EncodedInstruction {
    std::string mnemonic;
    uint8_t opcode;
    std::vector<uint8_t> bytes;
    size_t length;
    std::string operandType; // e.g. "reg,r/m", "imm32", "rel8"
};

// 🔹 Register binary encoding map (3-bit)
std::unordered_map<std::string, uint8_t> regMap = {
    {"rax", 0b000}, {"rcx", 0b001}, {"rdx", 0b010}, {"rbx", 0b011},
    {"rsp", 0b100}, {"rbp", 0b101}, {"rsi", 0b110}, {"rdi", 0b111}
};

// 🔹 Dodecagram opcode map with metadata
std::unordered_map<std::string, EncodedInstruction> dgOpcodeMap = {
    {"dg_01", {"dg_01", 0x8B, {}, 2, "reg,r/m"}}, // mov r, [m]
    {"dg_02", {"dg_02", 0x89, {}, 2, "r/m,reg"}}, // mov [m], r
    {"dg_03", {"dg_03", 0x01, {}, 2, "r/m,reg"}}, // add
    {"dg_04", {"dg_04", 0x29, {}, 2, "r/m,reg"}}, // sub
    {"dg_05", {"dg_05", 0xEB, {}, 2, "rel8"}},    // jmp short
    {"dg_06", {"dg_06", 0x39, {}, 2, "r/m,reg"}}, // cmp
    {"dg_07", {"dg_07", 0x75, {}, 2, "rel8"}},    // jne short
    {"dg_08", {"dg_08", 0xE8, {}, 5, "rel32"}},   // call
    {"dg_09", {"dg_09", 0xC3, {}, 1, ""}},        // ret
    {"dg_0A", {"dg_0A", 0x50, {}, 1, "reg"}},     // push
    {"dg_0B", {"dg_0B", 0x58, {}, 1, "reg"}},     // pop
    {"dg_10", {"dg_10", 0xB8, {}, 5, "reg,imm32"}}, // mov r, imm32
    {"dg_11", {"dg_11", 0xC7, {}, 6, "r/m,imm32"}}, // mov [r/m], imm32
    {"dg_12", {"dg_12", 0x83, {}, 3, "r/m,imm8"}},  // add/cmp/sub r/m, imm8
    {"dg_16", {"dg_16", 0x74, {}, 2, "rel8"}},      // je short
    {"dg_1F", {"dg_1F", 0x90, {}, 1, ""}},          // nop
};

// 🔁 Reverse opcode → mnemonic
std::unordered_map<uint8_t, std::string> opcodeToMnemonic;

// 🔧 Encode ModR/M byte: Mod (2) | Reg (3) | R/M (3)
uint8_t encodeModRM(uint8_t mod, uint8_t reg, uint8_t rm) {
    return (mod << 6) | ((reg & 0b111) << 3) | (rm & 0b111);
}

// 🔁 Relative offset calculator for jmp/call/je (rel8 or rel32)
std::vector<uint8_t> encodeRelOffset(int32_t currentAddr, int32_t targetAddr, bool isShort = true) {
    int32_t offset = targetAddr - (currentAddr + (isShort ? 2 : 5));
    std::vector<uint8_t> result;
    if (isShort) {
        result.push_back(static_cast<uint8_t>(offset & 0xFF));
    } else {
        for (int i = 0; i < 4; ++i)
            result.push_back((offset >> (8 * i)) & 0xFF);
    }
    return result;
}

// 📄 Write out binary .qbyte + symbol table
void writeQByte(const std::vector<uint8_t>& code, const std::string& path) {
    std::ofstream out(path, std::ios::binary);
    if (!out) throw std::runtime_error("Failed to open .qbyte output.");
    for (uint8_t byte : code) out.put(static_cast<char>(byte));
    out.close();
}

// 🔨 Assemble instruction from mnemonic + operands
std::vector<uint8_t> assemble(const std::string& mnemonic, const std::vector<std::string>& operands, int32_t pc = 0) {
    auto it = dgOpcodeMap.find(mnemonic);
    if (it == dgOpcodeMap.end()) throw std::runtime_error("Unknown mnemonic: " + mnemonic);

    const auto& inst = it->second;
    std::vector<uint8_t> bytes = {inst.opcode};

    // Reg-reg or reg-mem
    if (inst.operandType == "reg,r/m" || inst.operandType == "r/m,reg") {
        uint8_t reg = regMap[operands[0]];
        uint8_t rm  = regMap[operands[1]];
        bytes.push_back(encodeModRM(0b11, reg, rm));
    }

    // reg, imm32
    else if (inst.operandType == "reg,imm32") {
        uint8_t reg = regMap[operands[0]];
        bytes[0] += reg; // Adjust opcode B8+r
        int32_t imm = std::stoi(operands[1]);
        for (int i = 0; i < 4; ++i)
            bytes.push_back((imm >> (8 * i)) & 0xFF);
    }

    // r/m, imm8 or imm32
    else if (inst.operandType == "r/m,imm32" || inst.operandType == "r/m,imm8") {
        uint8_t rm = regMap[operands[0]];
        bytes.push_back(encodeModRM(0b11, 0, rm)); // /0 = opcode extension
        int32_t imm = std::stoi(operands[1]);
        int len = inst.operandType == "r/m,imm32" ? 4 : 1;
        for (int i = 0; i < len; ++i)
            bytes.push_back((imm >> (8 * i)) & 0xFF);
    }

    // rel8 / rel32 jump
    else if (inst.operandType == "rel8" || inst.operandType == "rel32") {
        int32_t target = std::stoi(operands[0]); // Assume label addr resolved already
        auto offsetBytes = encodeRelOffset(pc, target, inst.operandType == "rel8");
        bytes.insert(bytes.end(), offsetBytes.begin(), offsetBytes.end());
    }

    return bytes;
}

// 🔍 Disassemble a byte stream back to readable format
std::string disassemble(const std::vector<uint8_t>& code, size_t& pc) {
    uint8_t opcode = code[pc];
    std::string mnemonic = opcodeToMnemonic.count(opcode) ? opcodeToMnemonic[opcode] : "unknown";
    std::string output = mnemonic;

    if (opcode == 0x8B || opcode == 0x89 || opcode == 0x01 || opcode == 0x29) {
        uint8_t modrm = code[pc + 1];
        uint8_t reg = (modrm >> 3) & 0b111;
        uint8_t rm  = modrm & 0b111;
        output += " " + std::to_string(reg) + ", " + std::to_string(rm);
        pc += 2;
    } else if (opcode == 0xC3 || opcode == 0x90) {
        pc += 1;
    } else {
        output += " ...";
        pc += 1;
    }

    return output;
}

// 🔁 Populate reverse lookup
void initReverseMap() {
    for (const auto& pair : dgOpcodeMap) {
        opcodeToMnemonic[pair.second.opcode] = pair.first;
    }
}
