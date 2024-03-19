#include "util/defs.h"
#include "gfx/game.h"

int main(void) {
    GLFWwindow *window;
    struct Game game;

    assert(glfwInit());
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Breakout", NULL, NULL);
    assert(window != NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));
    game_init(&game);
 
    glEnable(GL_CULL_FACE);
    glViewport(0, 0, WIDTH, HEIGHT);
    game_run(window, &game);
    
    game_destroy(&game);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
