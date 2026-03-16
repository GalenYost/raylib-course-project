#include "raylib.h"
#include <registry.h>
#include <object.h>
#include <cstring>
#include <cstdio>
#include <scene.h>

#include <raygui.h>

static const Vector2 defaultPos = Vector2 { 0, 0 };
static const Color defaultColor = RED;
static const bool defaultSelected = false;
static const bool defaultVisible = true;

static const int defaultRandomXMinValueVelocity = -3;
static const int defaultRandomXMaxValueVelocity = 3;

static const int defaultRandomYMinValueVelocity = -3;
static const int defaultRandomYMaxValueVelocity = 3;

static const int velocityXDefault = 2;

Object::Object(Vector2 pos, Color c)
    : position(pos), originalPosition(pos),
      color(c), originalColor(c),
      selected(false), visible(true),
      baseName("Object")
{
    velocity = {
        (float)GetRandomValue(defaultRandomXMinValueVelocity, defaultRandomXMaxValueVelocity),
        (float)GetRandomValue(defaultRandomYMinValueVelocity, defaultRandomYMaxValueVelocity)
    };
    if (velocity.x == 0) velocity.x = velocityXDefault;
};

Object::Object() {
    position = originalPosition = defaultPos;
    color = originalColor = defaultColor;
    selected = defaultSelected; visible = defaultVisible;

    velocity = {
        (float)GetRandomValue(defaultRandomXMinValueVelocity, defaultRandomXMaxValueVelocity),
        (float)GetRandomValue(defaultRandomYMinValueVelocity, defaultRandomYMaxValueVelocity)
    };
    if (velocity.x == 0) velocity.x = velocityXDefault;
}

Object::~Object() = default;

void Object::moveManual(Vector2 delta) {
    position.x += delta.x;
    position.y += delta.y;
}

void Object::randomizeColor() {
    color = (Color){(unsigned char)GetRandomValue(50, 250),
                    (unsigned char)GetRandomValue(50, 250),
                    (unsigned char)GetRandomValue(50, 250), 255};
}

void Object::reset() {
    position = originalPosition;
    color = originalColor;
    visible = true;
    selected = false;
}

bool Object::isSelected() const { return selected; }
void Object::setSelected(bool val) { selected = val; }
bool Object::isVisible() const { return visible; }
void Object::toggleVisibility() { visible = !visible; }
Vector2 Object::getPos() const { return position; }
void Object::setVisible(bool visible) { this->visible = visible; }

void Object::setVelocity(Vector2 vel) {
    velocity = vel;
}

std::string Object::getName() const {
    if (name.empty()) return baseName;
    else return name;
}
void Object::setName(const std::string &name) {
    this->name = name;
}

void Object::setPosition(Vector2 newPos) {
    this->position = newPos;
}
void Object::setInitialPos(Vector2 newPos) {
    this->originalPosition = newPos;
}

void Object::setColor(Color c) {
    this->color = c;
}

