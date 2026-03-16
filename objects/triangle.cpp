#include <cstdio>
#include <object.h>
#include <triangle.h>
#include <group.h>

#include <cmath>
#include <cstring>
#include <raymath.h>
#include <raygui.h>

static const float defaultSize = 12.0f;
static const int defaultTrailValue = 0;
static const std::string defaultTriangleName = "Triangle";

Triangle::Triangle(Vector2 pos, float s, Color c)
    : Object(pos, c), size(s), originalSize(s) {
    showTrail = trailHead = trailCount = defaultTrailValue;
    baseName = defaultTriangleName;
}

Triangle::Triangle() : Object() {
    size = originalSize = defaultSize;
    showTrail = trailHead = trailCount = defaultTrailValue;
    baseName = defaultTriangleName;
}

Triangle::~Triangle() = default;

Object *Triangle::clone() {
    Triangle *copy = new Triangle(position, size, color);
    copy->velocity = this->velocity;
    copy->showTrail = this->showTrail;
    copy->name = this->name;
    return copy;
}

void Triangle::update() {
    position.x += velocity.x;
    position.y += velocity.y;

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    if (position.x > sw + size)
        position.x = -size;
    else if (position.x < -size)
        position.x = sw + size;

    if (position.y > sh + size)
        position.y = -size;
    else if (position.y < -size)
        position.y = sh + size;

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

void Triangle::draw() {
    if (!visible) return;

    Vector2 v1 = {position.x, position.y - size};
    Vector2 v2 = {position.x - size * 0.866f, position.y + size * 0.5f};
    Vector2 v3 = {position.x + size * 0.866f, position.y + size * 0.5f};

    if (showTrail) {
        for (int i = 0; i < trailCount; i++) {
            DrawCircleV(trail[i], 2.0f, Fade(color, 0.4f));
        }
    }

    DrawTriangle(v1, v2, v3, color);

    if (size < originalSize) { DrawTriangle(v1, v2, v3, Fade(WHITE, 0.5f)); }

    if (selected) {
        DrawTriangleLines(v1, v2, v3, GREEN);
        DrawCircleV(v1, 3, GREEN);
    }
}

bool Triangle::contains(Vector2 point) {
    if (!visible) return false;

    Vector2 v1 = {position.x, position.y - size};
    Vector2 v2 = {position.x - size * 0.866f, position.y + size * 0.5f};
    Vector2 v3 = {position.x + size * 0.866f, position.y + size * 0.5f};

    return CheckCollisionPointTriangle(point, v1, v2, v3);
}

float Triangle::getRadius() { return size; }

bool Triangle::checkCollision(Object *other) {
    Group *otherGroup = dynamic_cast<Group *>(other);
    if (otherGroup != nullptr) { return otherGroup->checkCollision(this); }

    return CheckCollisionCircles(position, size * 0.6f, other->getPos(),
                                 other->getRadius());
}

void Triangle::deform() {
    if (size <= 5.0f) return;
    size -= 1.0f;
}

void Triangle::reset() {
    Object::reset();
    size = originalSize;
    trailCount = 0;
}

void Triangle::toggleTrail() {
    showTrail = !showTrail;
    trailCount = 0;
}

void Triangle::setVisible(bool v) { this->visible = v; }

Object *Triangle::drawEditUI(float startX, float startY) {
    if (parentGroup != nullptr) {
        if (GuiButton({ startX, startY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT}, "Back")) {
            return parentGroup;
        }
        startY += 30;
    }

    Object::drawEditUI(startX, startY);

    // update buffers
    if (!editModeSize) snprintf(textBufSize, sizeof(textBufSize), "%.2f", this->size);

    GuiLabel({ startX, cursorY, LABEL_TITLE_WIDTH, LABEL_TITLE_HEIGHT}, "--- Triangle ---");
    cursorY += ROW_SPACING;

    GuiCheckBox({ startX, cursorY, CHECKBOX_SIZE, CHECKBOX_SIZE }, "Show Trail", &this->showTrail);
    cursorY += ROW_SPACING + SECTION_PADDING;

    GuiLabel({ startX, cursorY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT}, "Size");
    cursorY += ROW_SPACING;

    if (GuiValueBoxFloat({ startX, cursorY, VALUE_BOX_WIDTH, ELEMENT_HEIGHT }, NULL, textBufSize, &this->size, editModeSize)) {
        editModeSize = !editModeSize;
    }

    return this;
}

// serialization pattern:               TRIANGLE // OBJECT:[ props ] DATA:[ s;os;t;tc;[(tx;ty)];show ]
// float size;                          s - size
// float originalSize;                  os - originalSize
// Vector2 trail[count];                [(tx;ty)] - trail array
// int trailHead, trailCount;           t - trailHead & tc - trailCount
// bool showTrail;                      show - showTrail
// NOTE: separators arent strictly as specified in pattern
// but must be provided because serializer will skip those
// places automatically regardless of the character

void Triangle::serialize(std::ostream &os) const {
    os << TRIANGLE_PREFIX << " " << TYPE_SEP << " ";
    Object::serialize(os);

    os << " ";

    os << OBJ_DATA_PREFIX << PREFIX_SEP << OBJ_DATA_OPEN << " ";
    os << size << ";" << originalSize << ";";
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

void Triangle::deserialize(std::istream &is) {
    char d; // tmp

    is >> d >> d;
    
    Object::deserialize(is);

    std::string temp;
    std::getline(is, temp, ':');
    is >> d;

    is >> size >> d >> originalSize >> d;
    is >> trailHead >> d >> trailCount >> d;
    is >> d;
    for(int i = 0; i < trailCount; i++) {
        is >> d >> trail[i].x >> d >> trail[i].y >> d;
        if(i < trailCount - 1) is >> d;
    }
    is >> d >> d;
    is >> showTrail >> d;
}
