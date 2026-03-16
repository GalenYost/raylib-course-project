#include <object.h>

class Rect : public Object {
    private:
        float width, originalWidth;
        float height, originalHeight;
        int trailHead, trailCount;
        bool showTrail;
        Vector2 trail[MAX_TRAIL_COUNT];

        // edit ui
        bool editModeWidth = false;
        char textBufWidth[MAX_FLOAT_NUM_BUFFER] = "";

        bool editModeHeight = false;
        char textBufHeight[MAX_FLOAT_NUM_BUFFER] = "";

    protected:
        void serialize(std::ostream &os) const override;
        void deserialize(std::istream &is) override;

    public:
        Rect(Vector2 pos, float width, float height, Color c);
        Rect();
        ~Rect();

        void update() override;
        void draw() override;
        Object *clone() override;

        bool contains(Vector2 point) override;

        float getRadius() override;
        void deform() override;
        void reset() override;
        void toggleTrail() override;

        bool checkCollision(Object *other) override;

        Object *drawEditUI(float startX, float startY) override;
};
