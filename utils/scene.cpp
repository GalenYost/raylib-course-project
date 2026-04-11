#include <utils/registry.h>
#include <utils/scene.h>
#include <utils/parser.h>

#include <objects/circle.h>
#include <objects/group.h>
#include <objects/polygon.h>
#include <objects/rect.h>
#include <objects/triangle.h>

#include <include/raylib.h>

#include <limits>
#include <sstream>
#include <fstream>
#include <variant>

#define RAYGUI_IMPLEMENTATION
#include <include/raygui.h>

#define FILE_DIALOG_WINDOW_WIDTH 300
#define FILE_DIALOG_WINDOW_HEIGHT 150

#define REGISTRY_WINDOW_WIDTH 250.0f
#define REGISTRY_WINDOW_HEIGHT 300.0f
#define REGISTRY_ROW_HEIGHT 25.0f

static const float fontSize = 16.0f;
static const float fontSpacing = 2.5f;
static const int lineSpacing = 20;
static const int margin = 5;
static const int startY = 0;
static Font font;

Scene *Scene::instance = nullptr;
ObjectRegistry *Scene::registry = ObjectRegistry::getInstance();

Scene::Scene() : paused(true) {
    Rect *rect = new Rect();
    Circle *circ = new Circle();
    Triangle *tri = new Triangle();
    Polygon *poly = new Polygon();

    registry->add(((Object*)rect)->getName(), (Object *)rect);
    registry->add(((Object*)circ)->getName(), (Object *)circ);
    registry->add(((Object*)tri)->getName(), (Object *)tri);
    registry->add(((Object*)poly)->getName(), (Object *)poly);
}

Scene *Scene::getInstance() {
    if (instance == nullptr) instance = new Scene;
    return instance;
}

Scene::~Scene() {
    for (unsigned i = 0; i < objects.len(); i++) {
        delete objects[i];
    }
}

void Scene::init() {
    InitWindow(WIDTH, HEIGHT, TITLE);
    SetTargetFPS(FPS);

    font = LoadFontEx("DejaVuSans.ttf", 24, 0, 0);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    GuiSetFont(font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, fontSize);
}

void Scene::run() {
    while (!WindowShouldClose()) {
        handleInput();
        if (!paused) update();
        draw();
    }
    CloseWindow();
}

