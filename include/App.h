#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

class App {
public:
    App();
    ~App();

    void run();

private:
    GLFWwindow* window;

    unsigned int VBO, EBO, VAO, shaderProgram1;
    //unsigned int  shderProgram2;
    int currentState = 1;
    void init();
    void mainLoop();
    void cleanup();

    void keyCalback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void initTriangle();
    void initShaders();
    std::string loadShaderSource(const std::string& path);
};
