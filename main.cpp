#include <utils/scene.h>

int main(void) {
    Scene *scene = Scene::getInstance();

    scene->init();
    scene->run();

    return 0;
}
