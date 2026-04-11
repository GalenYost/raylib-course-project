#pragma once

#include <cstring>
#include <cstdlib>

#include <functional>
#include <unordered_map>
#include <string>
#include <variant>

#include <utils/vec.h>

using Value = std::variant<std::monostate, int, bool, std::string, float>;
using NativeFn = std::function<Value(Vector<Value>&)>;

class FunctionDeclaration;

class Context {
    private:
        std::unordered_map<std::string, Value> variables;
        std::unordered_map<std::string, NativeFn> functions;
        std::unordered_map<std::string, const FunctionDeclaration*> user_funcs;

    public:
        void set(const std::string &name, Value value);
        Value get(const std::string &name);

        void bind(const std::string &name, NativeFn fn);
        Value call(const std::string &name, Vector<Value> &args);
        void define_fn(const std::string &name, const FunctionDeclaration* stmt);

        bool find(const std::string &name) const;
};
