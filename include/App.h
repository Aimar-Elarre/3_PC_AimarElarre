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

    unsigned int VBO, EBO, VAO, shaderProgram;
    void init();
    void mainLoop();
    void cleanup();

    void initTriangle();
    void initShaders();
    std::string loadShaderSource(const std::string& path);
};
