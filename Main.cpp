#include "DodecagramAssembler.h"

int main() {
    DodecagramAssembler asmblr;

    asmblr.addLabel("start");
    asmblr.emit("dg_10", {"rax", "42"});       // mov rax, 42
    asmblr.emit("dg_01", {"rdx", "rax"});      // mov rdx, rax
    asmblr.emit("dg_08", {"end"});             // call end
    asmblr.emit("dg_09", {});                  // ret
    asmblr.addLabel("end");
    asmblr.emit("dg_09", {});                  // ret

    asmblr.backpatch();

    asmblr.writeQByte("program.qbyte");
    asmblr.dumpSymbolTable("program.symtab");
    asmblr.dumpNASMStyle();

    size_t pc = 0;
    while (pc < 64) {
        auto line = asmblr.disassembleAt(pc);
        if (line.empty()) break;
        std::cout << "[disasm] " << line << "\n";
    }

    return 0;
}
