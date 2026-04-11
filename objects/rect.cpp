#include <cstdio>
#include <cstring>
#include <cmath>
#include <iostream>

#include <object.h>
#include <objects/rect.h>
#include <objects/group.h>

#include <include/raymath.h>
#include <include/raygui.h>

static const float defaultWidth = 10.0f;
static const float defaultHeight = 10.0f;
static const std::string defaultRectName = "Rectangle";

static const int defaultTrailValue = 0;

Rect::Rect(Vector2 pos, float width, float height, Color c)
    : Object(pos, c), width(width), height(height), originalWidth(width),
      originalHeight(height) {
    showTrail = trailHead = trailCount = defaultTrailValue;
    baseName = defaultRectName;
}

Rect::Rect() : Object() {
    width = originalWidth = defaultWidth;
    height = originalHeight = defaultHeight;
    showTrail = trailHead = trailCount = defaultTrailValue;
    baseName = defaultRectName;
}

Rect::~Rect() = default;

Object *Rect::clone() {
    Rect *copy = new Rect(position, width, height, color);
    copy->velocity = this->velocity;
    copy->showTrail = this->showTrail;
    copy->name = this->name;
    return copy;
}

void Rect::update() {
    position.x += velocity.x;
    position.y += velocity.y;

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    float halfW = width / 2.0f;
    float halfH = height / 2.0f;

    if (position.x > sw + halfW)
        position.x = -halfW;
    else if (position.x < -halfW)
        position.x = sw + halfW;

    if (position.y > sh + halfH)
        position.y = -halfH;
    else if (position.y < -halfH)
        position.y = sh + halfH;

    if (showTrail) {
        if (trailCount == 0 ||
            Vector2Distance(trail[trailHead == 0 ? 49 : trailHead - 1],
                            position) > 5.0f) {
            trail[trailHead] = position;
            trailHead = (trailHead + 1) % 50;
            if (trailCount < 50) trailCount++;
        }
    }
}

void Rect::draw() {
    if (!visible) return;

    if (showTrail) {
        for (int i = 0; i < trailCount; i++) {
            DrawRectangle(trail[i].x - 3, trail[i].y - 3, 6, 6,
                          Fade(color, 0.4f));
        }
    }

    DrawRectangle(position.x - width / 2, position.y - height / 2, width,
                  height, color);

    if (width < originalWidth && height < originalHeight) {
        DrawRectangleLines(position.x - width / 2, position.y - height / 2,
                           width, height, color);
    }

    if (selected) {
        DrawRectangleLines(position.x - width / 2 - 3,
                           position.y - height / 2 - 3, width + 6, height + 6,
                           GREEN);
    }
}

bool Rect::contains(Vector2 point) {
    if (!visible) return false;

    Rectangle rec = {position.x - width / 2.0f, position.y - height / 2.0f,
                     width, height};

    return CheckCollisionPointRec(point, rec);
}

float Rect::getRadius() {
    float hypot = std::sqrt((std::pow(width, 2) + std::pow(height, 2)));
    return hypot / 2;
}

void Rect::deform() {
    if (width <= 15.0f || height <= 15.0f) return;

    width -= 1.0f;
    height -= 1.0f;
}

void Rect::reset() {
    Object::reset();
    width = originalWidth;
    height = originalHeight;
    trailCount = 0;
}

bool Rect::checkCollision(Object *other) {
    Group *otherGroup = dynamic_cast<Group *>(other);

    if (otherGroup != nullptr) { return otherGroup->checkCollision(this); }

    Rectangle myRect = {position.x - width / 2.0f, position.y - height / 2.0f,
                        width, height};

    return CheckCollisionCircleRec(other->getPos(), other->getRadius(), myRect);
}

void Rect::toggleTrail() {
    showTrail = !showTrail;
    trailCount = 0;
}

Object *Rect::drawEditUI(float startX, float startY) {
    if (parentGroup != nullptr) {
        if (GuiButton({ startX, startY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT}, "Back")) {
            return parentGroup;
        }
        startY += 30;
    }

    Object::drawEditUI(startX, startY);

    // update buffers
    if (!editModeWidth) snprintf(textBufWidth, sizeof(textBufWidth), "%.2f", this->width);
    if (!editModeHeight) snprintf(textBufHeight, sizeof(textBufHeight), "%.2f", this->height);

    GuiLabel({ startX, cursorY, LABEL_TITLE_WIDTH, LABEL_TITLE_HEIGHT}, "--- Rectangle ---");
    cursorY += ROW_SPACING;

    GuiCheckBox({ startX, cursorY, CHECKBOX_SIZE, CHECKBOX_SIZE }, "Show Trail", &this->showTrail);
    cursorY += ROW_SPACING + SECTION_PADDING;

    GuiLabel({ startX, cursorY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT}, "Width");
    GuiLabel({ startX + COL_OFFSET, cursorY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT}, "Height");
    cursorY += ROW_SPACING - SECTION_PADDING;

    if (GuiValueBoxFloat({ startX, cursorY, VALUE_BOX_WIDTH, ELEMENT_HEIGHT }, NULL, textBufWidth, &this->width, editModeWidth)) {
        editModeWidth = !editModeWidth;
    }
    if (GuiValueBoxFloat({ startX + COL_OFFSET, cursorY, VALUE_BOX_WIDTH, ELEMENT_HEIGHT }, NULL, textBufHeight, &this->height, editModeHeight)) {
        editModeHeight = !editModeHeight;
    }

    return this;
}

// serialization pattern for rectangle: RECTANGLE // OBJECT:[ props ] DATA:[ w:h;ow:oh;t;tc;[(tx;ty)];show ]
// float width, originalWidth;          w - width & ow - originalWidth
// float height, originalHeight;        h - height & oh - originalHeight
// Vector2 trail[count];                [(tx;ty)] - trail array
// int trailHead, trailCount;           t - trailHead & tc - trailCount
// bool showTrail;                      show - showTrail
// NOTE: separators arent strictly as specified in pattern
// but must be provided because serializer will skip those
// places automatically regardless of the character

void Rect::serialize(std::ostream &os) const {
    os << RECTANGLE_PREFIX << " " << TYPE_SEP << " ";
    Object::serialize(os);

    os << " ";

    os << OBJ_DATA_PREFIX << PREFIX_SEP << OBJ_DATA_OPEN << " ";
    os << width << ":" << height << ";";
    os << originalWidth << ":" << originalHeight << ";";
    os << trailHead << ";" << trailCount << ";";
    os << "[";

    int safeCount = trailCount; 
    if (safeCount > MAX_TRAIL_COUNT) safeCount = MAX_TRAIL_COUNT;
    if (safeCount < 0) safeCount = 0;
    for(int i = 0; i < safeCount; i++) {
        os << "(" << trail[i].x << ";" << trail[i].y << ")";
        if(i < safeCount - 1) os << ",";
    }
    os << "];";
    os << showTrail << " " << OBJ_DATA_CLOSE;
}
void Rect::deserialize(std::istream &is) {
    char d; // tmp

    is >> d >> d;
    
    Object::deserialize(is);

    std::string temp;
    std::getline(is, temp, ':');
    is >> d;

    is >> width >> d >> height >> d;
    is >> originalWidth >> d >> originalHeight >> d;
    is >> trailHead >> d >> trailCount >> d;
    is >> d;
    for(int i = 0; i < trailCount; i++) {
        is >> d >> trail[i].x >> d >> trail[i].y >> d;
        if(i < trailCount - 1) is >> d;
    }
    is >> d >> d;
    is >> showTrail >> d;
}
