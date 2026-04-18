#include <utils/lexer.h>

char Lexer::peek() const {
    if (pos >= source.length()) return '\0';
    return source[pos];
}

char Lexer::move() {
    if (pos >= source.length()) return '\0';
    return source[pos++];
}

void Lexer::skipSpace() {
    while (pos < source.length()) {
        char c = peek();
        if (c == '\n') {
            move();
            line++;
        } else if (c == ' ' || c == '\t' || c == '\r') {
            move();
        } else {
            break;
        }
    }
}

Lexer::Lexer(std::string src) : source(std::move(src)) {}

Vector<Token> Lexer::tokenize() {
    Vector<Token> tokens;

    while (pos < source.length()) {
        skipSpace();
        if (pos >= source.length()) break;

        char c = peek();

        if (isdigit(c)) {
            std::string num;

            while (isdigit(peek())) num += move();

            if (peek() == '.') {
                num += move();
                while (isdigit(peek())) num += move();
                tokens.push(Token(TokenType::FLOAT, num, line, pos));
            } else {
                tokens.push(Token(TokenType::NUMBER, num, line, pos));
            }
            continue;
        }

        if (isalpha(c) || c == '_') {
            std::string ident;
            while (isalnum(peek()) || peek() == '_') ident += move(); 

            if (ident == "while") {
                tokens.push(Token(TokenType::WHILE, ident, line, pos - ident.length()));
            } else if (ident == "print") {
                tokens.push(Token(TokenType::PRINT, ident, line, pos - ident.length()));
            } else if (ident == "true") {
                tokens.push(Token(TokenType::TRUE, ident, line, pos - ident.length()));
            } else if (ident == "false") {
                tokens.push(Token(TokenType::FALSE, ident, line, pos - ident.length()));
            } else if (ident == "null") {
                tokens.push(Token(TokenType::NONE, ident, line, pos - ident.length()));
            } else if (ident == "fn") {
                tokens.push(Token(TokenType::FN_KEYWORD, ident, line, pos - ident.length()));
            } else if (ident == "if") {
                tokens.push(Token(TokenType::IF, ident, line, pos - ident.length()));
            } else if (ident == "else") {
                tokens.push(Token(TokenType::ELSE, ident, line, pos - ident.length()));
            } else {
                tokens.push(Token(TokenType::IDENTIFIER, ident, line, pos - ident.length()));
            }
            continue;
        }

        if (c == '+') {
            tokens.push(Token(TokenType::PLUS, std::string(1, move()), line, pos));
            continue;
        } else if (c == '-') {
            tokens.push(Token(TokenType::MINUS, std::string(1, move()), line, pos));
            continue;
        } else if (c == '*') {
            tokens.push(Token(TokenType::MULTIPLY, std::string(1, move()), line, pos));
            continue;
        } else if (c == '/') {
            tokens.push(Token(TokenType::DIVIDE, std::string(1, move()), line, pos));
            continue;
        } else if (c == '=') {
            move();
            if (peek() == '=') {
                move();
                tokens.push(Token(TokenType::EQ_EQ, "==", line, pos));
            } else {
                tokens.push(Token(TokenType::EQUALS, "=", line, pos));
            }
            continue;
        } else if (c == '{') {
            tokens.push(Token(TokenType::LBRACE, std::string(1, move()), line, pos));
            continue;
        } else if (c == '}') {
            tokens.push(Token(TokenType::RBRACE, std::string(1, move()), line, pos));
            continue;
        } else if (c == '(') {
            tokens.push(Token(TokenType::LPAREN, std::string(1, move()), line, pos));
            continue;
        } else if (c == ')') {
            tokens.push(Token(TokenType::RPAREN, std::string(1, move()), line, pos));
            continue;
        } else if (c == ',') {
            tokens.push(Token(TokenType::COMMA, std::string(1, move()), line, pos));
            continue;
        } else if (c == '>') {
            move();
            if (peek() == '=') {
                move();
                tokens.push(Token(TokenType::MORE_EQ, ">=", line, pos));
            } else {
                tokens.push(Token(TokenType::MORE, ">", line, pos));
            }
            continue;
        } else if (c == '<') {
            move();
            if (peek() == '=') {
                move();
                tokens.push(Token(TokenType::LESS_EQ, "<=", line, pos));
            } else {
                tokens.push(Token(TokenType::LESS, "<", line, pos));
            }
            continue;
        } else if (c == '"') {
            move();
            std::string val = "";
            while (peek() != '"' && peek() != '\0') {
                val += move();
            }
            move();
            tokens.push(Token(TokenType::STRING_LIT, val, line, pos));
            continue;
        }

        tokens.push(Token(TokenType::ERROR, std::string(1, move()), line, pos));
    }

    tokens.push(Token(TokenType::TOKEN_EOF, "", line, pos));
    return tokens;
}
