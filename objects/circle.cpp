#include <circle.h>
#include <group.h>
#include <raymath.h>
#include <raygui.h>
#include <cstring>

static const float defaultRadius = 10.0f;
static const int defaultTrailValue = 0;
static const std::string defaultCircleName = "Circle";

Circle::Circle(Vector2 pos, float r, Color c)
    : Object(pos, c), radius(r), originalRadius(r) {
    showTrail = trailHead = trailCount = defaultTrailValue;
    baseName = defaultCircleName;
}

Circle::Circle() : Object() {
    radius = originalRadius = defaultRadius;
    showTrail = trailHead = trailCount = defaultTrailValue;
    baseName = defaultCircleName;
}

Circle::~Circle() = default;

Object *Circle::clone() {
    Circle *copy = new Circle(position, radius, color);
    copy->velocity = this->velocity;
    copy->showTrail = this->showTrail;
    copy->name = this->name;
    return copy;
}

void Circle::update() {
    position.x += velocity.x;
    position.y += velocity.y;

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

void Circle::draw() {
    if (!visible) return;

    if (showTrail) {
        for (int i = 0; i < trailCount; i++) {
            DrawCircleV(trail[i], 3.0f, Fade(color, 0.4f));
        }
    }

    DrawCircleV(position, radius, color);

    if (radius < originalRadius) {
        DrawCircleV(position, radius, Fade(WHITE, 0.5f));
    }

    if (selected) {
        DrawRing(position, radius + 2, radius + 5, 0, 360, 0, GREEN);
    }
}

bool Circle::contains(Vector2 point) {
    if (!visible) return false;
    return CheckCollisionPointCircle(point, position, radius);
}

float Circle::getRadius() { return radius; }

bool Circle::checkCollision(Object *other) {
    Group *otherGroup = dynamic_cast<Group *>(other);
    if (otherGroup != nullptr) { return otherGroup->checkCollision(this); }

    return CheckCollisionCircles(position, radius, other->getPos(),
                                 other->getRadius());
}

void Circle::deform() {
    if (radius <= 5.0f) return;
    radius -= 1.0f;
}

void Circle::reset() {
    Object::reset();
    radius = originalRadius;
    trailCount = 0;
}

void Circle::toggleTrail() {
    showTrail = !showTrail;
    trailCount = 0;
}

void Circle::setVisible(bool v) { this->visible = v; }

Object *Circle::drawEditUI(float startX, float startY) {
    if (parentGroup != nullptr) {
        if (GuiButton({ startX, startY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT}, "Back")) {
            return parentGroup;
        }
        startY += 30;
    }

    Object::drawEditUI(startX, startY);

    // update buffers
    if (!editModeRadius) snprintf(textBufRadius, sizeof(textBufRadius), "%.2f", this->radius);

    GuiLabel({ startX, cursorY, LABEL_TITLE_WIDTH, LABEL_TITLE_HEIGHT}, "--- Circle ---");
    cursorY += ROW_SPACING;

    GuiCheckBox({ startX, cursorY, CHECKBOX_SIZE, CHECKBOX_SIZE }, "Show Trail", &this->showTrail);
    cursorY += ROW_SPACING + SECTION_PADDING;

    GuiLabel({ startX, cursorY, LABEL_LONG_WIDTH, ELEMENT_HEIGHT}, "Radius");
    cursorY += ROW_SPACING;

    if (GuiValueBoxFloat({ startX, cursorY, VALUE_BOX_WIDTH, ELEMENT_HEIGHT }, NULL, textBufRadius, &this->radius, editModeRadius)) {
        editModeRadius = !editModeRadius;
    }

    return this;
}

// serialization pattern:               CIRCLE // OBJECT:[ props ] DATA:[ r;or;t;tc;[(tx;ty)];show ]
// float radius, originalRadius;        r - radius & or - original radius
// Vector2 trail[count];                [(tx;ty)] - trail array
// int trailHead, trailCount;           t - trailHead & tc - trailCount
// bool showTrail;                      show - showTrail
// NOTE: separators arent strictly as specified in pattern
// but must be provided because serializer will skip those
// places automatically regardless of the character

void Circle::serialize(std::ostream &os) const {
    os << CIRCLE_PREFIX << " " << TYPE_SEP << " ";
    Object::serialize(os);

    os << " ";

    os << OBJ_DATA_PREFIX << PREFIX_SEP << OBJ_DATA_OPEN << " ";
    os << radius << ";" << originalRadius << ";";
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

void Circle::deserialize(std::istream &is) {
    char d; // tmp

    is >> d >> d;
    
    Object::deserialize(is);

    std::string temp;
    std::getline(is, temp, ':');
    is >> d;

    is >> radius >> d >> originalRadius >> d;
    is >> trailHead >> d >> trailCount >> d;
    is >> d;
    for(int i = 0; i < trailCount; i++) {
        is >> d >> trail[i].x >> d >> trail[i].y >> d;
        if(i < trailCount - 1) is >> d;
    }
    is >> d >> d;
    is >> showTrail >> d;
}
