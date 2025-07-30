// QuarterLang (Pyrus Edition) Compiler Bootstrap
// Now includes: VM Loader, Capsule Import, ASCII AST, REPL Scrollback, Performance Log

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <memory>
#include <cmath>
#include <iomanip>
#include <deque>
#include <ctime>

//--------------------------------------------
// ENUMS + TOKEN + AST DEFINITIONS
//--------------------------------------------

enum class TokenType {
    IDENTIFIER, NUMBER, KEYWORD, COLON, NEWLINE, INDENT, DEDENT, END, UNKNOWN
};

struct Token {
    TokenType type;
    std::string text;
};

//--------------------------------------------
// AST NODES
//--------------------------------------------

struct ASTNode { virtual ~ASTNode() = default; };

struct IdentifierNode : ASTNode {
    std::string name;
    IdentifierNode(const std::string& n) : name(n) {}
};

struct NumberNode : ASTNode {
    std::string value;
    NumberNode(const std::string& v) : value(v) {}
};

struct SayNode : ASTNode {
    std::shared_ptr<ASTNode> message;
    SayNode(const std::shared_ptr<ASTNode>& msg) : message(msg) {}
};

struct ValNode : ASTNode {
    std::string name;
    std::shared_ptr<ASTNode> value;
    ValNode(const std::string& n, const std::shared_ptr<ASTNode>& v) : name(n), value(v) {}
};

struct LoopNode : ASTNode {
    int start, end;
    std::vector<std::shared_ptr<ASTNode>> body;
    LoopNode(int s, int e, const std::vector<std::shared_ptr<ASTNode>>& b) : start(s), end(e), body(b) {}
};

struct ProgramNode : ASTNode {
    std::vector<std::shared_ptr<ASTNode>> statements;
    ProgramNode(const std::vector<std::shared_ptr<ASTNode>>& s) : statements(s) {}
};

//--------------------------------------------
// DODECAGRAM MATH
//--------------------------------------------

class Dodecagram {
public:
    static int toDecimal(const std::string& dodecStr) {
        int value = 0;
        for (char ch : dodecStr) {
            value *= 12;
            if (std::isdigit(ch)) value += ch - '0';
            else if (ch >= 'A' && ch <= 'B') value += 10 + (ch - 'A');
        }
        return value;
    }

    static std::string fromDecimal(int decimal) {
        if (decimal == 0) return "0";
        std::string result;
        while (decimal > 0) {
            int digit = decimal % 12;
            result = (digit < 10 ? char('0' + digit) : char('A' + digit - 10)) + result;
            decimal /= 12;
        }
        return result;
    }
};

//--------------------------------------------
// LEXER
//--------------------------------------------

class Lexer {
    std::string src;
    std::unordered_map<std::string, TokenType> keywords = {
        {"star", TokenType::KEYWORD},
        {"end", TokenType::KEYWORD},
        {"say", TokenType::KEYWORD},
        {"val", TokenType::KEYWORD},
        {"as", TokenType::KEYWORD},
        {"int", TokenType::KEYWORD},
        {"loop", TokenType::KEYWORD},
        {"from", TokenType::KEYWORD},
        {"to", TokenType::KEYWORD}
    };

public:
    Lexer(const std::string& s) : src(s) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        std::istringstream stream(src);
        std::string word;
        while (stream >> word) {
            if (keywords.count(word)) tokens.push_back({ TokenType::KEYWORD, word });
            else if (word == ":") tokens.push_back({ TokenType::COLON, word });
            else if (std::isdigit(word[0]) || (word[0] >= 'A' && word[0] <= 'B'))
                tokens.push_back({ TokenType::NUMBER, word });
            else tokens.push_back({ TokenType::IDENTIFIER, word });
        }
        return tokens;
    }
};

//--------------------------------------------
// PARSER
//--------------------------------------------

class Parser {
    std::vector<Token> tokens;
    size_t pos = 0;

public:
    Parser(const std::vector<Token>& t) : tokens(t) {}

    std::shared_ptr<ProgramNode> parse() {
        std::vector<std::shared_ptr<ASTNode>> stmts;
        while (pos < tokens.size()) {
            if (match("say")) {
                auto id = std::make_shared<IdentifierNode>(advance().text);
                stmts.push_back(std::make_shared<SayNode>(id));
            }
            else if (match("val")) {
                auto name = advance().text;
                advance(); // 'as'
                advance(); // 'int'
                advance(); // ':'
                auto value = std::make_shared<NumberNode>(advance().text);
                stmts.push_back(std::make_shared<ValNode>(name, value));
            }
            else if (match("loop")) {
                advance(); // loop
                advance(); // from
                int start = std::stoi(advance().text);
                advance(); // to
                int end = std::stoi(advance().text);
                advance(); // :
                std::vector<std::shared_ptr<ASTNode>> body;
                if (match("say")) {
                    auto id = std::make_shared<IdentifierNode>(advance().text);
                    body.push_back(std::make_shared<SayNode>(id));
                }
                stmts.push_back(std::make_shared<LoopNode>(start, end, body));
            }
            else {
                advance();
            }
        }
        return std::make_shared<ProgramNode>(stmts);
    }

private:
    bool match(const std::string& kw) {
        return pos < tokens.size() && tokens[pos].text == kw;
    }

    Token advance() {
        return tokens[pos++];
    }
};



//--------------------------------------------
// INTERPRETER + IR + CAPSULE
//--------------------------------------------

std::unordered_map<std::string, int> runtimeVars;

int evaluateDG(const std::shared_ptr<ASTNode>& node) {
    if (auto num = std::dynamic_pointer_cast<NumberNode>(node)) {
        return Dodecagram::toDecimal(num->value);
    }
    return 0;
}