void Scene::handleInput() {
    if (tool == AppMode::DIALOG) return;

    if (IsKeyPressed(KEY_SPACE)) togglePause();
    if (IsKeyPressed(KEY_L)) {
        for (unsigned i = 0; i < objects.len() ; i++) {
            if (!objects[i]->isVisible()) objects[i]->setVisible(true);
        }
    }
    if (IsKeyPressed(KEY_G)) {
        Vector<Object *> itemsToGroup;
        int selectedCount = 0;

        for (int i = objects.len() - 1; i >= 0; i--) {
            Object *obj = objects[i];
            if (obj->isSelected()) {
                obj->setSelected(false);
                Object *cloned = obj->clone();
                itemsToGroup.push(cloned);
                selectedCount++;
            }
        }

        if (selectedCount < 2 && itemsToGroup.len() > 0) {
            for (unsigned i = 0; i < itemsToGroup.len(); i++) objects.push(itemsToGroup[i]);
        } else if (selectedCount > 0) {
            Group *masterGroup = new Group();

            for (unsigned i = 0; i < itemsToGroup.len(); i++) {
                Object *obj = itemsToGroup[i];
                Group *subGroup = dynamic_cast<Group *>(obj);

                if (subGroup != nullptr) {
                    Vector<Object *> &kids = subGroup->getMembers();

                    for (unsigned k = 0; k < kids.len(); k++) {
                        kids[k]->setSelected(false);
                        masterGroup->addChild(kids[k]);
                    }

                    while (!kids.empty()) { kids.pop(); }
                    delete subGroup;
                } else {
                    obj->setSelected(false);
                    masterGroup->addChild(obj);
                }
            }

            masterGroup->setSelected(true);
            objects.push(masterGroup);
        }
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();

        // dialogs collision
        if (drawObjectRegistryList) {
            float sw = (float)GetScreenWidth();
            float sh = (float)GetScreenHeight();
            Rectangle regBounds = { sw - REGISTRY_WINDOW_WIDTH - 10.0f, sh - REGISTRY_WINDOW_HEIGHT - 10.0f, REGISTRY_WINDOW_WIDTH, REGISTRY_WINDOW_HEIGHT};
            if (CheckCollisionPointRec(mousePos, regBounds)) return;
        }

        if (drawEditDialog) {
            float startX = (float)(GetScreenWidth() - EDIT_UI_WINDOW_WIDTH) / 2;
            float startY = (float)(GetScreenHeight() - EDIT_UI_WINDOW_HEIGHT) / 2;
            Rectangle editBounds = { startX, startY, (float)EDIT_UI_WINDOW_WIDTH, (float)EDIT_UI_WINDOW_HEIGHT };
            if (CheckCollisionPointRec(mousePos, editBounds)) return;
        }

        switch (tool) {
            case AppMode::SELECT: {
                for (unsigned i = 0; i < objects.len(); i++) {
                    if (!objects[i]->contains(mousePos)) continue;
                    objects[i]->setSelected(!objects[i]->isSelected());
                    return;
                }
                break;
            }
            case AppMode::EDIT: {
                if (drawEditDialog) break;
                for (unsigned i = 0; i < objects.len(); i++) {
                    if (editUIObject != nullptr) return;
                    if (!objects[i]->contains(mousePos)) continue;
                    drawEditDialog = true;
                    editUIObject = objects[i];
                    return;
                }
                break;
            }
            case AppMode::CREATE: {
                if (spawnPrototype == nullptr) break;
                Object *clone = spawnPrototype->clone();
                Vector2 pos = clone->getPos();
                Vector2 delta = { mousePos.x - pos.x, mousePos.y - pos.y };

                clone->moveManual(delta);

                objects.push(clone);
            }
            default: break;
        }
    }

    if (tool == AppMode::SELECT && !IsKeyDown(KEY_LEFT_CONTROL)) {
        Vector2 manualDelta = {0, 0};
        if (IsKeyDown(KEY_W)) manualDelta.y -= 2;
        if (IsKeyDown(KEY_S)) manualDelta.y += 2;
        if (IsKeyDown(KEY_A)) manualDelta.x -= 2;
        if (IsKeyDown(KEY_D)) manualDelta.x += 2;

        if (manualDelta.x != 0 || manualDelta.y != 0) {
            for (unsigned i = 0; i < objects.len(); i++) {
                if (objects[i]->isSelected()) objects[i]->moveManual(manualDelta);
            }
        }
    }

    if (IsKeyDown(KEY_LEFT_CONTROL)) {
        if (IsKeyPressed(KEY_S)) setAppMode(AppMode::SELECT);
        if (IsKeyPressed(KEY_C)) setAppMode(AppMode::CREATE);
        if (IsKeyPressed(KEY_E)) setAppMode(AppMode::EDIT);
        if (IsKeyPressed(KEY_D)) {
            for (unsigned i = 0; i < objects.len(); i++) {
                if (objects[i]->isSelected()) objects.push(objects[i]->clone());
            }
        }
        if (IsKeyPressed(KEY_X)) {
            for (int i = objects.len() - 1; i >= 0; i--) {
                if (objects[i]->isSelected()) {
                    delete objects[i];
                    objects.remove(i);
                }
            }
        }
    }

    if (IsKeyPressed(KEY_F10)) triggerSave();
    if (IsKeyPressed(KEY_F11)) triggerLoad();
}

void Scene::update() {
    if (env.find("update")) {
        Vector<Value> args;
        env.call("update", args);
    }

    for (unsigned i = 0; i < objects.len(); i++) {
        objects[i]->update();
        for (unsigned j = 0; j < objects.len(); j++) {
            if (i == j) continue;
            if (objects[i]->checkCollision(objects[j])) {
                objects[i]->deform();
            }
        }
    }
}

