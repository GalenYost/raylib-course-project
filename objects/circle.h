#include <object.h>

class Circle : public Object {
    private:
        float radius, originalRadius;
        Vector2 trail[MAX_TRAIL_COUNT];
        int trailHead, trailCount;
        bool showTrail;

        // edit ui
        bool editModeRadius = false;
        char textBufRadius[MAX_FLOAT_NUM_BUFFER] = "";

    protected:
        void serialize(std::ostream &os) const override;
        void deserialize(std::istream &is) override;

    public:
        Circle(Vector2 pos, float radius, Color c);
        Circle();
        ~Circle();

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
