#include "utils/lexer.h"
#include <memory>
#include <stdexcept>
#include <utils/parser.h>

Token Parser::peek() const {
    if (pos >= tokens.len()) return tokens[tokens.len() - 1];
    return tokens[pos];
}

Token Parser::prev() const {
    return tokens[pos - 1];
}

bool Parser::check(TokenType type) const {
    if (peek().type == TokenType::TOKEN_EOF) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        pos++; return true;
    }
    return false;
}

Token Parser::consume(TokenType type, const char *err) {
    if (check(type)) {
        return tokens[pos++];
    }
    throw std::runtime_error(err);
}

std::unique_ptr<Statement> Parser::parse_statement() {
    if (match(TokenType::TOKEN_EOF)) {
        throw std::runtime_error("Syntax Error: Unexpected End of File. A loop or branch is missing its body.");
    }

    if (match(TokenType::IDENTIFIER)) {
        std::string name = prev().name;

        if (match(TokenType::LPAREN)) {
            Vector<std::unique_ptr<Expression>> args;

            if (peek().type != TokenType::RPAREN) {
                args.push(parse_expression());
                while (match(TokenType::COMMA)) {
                    args.push(parse_expression());
                }
            }

            consume(TokenType::RPAREN, "Expected ')' after arguments");

            std::unique_ptr<Expression> expr = std::make_unique<Call>(name, std::move(args));
            return std::make_unique<ExprStatement>(std::move(expr));
        }

        consume(TokenType::EQUALS, "Expected '=' after identifier");

        std::unique_ptr<Expression> value = parse_expression();
        return std::make_unique<Assign>(name, std::move(value));
    }

    if (match(TokenType::FN_KEYWORD)) {
        consume(TokenType::IDENTIFIER, "Expected function name");
        std::string name = prev().name;

        consume(TokenType::LPAREN, "Expected '(' after function name");

        Vector<std::string> params;
        if (peek().type != TokenType::RPAREN) {
            consume(TokenType::IDENTIFIER, "Expected parameter name");
            params.push(prev().name);
            
            while (match(TokenType::COMMA)) {
                consume(TokenType::IDENTIFIER, "Expected parameter name");
                params.push(prev().name);
            }
        }
        
        consume(TokenType::RPAREN, "Expected ')' after parameters");
        consume(TokenType::LBRACE, "Expected '{' before function body");
        
        Vector<std::unique_ptr<Statement>> stmts;
        while (peek().type != TokenType::RBRACE && peek().type != TokenType::TOKEN_EOF) {
            stmts.push(parse_statement());
        }
        
        consume(TokenType::RBRACE, "Expected '}' after function body");

        std::unique_ptr<Block> body = std::make_unique<Block>(std::move(stmts));
        return std::make_unique<FunctionDeclaration>(name, std::move(params), std::move(body));
    }

    if (match(TokenType::WHILE)) {
        std::unique_ptr<Expression> cond = parse_expression();
        std::unique_ptr<Statement> body = parse_statement();
        return std::make_unique<WhileLoop>(std::move(cond), std::move(body));
    }

    if (match(TokenType::PRINT)) {
        std::unique_ptr<Expression> expr = parse_expression();
        return std::make_unique<Print>(std::move(expr));
    }

    if (match(TokenType::LBRACE)) {
        Vector<std::unique_ptr<Statement>> stmts;
        
        while (peek().type != TokenType::RBRACE && peek().type != TokenType::TOKEN_EOF) {
            stmts.push(parse_statement());
        }
        
        consume(TokenType::RBRACE, "Expected '}' after block");
        return std::make_unique<Block>(std::move(stmts));
    }

    if (match(TokenType::IF)) {
        std::unique_ptr<Expression> cond = parse_expression();

        consume(TokenType::LBRACE, "Expected '{' after if condition");
        
        Vector<std::unique_ptr<Statement>> if_stmts;
        while (peek().type != TokenType::RBRACE && peek().type != TokenType::TOKEN_EOF) {
            if_stmts.push(parse_statement());
        }
        
        consume(TokenType::RBRACE, "Expected '}' after if block");

        std::unique_ptr<Block> if_body = std::make_unique<Block>(std::move(if_stmts));
        std::unique_ptr<Block> else_body = nullptr;
        
        if (match(TokenType::ELSE)) {
            consume(TokenType::LBRACE, "Expected '{' after else");
            
            Vector<std::unique_ptr<Statement>> else_stmts;
            while (peek().type != TokenType::RBRACE && peek().type != TokenType::TOKEN_EOF) {
                else_stmts.push(parse_statement());
            }
            
            consume(TokenType::RBRACE, "Expected '}' after else block");
            else_body = std::make_unique<Block>(std::move(else_stmts));
        }

        return std::make_unique<IfElse>(std::move(cond), std::move(if_body), std::move(else_body));
    }

    throw std::runtime_error("Unknown statement");
}

std::unique_ptr<Expression> Parser::parse_expression() {
    return parse_equality();
}

std::unique_ptr<Expression> Parser::parse_primary() {
    if (match(TokenType::NUMBER)) {
        return std::make_unique<Literal>(std::stoi(prev().name));
    } else if (match(TokenType::FLOAT)) {
        return std::make_unique<Literal>(std::stof(prev().name));
    } else if (match(TokenType::STRING_LIT)) {
        return std::make_unique<Literal>(prev().name);
    } else if (match(TokenType::TRUE)) {
        return std::make_unique<Literal>(true);
    } else if (match(TokenType::FALSE)) {
        return std::make_unique<Literal>(false);
    } else if (match(TokenType::NONE)) {
        return std::make_unique<Literal>(std::monostate {});
    }

    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<Variable>(prev().name);
    }

    throw std::runtime_error("Syntax Error: Expected an expression.");
}

std::unique_ptr<Expression> Parser::parse_equality() {
    std::unique_ptr<Expression> expr = parse_comparison();

    while (match(TokenType::EQ_EQ)) {
        Token op = prev();
        std::unique_ptr<Expression> right = parse_comparison();
        expr = std::make_unique<BinOp>(std::move(expr), op.type, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parse_comparison() {
    std::unique_ptr<Expression> expr = parse_term();

    while (match(TokenType::MORE) || match(TokenType::MORE_EQ) ||
           match(TokenType::LESS) || match(TokenType::LESS_EQ)) {
        Token op = prev();
        std::unique_ptr<Expression> right = parse_term();
        expr = std::make_unique<BinOp>(std::move(expr), op.type, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parse_term() {
    std::unique_ptr<Expression> expr = parse_factor();

    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        Token op = prev();
        std::unique_ptr<Expression> right = parse_factor();
        expr = std::make_unique<BinOp>(std::move(expr), op.type, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parse_factor() {
    std::unique_ptr<Expression> expr = parse_primary();

    while (match(TokenType::MULTIPLY) || match(TokenType::DIVIDE)) {
        Token op = prev();
        std::unique_ptr<Expression> right = parse_primary();
        expr = std::make_unique<BinOp>(std::move(expr), op.type, std::move(right));
    }

    return expr;
}

Parser::Parser(Vector<Token> t) : tokens(std::move(t)) {}

Vector<std::unique_ptr<Statement>> Parser::parse_all() {
    Vector<std::unique_ptr<Statement>> statements;
    while (peek().type != TokenType::TOKEN_EOF) {
        std::unique_ptr<Statement> stmt = parse_statement();
        statements.push(std::move(stmt));
    }
    return statements;
}
