#pragma once

#include <string>
#include <utils/vec.h>

enum class TokenType {
    // types
    NUMBER, FLOAT,
    TRUE, FALSE,
    STRING_LIT, NONE,

    // keywords
    IDENTIFIER, FN_KEYWORD,
    WHILE, PRINT, IF, ELSE,

    // operations
    PLUS, MINUS,
    EQUALS,
    MULTIPLY, DIVIDE,

    // specials
    LBRACE, RBRACE,
    LPAREN, RPAREN,
    COMMA,
    MORE, LESS,
    MORE_EQ, LESS_EQ,
    EQ_EQ,

    TOKEN_EOF,
    ERROR
};

struct Token {
    TokenType type;
    std::string name;
    size_t line;
    size_t col;

    Token() : type(TokenType::ERROR), name(""), line(0), col(0) {}

    Token(TokenType type, std::string lexeme, size_t line, size_t col) 
        : type(type), name(std::move(lexeme)), line(line), col(col) {}
};

class Lexer {
    private:
        std::string source;
        size_t pos = 0;
        size_t line = 1;

        char peek() const;
        char move();
        void skipSpace();

    public:
        Lexer(std::string src);
        Vector<Token> tokenize();
};
