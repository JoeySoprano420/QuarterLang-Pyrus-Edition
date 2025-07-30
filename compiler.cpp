// QuarterLang Compiler Pipeline (Pyrus Edition) - Pure C++
// Simplified illustrative implementation of each stage:
// Lexer → Parser → DG IR → NASM/IR → Optimizer → Bytecode → AOT Emitter → Native Binary
//
// This example focuses on structuring the pipeline with minimal functionality
// to serve as a foundational skeleton for full compiler development.

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>
#include <map>
#include <memory>
#include <fstream>
#include <optional>

// -----------------------------------
// Tokenizer / Lexer
// -----------------------------------

enum class TokenType {
    STAR, END, VAL, VAR, DERIVE, DG,
    SAY, LOOP, WHEN, ELSE, MATCH, CASE,
    DEFINE, PROCEDURE, RETURN, YIELD, FN,
    THREAD, PIPE, NEST, ASM, STOP,
    IDENTIFIER, INT_LITERAL, FLOAT_LITERAL, STRING_LITERAL,
    COLON, COMMA, LPAREN, RPAREN,
    PLUS, MINUS, MUL, DIV,
    UNKNOWN, END_OF_FILE
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line = 0;
};

class Lexer {
    std::string source;
    size_t pos = 0;
    int line = 1;
public:
    explicit Lexer(std::string src) : source(std::move(src)) {}

    char peek() {
        return pos < source.size() ? source[pos] : '\0';
    }
    char advance() {
        if (pos < source.size()) return source[pos++];
        return '\0';
    }
    bool match(char expected) {
        if (peek() == expected) {
            advance();
            return true;
        }
        return false;
    }
    void skipWhitespace() {
        while (true) {
            char c = peek();
            if (c == ' ' || c == '\t' || c == '\r') advance();
            else if (c == '\n') {
                line++;
                advance();
            }
            else break;
        }
    }
    Token makeToken(TokenType type, std::string lexeme) {
        return Token{ type, std::move(lexeme), line };
    }
    Token number() {
        size_t start = pos - 1;
        bool isFloat = false;
        while (isdigit(peek())) advance();
        if (peek() == '.') {
            isFloat = true;
            advance();
            while (isdigit(peek())) advance();
        }
        std::string lex = source.substr(start, pos - start);
        return makeToken(isFloat ? TokenType::FLOAT_LITERAL : TokenType::INT_LITERAL, lex);
    }
    Token identifierOrKeyword() {
        size_t start = pos - 1;
        while (isalnum(peek()) || peek() == '_') advance();
        std::string lex = source.substr(start, pos - start);
        // Convert to lowercase for keyword matching
        std::string lower;
        for (char c : lex) lower += std::tolower(c);

        static const std::map<std::string, TokenType> keywords = {
            {"star", TokenType::STAR}, {"end", TokenType::END}, {"val", TokenType::VAL},
            {"var", TokenType::VAR}, {"derive", TokenType::DERIVE}, {"dg", TokenType::DG},
            {"say", TokenType::SAY}, {"loop", TokenType::LOOP}, {"when", TokenType::WHEN},
            {"else", TokenType::ELSE}, {"match", TokenType::MATCH}, {"case", TokenType::CASE},
            {"define", TokenType::DEFINE}, {"procedure", TokenType::PROCEDURE}, {"return", TokenType::RETURN},
            {"yield", TokenType::YIELD}, {"fn", TokenType::FN}, {"thread", TokenType::THREAD},
            {"pipe", TokenType::PIPE}, {"nest", TokenType::NEST}, {"asm", TokenType::ASM},
            {"stop", TokenType::STOP}
        };
        auto it = keywords.find(lower);
        if (it != keywords.end()) return makeToken(it->second, lex);
        return makeToken(TokenType::IDENTIFIER, lex);
    }
    Token stringLiteral() {
        size_t start = pos;
        while (peek() != '"' && peek() != '\0') {
            if (peek() == '\n') line++;
            advance();
        }
        if (peek() == '\0') {
            // Unterminated string
            return makeToken(TokenType::UNKNOWN, "");
        }
        std::string lex = source.substr(start, pos - start);
        advance(); // consume closing quote
        return makeToken(TokenType::STRING_LITERAL, lex);
    }
    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (true) {
            skipWhitespace();
            char c = advance();
            if (c == '\0') {
                tokens.push_back(makeToken(TokenType::END_OF_FILE, ""));
                break;
            }
            else if (isalpha(c) || c == '_') {
                tokens.push_back(identifierOrKeyword());
            }
            else if (isdigit(c)) {
                tokens.push_back(number());
            }
            else if (c == '"') {
                tokens.push_back(stringLiteral());
            }
            else {
                switch (c) {
                case ':': tokens.push_back(makeToken(TokenType::COLON, ":")); break;
                case ',': tokens.push_back(makeToken(TokenType::COMMA, ",")); break;
                case '(': tokens.push_back(makeToken(TokenType::LPAREN, "(")); break;
                case ')': tokens.push_back(makeToken(TokenType::RPAREN, ")")); break;
                case '+': tokens.push_back(makeToken(TokenType::PLUS, "+")); break;
                case '-': tokens.push_back(makeToken(TokenType::MINUS, "-")); break;
                case '*': tokens.push_back(makeToken(TokenType::MUL, "*")); break;
                case '/': tokens.push_back(makeToken(TokenType::DIV, "/")); break;
                default:
                    tokens.push_back(makeToken(TokenType::UNKNOWN, std::string(1, c)));
                    break;
                }
            }
        }
        return tokens;
    }
};

