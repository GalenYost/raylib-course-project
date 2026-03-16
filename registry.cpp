#include <registry.h>
#include <raygui.h>

#include <limits>
#include <sstream>

#include <circle.h>
#include <polygon.h>
#include <rect.h>
#include <triangle.h>
#include <group.h>

ObjectRegistry *ObjectRegistry::instance = nullptr;
ObjectRegistry::ObjectRegistry() = default;

ObjectRegistry *ObjectRegistry::getInstance() {
    if (instance == nullptr) {
        instance = new ObjectRegistry;
    }
    return instance;
}

void ObjectRegistry::add(const std::string &name, Object *obj) {
    prototypes[name] = obj;
}

void ObjectRegistry::remove(const std::string &name) {
    prototypes.erase(name);
}

Object *ObjectRegistry::clone(const std::string &name) {
    return prototypes[name]->clone();
}

Object *ObjectRegistry::get(const std::string &name) {
    return prototypes[name];
}

std::map<std::string, Object *> &ObjectRegistry::getMap() {
    return prototypes;
}

std::istream &operator>>(std::istream &is, ObjectRegistry &r) {
    int count;
    is >> count;
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < count; i++) { 
        std::string name;
        while (std::getline(is, name) && name.empty()) {}
        if (name.empty()) break;

        std::string line; 
        if (!std::getline(is, line)) break;

        std::stringstream ss(line); 
        std::string type; 
        ss >> type;  

        Object* child = nullptr; 
        bool isMultiline = false; 
        
        if (type == RECTANGLE_PREFIX) child = new Rect(); 
        else if (type == CIRCLE_PREFIX) child = new Circle(); 
        else if (type == TRIANGLE_PREFIX) child = new Triangle(); 
        else if (type == POLYGON_PREFIX) child = new Polygon(); 
        else if (type == GROUP_PREFIX) { 
            child = new Group(); 
            isMultiline = true; 
        } 

        if (child) { 
            if (isMultiline) is >> *child; 
            else ss >> *child; 
            
            child->setName(name);
            if (r.prototypes.count(name)) {
                delete r.prototypes[name];
            }
            r.prototypes[name] = child; 
        } 
    } 
    return is;
}

std::ostream &operator<<(std::ostream &os, const ObjectRegistry &r) {
    os << r.prototypes.size() << std::endl;
    for (auto const &pair : r.prototypes) {
        os << pair.first << std::endl;
        os << *pair.second << std::endl;
    }
    return os;
}
