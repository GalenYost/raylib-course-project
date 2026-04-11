#include <include/raylib.h>
#include <include/raygui.h>

#include <object.h>
#include <objects/group.h>
#include <objects/circle.h>
#include <objects/polygon.h>
#include <objects/rect.h>
#include <objects/triangle.h>

#include <utils/registry.h>
#include <utils/scene.h>

#include <cstring>
#include <limits>
#include <sstream>

Group::Group() : Object({0, 0}, WHITE) {
    visible = true;
    baseName = "Group";
}
Group::~Group() {
    for (unsigned i = 0; i < children.len(); i++) { delete children[i]; }
}

Vector<Object *> Group::disband() {
    Vector<Object *> stolenChildren = children;
    children = Vector<Object *>();
    return stolenChildren;
}

Vector<Object *> &Group::getMembers() { return children; }

void Group::addChild(Object *obj) {
    obj->parentGroup = this;
    children.push(obj);
}

Object *Group::clone() {
    Group *newGroup = new Group();

    for (unsigned i = 0; i < children.len(); i++) {
        newGroup->addChild(children[i]->clone());
    }

    newGroup->visible = this->visible;
    newGroup->selected = false;
    newGroup->name = this->name;
    return newGroup;
}

void Group::update() {
    if (children.len() == 0) return;

    float sumX = 0;
    float sumY = 0;

    for (unsigned i = 0; i < children.len(); i++) {
        children[i]->update();

        Vector2 childPos = children[i]->getPos();

        sumX += childPos.x;
        sumY += childPos.y;
    }

    this->position = {sumX / children.len(), sumY / children.len()};
}

void Group::draw() {
    if (!visible) return;

    for (unsigned i = 0; i < children.len(); i++) {
        bool wasSelected = children[i]->isSelected();
        if (this->selected) children[i]->setSelected(false);

        children[i]->draw();

        if (this->selected) children[i]->setSelected(wasSelected);
    }

    if (selected && children.len() > 1) {
        Vector2 center = getPos();
        for (unsigned i = 0; i < children.len(); i++) {
            DrawLineV(center, children[i]->getPos(), Fade(GREEN, 0.5f));
        }
        DrawCircleV(center, 4, GREEN);
    }
}

bool Group::contains(Vector2 point) {
    if (!visible) return false;
    for (unsigned i = 0; i < children.len(); i++) {
        if (children[i]->contains(point)) return true;
    }
    return false;
}

float Group::getRadius() { return 30.0f; }

void Group::moveManual(Vector2 delta) {
    for (unsigned i = 0; i < children.len(); i++) children[i]->moveManual(delta);
}

void Group::randomizeColor() {
    for (unsigned i = 0; i < children.len(); i++) children[i]->randomizeColor();
}

void Group::toggleTrail() {
    for (unsigned i = 0; i < children.len(); i++) children[i]->toggleTrail();
}

void Group::deform() {
    for (unsigned i = 0; i < children.len(); i++) children[i]->deform();
}

void Group::reset() {
    Object::reset();
    for (unsigned i = 0; i < children.len(); i++) children[i]->reset();
}

Vector2 Group::getPos() const {
    if (children.len() == 0) return {0, 0};

    float sumX = 0, sumY = 0;
    for (unsigned i = 0; i < children.len(); i++) {
        Vector2 p = children[i]->getPos();
        sumX += p.x;
        sumY += p.y;
    }
    return {sumX / children.len(), sumY / children.len()};
}

bool Group::checkCollision(Object *other) {
    if (!visible) return false;

    for (unsigned i = 0; i < children.len(); i++) {
        if (children[i]->checkCollision(other)) { return true; }
    }
    return false;
}

void Group::setVisible(bool visible) {
    this->visible = visible;
    for (unsigned i = 0; i < children.len(); i++) {
        children[i]->setVisible(visible);
    }
}

void Group::setColor(Color c) {
    this->editUIColor = c;
    for (unsigned i = 0; i < children.len(); i++) {
        children[i]->setColor(c);
    }
}

