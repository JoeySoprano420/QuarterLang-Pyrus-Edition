#include "DodecagramAssembler.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>

DodecagramAssembler::DodecagramAssembler() {
    initOpcodeMap();
    initRegisterMap();
    initReverseMap();
}

void DodecagramAssembler::initOpcodeMap() {
    opcodeMap = {
        {"dg_01", 0x8B}, {"dg_02", 0x89}, {"dg_03", 0x01},
        {"dg_04", 0x29}, {"dg_05", 0xEB}, {"dg_06", 0x39},
        {"dg_07", 0x75}, {"dg_08", 0xE8}, {"dg_09", 0xC3},
        {"dg_0A", 0x50}, {"dg_0B", 0x58}, {"dg_0C", 0x31},
        {"dg_0D", 0x21}, {"dg_0E", 0x09}, {"dg_0F", 0xF7},
        {"dg_10", 0xB8}, {"dg_11", 0xC7}, {"dg_12", 0x83},
        {"dg_13", 0xD1}, {"dg_14", 0xD3}, {"dg_15", 0xC1},
        {"dg_16", 0x74}, {"dg_17", 0x7C}, {"dg_18", 0x7F},
        {"dg_19", 0x7E}, {"dg_1A", 0x7D}, {"dg_1B", 0xD9},
        {"dg_1C", 0xDD}, {"dg_1D", 0x0F}, {"dg_1E", 0xCC},
        {"dg_1F", 0x90}
    };
}

void DodecagramAssembler::initRegisterMap() {
    registerMap = {
        {"rax", 0x00}, {"rcx", 0x01}, {"rdx", 0x02},
        {"rbx", 0x03}, {"rsp", 0x04}, {"rbp", 0x05},
        {"rsi", 0x06}, {"rdi", 0x07}
    };
}

void DodecagramAssembler::initReverseMap() {
    for (const auto& pair : opcodeMap)
        opcodeToMnemonic[pair.second] = pair.first;
}

uint8_t DodecagramAssembler::encodeModRM(uint8_t reg, uint8_t rm) {
    return 0xC0 | ((reg & 0x07) << 3) | (rm & 0x07);
}

void DodecagramAssembler::addLabel(const std::string& label) {
    labelPositions[label] = binary.size();
}

void DodecagramAssembler::emit(const std::string& mnemonic, const std::vector<std::string>& operands) {
    Instruction inst{mnemonic, operands, {}};
    instructions.push_back(inst);
    encodeInstruction(inst);
}

void DodecagramAssembler::encodeInstruction(const Instruction& inst) {
    uint8_t opcode = opcodeMap[inst.mnemonic];
    binary.push_back(opcode);

    if (inst.operands.size() == 2) {
        uint8_t reg = registerMap[inst.operands[0]];
        uint8_t rm = registerMap[inst.operands[1]];
        binary.push_back(encodeModRM(reg, rm));
    }
    else if (inst.operands.size() == 1 && labelPositions.count(inst.operands[0]) == 0) {
        pendingPatches[binary.size()] = inst.operands[0];
        binary.insert(binary.end(), {0, 0, 0, 0});
    }
    else if (inst.mnemonic == "dg_10") {  // mov rax, imm32
        uint8_t reg = registerMap[inst.operands[0]];
        binary.back() += reg;  // B8+r
        int32_t imm = std::stoi(inst.operands[1]);
        for (int i = 0; i < 4; ++i)
            binary.push_back((imm >> (i * 8)) & 0xFF);
    }
}

void DodecagramAssembler::backpatch() {
    for (const auto& [pos, label] : pendingPatches) {
        int32_t offset = calculateRelOffset(pos, labelPositions[label]);
        for (int i = 0; i < 4; ++i)
            binary[pos + i] = (offset >> (i * 8)) & 0xFF;
    }
}

int32_t DodecagramAssembler::calculateRelOffset(size_t srcPos, size_t dstPos) {
    return static_cast<int32_t>(dstPos - (srcPos + 4));
}

void DodecagramAssembler::writeQByte(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    out.write(reinterpret_cast<const char*>(binary.data()), binary.size());
    out.close();
}

void DodecagramAssembler::dumpNASMStyle() {
    for (size_t i = 0; i < binary.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)binary[i] << " ";
        if ((i + 1) % 8 == 0) std::cout << "\n";
    }
    std::cout << "\n";
}

void DodecagramAssembler::dumpSymbolTable(const std::string& filename) {
    std::ofstream out(filename);
    for (const auto& [label, pos] : labelPositions) {
        out << label << ": 0x" << std::hex << pos << "\n";
    }
    out.close();
}

std::string DodecagramAssembler::disassembleAt(size_t& pc) {
    if (pc >= binary.size()) return "";
    uint8_t opcode = binary[pc];
    std::string mnemonic = opcodeToMnemonic.count(opcode) ? opcodeToMnemonic[opcode] : "unknown";
    std::string out = mnemonic;

    if (opcode == 0x8B || opcode == 0x89 || opcode == 0x01 || opcode == 0x29) {
        uint8_t modrm = binary[pc + 1];
        uint8_t reg = (modrm >> 3) & 7;
        uint8_t rm = modrm & 7;
        out += " r" + std::to_string(reg) + ", r" + std::to_string(rm);
        pc += 2;
    } else {
        pc += 1;
    }

    return out;
}

void DodecagramAssembler::writeELFStub(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    out.write("\x7F""ELF...\0", 8);  // Placeholder ELF header
    out.write(reinterpret_cast<const char*>(binary.data()), binary.size());
    out.close();
}

void DodecagramAssembler::writePEStub(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    out.write("MZ...\0", 4);  // Placeholder PE header
    out.write(reinterpret_cast<const char*>(binary.data()), binary.size());
    out.close();
}
