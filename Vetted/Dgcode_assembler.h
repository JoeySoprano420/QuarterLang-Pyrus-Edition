
#ifndef DGCODE_ASSEMBLER_HPP
#define DGCODE_ASSEMBLER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

struct Instruction {
    std::string mnemonic;
    std::vector<std::string> operands;
    std::vector<uint8_t> encoded;
    size_t offset;
};

class DodecagramAssembler {
public:
    void loadDGCodeFile(const std::string& filename);
    void assemble();
    void saveQByte(const std::string& filename);
    void dumpSymbols(const std::string& filename);
    void launchDebuggerShell();

private:
    std::vector<Instruction> instructions;
    std::unordered_map<std::string, uint64_t> symbolTable;
    std::unordered_map<std::string, uint8_t> registerMap;
    std::unordered_map<std::string, uint8_t> opcodeMap;

    void initializeMaps();
    std::vector<uint8_t> encodeInstruction(const Instruction& inst);
    uint8_t encodeModRM(uint8_t reg, uint8_t rm);
    void resolveSymbols();
    uint32_t calculateRel32(size_t from, size_t to);
    void backpatch();
};

#endif