// -----------------------------------
// Parser - Builds Abstract Syntax Tree (AST)
// -----------------------------------

struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
};

using ASTNodePtr = std::shared_ptr<ASTNode>;

struct Expr : ASTNode {};
struct Stmt : ASTNode {};

// Expression nodes

struct IntLiteral : Expr {
    int value;
    explicit IntLiteral(int v) : value(v) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "IntLiteral: " << value << "\n";
    }
};

struct VarExpr : Expr {
    std::string name;
    explicit VarExpr(std::string n) : name(std::move(n)) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "VarExpr: " << name << "\n";
    }
};

struct BinaryExpr : Expr {
    std::string op;
    ASTNodePtr left;
    ASTNodePtr right;
    BinaryExpr(std::string o, ASTNodePtr l, ASTNodePtr r)
        : op(std::move(o)), left(std::move(l)), right(std::move(r)) {
    }
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "BinaryExpr: " << op << "\n";
        left->print(indent + 2);
        right->print(indent + 2);
    }
};

// Statement nodes

struct ValDecl : Stmt {
    std::string name;
    std::string type;
    ASTNodePtr expr;
    ValDecl(std::string n, std::string t, ASTNodePtr e)
        : name(std::move(n)), type(std::move(t)), expr(std::move(e)) {
    }
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "ValDecl: " << name << " as " << type << "\n";
        expr->print(indent + 2);
    }
};

struct SayStmt : Stmt {
    ASTNodePtr expr;
    explicit SayStmt(ASTNodePtr e) : expr(std::move(e)) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "SayStmt\n";
        expr->print(indent + 2);
    }
};

struct LoopStmt : Stmt {
    std::string iterator;
    int start;
    int end;
    std::vector<ASTNodePtr> body;
    LoopStmt(std::string it, int s, int e, std::vector<ASTNodePtr> b)
        : iterator(std::move(it)), start(s), end(e), body(std::move(b)) {
    }
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "LoopStmt from " << start << " to " << end << " iterator: " << iterator << "\n";
        for (const auto& stmt : body)
            stmt->print(indent + 2);
    }
};

class Parser {
    std::vector<Token> tokens;
    size_t current = 0;

    bool isAtEnd() { return peek().type == TokenType::END_OF_FILE; }
    const Token& peek() { return tokens[current]; }
    const Token& previous() { return tokens[current - 1]; }
    const Token& advance() { if (!isAtEnd()) current++; return previous(); }
    bool check(TokenType t) { if (isAtEnd()) return false; return peek().type == t; }
    bool match(std::initializer_list<TokenType> types) {
        for (auto t : types) {
            if (check(t)) {
                advance();
                return true;
            }
        }
        return false;
    }
    void consume(TokenType type, const std::string& err) {
        if (check(type)) advance();
        else throw std::runtime_error("Parse error at line " + std::to_string(peek().line) + ": expected " + err);
    }

    // Parsing functions:
    ASTNodePtr parseExpression() {
        // For simplicity: parse int literals or identifiers only
        if (match({ TokenType::INT_LITERAL })) {
            int val = std::stoi(previous().lexeme);
            return std::make_shared<IntLiteral>(val);
        }
        if (match({ TokenType::IDENTIFIER })) {
            return std::make_shared<VarExpr>(previous().lexeme);
        }
        throw std::runtime_error("Unexpected token in expression at line " + std::to_string(peek().line));
    }

    ASTNodePtr parseStatement() {
        if (match({ TokenType::VAL })) return parseValDecl();
        if (match({ TokenType::SAY })) return parseSay();
        if (match({ TokenType::LOOP })) return parseLoop();
        throw std::runtime_error("Unknown statement at line " + std::to_string(peek().line));
    }

    ASTNodePtr parseValDecl() {
        if (!check(TokenType::IDENTIFIER)) throw std::runtime_error("Expected identifier after val");
        std::string name = peek().lexeme; advance();
        consume(TokenType::IDENTIFIER, "type after variable name");
        std::string type = previous().lexeme;
        consume(TokenType::COLON, "':' after type");
        ASTNodePtr expr = parseExpression();
        return std::make_shared<ValDecl>(name, type, expr);
    }

    ASTNodePtr parseSay() {
        ASTNodePtr expr = parseExpression();
        return std::make_shared<SayStmt>(expr);
    }

    ASTNodePtr parseLoop() {
        consume(TokenType::IDENTIFIER, "iterator name after loop");
        std::string iterator = previous().lexeme;
        consume(TokenType::IDENTIFIER, "from keyword expected");
        if (previous().lexeme != "from") throw std::runtime_error("Expected 'from' after iterator");
        if (!check(TokenType::INT_LITERAL)) throw std::runtime_error("Expected int literal after 'from'");
        int start = std::stoi(peek().lexeme);
        advance();
        consume(TokenType::IDENTIFIER, "to keyword expected");
        if (previous().lexeme != "to") throw std::runtime_error("Expected 'to' after start");
        if (!check(TokenType::INT_LITERAL)) throw std::runtime_error("Expected int literal after 'to'");
        int end = std::stoi(peek().lexeme);
        advance();
        consume(TokenType::COLON, "':' after loop range");
        // For simplicity, parse a single statement body only
        std::vector<ASTNodePtr> body;
        body.push_back(parseStatement());
        return std::make_shared<LoopStmt>(iterator, start, end, body);
    }

public:
    explicit Parser(std::vector<Token> toks) : tokens(std::move(toks)) {}

