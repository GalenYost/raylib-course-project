#pragma once

#include <core/context.h>
#include <utils/lexer.h>
#include <memory>

class Expression {
    public:
        virtual ~Expression() = default;
        virtual Value evaluate(Context &context) const = 0;
};

class Literal : public Expression {
    private:
        Value value;

    public:
        explicit Literal(Value val);
        Value evaluate(Context &context) const override;
};

class Variable : public Expression {
    private:
        std::string name;

    public:
        explicit Variable(std::string val_name);
        Value evaluate(Context &context) const override;
};

class BinOp : public Expression {
    private:
        TokenType type;
        std::unique_ptr<Expression> left;
        std::unique_ptr<Expression> right;

    public:
        explicit BinOp(std::unique_ptr<Expression> l, TokenType o, std::unique_ptr<Expression> r);
        Value evaluate(Context &context) const override;
};

class Call : public Expression {
    private:
        std::string name;
        Vector<std::unique_ptr<Expression>> args;

    public:
        Call(std::string n, Vector<std::unique_ptr<Expression>> a);
        Value evaluate(Context &context) const override;
};
