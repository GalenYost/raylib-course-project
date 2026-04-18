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

Value BinOp::evaluate(Context& context) const {
    Value left_val = left->evaluate(context);
    Value right_val = right->evaluate(context);

    auto get_num = [](const Value& v) -> float {
        if (std::holds_alternative<int>(v)) return (float)std::get<int>(v);
        return std::get<float>(v);
    };

    bool left_is_num = std::holds_alternative<int>(left_val) || std::holds_alternative<float>(left_val);
    bool right_is_num = std::holds_alternative<int>(right_val) || std::holds_alternative<float>(right_val);

    if (left_is_num && right_is_num) {
        float l = get_num(left_val);
        float r = get_num(right_val);

        switch (type) {
            case TokenType::PLUS: return l + r;
            case TokenType::MINUS: return l - r;
            case TokenType::MULTIPLY: return l * r;
            case TokenType::DIVIDE: return l / r;
            case TokenType::MORE: return l > r;
            case TokenType::MORE_EQ: return l >= r;
            case TokenType::LESS: return l < r;
            case TokenType::LESS_EQ: return l <= r;
            case TokenType::EQ_EQ: return l == r;
            default: break;
        }
    }

    if (type == TokenType::EQ_EQ) {
        if (std::holds_alternative<std::string>(left_val) && std::holds_alternative<std::string>(right_val)) {
            return std::get<std::string>(left_val) == std::get<std::string>(right_val);
        }
        if (std::holds_alternative<bool>(left_val) && std::holds_alternative<bool>(right_val)) {
            return std::get<bool>(left_val) == std::get<bool>(right_val);
        }
    }

    throw std::runtime_error("Invalid operands for binary operation");
}

Call::Call(std::string n, Vector<std::unique_ptr<Expression>> a) : name(n), args(std::move(a)) {}

Value Call::evaluate(Context &context) const {
    Vector<Value> evaluated_args;

    for (unsigned i = 0; i < args.len(); i++) {
        evaluated_args.push(args[i]->evaluate(context));
    }
    
    return context.call(name, evaluated_args);
}
