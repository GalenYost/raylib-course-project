#pragma once

#include <core/context.h>
#include <core/ast.h>
#include <memory>

class Statement {
    public:
        virtual ~Statement() = default;
        virtual void execute(Context &context) = 0;
};

class WhileLoop : public Statement {
    private:
        std::unique_ptr<Expression> condition;
        std::unique_ptr<Statement> body;

    public:
        explicit WhileLoop(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> b);
        void execute(Context &context) override;
};

class Assign : public Statement {
    private:
        std::string name;
        std::unique_ptr<Expression> value;

    public:
        explicit Assign(std::string var_name, std::unique_ptr<Expression> val);
        void execute(Context &context) override;
};

class Print : public Statement {
    private:
        std::unique_ptr<Expression> expr;

    public:
        Print(std::unique_ptr<Expression> e);
        void execute(Context &context) override;
};

class Block : public Statement {
    private:
        Vector<std::unique_ptr<Statement>> statements;

    public:
        Block(Vector<std::unique_ptr<Statement>> stmts);
        void execute(Context &context) override;
};

class ExprStatement : public Statement {
    private:
        std::unique_ptr<Expression> expr;

    public:
        ExprStatement(std::unique_ptr<Expression> e);
        void execute(Context &context) override;
};

class FunctionDeclaration : public Statement {
    private:
        std::string name;
        Vector<std::string> params;

    public:
        std::unique_ptr<Block> body;

        FunctionDeclaration(std::string n, Vector<std::string> params, std::unique_ptr<Block> b);
        void execute(Context &context) override;
};

class IfElse : public Statement {
    private:
        std::unique_ptr<Expression> condition;

    public:
        std::unique_ptr<Block> if_body;
        std::unique_ptr<Block> else_body;

        IfElse(std::unique_ptr<Expression> e, std::unique_ptr<Block> if_b, std::unique_ptr<Block> else_b);
        void execute(Context &context) override;
};
