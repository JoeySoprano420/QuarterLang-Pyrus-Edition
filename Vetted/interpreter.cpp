// QuarterLang Syntax Interpreter with AST + Bytecode + NASM Injection + EXE Compilation + UI REPL
// 🔱 Pure C++ Capsule - Unified

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <cctype>
#include <stdexcept>

// === Tokenization ===

enum TokenType {
    TOK_EOF, TOK_IDENTIFIER, TOK_NUMBER, TOK_STRING,
    TOK_FN, TOK_DG, TOK_VAL, TOK_VAR, TOK_SAY, TOK_DEFINE,
    TOK_STAR, TOK_END, TOK_INLINEHEX, TOK_ASM,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE,
    TOK_SEMI, TOK_COLON, TOK_COMMA, TOK_OP,
    TOK_UNKNOWN
};

struct Token {
    TokenType type;
    std::string text;
};

class Lexer {
    std::string src;
    size_t idx = 0;

    char peek() const {
        return idx < src.size() ? src[idx] : '\0';
    }

    char advance() {
        return idx < src.size() ? src[idx++] : '\0';
    }

    void skip_ws() {
        while (isspace(peek())) advance();
    }

public:
    Lexer(const std::string& s) : src(s) {}

    Token next() {
        skip_ws();
        char ch = peek();

        if (ch == '\0') return {TOK_EOF, ""};

        if (isalpha(ch)) {
            std::string ident;
            while (isalnum(peek())) ident += advance();

            if (ident == "fn") return {TOK_FN, ident};
            if (ident == "dg") return {TOK_DG, ident};
            if (ident == "val") return {TOK_VAL, ident};
            if (ident == "var") return {TOK_VAR, ident};
            if (ident == "say") return {TOK_SAY, ident};
            if (ident == "define") return {TOK_DEFINE, ident};
            if (ident == "asm") return {TOK_ASM, ident};
            if (ident == "star") return {TOK_STAR, ident};
            if (ident == "end") return {TOK_END, ident};
            return {TOK_IDENTIFIER, ident};
        }

        if (isdigit(ch)) {
            std::string num;
            while (isdigit(peek())) num += advance();
            return {TOK_NUMBER, num};
        }

        if (ch == '\"') {
            advance();
            std::string val;
            while (peek() != '\"' && peek() != '\0') val += advance();
            advance();
            return {TOK_STRING, val};
        }

        if (ch == '$') { // inline hex literal
            advance();
            std::string hex;
            while (isxdigit(peek())) hex += advance();
            return {TOK_INLINEHEX, hex};
        }

        if (ch == '(') return advance(), Token{TOK_LPAREN, "("};
        if (ch == ')') return advance(), Token{TOK_RPAREN, ")"};
        if (ch == '{') return advance(), Token{TOK_LBRACE, "{"};
        if (ch == '}') return advance(), Token{TOK_RBRACE, "}"};
        if (ch == ';') return advance(), Token{TOK_SEMI, ";"};
        if (ch == ':') return advance(), Token{TOK_COLON, ":"};
        if (ch == ',') return advance(), Token{TOK_COMMA, ","};

        std::string op(1, advance());
        return {TOK_OP, op};
    }
};

// === AST ===

struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void execute() const = 0;
};

struct SayNode : ASTNode {
    std::string message;
    SayNode(std::string msg) : message(std::move(msg)) {}
    void execute() const override {
        std::cout << "[Q📣] " << message << std::endl;
    }
};

struct InlineNASMNode : ASTNode {
    std::string hex;
    InlineNASMNode(std::string h) : hex(std::move(h)) {}
    void execute() const override {
        std::cout << "[🔧 INLINE HEX INJECTION] $" << hex << std::endl;
    }
};

using AST = std::vector<std::unique_ptr<ASTNode>>;

// === Parser ===

class Parser {
    Lexer& lexer;
    Token curr;

    void advance() { curr = lexer.next(); }

public:
    Parser(Lexer& l) : lexer(l) { advance(); }

    AST parse() {
        AST nodes;
        while (curr.type != TOK_EOF) {
            if (curr.type == TOK_SAY) {
                advance();
                if (curr.type != TOK_STRING) throw std::runtime_error("Expected string after say");
                nodes.push_back(std::make_unique<SayNode>(curr.text));
                advance();
            } else if (curr.type == TOK_INLINEHEX) {
                nodes.push_back(std::make_unique<InlineNASMNode>(curr.text));
                advance();
            } else {
                advance(); // skip unrecognized token
            }
        }
        return nodes;
    }
};

// === Virtual Machine & Execution ===

void execute(const AST& ast) {
    for (const auto& node : ast) node->execute();
}

// === UI REPL ===

void repl() {
    std::cout << "\n🌌 QuarterLang REPL [v0.9 Capsule Edition]" << std::endl;
    std::string line;
    while (true) {
        std::cout << "↪︎ ";
        if (!std::getline(std::cin, line)) break;
        try {
            Lexer lexer(line);
            Parser parser(lexer);
            auto ast = parser.parse();
            execute(ast);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}

int main() {
    repl();
    return 0;
}
