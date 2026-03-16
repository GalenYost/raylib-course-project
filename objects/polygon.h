#include <object.h>

class Polygon : public Object {
    private:
        int sides;
        float radius;
        float originalRadius;
        float rotation;
        float rotSpeed;

        Vector2 trail[MAX_TRAIL_COUNT];
        int trailHead, trailCount;
        bool showTrail;

        // edit ui
        bool editModeSides = false;
        bool editModeRadius = false;
        bool editModeRotation = false;
        bool editModeRotSpeed = false;
        char textBufRadius[MAX_FLOAT_NUM_BUFFER] = "";
        char textBufRotation[MAX_FLOAT_NUM_BUFFER] = "";
        char textBufRotSpeed[MAX_FLOAT_NUM_BUFFER] = "";

    protected:
        void serialize(std::ostream &os) const override;
        void deserialize(std::istream &is) override;

    public:
        Polygon(Vector2 pos, int sides, float r, Color c);
        Polygon();
        ~Polygon();

        void update() override;
        void draw() override;
        Object *clone() override;
        void setVisible(bool visible) override;

        bool contains(Vector2 point) override;

        float getRadius() override;
        void deform() override;
        void reset() override;
        void toggleTrail() override;

        bool checkCollision(Object *other) override;

        Object *drawEditUI(float startX, float startY) override;
};