struct Bytecode {
    std::vector<std::string> instructions;
    void emit(const std::string& inst) { instructions.push_back(inst); }
    void writeToCapsule(const std::string& file) {
        std::ofstream out(file);
        out << "[QuarterCapsule]\n";
        for (const auto& line : instructions) out << line << "\n";
    }
};

class NeuralTuner {
public:
    void observe(const std::string& op) {
        std::cout << "[TUNER] observing: " << op << std::endl;
    }
    void reinforce() {
        std::cout << "[TUNER] reinforcing affinity.\n";
    }
};

void execute(const std::shared_ptr<ProgramNode>& prog, Bytecode& bc, NeuralTuner& tuner) {
    for (const auto& stmt : prog->statements) {
        if (auto say = std::dynamic_pointer_cast<SayNode>(stmt)) {
            auto id = std::dynamic_pointer_cast<IdentifierNode>(say->message);
            if (runtimeVars.count(id->name)) {
                std::cout << "=> " << runtimeVars[id->name] << std::endl;
            }
            else {
                std::cout << "=> " << id->name << std::endl;
            }
            bc.emit("SAY " + id->name);
            tuner.observe("SAY");
        }
        else if (auto val = std::dynamic_pointer_cast<ValNode>(stmt)) {
            int value = evaluateDG(val->value);
            runtimeVars[val->name] = value;
            bc.emit("VAL " + val->name + " = " + std::to_string(value));
            tuner.observe("VAL");
        }
        else if (auto loop = std::dynamic_pointer_cast<LoopNode>(stmt)) {
            for (int i = loop->start; i <= loop->end; ++i) {
                for (const auto& body : loop->body) {
                    if (auto say = std::dynamic_pointer_cast<SayNode>(body)) {
                        auto id = std::dynamic_pointer_cast<IdentifierNode>(say->message);
                        std::cout << "=> " << id->name << std::endl;
                        bc.emit("SAY " + id->name);
                        tuner.observe("SAY");
                    }
                }
            }
            bc.emit("LOOP " + std::to_string(loop->start) + " TO " + std::to_string(loop->end));
            tuner.observe("LOOP");
        }
    }
}

//--------------------------------------------
// VM Loader for Capsules + IR Execution
//--------------------------------------------

void runCapsuleFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "[ERROR] Cannot open capsule: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '[') continue;
        std::istringstream iss(line);
        std::string opcode;
        iss >> opcode;

        if (opcode == "SAY") {
            std::string id;
            iss >> id;
            if (runtimeVars.count(id)) {
                std::cout << ">> " << runtimeVars[id] << std::endl;
            }
            else {
                std::cout << ">> " << id << std::endl;
            }
        }
        else if (opcode == "VAL") {
            std::string var;
            std::string eq;
            int val;
            iss >> var >> eq >> val;
            runtimeVars[var] = val;
        }
        else if (opcode == "LOOP") {
            int start, to, end;
            std::string temp;
            iss >> start >> temp >> end;
            for (int i = start; i <= end; ++i) {
                std::cout << "[LOOP] iteration: " << i << std::endl;
            }
        }
    }
}

//--------------------------------------------
// AST Inspector (ASCII Visualization)
//--------------------------------------------

void printAST(const std::shared_ptr<ASTNode>& node, int indent = 0) {
    std::string prefix(indent, ' ');
    if (auto prog = std::dynamic_pointer_cast<ProgramNode>(node)) {
        std::cout << prefix << "[Program]\n";
        for (const auto& stmt : prog->statements)
            printAST(stmt, indent + 2);
    }
    else if (auto say = std::dynamic_pointer_cast<SayNode>(node)) {
        std::cout << prefix << "[Say] ";
        printAST(say->message, 0);
    }
    else if (auto id = std::dynamic_pointer_cast<IdentifierNode>(node)) {
        std::cout << prefix << "Identifier: " << id->name << "\n";
    }
    else if (auto val = std::dynamic_pointer_cast<ValNode>(node)) {
        std::cout << prefix << "[Val] " << val->name << " = ";
        printAST(val->value, 0);
    }
    else if (auto num = std::dynamic_pointer_cast<NumberNode>(node)) {
        std::cout << prefix << "Number: " << num->value << "\n";
    }
    else if (auto loop = std::dynamic_pointer_cast<LoopNode>(node)) {
        std::cout << prefix << "[Loop from " << loop->start << " to " << loop->end << "]\n";
        for (const auto& stmt : loop->body)
            printAST(stmt, indent + 2);
    }
}

//--------------------------------------------
// Scrollback Log + Enhanced REPL
//--------------------------------------------

std::deque<std::string> scrollback;
void logInput(const std::string& line) {
    if (scrollback.size() >= 20) scrollback.pop_front();
    scrollback.push_back(line);
}

void showScrollback() {
    std::cout << "\n--- [Scrollback Log] ---\n";
    for (const auto& l : scrollback)
        std::cout << ">> " << l << "\n";
    std::cout << "------------------------\n\n";
}

//--------------------------------------------
// Main REPL Mode with Import Option
//--------------------------------------------

void repl() {
    std::string line;
    while (true) {
        std::cout << "QL>> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit") break;
        if (line == "import capsule") {
            runCapsuleFile("output.qtrcapsule");
            continue;
        }
        if (line == "scrollback") {
            showScrollback();
            continue;
        }
        logInput(line);
        Lexer lexer(line);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto program = parser.parse();
        Bytecode bc;
        NeuralTuner tuner;
        execute(program, bc, tuner);
        bc.writeToCapsule("output.qtrcapsule");
        printAST(program); // optional AST viewer
    }
}

int main() {
    repl();
    return 0;
}

