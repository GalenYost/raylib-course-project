#include <core/statements.h>
#include <memory>
#include <variant>
#include <iostream>

static bool is_truthy(const Value &val) {
    if (std::holds_alternative<bool>(val)) return std::get<bool>(val);
    if (std::holds_alternative<int>(val)) return std::get<int>(val) > 0;
    return false;
}

WhileLoop::WhileLoop(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> b) :
    condition(std::move(cond)), body(std::move(b)) {}

void WhileLoop::execute(Context &context) {
    while (is_truthy(condition->evaluate(context))) {
        body->execute(context);
    }
}

Assign::Assign(std::string var_name, std::unique_ptr<Expression> val) :
    name(std::move(var_name)), value(std::move(val)) {}

void Assign::execute(Context &context) {
    Value result = value->evaluate(context);
    context.set(name, result);
}

Print::Print(std::unique_ptr<Expression> e) : expr(std::move(e)) {}

void Print::execute(Context &context) {
    Value val = expr->evaluate(context);

    std::visit([](auto&& v) {
        using T = std::decay_t<decltype(v)>;
        
        if constexpr (std::is_same_v<T, std::monostate>) {
            std::cout << "null\n";
        } else if constexpr (std::is_same_v<T, bool>) {
            std::cout << (v ? "true" : "false") << "\n";
        } else std::cout << v << "\n";
    }, val);
}

Block::Block(Vector<std::unique_ptr<Statement>> stmts) : statements(std::move(stmts)) {}

void Block::execute(Context &context) {
    for (unsigned i = 0; i < statements.len(); i++) {
        statements[i]->execute(context);
    }
}

ExprStatement::ExprStatement(std::unique_ptr<Expression> e) : expr(std::move(e)) {}

void ExprStatement::execute(Context &context) {
    expr->evaluate(context);
}

FunctionDeclaration::FunctionDeclaration(std::string n, Vector<std::string> p, std::unique_ptr<Block> b) :
    name(std::move(n)), params(std::move(p)), body(std::move(b)) {}

void FunctionDeclaration::execute(Context &context) {
    context.define_fn(this->name, this);
}

IfElse::IfElse(std::unique_ptr<Expression> e, std::unique_ptr<Block> if_b, std::unique_ptr<Block> else_b) :
    condition(std::move(e)), if_body(std::move(if_b)), else_body(std::move(else_b)) {}

void IfElse::execute(Context &context) {
    Value cond_val = condition->evaluate(context);

    if (is_truthy(cond_val)) {
        if_body->execute(context);
    } else {
        if (else_body) else_body->execute(context);
    }
}