void Scene::drawToolsMenu() {
    const char *lines[] = {
        "CTRL+S - Selection Mode",
        "CTRL+C - Creation Mode",
        "CTRL+E - Edit Mode",
        "CTRL+D - Clone Object",
        "CTRL+X - Delete Object",
        "F10 - Save State",
        "F11 - Load State",
    };

    float maxTextWidth = 0.0f;
    int count = sizeof(lines) / sizeof(lines[0]);

    float currentFontSize = (float)fontSize; 
    float currentSpacing = (float)fontSpacing;

    for (int i = 0; i < count; i++) {
        Vector2 size = MeasureTextEx(font, lines[i], currentFontSize, currentSpacing);
        if (size.x > maxTextWidth) {
            maxTextWidth = size.x;
        }
    }

    float menuX = GetScreenWidth() - maxTextWidth - margin;
    for (int i = 0; i < count; i++) {
        DrawTextEx(font, lines[i], Vector2{menuX, startY + (i * (float)lineSpacing)}, currentFontSize, currentSpacing, BLACK);
    }
}

void Scene::drawFileDialogUI() {
    float startX = (GetScreenWidth() - FILE_DIALOG_WINDOW_WIDTH) / 2.0f;
    float startY = (GetScreenHeight() - FILE_DIALOG_WINDOW_HEIGHT) / 2.0f;

    const char *title = fileDialogType ? "Save" : "Load";

    if (GuiWindowBox({ startX, startY, FILE_DIALOG_WINDOW_WIDTH, FILE_DIALOG_WINDOW_HEIGHT }, title)) {
        setAppMode(lastUsedMode);
        return;
    }

    float cursorX = startX + SECTION_PADDING * 2.0f;
    float cursorY = startY + ROW_SPACING + SECTION_PADDING;
    float contentWidth = FILE_DIALOG_WINDOW_WIDTH - (SECTION_PADDING * 4.0f);

    GuiLabel({ cursorX, cursorY, LABEL_TITLE_WIDTH, ELEMENT_HEIGHT }, "Filename:");
    cursorY += ELEMENT_HEIGHT + INNER_PADDING;

    if (GuiTextBox({ cursorX, cursorY, contentWidth, ELEMENT_HEIGHT * 1.5f }, fileDialogInput, INPUT_BUFFER_SIZE, fileDialogEditMode)) {
        fileDialogEditMode = !fileDialogEditMode;
    }
    cursorY += ELEMENT_HEIGHT + (SECTION_PADDING*2) + INNER_PADDING;

    float btnWidth = (contentWidth - SECTION_PADDING) / 2.0f;

    if (GuiButton({ cursorX, cursorY, btnWidth, ELEMENT_HEIGHT * 1.5f }, title)) {
        if (fileDialogType) {
            std::ofstream ofile(fileDialogInput);
            if (ofile.is_open()) ofile << *this;
            else std::cout << "[ERROR] Couldnt open file " << fileDialogInput << std::endl;

            std::ofstream protos_save(PROTOTYPES_SAVE);
            if (protos_save.is_open()) protos_save << *(this->registry);
            else std::cout << "[ERROR] Couldnt open file " << PROTOTYPES_SAVE << std::endl;
        } else {
            std::ifstream ifile(fileDialogInput);
            std::ifstream protos_save(PROTOTYPES_SAVE);

            if (!ifile.is_open() || !protos_save.is_open()) {
                std::cout << "[ERROR] Couldnt open load files\n";
                setAppMode(lastUsedMode);
                return;
            }

            objects.clear();
            ifile >> *this;
            protos_save >> *(this->registry);
        }

        setAppMode(lastUsedMode);
    }

    cursorX += btnWidth + SECTION_PADDING;

    if (GuiButton({ cursorX, cursorY, btnWidth, ELEMENT_HEIGHT * 1.5f }, "Cancel")) {
        setAppMode(lastUsedMode);
    }
}

