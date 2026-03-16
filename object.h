#pragma once

#include <raylib.h>

#include <string>
#include <iostream>

// separators
#define PREFIX_SEP ":"
#define TYPE_SEP "//"

#define OBJ_DATA_OPEN "["
#define OBJ_DATA_CLOSE "]"

// prefixes
#define OBJ_DATA_PREFIX "DATA"
#define OBJECT_PREFIX "OBJECT"
#define RECTANGLE_PREFIX "RECTANGLE"
#define CIRCLE_PREFIX "CIRCLE"
#define TRIANGLE_PREFIX "TRIANGLE"
#define POLYGON_PREFIX "POLYGON"
#define GROUP_PREFIX "GROUP"

#define MAX_TRAIL_COUNT 100

// edit ui draw properties
#define MAX_FLOAT_NUM_BUFFER 36
#define MAX_NAME_SIZE 20

#define ELEMENT_HEIGHT 20.0f
#define CHECKBOX_SIZE 20.0f
#define VALUE_BOX_WIDTH 75.0f

#define LIST_HEIGHT 150.0f

#define LABEL_SHORT_WIDTH 15.0f
#define LABEL_LONG_WIDTH 100.0f
#define LABEL_TITLE_WIDTH 200.0f
#define LABEL_TITLE_HEIGHT 20.0f

#define COLOR_PICKER_SIZE 100.0f

#define ROW_SPACING 30.0f
#define SECTION_PADDING 10.0f
#define INNER_PADDING 5.0f
#define COL_OFFSET 110.0f

#define INPUT_BUFFER_SIZE 50

enum class ObjectType {
    Rectangle,
    Circle,
    Triangle,
    Polygon,
    Group,
};

class Object {
    private:
        friend std::ostream &operator<<(std::ostream &os, const Object &obj);
        friend std::istream &operator>>(std::istream &is, Object &obj);

        // edit ui
        bool editModePosX = false;
        bool editModePosY = false;
        char textBufPosX[32] = "";
        char textBufPosY[32] = "";

        bool nameDialogEditMode = false;
        char nameDialogInput[INPUT_BUFFER_SIZE] = "";

    protected:
        Vector2 position;
        Vector2 originalPosition;
        Vector2 velocity;
        Color color;
        Color originalColor;
        bool selected;
        bool visible;

        float cursorY = 0.0f;

        std::string baseName = "";
        std::string name = "";

        virtual void serialize(std::ostream &os) const;
        virtual void deserialize(std::istream &is);

    public:
        Object(Vector2 pos, Color c);
        Object();
        virtual ~Object();

        virtual void update() = 0;
        virtual void draw() = 0;
        virtual void reset();
        virtual Object *clone() = 0;

        virtual bool contains(Vector2 point) = 0;

        virtual float getRadius() = 0;
        virtual void deform() = 0;

        virtual bool checkCollision(Object *other) = 0;
        virtual void toggleTrail() = 0;

        virtual void moveManual(Vector2 delta);
        virtual void randomizeColor();
        virtual void setVisible(bool visible);

        virtual void setColor(Color c);

        void setPosition(Vector2 newPos);
        void setInitialPos(Vector2 newPos);

        std::string getName() const;
        void setName(const std::string &name);

        bool isSelected() const;
        void setSelected(bool val);
        bool isVisible() const;
        virtual Vector2 getPos() const;

        void setVelocity(Vector2 vel);
        void toggleVisibility();

        virtual Object *drawEditUI(float startX, float startY);

        Object *parentGroup = nullptr;
};
