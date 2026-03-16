#pragma once

#include <object.h>
#include <vec.h>
#include <registry.h>

#include <exception>

#define WIDTH 1270
#define HEIGHT 800
#define TITLE "Курсова"
#define FPS 60

#define DEFAULT_SAVE_FILE "save.txt"

#define EDIT_UI_WINDOW_WIDTH 350
#define EDIT_UI_WINDOW_HEIGHT 600

enum class AppMode {
    SELECT,
    CREATE,
    DIALOG,
    EDIT,
};

class ObjectNotSupported : public std::exception {
    private:
        ObjectType type;

    public:
        ObjectNotSupported(ObjectType type);
        const char *what() const noexcept override;
};

class Scene {
    private:
        static Scene *instance;
        static ObjectRegistry *registry;
        Vector<Object *> objects;
        bool paused = false;

        AppMode tool = AppMode::SELECT;
        AppMode lastUsedMode = AppMode::SELECT;

        // file dialog
        bool drawFileDialog = false;
        bool fileDialogType = false; // false - load, true - save
        bool fileDialogEditMode = false;
        char fileDialogInput[INPUT_BUFFER_SIZE] = ""; // remember last input
        void drawFileDialogUI();

        bool drawEditDialog = false;
        Object *editUIObject = nullptr;

        // registry UI
        bool drawObjectRegistryList = false;
        Object *spawnPrototype = nullptr;
        void drawObjectRegistryUI();

        void drawToolsMenu();
        Scene();

    public:
        Scene(const Scene&) = delete;
        void operator=(const Scene&) = delete;
        ~Scene();

        static Scene *getInstance();

        void init();
        void run();

        void handleInput();
        void update();
        void draw();

        void setAppMode(AppMode tool);

        void togglePause();
        void setFps(int fps);

        void triggerSave();
        void triggerLoad();

        // param1 - width or radius
        // param2 - height or sides
        void createObject(ObjectType type, Vector2 pos, Color c, float param1, float param2);

        friend std::istream &operator>>(std::istream &is, Scene &s);
        friend std::ostream &operator<<(std::ostream &os, const Scene &s);
};
