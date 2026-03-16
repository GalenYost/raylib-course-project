#include <object.h>

class Triangle : public Object {
    private:
        float size;
        float originalSize;
        Vector2 trail[MAX_TRAIL_COUNT];
        int trailHead, trailCount;
        bool showTrail;

        // edit ui
        bool editModeSize = false;
        char textBufSize[36] = "";

    protected:
        void serialize(std::ostream &os) const override;
        void deserialize(std::istream &is) override;

    public:
        Triangle(Vector2 pos, float s, Color c);
        Triangle();
        ~Triangle();

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