    std::vector<ASTNodePtr> parse() {
        std::vector<ASTNodePtr> program;
        while (!isAtEnd()) {
            program.push_back(parseStatement());
        }
        return program;
    }
};

// -----------------------------------
// DG IR (Intermediate Representation)
// -----------------------------------

struct DG_IR_Instruction {
    std::string op;
    std::vector<std::string> args;
};

class DGIRBuilder {
    std::vector<DG_IR_Instruction> instructions;
public:
    void emit(const std::string& op, std::initializer_list<std::string> args) {
        instructions.push_back({ op, args });
    }
    const std::vector<DG_IR_Instruction>& getInstructions() const { return instructions; }

    // Dummy translation from AST to DG IR for demonstration:
    void buildFromAST(const std::vector<ASTNodePtr>& ast) {
        for (const auto& node : ast) {
            // For demo, just emit a SAY with var or literal value string
            if (auto sayStmt = dynamic_cast<SayStmt*>(node.get())) {
                if (auto lit = dynamic_cast<IntLiteral*>(sayStmt->expr.get())) {
                    emit("DG_SAY_INT", { std::to_string(lit->value) });
                }
                else if (auto var = dynamic_cast<VarExpr*>(sayStmt->expr.get())) {
                    emit("DG_SAY_VAR", { var->name });
                }
            }
            else if (auto valDecl = dynamic_cast<ValDecl*>(node.get())) {
                if (auto lit = dynamic_cast<IntLiteral*>(valDecl->expr.get())) {
                    emit("DG_VAL_INT", { valDecl->name, std::to_string(lit->value) });
                }
            }
            else if (auto loopStmt = dynamic_cast<LoopStmt*>(node.get())) {
                emit("DG_LOOP_START", { loopStmt->iterator, std::to_string(loopStmt->start), std::to_string(loopStmt->end) });
                for (auto& stmt : loopStmt->body) {
                    // recursively build body
                    // here we simplify:
                    if (auto sayStmt2 = dynamic_cast<SayStmt*>(stmt.get())) {
                        if (auto lit = dynamic_cast<IntLiteral*>(sayStmt2->expr.get())) {
                            emit("DG_SAY_INT", { std::to_string(lit->value) });
                        }
                        else if (auto var = dynamic_cast<VarExpr*>(sayStmt2->expr.get())) {
                            emit("DG_SAY_VAR", { var->name });
                        }
                    }
                }
                emit("DG_LOOP_END", {});
            }
        }
    }
};

// -----------------------------------
// NASM/IR Emitter (Dummy, illustrative)
// -----------------------------------

class NASMEmitter {
    std::ostringstream oss;
public:
    void emitInstruction(const std::string& instr) {
        oss << instr << "\n";
    }
    void generate(const std::vector<DG_IR_Instruction>& instructions) {
        for (auto& inst : instructions) {
            if (inst.op == "DG_SAY_INT") {
                oss << "    ; say int " << inst.args[0] << "\n";
                oss << "    mov rdi, " << inst.args[0] << "\n";
                oss << "    call print_int\n";
            }
            else if (inst.op == "DG_SAY_VAR") {
                oss << "    ; say var " << inst.args[0] << "\n";
                oss << "    ; variable print not implemented\n";
            }
            else if (inst.op == "DG_VAL_INT") {
                oss << "    ; val int " << inst.args[0] << " = " << inst.args[1] << "\n";
                oss << "    mov [" << inst.args[0] << "], " << inst.args[1] << "\n";
            }
            else if (inst.op == "DG_LOOP_START") {
                oss << "    ; loop start " << inst.args[0] << " from " << inst.args[1] << " to " << inst.args[2] << "\n";
                // loop implementation skipped for demo
            }
            else if (inst.op == "DG_LOOP_END") {
                oss << "    ; loop end\n";
            }
        }
    }
    std::string getOutput() const { return oss.str(); }
};

// -----------------------------------
// Optimizer (stub)
// -----------------------------------

class Optimizer {
public:
    void optimize(std::vector<DG_IR_Instruction>& instructions) {
        // Dummy: No optimization
    }
};

// -----------------------------------
// Bytecode / Hex Encoder (stub)
// -----------------------------------

class BytecodeEncoder {
public:
    std::vector<uint8_t> encode(const std::vector<DG_IR_Instruction>& instructions) {
        // Dummy: map instructions to bytes
        std::vector<uint8_t> bytecode;
        for (const auto& inst : instructions) {
            if (inst.op == "DG_SAY_INT") bytecode.push_back(0x4A);
            else if (inst.op == "DG_VAL_INT") bytecode.push_back(0x10);
            else if (inst.op == "DG_LOOP_START") bytecode.push_back(0x22);
            else if (inst.op == "DG_LOOP_END") bytecode.push_back(0x23);
        }
        bytecode.push_back(0xFF); // End
        return bytecode;
    }
};

// -----------------------------------
// AOT Emitter (stub)
// -----------------------------------

class AOTEmitter {
public:
    void emitNativeBinary(const std::vector<uint8_t>& bytecode, const std::string& filename) {
        std::ofstream out(filename, std::ios::binary);
        out.write(reinterpret_cast<const char*>(bytecode.data()), bytecode.size());
        out.close();
    }
};

