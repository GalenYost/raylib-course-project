#include <core/context.h>
#include <core/statements.h>
#include <stdexcept>

void Context::set(const std::string &name, Value value) {
    variables[name] = std::move(value);
}

Value Context::get(const std::string &name) {
    if (variables.find(name) != variables.end()) {
        return variables[name];
    }
    throw std::runtime_error("Variable '" + name + "' doesnt exist");
}

void Context::bind(const std::string &name, NativeFn fn) {
    functions[name] = fn;
}

Value Context::call(const std::string &name, Vector<Value> &args) {
    if (functions.find(name) != functions.end()) {
        return functions[name](args);
    }

    if (user_funcs.find(name) != user_funcs.end()) {
        const FunctionDeclaration* fn = user_funcs[name];
        fn->body->execute(*this);
        return std::monostate{}; 
    }

    throw std::runtime_error("Function '" + name + "' not found");
}

bool Context::find(const std::string &name) const {
    return functions.find(name) != functions.end() ||
           user_funcs.find(name) != user_funcs.end();
}

void Context::define_fn(const std::string &name, const FunctionDeclaration* stmt) {
    user_funcs[name] = stmt;
}
