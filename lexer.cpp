// QuarterLang Lexer - Unified Tokenizer with Full Enum Refactor, Token Table Output, and Runtime Performance Tuning

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <cctype>
#include <cmath>
#include <locale>
#include <codecvt>
#include <set>
#include <chrono>

//--------------------------------------------
// ENUM REFACTORING
//--------------------------------------------

enum class TokenType {
    Identifier,
    Keyword,
    Number,
    Float,
    Fraction,
    Negative,
    Irrational,
    Rational,
    DivideByZero,
    String,
    Char,
    RawString,
    InterpolatedString,
    Emoji,
    EscapeSequence,
    Operator,
    Punctuation,
    Comment,
    MultiLineComment,
    DGBlock,
    Capsule,
    Constant,
    Eval,
    EOFToken,
    Unknown
};

const std::unordered_map<TokenType, std::string> TokenTypeName = {
    {TokenType::Identifier, "Identifier"},
    {TokenType::Keyword, "Keyword"},
    {TokenType::Number, "Number"},
    {TokenType::Float, "Float"},
    {TokenType::Fraction, "Fraction"},
    {TokenType::Negative, "Negative"},
    {TokenType::Irrational, "Irrational"},
    {TokenType::Rational, "Rational"},
    {TokenType::DivideByZero, "DivideByZero"},
    {TokenType::String, "String"},
    {TokenType::Char, "Char"},
    {TokenType::RawString, "RawString"},
    {TokenType::InterpolatedString, "InterpolatedString"},
    {TokenType::Emoji, "Emoji"},
    {TokenType::EscapeSequence, "EscapeSequence"},
    {TokenType::Operator, "Operator"},
    {TokenType::Punctuation, "Punctuation"},
    {TokenType::Comment, "Comment"},
    {TokenType::MultiLineComment, "MultiLineComment"},
    {TokenType::DGBlock, "DGBlock"},
    {TokenType::Capsule, "Capsule"},
    {TokenType::Constant, "Constant"},
    {TokenType::Eval, "Eval"},
    {TokenType::EOFToken, "EOF"},
    {TokenType::Unknown, "Unknown"}
};

//--------------------------------------------
// TOKEN STRUCTURE
//--------------------------------------------

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;

    Token(TokenType t, const std::string& v, int l, int c)
        : type(t), value(v), line(l), column(c) {
    }

    void print() const {
        std::cout << std::setw(20) << TokenTypeName.at(type)
            << " | Line: " << std::setw(3) << line
            << " Col: " << std::setw(3) << column
            << " | " << value << '\n';
    }
};

//--------------------------------------------
// PERFORMANCE PROFILER
//--------------------------------------------

class Timer {
    std::chrono::high_resolution_clock::time_point start;
public:
    void begin() { start = std::chrono::high_resolution_clock::now(); }
    void end(const std::string& label) {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "[Profiler] " << label << ": " << duration.count() << "us\n";
    }
};

//--------------------------------------------
// PLACEHOLDER LEXER STRUCTURE (Extendable)
//--------------------------------------------

class Lexer {
    std::string source;
    size_t pos = 0;
    int line = 1;
    int col = 1;
    std::vector<Token> tokens;

public:
    Lexer(const std::string& input) : source(input) {}

    void tokenize() {
        Timer timer;
        timer.begin();

        while (pos < source.size()) {
            char current = source[pos];
            if (std::isspace(current)) {
                if (current == '\n') { ++line; col = 1; }
                else ++col;
                ++pos;
                continue;
            }
            if (std::isalpha(current) || current == '_') {
                tokenizeIdentifier();
                continue;
            }
            if (std::isdigit(current)) {
                tokenizeNumber();
                continue;
            }
            tokenizeOperator();
        }

        tokens.emplace_back(TokenType::EOFToken, "<EOF>", line, col);
        timer.end("Lexing Completed");
    }

    void tokenizeIdentifier() {
        size_t start = pos;
        while (pos < source.size() && (std::isalnum(source[pos]) || source[pos] == '_')) ++pos;
        std::string word = source.substr(start, pos - start);
        TokenType t = isKeyword(word) ? TokenType::Keyword : TokenType::Identifier;
        tokens.emplace_back(t, word, line, col);
        col += word.size();
    }

    void tokenizeNumber() {
        size_t start = pos;
        bool hasDot = false;
        while (pos < source.size() && (std::isdigit(source[pos]) || source[pos] == '.')) {
            if (source[pos] == '.') hasDot = true;
            ++pos;
        }
        std::string num = source.substr(start, pos - start);
        tokens.emplace_back(hasDot ? TokenType::Float : TokenType::Number, num, line, col);
        col += num.size();
    }

    void tokenizeOperator() {
        std::string op(1, source[pos]);
        tokens.emplace_back(TokenType::Operator, op, line, col);
        ++pos;
        ++col;
    }

    bool isKeyword(const std::string& word) {
        static std::set<std::string> keywords = {
            "if", "else", "while", "return", "fn", "let", "const", "eval", "DG", "Capsule"
        };
        return keywords.count(word);
    }

    void printTokens() const {
        std::cout << "\n==== TOKEN TABLE ====" << std::endl;
        for (const auto& token : tokens)
            token.print();
    }
};

//--------------------------------------------
// MAIN FOR TESTING
//--------------------------------------------

