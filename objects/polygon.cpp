#include <cmath>
#include <cstring>

#include <include/raygui.h>
#include <include/raymath.h>

#include <object.h>
#include <objects/group.h>
#include <objects/polygon.h>

static const int defaultSides = 5;
static const float defaultRadius = 10.0f;
static const float defaultRotation = 0.0f;

static const int rotSpeedRandomValueMin = -2;
static const int rotSpeedRandomValueMax = 2;
static const float defaultRotSpeed = 1.0f;

static const int defaultTrailValue = 0;

static const std::string defaultName = "Polygon";

Polygon::Polygon(Vector2 pos, int sides, float r, Color c)
    : Object(pos, c), sides(sides), radius(r), originalRadius(r) {
    rotation = defaultRotation;
    rotSpeed = (float)GetRandomValue(rotSpeedRandomValueMin, rotSpeedRandomValueMax);
    showTrail = trailHead = trailCount = defaultTrailValue;
    baseName = defaultName;
}

Polygon::Polygon() : Object() {
    sides = defaultSides;
    rotation = defaultRotation;
    rotSpeed = defaultRotSpeed;
    radius = originalRadius = defaultRadius;
    showTrail = trailHead = trailCount = defaultTrailValue;
    baseName = defaultName;
}

Polygon::~Polygon() = default;

Object *Polygon::clone() {
    Polygon *copy = new Polygon(position, sides, radius, color);
    copy->velocity = this->velocity;
    copy->rotation = this->rotation;
    copy->rotSpeed = this->rotSpeed;
    copy->showTrail = this->showTrail;
    copy->name = this->name;
    return copy;
}

