#include <object.h>
#include <utils/vec.h>

class Group : public Object {
    private:
        Vector<Object *> children;

        bool nameDialogEditMode = false;
        char nameDialogInput[INPUT_BUFFER_SIZE] = "";
        Color editUIColor = RED;

    protected:
        void serialize(std::ostream &os) const override;
        void deserialize(std::istream &is) override;

    public:
        Group();
        ~Group();

        Vector<Object *> disband();
        Vector<Object *> &getMembers();

        void addChild(Object *obj);

        void update() override;
        void draw() override;
        Object *clone() override;

        void setVisible(bool visible) override;

        bool contains(Vector2 point) override;

        Vector2 getPos() const override;
        void moveManual(Vector2 delta) override;
        void randomizeColor() override;

        void setColor(Color c) override;

        float getRadius() override;
        void deform() override;
        void reset() override;
        void toggleTrail() override;
        bool checkCollision(Object *other) override;

        Object *drawEditUI(float startX, float startY) override;
};
