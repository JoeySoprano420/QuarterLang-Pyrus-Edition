// QuarterLang Interpreter Core + VM + .EXE Compiler

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <variant>
#include <optional>
#include <cctype>
#include <functional>
#include <fstream>
#include <iomanip>
using namespace std;

// === Token Types === //
enum class TokenType {
    IDENT, NUMBER, STRING, HEX, NASM,
    VAL, VAR, DERIVE, SAY, DG, LOOP, WHEN, ELSE, DEFINE, FN, THREAD, PIPE, ASM,
    LPAREN, RPAREN, LBRACE, RBRACE, COMMA, SEMICOLON, EQ, STAR, END, EOF_TOK
};

string tokenTypeToStr(TokenType t) {
    switch (t) {
    case TokenType::VAL: return "val";
    case TokenType::VAR: return "var";
    case TokenType::DERIVE: return "derive";
    case TokenType::SAY: return "say";
    case TokenType::DG: return "dg";
    case TokenType::LOOP: return "loop";
    case TokenType::WHEN: return "when";
    case TokenType::ELSE: return "else";
    case TokenType::DEFINE: return "define";
    case TokenType::FN: return "fn";
    case TokenType::THREAD: return "thread";
    case TokenType::PIPE: return "pipe";
    case TokenType::ASM: return "asm";
    default: return "tok";
    }
}

struct Token {
    TokenType type;
    string text;
};

// === Lexer === //
class Lexer {
    string src;
    size_t i = 0;
    unordered_map<string, TokenType> keywords = {
        {"val", TokenType::VAL}, {"var", TokenType::VAR}, {"derive", TokenType::DERIVE},
        {"say", TokenType::SAY}, {"dg", TokenType::DG}, {"loop", TokenType::LOOP},
        {"when", TokenType::WHEN}, {"else", TokenType::ELSE}, {"define", TokenType::DEFINE},
        {"fn", TokenType::FN}, {"thread", TokenType::THREAD}, {"pipe", TokenType::PIPE},
        {"asm", TokenType::ASM}
    };
public:
    Lexer(const string& s) : src(s) {}

    char peek() const { return i < src.size() ? src[i] : '\0'; }
    char next() { return i < src.size() ? src[i++] : '\0'; }

    Token nextToken() {
        while (isspace(peek())) next();
        if (isalpha(peek())) {
            string ident;
            while (isalnum(peek()) || peek() == '_') ident += next();
            if (keywords.count(ident)) return { keywords[ident], ident };
            return { TokenType::IDENT, ident };
        }
        if (isdigit(peek())) {
            string num;
            while (isdigit(peek())) num += next();
            return { TokenType::NUMBER, num };
        }
        if (peek() == '"') {
            next(); string str;
            while (peek() != '"' && peek() != '\0') str += next();
            next(); return { TokenType::STRING, str };
        }
        if (peek() == '0' && src[i + 1] == 'x') {
            string hex = "0x"; next(); next();
            while (isxdigit(peek())) hex += next();
            return { TokenType::HEX, hex };
        }
        if (peek() == '#') {
            while (peek() != '\n' && peek() != '\0') next();
            return nextToken();
        }
        char ch = next();
        switch (ch) {
        case '(': return { TokenType::LPAREN, "(" };
        case ')': return { TokenType::RPAREN, ")" };
        case '{': return { TokenType::LBRACE, "{" };
        case '}': return { TokenType::RBRACE, "}" };
        case ',': return { TokenType::COMMA, "," };
        case ';': return { TokenType::SEMICOLON, ";" };
        case '=': return { TokenType::EQ, "=" };
        case '*': return { TokenType::STAR, "*" };
        }
        return { TokenType::EOF_TOK, "" };
    }
};

// === AST === //
struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void eval() = 0;
};

struct ASTSay : ASTNode {
    string text;
    ASTSay(string t) : text(t) {}
    void eval() override { cout << "[SAY] " << text << endl; }
};

// === Parser === //
class Parser {
    Lexer& lexer;
    Token current;

    void next() { current = lexer.nextToken(); }
public:
    Parser(Lexer& l) : lexer(l) { next(); }

    vector<unique_ptr<ASTNode>> parseAll() {
        vector<unique_ptr<ASTNode>> nodes;
        while (current.type != TokenType::EOF_TOK) {
            if (current.type == TokenType::SAY) {
                next();
                if (current.type == TokenType::STRING) {
                    nodes.push_back(make_unique<ASTSay>(current.text));
                    next();
                }
            }
            else next();
        }
        return nodes;
    }
};

// === VM === //
class VM {
public:
    void run(const vector<unique_ptr<ASTNode>>& nodes) {
        for (const auto& n : nodes) n->eval();
    }
};

// === Main === //
int main() {
    string input;
    cout << "QuarterLang REPL. Enter code (type 'exit' to quit):\n";
    while (true) {
        cout << "> ";
        getline(cin, input);
        if (input == "exit") break;
        Lexer lexer(input);
        Parser parser(lexer);
        auto nodes = parser.parseAll();
        VM vm;
        vm.run(nodes);
    }
    return 0;
}


