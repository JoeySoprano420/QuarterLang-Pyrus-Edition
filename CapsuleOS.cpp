#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdint>

// === Virtual Machine Core ===

class VirtualMachine {
public:
    std::vector<uint8_t> memory;
    size_t ip = 0;  // Instruction pointer
    bool running = false;

    VirtualMachine(size_t memSize = 1024 * 1024) : memory(memSize, 0) {}

    void loadProgram(const std::vector<uint8_t>& program) {
        std::copy(program.begin(), program.end(), memory.begin());
        ip = 0;
    }

    void execute() {
        running = true;
        while (running && ip < memory.size()) {
            uint8_t opcode = memory[ip++];
            switch (opcode) {
                case 0x01: { // HALT
                    running = false;
                    break;
                }
                case 0x02: { // PRINT_BYTE
                    uint8_t val = memory[ip++];
                    std::cout << "[VM PRINT] " << (int)val << std::endl;
                    break;
                }
                default:
                    std::cerr << "[VM ERROR] Unknown opcode: 0x" << std::hex << (int)opcode << std::dec << std::endl;
                    running = false;
                    break;
            }
        }
    }
};

// === Capsule Definition ===

struct Capsule {
    std::string name;
    std::vector<std::string> sourceLines;
    std::vector<uint8_t> bytecode;

    Capsule(const std::string& name) : name(name) {}
};

// === Capsule System Management ===

class CapsuleSystem {
public:
    std::map<std::string, std::shared_ptr<Capsule>> capsules;

    void registerCapsule(const std::string& name, const std::shared_ptr<Capsule>& capsule) {
        capsules[name] = capsule;
    }

    std::shared_ptr<Capsule> getCapsule(const std::string& name) {
        if (capsules.count(name)) return capsules[name];
        return nullptr;
    }
};

// === Full Integrated Compiler ===

class CapsuleCompiler {
public:
    std::vector<uint8_t> compile(const std::vector<std::string>& lines) {
        std::vector<uint8_t> bytecode;

        for (const std::string& line : lines) {
            std::istringstream iss(line);
            std::string token;
            iss >> token;

            if (token == "HALT") {
                bytecode.push_back(0x01);
            } else if (token == "PRINT_BYTE") {
                int val;
                iss >> val;
                bytecode.push_back(0x02);
                bytecode.push_back(static_cast<uint8_t>(val));
            } else {
                std::cerr << "[Compiler ERROR] Unknown instruction: " << token << std::endl;
            }
        }

        return bytecode;
    }

    std::shared_ptr<Capsule> compileCapsuleFromFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "[Compiler ERROR] Cannot open file: " << filepath << std::endl;
            return nullptr;
        }

        auto capsule = std::make_shared<Capsule>(filepath);
        std::string line;
        while (std::getline(file, line)) {
            capsule->sourceLines.push_back(line);
        }

        capsule->bytecode = compile(capsule->sourceLines);
        return capsule;
    }
};

// === OS Boot Entry Point ===

int main(int argc, char** argv) {
    std::cout << "=== CapsuleOS v1.0 ===" << std::endl;

    if (argc < 2) {
        std::cerr << "[OS ERROR] No capsule file provided. Usage: CapsuleOS <capsule.qs>" << std::endl;
        return 1;
    }

    std::string capsuleFile = argv[1];

    CapsuleSystem capsuleSystem;
    CapsuleCompiler compiler;
    auto capsule = compiler.compileCapsuleFromFile(capsuleFile);

    if (!capsule) {
        std::cerr << "[OS ERROR] Capsule compilation failed." << std::endl;
        return 1;
    }

    capsuleSystem.registerCapsule(capsule->name, capsule);

    VirtualMachine vm;
    vm.loadProgram(capsule->bytecode);
    vm.execute();

    std::cout << "=== CapsuleOS Shutdown ===" << std::endl;
    return 0;
}