void Polygon::update() {
    position.x += velocity.x;
    position.y += velocity.y;

    rotation += rotSpeed;

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    if (position.x > sw + radius)
        position.x = -radius;
    else if (position.x < -radius)
        position.x = sw + radius;

    if (position.y > sh + radius)
        position.y = -radius;
    else if (position.y < -radius)
        position.y = sh + radius;

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

void Polygon::draw() {
    if (!visible) return;

    if (showTrail) {
        for (int i = 0; i < trailCount; i++) {
            DrawPoly(trail[i], sides, 5.0f, rotation, Fade(color, 0.4f));
        }
    }

    DrawPoly(position, sides, radius, rotation, color);

    if (radius < originalRadius) {
        DrawPolyLines(position, sides, radius, rotation, WHITE);
        DrawPoly(position, sides, radius * 0.9f, rotation, Fade(WHITE, 0.5f));
    }

    if (selected) {
        DrawPolyLines(position, sides, radius + 4.0f, rotation, GREEN);
    }
}

bool Polygon::contains(Vector2 point) {
    if (!visible) return false;
    return CheckCollisionPointCircle(point, position, radius);
}

float Polygon::getRadius() { return radius; }

bool Polygon::checkCollision(Object *other) {
    Group *otherGroup = dynamic_cast<Group *>(other);
    if (otherGroup != nullptr) { return otherGroup->checkCollision(this); }

    return CheckCollisionCircles(position, radius, other->getPos(),
                                 other->getRadius());
}

void Polygon::deform() {
    if (radius <= 10.0f) return;
    radius -= 1.0f;
}

void Polygon::reset() {
    Object::reset();
    radius = originalRadius;
    trailCount = 0;
}

void Polygon::toggleTrail() {
    showTrail = !showTrail;
    trailCount = 0;
}

void Polygon::setVisible(bool v) { this->visible = v; }

Object *Polygon::drawEditUI(float startX, float startY) {
    if (parentGroup != nullptr) {
        if (GuiButton({ startX, startY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT}, "Back")) {
            return parentGroup;
        }
        startY += 30;
    }

    Object::drawEditUI(startX, startY);

    // update buffers
    if (!editModeRadius) snprintf(textBufRadius, sizeof(textBufRadius), "%.2f", this->radius);
    if (!editModeRotation) snprintf(textBufRotation, sizeof(textBufRotation), "%.2f", this->rotation);
    if (!editModeRotSpeed) snprintf(textBufRotSpeed, sizeof(textBufRotSpeed), "%.2f", this->rotSpeed);

    GuiLabel({ startX, cursorY, LABEL_TITLE_WIDTH, LABEL_TITLE_HEIGHT}, "--- Polygon ---");
    cursorY += ROW_SPACING;

    GuiCheckBox({ startX, cursorY, CHECKBOX_SIZE, CHECKBOX_SIZE }, "Show Trail", &this->showTrail);
    GuiLabel({ startX + COL_OFFSET*2, cursorY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT}, "Sides");
    cursorY += ROW_SPACING;

    if (GuiValueBox({ startX + COL_OFFSET*2, cursorY, VALUE_BOX_WIDTH, ELEMENT_HEIGHT}, NULL, &this->sides, 3, 100, editModeSides)) {
        editModeSides = !editModeSides;
    }
    cursorY += ROW_SPACING + SECTION_PADDING;

    GuiLabel({ startX, cursorY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT}, "Radius");
    GuiLabel({ startX + COL_OFFSET, cursorY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT}, "Rotation");
    GuiLabel({ startX + COL_OFFSET*2, cursorY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT}, "RotSpeed");
    cursorY += ROW_SPACING;

    if (GuiValueBoxFloat({ startX, cursorY, VALUE_BOX_WIDTH, ELEMENT_HEIGHT }, NULL, textBufRadius, &this->radius, editModeRadius)) {
        editModeRadius = !editModeRadius;
    }
    if (GuiValueBoxFloat({ startX + COL_OFFSET, cursorY, VALUE_BOX_WIDTH, ELEMENT_HEIGHT }, NULL, textBufRotation, &this->rotation, editModeRotation)) {
        editModeRotation = !editModeRotation;
    }
    if (GuiValueBoxFloat({ startX + COL_OFFSET*2, cursorY, VALUE_BOX_WIDTH, ELEMENT_HEIGHT }, NULL, textBufRotSpeed, &this->rotSpeed, editModeRotSpeed)) {
        editModeRotSpeed = !editModeRotSpeed;
    }

    return this;
}

// serialization pattern:               POLYGON // OBJECT:[ props ] DATA:[ s;r;or;rot;rots;t;tc;[(tx;ty)];show ]
// int sides;                           s - sides
// float radius;                        r - radius
// float originalRadius;                or - originalRadius
// float rotation;                      rot - rotation
// float rotSpeed;                      rots - rotSpeed
// Vector2 trail[count];                [(tx;ty)] - trail array
// int trailHead, trailCount;           t - trailHead & tc - trailCount
// bool showTrail;                      show - showTrail
// NOTE: separators arent strictly as specified in pattern
// but must be provided because serializer will skip those
// places automatically regardless of the character

void Polygon::serialize(std::ostream &os) const {
    os << POLYGON_PREFIX << " " << TYPE_SEP << " ";
    Object::serialize(os);

    os << " ";

    os << OBJ_DATA_PREFIX << PREFIX_SEP << OBJ_DATA_OPEN << " ";
    os << sides << ";" << radius << ";" << originalRadius << ";";
    os << rotation << ";" << rotSpeed << ";";
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

void Polygon::deserialize(std::istream &is) {
    char d; // tmp

    is >> d >> d;
    
    Object::deserialize(is);

    std::string temp;
    std::getline(is, temp, ':');
    is >> d;

    is >> sides >> d >> radius >> d >> originalRadius >> d;
    is >> rotation >> d >> rotSpeed >> d;
    is >> trailHead >> d >> trailCount >> d;
    is >> d;
    for(int i = 0; i < trailCount; i++) {
        is >> d >> trail[i].x >> d >> trail[i].y >> d;
        if(i < trailCount - 1) is >> d;
    }
    is >> d >> d;
    is >> showTrail >> d;
}
