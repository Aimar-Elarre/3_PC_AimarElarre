#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "Camera.h"

class App {
public:
    App();
    ~App();

    void run();

    // Callbacks públicos
    void mouse_callback(double xpos, double ypos);
    void scroll_callback(double xoffset, double yoffset); // <--- NUEVO

private:
    GLFWwindow* window = nullptr;

    // Variables OpenGL
    unsigned int VBO, EBO, VAO;
    int numIndices = 0;

    // Mapa y Objetos
    std::vector<float> heightMap;
    int terrainWidth = 0;
    int terrainHeight = 0;
    float terrainYScale = 64.0f / 256.0f;
    float terrainYShift = -16.0f;

    unsigned int waterVAO, waterVBO;
    unsigned int modelVAO, modelVBO;
    int modelVertexCount = 0;
    std::vector<glm::vec3> treePositions;
    unsigned int castleVAO, castleVBO;
    int castleVertexCount = 0;
    glm::vec3 castlePosition;

    // Shaders y Texturas
    unsigned int shaderProgram1;
    unsigned int textureRockID;
    unsigned int textureGrassID;
    unsigned int textureWaterID;
    unsigned int textureTreeID;

    // Cámara y Control
    Camera* camera = nullptr;
    float lastFrame = 0.0f;
    bool firstMouse = true;
    float lastX = 400.0f;
    float lastY = 300.0f;

    void init();
    void mainLoop();
    void cleanup();
    void processInput(GLFWwindow* window, float deltaTime);

    void initTerrain();
    void initWater();
    void initExternalModel();
    void plantTrees();
    void drawModelAt(glm::mat4 view, glm::mat4 projection, glm::vec3 pos);
    void initCastle();
    void drawCastle(glm::mat4 view, glm::mat4 projection);

    float getHeightAt(float x, float z);
    bool loadOBJ(const char* path, std::vector<float>& outVertices);
    unsigned int loadTexture(const char* path);
    void initShaders();
    void initTextures();
    std::string loadShaderSource(const std::string& path);
};