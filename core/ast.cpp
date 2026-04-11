#include "utils/lexer.h"
#include <core/ast.h>
#include <variant>
#include <stdexcept>

#include <cstring>
#include <cstdlib>

Literal::Literal(Value v) : value(v) {}

Value Literal::evaluate(Context &context) const {
    (void)context;
    return value;
}

Variable::Variable(std::string val_name) : name(val_name) {}

Value Variable::evaluate(Context &context) const {
    return context.get(name);
}

BinOp::BinOp(std::unique_ptr<Expression> l, TokenType o, std::unique_ptr<Expression> r) :
   type(o), left(std::move(l)), right(std::move(r)) {}

static Value exec_add(const Value& lval, const Value& rval) {
    if (std::holds_alternative<int>(lval) && std::holds_alternative<int>(rval)) {
        return std::get<int>(lval) + std::get<int>(rval);
    }
    
    if (std::holds_alternative<float>(lval) && std::holds_alternative<float>(rval)) {
        return std::get<float>(lval) + std::get<float>(rval);
    }

    if (std::holds_alternative<std::string>(lval) && std::holds_alternative<std::string>(rval)) {
        return std::get<std::string>(lval) + std::get<std::string>(rval);
    }

    throw std::runtime_error("Type mismatch in addition.");
}

static Value exec_sub(const Value& lval, const Value& rval) {
    if (std::holds_alternative<int>(lval) && std::holds_alternative<int>(rval)) {
        return std::get<int>(lval) - std::get<int>(rval);
    }
    
    if (std::holds_alternative<float>(lval) && std::holds_alternative<float>(rval)) {
        return std::get<float>(lval) - std::get<float>(rval);
    }

    throw std::runtime_error("Type mismatch in addition.");
}

static Value exec_mult(const Value& lval, const Value& rval) {
    if (std::holds_alternative<int>(lval) && std::holds_alternative<int>(rval)) {
        return std::get<int>(lval) * std::get<int>(rval);
    }
    
    if (std::holds_alternative<float>(lval) && std::holds_alternative<float>(rval)) {
        return std::get<float>(lval) * std::get<float>(rval);
    }

    throw std::runtime_error("Type mismatch in addition.");
}

static Value exec_div(const Value& lval, const Value& rval) {
    if (std::holds_alternative<int>(lval) && std::holds_alternative<int>(rval)) {
        return std::get<int>(lval) / std::get<int>(rval);
    }
    
    if (std::holds_alternative<float>(lval) && std::holds_alternative<float>(rval)) {
        return std::get<float>(lval) / std::get<float>(rval);
    }

    throw std::runtime_error("Type mismatch in addition.");
}

Value BinOp::evaluate(Context &context) const {
    Value lval = left->evaluate(context);
    Value rval = right->evaluate(context);

    switch (type) {
        case TokenType::PLUS: return exec_add(lval, rval);
        case TokenType::MINUS: return exec_sub(lval, rval);
        case TokenType::MULTIPLY: return exec_mult(lval, rval);
        case TokenType::DIVIDE: return exec_div(lval, rval);
        default: throw new std::runtime_error("Unknown operation");
    };
}

Call::Call(std::string n, Vector<std::unique_ptr<Expression>> a) : name(n), args(std::move(a)) {}

Value Call::evaluate(Context &context) const {
    Vector<Value> evaluated_args;

    for (unsigned i = 0; i < args.len(); i++) {
        evaluated_args.push(args[i]->evaluate(context));
    }
    
    return context.call(name, evaluated_args);
}
