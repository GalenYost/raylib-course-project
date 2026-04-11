#pragma once

#include <cstring>
#include <string>
#include <utils/vec.h>

enum class TokenType {
    NUMBER, FLOAT,
    TRUE, FALSE,
    STRING_LIT, NONE,
    EQUALS,
    IDENTIFIER, FN_KEYWORD,
    PLUS, MINUS,
    MULTIPLY, DIVIDE,
    WHILE, PRINT,
    LBRACE, RBRACE,
    LPAREN, RPAREN,
    COMMA,
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
