#include <memory>

#include <utils/lexer.h>
#include <core/statements.h>

class Parser {
    private:
        Vector<Token> tokens;
        size_t pos = 0;

        Token peek() const;
        Token prev() const;

        bool check(TokenType type) const;
        bool match(TokenType type);

        Token consume(TokenType type, const char *err);

        std::unique_ptr<Statement> parse_statement();
        std::unique_ptr<Expression> parse_expression();
        std::unique_ptr<Expression> parse_primary();

    public:
        explicit Parser(Vector<Token> t);
        Vector<std::unique_ptr<Statement>> parse_all();
};
