#pragma once

#include <object.h>
#include <map>
#include <string>
#include <iostream>

#define PROTOTYPES_SAVE "prototypes.txt"

class ObjectRegistry {
    private:
        static ObjectRegistry *instance;
        std::map<std::string, Object *> prototypes;

        ObjectRegistry();
    
    public:
        ObjectRegistry(const ObjectRegistry&) = delete;
        void operator=(const ObjectRegistry&) = delete;
        ~ObjectRegistry();

        static ObjectRegistry *getInstance();

        void add(const std::string &name, Object *obj);
        void remove(const std::string &name);
        Object *get(const std::string &name);
        Object *clone(const std::string &name);

        std::map<std::string, Object *> &getMap();

        friend std::istream &operator>>(std::istream &is, ObjectRegistry &s);
        friend std::ostream &operator<<(std::ostream &os, const ObjectRegistry &s);
};