Object *Object::drawEditUI(float startX, float startY) {
    // update buffers
    if (!editModePosX) snprintf(textBufPosX, sizeof(textBufPosX), "%.2f", this->position.x);
    if (!editModePosY) snprintf(textBufPosY, sizeof(textBufPosY), "%.2f", this->position.y);

    cursorY = startY;

    GuiLabel({ startX, cursorY, LABEL_TITLE_WIDTH, LABEL_TITLE_HEIGHT }, getName().c_str());
    cursorY += ROW_SPACING;

    GuiLabel({ startX, cursorY, LABEL_TITLE_WIDTH, LABEL_TITLE_HEIGHT }, "--- Object ---");
    if (GuiTextBox({ startX + COL_OFFSET*2, cursorY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT * 1.5f }, nameDialogInput, INPUT_BUFFER_SIZE, nameDialogEditMode)) {
        nameDialogEditMode = !nameDialogEditMode;
    }
    cursorY += ROW_SPACING;

    if (GuiButton({ startX + COL_OFFSET*2, cursorY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT}, "Save")) {
        Object *saved = this->clone();
        saved->setSelected(false);

        const std::string name = nameDialogInput;
        saved->setName(name);

        static int regCount = 1;
        ObjectRegistry::getInstance()->getMap()[name] = saved;
    }

    if (GuiButton({ startX, cursorY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT }, "Reset")) {
        this->reset();
    }
    cursorY += ROW_SPACING;

    GuiCheckBox({ startX, cursorY, CHECKBOX_SIZE, CHECKBOX_SIZE }, "Selected", &this->selected);
    GuiCheckBox({ startX + COL_OFFSET, cursorY, CHECKBOX_SIZE, CHECKBOX_SIZE }, "Visible", &this->visible);
    cursorY += ROW_SPACING + SECTION_PADDING;

    GuiLabel({ startX, cursorY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT }, "Position");
    cursorY += ELEMENT_HEIGHT + INNER_PADDING;

    GuiLabel({ startX, cursorY, LABEL_SHORT_WIDTH, ELEMENT_HEIGHT }, "X");
    if (GuiValueBoxFloat({ startX + LABEL_SHORT_WIDTH + INNER_PADDING, cursorY, VALUE_BOX_WIDTH, ELEMENT_HEIGHT }, NULL, textBufPosX, &this->position.x, editModePosX)) {
        editModePosX = !editModePosX;
    }

    GuiLabel({ startX + COL_OFFSET, cursorY, LABEL_SHORT_WIDTH, ELEMENT_HEIGHT }, "Y");
    if (GuiValueBoxFloat({ startX + COL_OFFSET + LABEL_SHORT_WIDTH + INNER_PADDING, cursorY, VALUE_BOX_WIDTH, ELEMENT_HEIGHT }, NULL, textBufPosY, &this->position.y, editModePosY)) {
        editModePosY = !editModePosY;
    }
    cursorY += ROW_SPACING + SECTION_PADDING;

    GuiLabel({ startX, cursorY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT }, "Color");
    cursorY += ELEMENT_HEIGHT + INNER_PADDING;

    GuiColorPicker({ startX, cursorY, COLOR_PICKER_SIZE*2, COLOR_PICKER_SIZE }, NULL, &this->color);
    cursorY += COLOR_PICKER_SIZE + INNER_PADDING;

    if (GuiButton({ startX, cursorY, LABEL_LONG_WIDTH*1.5, ELEMENT_HEIGHT}, "Randomize Color")) {
        randomizeColor();
    }
    cursorY += ELEMENT_HEIGHT + ROW_SPACING;

    return this;
}

std::ostream &operator<<(std::ostream &os, const Object &obj) {
    obj.serialize(os);
    return os;
}
std::istream &operator>>(std::istream &is, Object &obj) {
    obj.deserialize(is);
    return is;
}

// generally pattern for all objects is: TYPE // OBJECT:[ props ] DATA:[ props ]
// p.s for Object class TYPE should be ommited as well as DATA section
// serialization pattern:           OBJECT:[(px;py);(pox;poy);{r:g:b};{or:og:ob};s;v;name]
// Vector2 position;                (px;py) - position 
// Vector2 originalPosition;        (pox;poy) - originalPosition
// Color color;                     {r:g:b} - color
// Color originalColor;             {or:og:ob} - originalColor
// bool selected;                   s - selected
// bool visible;                    v - visible
// std::string name;                name - object name
// NOTE: separators arent strictly as specified in pattern
// but must be provided because serializer will skip those
// places automatically regardless of the character

void Object::serialize(std::ostream &os) const {
    os << OBJECT_PREFIX << PREFIX_SEP << OBJ_DATA_OPEN << " ";
    os << "(" << position.x << ";" << position.y << ");";
    os << "(" << originalPosition.x << ";" << originalPosition.y << ");";
    os << "{" << (int)color.r << ":" << (int)color.g << ":" << (int)color.b << "};";
    os << "{" << (int)originalColor.r << ":" << (int)originalColor.g << ":" << (int)originalColor.b << "};";
    os << selected << ";" << visible << ";";
    os << name << " " << OBJ_DATA_CLOSE;
}
void Object::deserialize(std::istream &is) {
    char d; // tmp
    int r,g,b;

    for (int i = 0; i < strlen(OBJECT_PREFIX); i++) is >> d;
    is >> d >> d;
    is >> d >> position.x >> d >> position.y >> d >> d;
    is >> d >> originalPosition.x >> d >> originalPosition.y >> d >> d;

    is >> d >> r >> d >> g >> d >> b >> d >> d;
    color.r = (unsigned char)r;
    color.g = (unsigned char)g;
    color.b = (unsigned char)b;

    is >> d >> r >> d >> g >> d >> b >> d >> d;
    originalColor.r = (unsigned char)r;
    originalColor.g = (unsigned char)g;
    originalColor.b = (unsigned char)b;

    is >> selected >> d >> visible >> d;
    is >> name >> d;
}