void Scene::drawObjectRegistryUI() {
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();
    
    float startX = sw - REGISTRY_WINDOW_WIDTH - 10.0f;
    float startY = sh - REGISTRY_WINDOW_HEIGHT - 10.0f;

    if (GuiWindowBox({ startX, startY, REGISTRY_WINDOW_WIDTH, REGISTRY_WINDOW_HEIGHT }, "Object Registry")) {
        setAppMode(lastUsedMode);
        return;
    }

    auto& map = registry->getMap(); 

    Rectangle panelBounds = { startX + 5, startY + 30, REGISTRY_WINDOW_WIDTH - 10, REGISTRY_WINDOW_HEIGHT - 35 };
    Rectangle contentBounds = { 0, 0, panelBounds.width - 15, map.size() * REGISTRY_ROW_HEIGHT };
    
    static Vector2 scroll = { 0, 0 };
    Rectangle view = {0, 0, 0, 0};

    GuiScrollPanel(panelBounds, nullptr, contentBounds, &scroll, &view);

    BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);

    float cursorY = panelBounds.y + scroll.y;
    float labelWidth = view.width * 0.75f;
    float btnWidth = view.width * 0.25f - 5.0f;

    std::string toDelete = "";

    for (auto it = map.begin(); it != map.end(); ++it) {
        if (cursorY + REGISTRY_ROW_HEIGHT > view.y && cursorY < view.y + view.height) {
            if (GuiButton({ panelBounds.x + 5, cursorY, labelWidth, REGISTRY_ROW_HEIGHT }, it->first.c_str())) {
                spawnPrototype = it->second;
                spawnPrototype->setName(it->first);
            }
            if (GuiButton({ panelBounds.x + labelWidth + 5, cursorY + 2, btnWidth, REGISTRY_ROW_HEIGHT - 4 }, "X")) toDelete = it->first;
        }
        cursorY += REGISTRY_ROW_HEIGHT;
    }

    EndScissorMode();

    if (!toDelete.empty()) {
        if (spawnPrototype == map[toDelete]) spawnPrototype = nullptr;
        delete map[toDelete]; 
        map.erase(toDelete);
    }
}

#define DRAW_FPS false
void Scene::draw() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    float startX, startY;
    if (DRAW_FPS) {
        DrawFPS(1, 1);
        startX = 3.0f;
        startY = 20.0f;
    } else {
        startX = 1.0f;
        startY = 0.0f;
    }

    static const char *lines[] = {
        "W/A/S/D - Move Object",
        "LMB - Select Object",
        "L - Show Hidden Objects",
        "G - Group Selected Objects",
        "Space - Toggle Auto-Movement",
    };

    for (unsigned i = 0; i < (sizeof(lines) / sizeof(lines[0])); i++) {
        DrawTextEx(font, lines[i], Vector2{startX, startY + lineSpacing * (i+1)}, fontSize, fontSpacing, BLACK);
    }

    drawToolsMenu();

    for (unsigned i = 0; i < objects.len(); i++) {
        objects[i]->draw();
    }

    if (drawFileDialog) drawFileDialogUI();
    if (drawEditDialog && editUIObject != nullptr) {
        float sw = (float)GetScreenWidth();
        float sh = (float)GetScreenHeight();
        
        float startX = sw - EDIT_UI_WINDOW_WIDTH - 10.0f;
        float startY = sh - EDIT_UI_WINDOW_HEIGHT - 10.0f;

        if (GuiWindowBox({ startX, startY, EDIT_UI_WINDOW_WIDTH, EDIT_UI_WINDOW_HEIGHT }, "Edit Properties")) {
            editUIObject = nullptr;
            drawEditDialog = false;
            return;
        }
        editUIObject = editUIObject->drawEditUI(startX + 20, startY + 40);
    }
    if (drawObjectRegistryList) drawObjectRegistryUI();

    EndDrawing();
}

void Scene::togglePause() {
    paused = !paused;
}

void Scene::setFps(int fps) {
    SetTargetFPS(fps);
}