int main() {
    std::string code = R"(
        let x = 42;
        const y = 3.14;
        fn greet(name) {
            return "Hello, ${name}!";
        }
        // Capsule DG token test
        Capsule { do_something(); }
    )";

    Lexer lexer(code);
    lexer.tokenize();
    lexer.printTokens();

    return 0;
}

//--------------------------------------------
// AST NODE DEFINITIONS - QuarterLang
//--------------------------------------------

#pragma once
#include <string>
#include <memory>
#include <vector>
#include <variant>

// Forward declarations
struct Expr;
struct Stmt;

// Base classes for Visitor pattern
struct ExprVisitor;
struct StmtVisitor;

//--------------------------------------------
// Expression Node Hierarchy
//--------------------------------------------

struct Expr {
    virtual ~Expr() = default;
    virtual void accept(ExprVisitor& visitor) = 0;
};

using ExprPtr = std::shared_ptr<Expr>;

struct LiteralExpr : public Expr {
    std::string value;
    LiteralExpr(const std::string& val) : value(val) {}
    void accept(ExprVisitor& visitor) override;
};

struct VariableExpr : public Expr {
    std::string name;
    VariableExpr(const std::string& n) : name(n) {}
    void accept(ExprVisitor& visitor) override;
};

struct BinaryExpr : public Expr {
    ExprPtr left;
    std::string op;
    ExprPtr right;
    BinaryExpr(ExprPtr l, const std::string& o, ExprPtr r)
        : left(std::move(l)), op(o), right(std::move(r)) {
    }
    void accept(ExprVisitor& visitor) override;
};

struct CallExpr : public Expr {
    std::string callee;
    std::vector<ExprPtr> args;
    CallExpr(const std::string& c, std::vector<ExprPtr> a)
        : callee(c), args(std::move(a)) {
    }
    void accept(ExprVisitor& visitor) override;
};

//--------------------------------------------
// Statement Node Hierarchy
//--------------------------------------------

struct Stmt {
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor& visitor) = 0;
};

using StmtPtr = std::shared_ptr<Stmt>;

struct ValStmt : public Stmt {
    std::string name;
    std::string type;
    ExprPtr initializer;
    ValStmt(const std::string& n, const std::string& t, ExprPtr i)
        : name(n), type(t), initializer(std::move(i)) {
    }
    void accept(StmtVisitor& visitor) override;
};

struct VarStmt : public Stmt {
    std::string name;
    std::string type;
    ExprPtr initializer;
    VarStmt(const std::string& n, const std::string& t, ExprPtr i)
        : name(n), type(t), initializer(std::move(i)) {
    }
    void accept(StmtVisitor& visitor) override;
};

struct SayStmt : public Stmt {
    ExprPtr expression;
    SayStmt(ExprPtr expr) : expression(std::move(expr)) {}
    void accept(StmtVisitor& visitor) override;
};

struct BlockStmt : public Stmt {
    std::vector<StmtPtr> statements;
    BlockStmt(std::vector<StmtPtr> stmts) : statements(std::move(stmts)) {}
    void accept(StmtVisitor& visitor) override;
};

struct LoopStmt : public Stmt {
    ExprPtr start;
    ExprPtr end;
    std::vector<StmtPtr> body;
    LoopStmt(ExprPtr s, ExprPtr e, std::vector<StmtPtr> b)
        : start(std::move(s)), end(std::move(e)), body(std::move(b)) {
    }
    void accept(StmtVisitor& visitor) override;
};

//--------------------------------------------
// Visitor Interfaces
//--------------------------------------------

struct ExprVisitor {
    virtual void visitLiteralExpr(LiteralExpr& expr) = 0;
    virtual void visitVariableExpr(VariableExpr& expr) = 0;
    virtual void visitBinaryExpr(BinaryExpr& expr) = 0;
    virtual void visitCallExpr(CallExpr& expr) = 0;
};

struct StmtVisitor {
    virtual void visitValStmt(ValStmt& stmt) = 0;
    virtual void visitVarStmt(VarStmt& stmt) = 0;
    virtual void visitSayStmt(SayStmt& stmt) = 0;
    virtual void visitBlockStmt(BlockStmt& stmt) = 0;
    virtual void visitLoopStmt(LoopStmt& stmt) = 0;
};

//--------------------------------------------
// Accept Implementations
//--------------------------------------------

inline void LiteralExpr::accept(ExprVisitor& visitor) { visitor.visitLiteralExpr(*this); }
inline void VariableExpr::accept(ExprVisitor& visitor) { visitor.visitVariableExpr(*this); }
inline void BinaryExpr::accept(ExprVisitor& visitor) { visitor.visitBinaryExpr(*this); }
inline void CallExpr::accept(ExprVisitor& visitor) { visitor.visitCallExpr(*this); }

inline void ValStmt::accept(StmtVisitor& visitor) { visitor.visitValStmt(*this); }
inline void VarStmt::accept(StmtVisitor& visitor) { visitor.visitVarStmt(*this); }
inline void SayStmt::accept(StmtVisitor& visitor) { visitor.visitSayStmt(*this); }
inline void BlockStmt::accept(StmtVisitor& visitor) { visitor.visitBlockStmt(*this); }
inline void LoopStmt::accept(StmtVisitor& visitor) { visitor.visitLoopStmt(*this); }