Object *Group::drawEditUI(float startX, float startY) {
    static int scrollIndex = 0;
    static int activeItem = -1;

    cursorY = startY;

    GuiLabel({ startX, cursorY, LABEL_TITLE_WIDTH, LABEL_TITLE_HEIGHT }, getName().c_str());
    cursorY += ROW_SPACING;

    GuiLabel({ startX, cursorY, LABEL_TITLE_WIDTH, LABEL_TITLE_HEIGHT}, "--- Group ---");

    if (GuiTextBox({ startX + COL_OFFSET*2, cursorY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT * 1.5f }, nameDialogInput, INPUT_BUFFER_SIZE, nameDialogEditMode)) {
        nameDialogEditMode = !nameDialogEditMode;
    }
    cursorY += ROW_SPACING;

    if (GuiButton({ startX + COL_OFFSET*2, cursorY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT}, "Save")) {
        Object *saved = this->clone();
        saved->setSelected(false);

        const std::string name = nameDialogInput;
        if (name.empty()) {
            delete saved;
            return this;
        }
        saved->setName(name);

        auto &map = ObjectRegistry::getInstance()->getMap();
        if (map.find(name) != map.end()) delete map[name];

        map[name] = saved;
    }
    cursorY += ROW_SPACING;

    GuiColorPicker({ startX, cursorY, COLOR_PICKER_SIZE*1.5, COLOR_PICKER_SIZE }, NULL, &this->editUIColor);
    cursorY += COLOR_PICKER_SIZE + INNER_PADDING;

    if (GuiButton({ startX, cursorY, LABEL_LONG_WIDTH*1.5, ELEMENT_HEIGHT}, "Set Color")) {
        this->setColor(editUIColor);
    }
    cursorY += ELEMENT_HEIGHT + SECTION_PADDING;

    if (GuiButton({ startX, cursorY, LABEL_LONG_WIDTH*1.5, ELEMENT_HEIGHT}, "Randomize Color")) {
        randomizeColor();
    }
    cursorY += ROW_SPACING*2;

    std::string listItems = "";
    for (unsigned i = 0; i < children.len(); i++) {
        listItems += children[i]->getName();
        if (i < children.len() - 1) {
            listItems += ";";
        }
    }

    GuiLabel({ startX, cursorY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT}, "Objects");
    cursorY += ROW_SPACING;

    GuiListView({ startX, cursorY, LABEL_TITLE_WIDTH*1.5, LIST_HEIGHT }, listItems.c_str(), &scrollIndex, &activeItem);
    if (activeItem >= 0 && activeItem < (int)children.len()) {
        Object* selectedChild = children[activeItem];
        activeItem = -1;
        return selectedChild;
    }

    return this;
}

// serialization pattern:
// GROUP//
// [[
// size // selected // visible
// <objects>
// ]]

void Group::serialize(std::ostream &os) const {
    os << GROUP_PREFIX << " " << TYPE_SEP << std::endl;
    os << OBJ_DATA_OPEN << OBJ_DATA_OPEN << std::endl;
    os <<
        children.len() << " " << TYPE_SEP << " " <<
        isSelected() << " " << TYPE_SEP << " " <<
        isVisible() << " " << TYPE_SEP << " " <<
        getName() << std::endl;
    for (unsigned i = 0; i < children.len(); i++) {
        os << *children[i];
        os << std::endl;
    }
    os << TYPE_SEP << OBJ_DATA_CLOSE << OBJ_DATA_CLOSE;
}
void Group::deserialize(std::istream &is) {
    char d; // tmp

    is >> d >> d;

    int count, selected, visible;
    is >> count;
    is >> d >> d;
    is >> selected;
    is >> d >> d;
    is >> visible;
    is >> d >> d;
    is >> this->name;

    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < count; i++) {
        std::string line;
        
        if (!std::getline(is, line) || line.empty()) {
            if (line.empty() && !is.eof()) { i--; continue; }
            break;
        }

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

            children.push(child);
        }
    }

    setVisible(visible);
    setSelected(selected);

    std::string dummy;
    std::getline(is, dummy);
}
