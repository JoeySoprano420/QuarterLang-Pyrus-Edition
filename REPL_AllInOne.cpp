// QuarterLang Lexer + Parser + REPL + Capsule/Comment/String Support
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <cctype>
#include <set>
#include <chrono>

//--------------------------------------------
// ENUMS + TOKEN TYPE NAMES
//--------------------------------------------
enum class TokenType {
    Identifier, Keyword, Number, Float, Fraction, Negative, Irrational, Rational, DivideByZero,
    String, Char, RawString, InterpolatedString, Emoji, EscapeSequence,
    Operator, Punctuation, Comment, MultiLineComment,
    DGBlock, Capsule, Constant, Eval, EOFToken, Unknown
};

const std::unordered_map<TokenType, std::string> TokenTypeName = {
    {TokenType::Identifier, "Identifier"}, {TokenType::Keyword, "Keyword"},
    {TokenType::Number, "Number"}, {TokenType::Float, "Float"},
    {TokenType::String, "String"}, {TokenType::Comment, "Comment"},
    {TokenType::MultiLineComment, "MultiLineComment"}, {TokenType::Capsule, "Capsule"},
    {TokenType::DGBlock, "DGBlock"}, {TokenType::Eval, "Eval"}, {TokenType::Constant, "Constant"},
    {TokenType::EOFToken, "EOF"}, {TokenType::Unknown, "Unknown"}
};

//--------------------------------------------
// TOKEN STRUCTURE
//--------------------------------------------
struct Token {
    TokenType type;
    std::string value;
    int line, column;
    Token(TokenType t, const std::string& v, int l, int c) : type(t), value(v), line(l), column(c) {}
    void print() const {
        std::cout << std::setw(16) << TokenTypeName.at(type) << " | Line " << line << " Col " << column << " | " << value << "\n";
    }
};

//--------------------------------------------
// LEXER + TOKENIZER
//--------------------------------------------
class Lexer {
    std::string source;
    size_t pos = 0;
    int line = 1, col = 1;
    std::vector<Token> tokens;

public:
    Lexer(const std::string& src) : source(src) {}

    void tokenize() {
        while (pos < source.length()) {
            char c = source[pos];
            if (std::isspace(c)) advance(c == '\n');
            else if (c == '"') tokenizeString();
            else if (c == '/' && peek() == '/') tokenizeComment();
            else if (c == '/' && peek() == '*') tokenizeMultilineComment();
            else if (std::isalpha(c) || c == '_') tokenizeIdentifier();
            else if (std::isdigit(c)) tokenizeNumber();
            else tokenizeOperator();
        }
        tokens.emplace_back(TokenType::EOFToken, "<EOF>", line, col);
    }

    void tokenizeString() {
        size_t start = ++pos; int startCol = col++;
        std::string value;
        while (pos < source.size() && source[pos] != '"') {
            if (source[pos] == '\\' && pos + 1 < source.size()) value += source[pos++];
            value += source[pos++];
            ++col;
        }
        ++pos; ++col;
        tokens.emplace_back(TokenType::String, source.substr(start, pos - start - 1), line, startCol);
    }

    void tokenizeComment() {
        size_t start = pos; int startCol = col;
        while (pos < source.size() && source[pos] != '\n') { ++pos; ++col; }
        tokens.emplace_back(TokenType::Comment, source.substr(start, pos - start), line, startCol);
    }

    void tokenizeMultilineComment() {
        size_t start = pos; int startCol = col;
        pos += 2; col += 2;
        while (pos + 1 < source.size() && !(source[pos] == '*' && source[pos + 1] == '/')) {
            if (source[pos] == '\n') { ++line; col = 1; }
            else ++col;
            ++pos;
        }
        pos += 2; col += 2;
        tokens.emplace_back(TokenType::MultiLineComment, source.substr(start, pos - start), line, startCol);
    }

    void tokenizeIdentifier() {
        size_t start = pos; int startCol = col;
        while (pos < source.size() && (std::isalnum(source[pos]) || source[pos] == '_')) { ++pos; ++col; }
        std::string word = source.substr(start, pos - start);
        TokenType type = getTokenType(word);
        tokens.emplace_back(type, word, line, startCol);
    }

    void tokenizeNumber() {
        size_t start = pos; bool dot = false; int startCol = col;
        while (pos < source.size() && (std::isdigit(source[pos]) || source[pos] == '.')) {
            if (source[pos] == '.') dot = true;
            ++pos; ++col;
        }
        tokens.emplace_back(dot ? TokenType::Float : TokenType::Number, source.substr(start, pos - start), line, startCol);
    }

    void tokenizeOperator() {
        tokens.emplace_back(TokenType::Operator, std::string(1, source[pos]), line, col);
        ++pos; ++col;
    }

    TokenType getTokenType(const std::string& word) {
        if (word == "Capsule") return TokenType::Capsule;
        if (word == "DG") return TokenType::DGBlock;
        if (word == "eval") return TokenType::Eval;
        if (word == "const") return TokenType::Constant;
        static std::set<std::string> keywords = {
            "if", "else", "loop", "define", "export", "asm", "return", "fn", "let"
        };
        return keywords.count(word) ? TokenType::Keyword : TokenType::Identifier;
    }

    void printTokens() const {
        for (const auto& t : tokens) t.print();
    }

    const std::vector<Token>& getTokens() const { return tokens; }

private:
    char peek() const { return pos + 1 < source.size() ? source[pos + 1] : '\0'; }
    void advance(bool newline = false) {
        if (newline) { ++line; col = 1; }
        else ++col;
        ++pos;
    }
};

//--------------------------------------------
// SIMPLE PARSER (prints identifiers and function headers)
//--------------------------------------------
class Parser {
    const std::vector<Token>& tokens;
    size_t current = 0;
public:
    Parser(const std::vector<Token>& t) : tokens(t) {}

    void parse() {
        while (!match(TokenType::EOFToken)) {
            if (match(TokenType::Keyword) && peek().value == "fn") parseFunction();
            else advance();
        }
    }

    void parseFunction() {
        Token fnToken = previous();
        if (!match(TokenType::Identifier)) return;
        std::string name = previous().value;
        std::cout << "[Function Decl] " << name << " at line " << fnToken.line << "\n";
    }

private:
    const Token& advance() { if (!isAtEnd()) current++; return previous(); }
    bool match(TokenType type) { if (check(type)) { advance(); return true; } return false; }
    bool check(TokenType type) const { return !isAtEnd() && tokens[current].type == type; }
    bool isAtEnd() const { return tokens[current].type == TokenType::EOFToken; }
    const Token& peek() const { return tokens[current]; }
    const Token& previous() const { return tokens[current - 1]; }
};

//--------------------------------------------
// REPL + PARSER INTEGRATION
//--------------------------------------------
void runREPL() {
    std::string input;
    std::cout << "> QuarterLang REPL Ready\n";
    while (true) {
        std::cout << ">> ";
        if (!std::getline(std::cin, input) || input == "exit") break;

        Lexer lexer(input);
        lexer.tokenize();
        lexer.printTokens();

        Parser parser(lexer.getTokens());
        parser.parse();
    }
}

int main() {
    runREPL();
    return 0;
}