void Scene::createObject(ObjectType type, Vector2 pos, Color c, float param1, float param2) {
    Object* obj;

    switch (type) {
        case ObjectType::Rectangle:
            obj = new Rect(pos, param1, param2, c);
            break;
        case ObjectType::Circle:
            obj = new Circle(pos, param1, c);
            break;
        case ObjectType::Triangle:
            obj = new Triangle(pos, param1, c);
            break;
        case ObjectType::Polygon:
            obj = new Polygon(pos, int(param2), param1, c);
            break;
        default:
            throw ObjectNotSupported();
    }

    if (!obj) return;
    objects.push(obj);
}

void Scene::setAppMode(AppMode tool) {
    if (tool == this->tool) return;

    if (tool == AppMode::CREATE) drawObjectRegistryList = true;
    else drawObjectRegistryList = false;

    if (tool == AppMode::DIALOG) drawFileDialog = true;
    else drawFileDialog = false;

    lastUsedMode = this->tool;
    this->tool = tool;
}

void Scene::triggerSave() {
    fileDialogType = true;
    setAppMode(AppMode::DIALOG);
}

void Scene::triggerLoad() {
    fileDialogType = false;
    setAppMode(AppMode::DIALOG);
}

std::istream &operator>>(std::istream &is, Scene &s) {
    if (is.eof()) return is;
    int count;
    is >> count;
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < count; i++) {
        std::string line;
        if (!std::getline(is, line) || line.empty()) break;

        std::stringstream ss(line);
        std::string type;
        
        ss >> type; 

        Object* child = nullptr;
        bool isMultiline = false;
        
        if (type == RECTANGLE_PREFIX) child = new Rect();
        else if (type == CIRCLE_PREFIX) child = new Circle();
        else if (type == TRIANGLE_PREFIX) child = new Triangle();
        else if (type == POLYGON_PREFIX) child = new Polygon();
        else if (type == GROUP_PREFIX) {
            child = new Group();
            isMultiline = true;
        }

        if (child) {
            if (isMultiline) is >> *child;
            else ss >> *child;
            s.objects.push(child);
        }
    }
    return is;
}
std::ostream &operator<<(std::ostream &os, const Scene &s) {
    os << s.objects.len() << std::endl;
    for (unsigned i = 0; i < s.objects.len(); i++) {
        Object *obj = s.objects[i];
        if (obj == nullptr) continue;
        os << *obj;
        os << std::endl;
    }
    return os;
}

void Scene::executeScript(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Couldnt read script with path '" << path << "'" << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string src = buffer.str();

    Lexer lex(src);
    Vector<Token> tokens = lex.tokenize();

    Parser parser(tokens);
    script_ast = parser.parse_all();

    std::cout << "Loaded '" << path << "'" << std::endl;

    env.bind("set_color", [this](Vector<Value> &args) -> Value {
        if (args.len() < 4) {
            std::cerr << "[ERROR]: not enough args provided" << "\n";
            std::cerr << "USAGE: set_color_for_selected(r, g, b, a), where r/g/b/a are of type 'Number'" << "\n";
            return std::monostate {};
        }

        if (std::holds_alternative<float>(args[0]) && 
            std::holds_alternative<float>(args[1]) && 
            std::holds_alternative<float>(args[2]) &&
            std::holds_alternative<float>(args[3])
        ) {
            unsigned char r = (unsigned char)std::get<float>(args[0]);
            unsigned char g = (unsigned char)std::get<float>(args[1]);
            unsigned char b = (unsigned char)std::get<float>(args[2]);
            unsigned char a = (unsigned char)std::get<float>(args[3]);
            Color c = Color { .r = r, .g = g, .b = b, .a = a };

            for (unsigned i = 0; i < this->objects.len(); i++) {
                objects[i]->setColor(c);
            }
        }

        return std::monostate {};
    });

    for (unsigned i = 0; i < script_ast.len(); i++) {
        script_ast[i]->execute(env);
    }
}
