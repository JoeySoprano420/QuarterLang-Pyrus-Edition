#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

struct Instruction {
    std::string mnemonic;
    std::vector<std::string> operands;
    std::vector<uint8_t> encoded;
};

class DodecagramAssembler {
public:
    DodecagramAssembler();

    void emit(const std::string& mnemonic, const std::vector<std::string>& operands);
    void addLabel(const std::string& label);
    void backpatch();
    void writeQByte(const std::string& filename);
    void dumpNASMStyle();
    void dumpSymbolTable(const std::string& filename);
    std::string disassembleAt(size_t& pc);
    void writeELFStub(const std::string& filename);
    void writePEStub(const std::string& filename);

private:
    std::vector<Instruction> instructions;
    std::vector<uint8_t> binary;
    std::unordered_map<std::string, size_t> labelPositions;
    std::unordered_map<size_t, std::string> pendingPatches;

    std::unordered_map<std::string, uint8_t> opcodeMap;
    std::unordered_map<std::string, uint8_t> registerMap;
    std::unordered_map<uint8_t, std::string> opcodeToMnemonic;

    void initOpcodeMap();
    void initRegisterMap();
    void encodeInstruction(const Instruction& inst);
    uint8_t encodeModRM(uint8_t reg, uint8_t rm);
    int32_t calculateRelOffset(size_t srcPos, size_t dstPos);
    void initReverseMap();
};