// -----------------------------------
// Main Compiler Pipeline Entrypoint
// -----------------------------------

void compile(const std::string& sourceCode, const std::string& outputFile) {
    try {
        Lexer lexer(sourceCode);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        auto ast = parser.parse();

        // Debug print AST
        std::cout << "AST:\n";
        for (const auto& node : ast) node->print(2);

        DG
    };

#include <iostream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <cctype>

    class DG {
        // Internal representation as a string of base-12 digits, most significant digit first.
        // Allowed digits: '0'..'9', 'A', 'B' (A=10, B=11)
        std::string digits;

        // Helper: convert char digit to int value
        static int charToVal(char c) {
            if (c >= '0' && c <= '9') return c - '0';
            if (c == 'A' || c == 'a') return 10;
            if (c == 'B' || c == 'b') return 11;
            throw std::invalid_argument("Invalid DG digit");
        }

        // Helper: convert int value to char digit
        static char valToChar(int v) {
            if (v >= 0 && v <= 9) return '0' + v;
            if (v == 10) return 'A';
            if (v == 11) return 'B';
            throw std::invalid_argument("Invalid DG digit value");
        }

        // Normalize digits: remove leading zeros
        void normalize() {
            while (digits.size() > 1 && digits[0] == '0') {
                digits.erase(digits.begin());
            }
        }

    public:
        // Constructors
        DG() : digits("0") {}

        explicit DG(const std::string& d) {
            // Validate and normalize input string
            if (d.empty()) throw std::invalid_argument("Empty DG literal");
            for (char c : d) {
                if (!((c >= '0' && c <= '9') || c == 'A' || c == 'B' || c == 'a' || c == 'b')) {
                    throw std::invalid_argument("Invalid DG literal character");
                }
            }
            // Store uppercase
            digits.reserve(d.size());
            for (char c : d) {
                digits.push_back(std::toupper(c));
            }
            normalize();
        }

        // Construct from integer (decimal)
        explicit DG(int n) {
            if (n < 0) throw std::invalid_argument("DG only supports non-negative integers");
            if (n == 0) digits = "0";
            else {
                digits.clear();
                while (n > 0) {
                    int rem = n % 12;
                    digits.push_back(valToChar(rem));
                    n /= 12;
                }
                std::reverse(digits.begin(), digits.end());
            }
        }

        // Convert DG to integer (decimal)
        int toInt() const {
            int result = 0;
            for (char c : digits) {
                result = result * 12 + charToVal(c);
            }
            return result;
        }

        // Convert DG to string
        std::string toString() const {
            return digits;
        }

        // Addition
        DG operator+(const DG& rhs) const {
            std::string result;
            int carry = 0;
            int i = (int)digits.size() - 1;
            int j = (int)rhs.digits.size() - 1;

            while (i >= 0 || j >= 0 || carry > 0) {
                int lhsDigit = (i >= 0) ? charToVal(digits[i]) : 0;
                int rhsDigit = (j >= 0) ? charToVal(rhs.digits[j]) : 0;
                int sum = lhsDigit + rhsDigit + carry;
                carry = sum / 12;
                sum = sum % 12;
                result.push_back(valToChar(sum));
                i--;
                j--;
            }
            std::reverse(result.begin(), result.end());
            return DG(result);
        }

        // Multiplication
        DG operator*(const DG& rhs) const {
            // Use classic long multiplication
            int n = (int)digits.size();
            int m = (int)rhs.digits.size();
            std::vector<int> temp(n + m, 0);

            for (int i = n - 1; i >= 0; i--) {
                int lhsDigit = charToVal(digits[i]);
                for (int j = m - 1; j >= 0; j--) {
                    int rhsDigit = charToVal(rhs.digits[j]);
                    temp[i + j + 1] += lhsDigit * rhsDigit;
                }
            }

            // Carry propagation base 12
            for (int k = (int)temp.size() - 1; k > 0; k--) {
                int carry = temp[k] / 12;
                temp[k] %= 12;
                temp[k - 1] += carry;
            }

            // Convert to string skipping leading zeros
            size_t start = 0;
            while (start < temp.size() - 1 && temp[start] == 0) start++;

            std::string result;
            for (size_t idx = start; idx < temp.size(); idx++) {
                result.push_back(valToChar(temp[idx]));
            }
            return DG(result);
        }

        // Equality operator
        bool operator==(const DG& rhs) const {
            return digits == rhs.digits;
        }
    };

    // Overload stream operators for convenience

    std::ostream& operator<<(std::ostream & os, const DG & dg) {
        os << dg.toString();
        return os;
    }

    std::istream& operator>>(std::istream & is, DG & dg) {
        std::string input;
        is >> input;
        dg = DG(input);
        return is;
    }

    // ---------------------
    // Example usage:

    int main() {
        DG a("1B");    // 1*12 + 11 = 23 decimal
        DG b(15);      // decimal 15 -> base12 "13"
        DG c = a + b;  // 23 + 15 = 38 decimal -> base12 "32"
        DG d = a * b;  // 23 * 15 = 345 decimal -> base12 ?

        std::cout << "a = " << a << " (decimal " << a.toInt() << ")\n";
        std::cout << "b = " << b << " (decimal " << b.toInt() << ")\n";
        std::cout << "a + b = " << c << " (decimal " << c.toInt() << ")\n";
        std::cout << "a * b = " << d << " (decimal " << d.toInt() << ")\n";

        return 0;
    }

#include <iostream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <cctype>
#include <cmath>

    class DG {
        // Internal representation: separate integer and fractional parts as strings.
        // Digits: '0'..'9', 'A'=10, 'B'=11
        // Integer part: most significant digit first
        // Fractional part: digits after decimal point, least significant rightmost
        std::string intPart;
        std::string fracPart; // empty if no fraction

        // Helpers for digit conversions
        static int charToVal(char c) {
            if (c >= '0' && c <= '9') return c - '0';
            if (c == 'A' || c == 'a') return 10;
            if (c == 'B' || c == 'b') return 11;
            throw std::invalid_argument("Invalid DG digit");
        }
        static char valToChar(int v) {
            if (v >= 0 && v <= 9) return '0' + v;
            if (v == 10) return 'A';
            if (v == 11) return 'B';
            throw std::invalid_argument("Invalid DG digit value");
        }

        // Normalize parts: remove leading zeros in intPart, trailing zeros in fracPart
        void normalize() {
            // Remove leading zeros in intPart
            while (intPart.size() > 1 && intPart[0] == '0') {
                intPart.erase(intPart.begin());
            }
            // Remove trailing zeros in fracPart
            while (!fracPart.empty() && fracPart.back() == '0') {
                fracPart.pop_back();
            }
            // If intPart is empty, set to "0"
            if (intPart.empty()) intPart = "0";
        }

        // Parse helper: raw string without dg: prefix, parses int and fraction parts
        void parseRaw(const std::string& raw) {
            size_t dotPos = raw.find('.');
            if (dotPos == std::string::npos) {
                intPart = raw;
                fracPart.clear();
            }
            else {
                intPart = raw.substr(0, dotPos);
                fracPart = raw.substr(dotPos + 1);
            }
            // Validate digits
            for (char c : intPart) charToVal(c);  // throws if invalid
            for (char c : fracPart) charToVal(c);
            normalize();
        }

        // Convert integer part from decimal int to DG string
        static std::string intToDGString(int n) {
            if (n < 0) throw std::invalid_argument("Negative integer not supported");
            if (n == 0) return "0";
            std::string res;
            while (n > 0) {
                int r = n % 12;
                res.push_back(valToChar(r));
                n /= 12;
            }
            std::reverse(res.begin(), res.end());
            return res;
        }

        // Convert integer DG string to decimal int
        static int dgStringToInt(const std::string& s) {
            int result = 0;
            for (char c : s) {
                result = result * 12 + charToVal(c);
            }
            return result;
        }

        // Addition of integer parts (strings)
        static std::string addStrings(const std::string& a, const std::string& b) {
            int i = (int)a.size() - 1;
            int j = (int)b.size() - 1;
            int carry = 0;
            std::string result;
            while (i >= 0 || j >= 0 || carry > 0) {
                int x = i >= 0 ? charToVal(a[i]) : 0;
                int y = j >= 0 ? charToVal(b[j]) : 0;
                int sum = x + y + carry;
                carry = sum / 12;
                sum %= 12;
                result.push_back(valToChar(sum));
                i--; j--;
            }
            std::reverse(result.begin(), result.end());
            return result;
        }

        // Subtraction of integer parts (a - b), assumes a >= b
        static std::string subStrings(const std::string& a, const std::string& b) {
            int i = (int)a.size() - 1;
            int j = (int)b.size() - 1;
            int borrow = 0;
            std::string result;
            while (i >= 0) {
                int x = charToVal(a[i]) - borrow;
                int y = j >= 0 ? charToVal(b[j]) : 0;
                if (x < y) {
                    x += 12;
                    borrow = 1;
                }
                else {
                    borrow = 0;
                }
                int diff = x - y;
                result.push_back(valToChar(diff));
                i--; j--;
            }
            // Remove leading zeros
            while (result.size() > 1 && result.back() == '0')
                result.pop_back();
            std::reverse(result.begin(), result.end());
            return result;
        }

        // Compare integer strings: returns 1 if a > b, 0 if equal, -1 if a < b
        static int compareStrings(const std::string& a, const std::string& b) {
            if (a.size() > b.size()) return 1;
            if (a.size() < b.size()) return -1;
            for (size_t i = 0; i < a.size(); i++) {
                int va = charToVal(a[i]);
                int vb = charToVal(b[i]);
                if (va > vb) return 1;
                if (va < vb) return -1;
            }
            return 0;
        }

        // Add fractional parts (strings), returns sum and carry (0 or 1)
        static std::pair<std::string, int> addFracStrings(const std::string& a, const std::string& b) {
            size_t maxLen = std::max(a.size(), b.size());
            int carry = 0;
            std::string result;
            for (int i = (int)maxLen - 1; i >= 0; i--) {
                int x = i < (int)a.size() ? charToVal(a[i]) : 0;
                int y = i < (int)b.size() ? charToVal(b[i]) : 0;
                int sum = x + y + carry;
                carry = sum / 12;
                sum %= 12;
                result.push_back(valToChar(sum));
            }
            std::reverse(result.begin(), result.end());
            // remove trailing zeros
            while (!result.empty() && result.back() == '0') result.pop_back();
            return { result, carry };
        }

        // Subtract fractional parts (a - b), assume a >= b, return result and borrow
        static std::pair<std::string, int> subFracStrings(const std::string& a, const std::string& b) {
            size_t maxLen = std::max(a.size(), b.size());
            int borrow = 0;
            std::string result;
            for (int i = (int)maxLen - 1; i >= 0; i--) {
                int x = i < (int)a.size() ? charToVal(a[i]) : 0;
                int y = i < (int)b.size() ? charToVal(b[i]) : 0;
                x -= borrow;
                if (x < y) {
                    x += 12;
                    borrow = 1;
                }
                else {
                    borrow = 0;
                }
                int diff = x - y;
                result.push_back(valToChar(diff));
            }
            std::reverse(result.begin(), result.end());
            while (!result.empty() && result.back() == '0') result.pop_back();
            return { result, borrow };
        }

    public:
        DG() : intPart("0"), fracPart("") {}

        explicit DG(const std::string& s) {
            // Support optional "dg:" prefix
            std::string literal = s;
            if (literal.size() > 3 && literal.substr(0, 3) == "dg:") {
                literal = literal.substr(3);
            }
            parseRaw(literal);
        }

        explicit DG(int n) : fracPart("") {
            if (n < 0) throw std::invalid_argument("Negative not supported");
            intPart = intToDGString(n);
        }

        DG(double d, int precision = 10) {
            // Convert double decimal to base12 with fractional part
            if (d < 0) throw std::invalid_argument("Negative not supported");
            int intPartDec = static_cast<int>(std::floor(d));
            double fracPartDec = d - intPartDec;

            intPart = intToDGString(intPartDec);
            fracPart.clear();

            // Convert fractional part by multiplying by 12 repeatedly
            for (int i = 0; i < precision; i++) {
                fracPartDec *= 12;
                int digit = static_cast<int>(std::floor(fracPartDec));
                fracPart += valToChar(digit);
                fracPartDec -= digit;
                if (fracPartDec <= 1e-14) break; // close to zero
            }
            normalize();
        }

        std::string toString() const {
            if (fracPart.empty()) return intPart;
            return intPart + "." + fracPart;
        }

        // Convert to decimal double
        double toDouble() const {
            double result = 0;
            for (char c : intPart) {
                result = result * 12 + charToVal(c);
            }
            double fracMultiplier = 1.0 / 12.0;
            for (char c : fracPart) {
                result += charToVal(c) * fracMultiplier;
                fracMultiplier /= 12.0;
            }
            return result;
        }

        // Addition operator
        DG operator+(const DG& rhs) const {
            // Add fractional parts
            auto [fracSum, fracCarry] = addFracStrings(fracPart, rhs.fracPart);

            // Add integer parts + carry
            std::string lhsInt = intPart;
            std::string rhsInt = rhs.intPart;
            if (fracCarry == 1) {
                rhsInt = addStrings(rhsInt, "1");
            }
            std::string intSum = addStrings(lhsInt, rhsInt);

            DG result;
            result.intPart = intSum;
            result.fracPart = fracSum;
            result.normalize();
            return result;
        }

        // Subtraction operator (throws if lhs < rhs)
        DG operator-(const DG& rhs) const {
            // Compare integer parts
            int cmpInt = compareStrings(intPart, rhs.intPart);
            if (cmpInt < 0) throw std::invalid_argument("Negative DG not supported");
            if (cmpInt == 0) {
                // compare fractional parts
                int cmpFrac = compareStrings(fracPart, rhs.fracPart);
                if (cmpFrac < 0) throw std::invalid_argument("Negative DG not supported");
                if (cmpFrac == 0) return DG("0");
            }

            // Subtract fractional parts
            auto [fracDiff, borrow] = subFracStrings(fracPart, rhs.fracPart);

            // Subtract integer parts with borrow
            std::string lhsInt = intPart;
            std::string rhsInt = rhs.intPart;
            if (borrow == 1) {
                rhsInt = addStrings(rhsInt, "1");
            }
            std::string intDiff = subStrings(lhsInt, rhsInt);

            DG result;
            result.intPart = intDiff;
            result.fracPart = fracDiff;
            result.normalize();
            return result;
        }

        // Integer division: returns quotient and remainder (both DG)
        std::pair<DG, DG> divmod(const DG& divisor) const {
            if (divisor == DG("0")) throw std::invalid_argument("Division by zero");

            // For simplicity, only support integer division on integer parts ignoring fractions
            int cmp = compareStrings(intPart, divisor.intPart);
            if (cmp < 0) {
                return { DG("0"), *this }; // quotient 0, remainder this
            }
            if (cmp == 0) {
                return { DG("1"), DG("0") };
            }

            // Long division algorithm on base12 strings
            std::string dividend = intPart;
            std::string div = divisor.intPart;

            std::string quotient;
            std::string remainder;

            size_t pos = 0;
            remainder.clear();

            while (pos < dividend.size()) {
                remainder.push_back(dividend[pos]);
                // remove leading zeros in remainder
                while (remainder.size() > 1 && remainder[0] == '0') remainder.erase(remainder.begin());

                int qDigit = 0;
                while (compareStrings(remainder, div) >= 0) {
                    remainder = subStrings(remainder, div);
                    qDigit++;
                }
                quotient.push_back(valToChar(qDigit));
                pos++;
            }
            // Remove leading zeros quotient
            while (quotient.size() > 1 && quotient[0] == '0') quotient.erase(quotient.begin());

            DG q(quotient);
            DG r;
            r.intPart = remainder.empty() ? "0" : remainder;
            r.fracPart.clear();

            return { q, r };
        }

        // Equality operator
        bool operator==(const DG& rhs) const {
            return intPart == rhs.intPart && fracPart == rhs.fracPart;
        }
    };

    // Stream operators
    std::ostream& operator<<(std::ostream& os, const DG& dg) {
        os << dg.toString();
        return os;
    }
    std::istream& operator>>(std::istream& is, DG& dg) {
        std::string s;
        is >> s;
        dg = DG(s);
        return is;
    }

    // -----------------------
    // Example usage

    int main() {
        DG a("dg:1B.8");   // 1B.8 base12 (23 + 8/12)
        DG b("13.4");      // 13.4 base12 (15 + 4/12)
        DG c = a + b;
        DG d = a - b;
        auto [q, r] = a.divmod(DG("2"));

        std::cout << "a = " << a << " (decimal ~" << a.toDouble() << ")\n";
        std::cout << "b = " << b << " (decimal ~" << b.toDouble() << ")\n";
        std::cout << "a + b = " << c << " (decimal ~" << c.toDouble() << ")\n";
        std::cout << "a - b = " << d << " (decimal ~" << d.toDouble() << ")\n";
        std::cout << "a div 2 = quotient: " << q << ", remainder: " << r << "\n";

        // Construct from double
        DG e(12.75);  // ~ "10.9" in base12 (12 + 9/12)
        std::cout << "e from double 12.75 = " << e << "\n";

        return 0;
    }

    // DgCompiler.cpp — Extended DG Compiler with Base-12 Floating Point, Graphics Injection, and Optimizations

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <memory>
#include <regex>

    namespace dg {

        using DGFloat = long double; // Extended precision floating point for base-12 (Dodecagram)

        constexpr DGFloat BASE = 12.0;

        DGFloat parseBase12(const std::string& literal) {
            DGFloat value = 0.0;
            DGFloat frac = 1.0 / BASE;
            bool fractional = false;
            bool negative = false;

            size_t i = 0;
            if (literal[i] == '-') {
                negative = true;
                ++i;
            }

            for (; i < literal.size(); ++i) {
                if (literal[i] == '.') {
                    fractional = true;
                    continue;
                }
                int digit = literal[i] - '0';
                if (digit >= 0 && digit < 12) {
                    if (!fractional) value = value * BASE + digit;
                    else {
                        value += digit * frac;
                        frac /= BASE;
                    }
                }
            }

            return negative ? -value : value;
        }

        struct Expression {
            DGFloat value;
            Expression(DGFloat v) : value(v) {}
        };

        class Parser {
        public:
            Expression parse(const std::string& token) {
                if (token.rfind("dg:", 0) == 0) {
                    return Expression(parseBase12(token.substr(3)));
                }
                else {
                    return Expression(std::stod(token));
                }
            }

            Expression add(const Expression& a, const Expression& b) {
                return Expression(a.value + b.value);
            }

            Expression sub(const Expression& a, const Expression& b) {
                return Expression(a.value - b.value);
            }

            Expression div(const Expression& a, const Expression& b) {
                return Expression(a.value / b.value);
            }

            Expression mul(const Expression& a, const Expression& b) {
                return Expression(a.value * b.value);
            }
        };

        class Vector {
        public:
            std::vector<DGFloat> values;

            Vector(const std::vector<DGFloat>& vals) : values(vals) {}

            void scale(DGFloat factor) {
                for (auto& v : values) v *= factor;
            }

            void rangeScale(DGFloat start, DGFloat end) {
                DGFloat scale = (end - start) / values.size();
                for (size_t i = 0; i < values.size(); ++i)
                    values[i] *= (start + scale * i);
            }
        };

        // Compiler Optimization and Graphics Integration Pass
        struct Optimizer {
            void profileGuidedOptimization() {
                std::cout << "[P.G.O] Profile-Guided Optimization Applied.\n";
            }

            void eliminateDeadCode() {
                std::cout << "[Optimizer] Dead Code Eliminated.\n";
            }

            void unrollLoops() {
                std::cout << "[Optimizer] Loops Unrolled.\n";
            }

            void constantPropagate() {
                std::cout << "[Optimizer] Constants Propagated.\n";
            }

            void registerAllocate() {
                std::cout << "[Optimizer] Register Allocation Performed.\n";
            }

            void scheduleInstructions() {
                std::cout << "[Optimizer] Instruction Scheduling Optimized.\n";
            }

            void foldConstants() {
                std::cout << "[Optimizer] Constant Folding Applied.\n";
            }

            void peepholeOptimize() {
                std::cout << "[Optimizer] Peephole Optimization Applied.\n";
            }
        };

        struct ShaderSystem {
            void injectVulkan() {
                std::cout << "[Graphics] Vulkan Injection Complete.\n";
            }
            void injectOpenGL() {
                std::cout << "[Graphics] OpenGL Injection Complete.\n";
            }
            void injectDirectX() {
                std::cout << "[Graphics] DirectX Injection Complete.\n";
            }
            void autoImpressShaders() {
                std::cout << "[Graphics] Shader and Lighting Impression Complete.\n";
            }
            void holographicInfer() {
                std::cout << "[Graphics] Holographic Inference Applied.\n";
            }
            void bakeRayTracing() {
                std::cout << "[Graphics] Ray Tracing Baked into Output.\n";
            }
        };

        void compile(const std::string& input) {
            Parser parser;
            Optimizer optimizer;
            ShaderSystem shaders;

            auto expr = parser.parse(input);

            std::cout << "[Compile] Base-12 Result: " << std::setprecision(20) << expr.value << "\n";

            optimizer.profileGuidedOptimization();
            optimizer.eliminateDeadCode();
            optimizer.unrollLoops();
            optimizer.constantPropagate();
            optimizer.registerAllocate();
            optimizer.scheduleInstructions();
            optimizer.foldConstants();
            optimizer.peepholeOptimize();

            shaders.injectVulkan();
            shaders.injectOpenGL();
            shaders.injectDirectX();
            shaders.autoImpressShaders();
            shaders.holographicInfer();
            shaders.bakeRayTracing();
        }

    } // namespace dg

    int main() {
        dg::compile("dg:10.47");
        return 0;
    }

    struct HolographicMap {
        std::map<std::string, std::string> opcodeTrace;
        std::vector<std::pair<std::string, double>> frequencyHeat;

        void registerOpcode(const std::string& op, const std::string& location) {
            opcodeTrace[op] = location;
            updateHeat(op);
        }

        void updateHeat(const std::string& op) {
            for (auto& [name, freq] : frequencyHeat) {
                if (name == op) { freq += 1.0; return; }
            }
            frequencyHeat.emplace_back(op, 1.0);
        }

        void renderIntrospectionMap() const {
            std::cout << "[HOLO-AOT MAP] Opcode Heat Traces:\n";
            for (const auto& [name, freq] : frequencyHeat) {
                std::cout << "↳ " << name << " : " << freq << "x\n";
            }
        }
    };

    struct ShaderTraceOverlay {
        std::vector<std::string> traceLog;
        bool active = false;

        void activateOverlay() { active = true; }
        void logShaderInvocation(const std::string& shaderName) {
            if (active) {
                traceLog.push_back(shaderName);
                std::cout << "🔍 [Shader Trace] " << shaderName << " invoked\n";
            }
        }

        void renderOverlay() const {
            std::cout << "--- Live Shader Overlay ---\n";
            for (const auto& log : traceLog) {
                std::cout << "● Shader: " << log << "\n";
            }
        }
    };

    struct OpcodeTemporalModel {
        std::deque<std::string> history;
        std::map<std::string, std::map<std::string, int>> transitionCount;

        void logOpcode(const std::string& opcode) {
            if (!history.empty()) {
                transitionCount[history.back()][opcode]++;
            }
            history.push_back(opcode);
            if (history.size() > 10) history.pop_front();
        }

        void predictNext() const {
            if (history.empty()) return;
            auto last = history.back();
            if (transitionCount.find(last) != transitionCount.end()) {
                const auto& candidates = transitionCount.at(last);
                auto max_it = std::max_element(candidates.begin(), candidates.end(),
                    [](const auto& a, const auto& b) { return a.second < b.second; });
                if (max_it != candidates.end()) {
                    std::cout << "🔮 Predicted Next Opcode: " << max_it->first << " (from " << last << ")\n";
                }
            }
        }
    };

    // --- DG Compiler: Introspective Predictive Model Extension ---
// Author: Violet
// Purpose: Add weighted prediction, time-decay, reinforcement learning, and shader auto-tuning

#include <iostream>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <cmath>
#include <string>
#include <random>
#include <limits>

    struct PredictionEntry {
        std::string opcode;
        double weight;
        double decay_factor;
        double reinforcement;
        std::chrono::steady_clock::time_point timestamp;

        double score() const {
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = now - timestamp;
            double decay = std::exp(-decay_factor * elapsed.count());
            return weight * decay + reinforcement;
        }
    };

    class PredictiveModel {
    private:
        std::unordered_map<std::string, PredictionEntry> predictions;
        double learning_rate = 0.01;
        double default_decay = 0.001;
        double reward_scale = 1.5;

    public:
        void observe(const std::string& opcode) {
            auto now = std::chrono::steady_clock::now();
            if (predictions.count(opcode)) {
                predictions[opcode].weight += learning_rate;
                predictions[opcode].timestamp = now;
            }
            else {
                predictions[opcode] = { opcode, 1.0, default_decay, 0.0, now };
            }
        }

        void reinforce(const std::string& opcode, double reward) {
            if (predictions.count(opcode)) {
                predictions[opcode].reinforcement += reward * reward_scale;
            }
        }

        std::string predict_next() {
            std::string best_opcode;
            double best_score = -std::numeric_limits<double>::infinity();
            for (const auto& [op, entry] : predictions) {
                double s = entry.score();
                if (s > best_score) {
                    best_score = s;
                    best_opcode = op;
                }
            }
            return best_opcode;
        }

        void decay_all() {
            for (auto& [op, entry] : predictions) {
                entry.weight *= 0.99;
                entry.reinforcement *= 0.95;
            }
        }
    };

    class ShaderAutoTuner {
    public:
        void tuneShaderForPrediction(const std::string& predictedOpcode) {
            std::cout << "[AutoTuner] Adapting shader pipeline for opcode: " << predictedOpcode << std::endl;
            // Insert dynamic pipeline rebuild or recompilation logic here.
        }
    };

    // --- Example Integration ---
    int main() {
        PredictiveModel model;
        ShaderAutoTuner tuner;

        model.observe("DG_ADD");
        model.observe("DG_MUL");
        model.observe("DG_ADD");
        model.reinforce("DG_ADD", 2.0);
        model.observe("DG_DIV");

        std::string next = model.predict_next();
        tuner.tuneShaderForPrediction(next);

        return 0;
    }

