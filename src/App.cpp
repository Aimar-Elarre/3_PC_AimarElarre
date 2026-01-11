#include "App.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm> 

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// --- CALLBACK GLOBAL ---
void glfw_mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    App* app = (App*)glfwGetWindowUserPointer(window);
    if (app) app->mouse_callback(xpos, ypos);
}

App::App() { init(); }
App::~App() { cleanup(); }

void App::run() { mainLoop(); }

void App::init() {
    if (!glfwInit()) exit(-1);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, "OpenGL - Isla con Castillo", nullptr, nullptr);
    if (!window) { glfwTerminate(); exit(-1); }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, glfw_mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) exit(-1);

    glViewport(0, 0, 800, 600);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Voltear texturas al cargar
    stbi_set_flip_vertically_on_load(true);

    camera = new Camera(glm::vec3(0.0f, 40.0f, 100.0f));
    camera->SetAspectRatio(800.0f / 600.0f);

    initShaders();
    initTextures();

    // 1. Generamos terreno (Esto llena heightMap)
    initTerrain();
    initWater();

    // 2. Cargamos árboles
    initExternalModel();
    plantTrees();

    // 3. Cargamos y colocamos el castillo (NUEVO)
    initCastle();
}

// --- PARSER OBJ ---
bool App::loadOBJ(const char* path, std::vector<float>& outVertices) {
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR: No se encuentra modelo: " << path << "\n";
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            glm::vec3 vertex;
            ss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
        }
        else if (prefix == "vt") {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (prefix == "vn") {
            glm::vec3 norm;
            ss >> norm.x >> norm.y >> norm.z;
            temp_normals.push_back(norm);
        }
        else if (prefix == "f") {
            std::string vertexStr;
            for (int i = 0; i < 3; i++) {
                ss >> vertexStr;
                size_t doubleSlash = vertexStr.find("//");
                if (doubleSlash != std::string::npos) vertexStr.replace(doubleSlash, 2, "/0/");
                std::replace(vertexStr.begin(), vertexStr.end(), '/', ' ');
                std::stringstream vss(vertexStr);

                int vIdx = 0, vtIdx = 0, vnIdx = 0;
                vss >> vIdx; if (vss.peek() != -1) vss >> vtIdx; if (vss.peek() != -1) vss >> vnIdx;
                vIdx--; vtIdx--; vnIdx--;

                glm::vec3 v = (vIdx >= 0 && vIdx < temp_vertices.size()) ? temp_vertices[vIdx] : glm::vec3(0.0f);
                glm::vec3 n = (vnIdx >= 0 && vnIdx < temp_normals.size()) ? temp_normals[vnIdx] : glm::vec3(0, 1, 0);
                glm::vec2 t = (vtIdx >= 0 && vtIdx < temp_uvs.size()) ? temp_uvs[vtIdx] : glm::vec2(0, 0);

                outVertices.push_back(v.x); outVertices.push_back(v.y); outVertices.push_back(v.z);
                outVertices.push_back(n.x); outVertices.push_back(n.y); outVertices.push_back(n.z);
                outVertices.push_back(t.x); outVertices.push_back(t.y);
            }
        }
    }
    return true;
}

// --- ÁRBOLES ---
void App::initExternalModel() {
    std::vector<float> modelVertices;
    if (!loadOBJ("../Assets/Models/tree.obj", modelVertices)) return;
    modelVertexCount = modelVertices.size() / 8;
    glGenVertexArrays(1, &modelVAO); glGenBuffers(1, &modelVBO);
    glBindVertexArray(modelVAO); glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
    glBufferData(GL_ARRAY_BUFFER, modelVertices.size() * sizeof(float), &modelVertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    textureTreeID = loadTexture("../Assets/Models/tree_texture.png");
}

void App::plantTrees() {
    srand(1234);
    treePositions.clear();
    int numberOfTrees = 80;

    // Límites seguros (dejando margen en los bordes)
    float minX = -(terrainWidth / 2.0f) + 20.0f;
    float maxX = (terrainWidth / 2.0f) - 20.0f;
    float minZ = -(terrainHeight / 2.0f) + 20.0f;
    float maxZ = (terrainHeight / 2.0f) - 20.0f;

    for (int i = 0; i < numberOfTrees; i++) {
        float rX = minX + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxX - minX)));
        float rZ = minZ + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxZ - minZ)));
        float y = getHeightAt(rX, rZ) + 2.5f;

        // Zonas válidas: No bajo el agua (-3) y no muy alto (12) para no chocar con el castillo
        if (y > -3.0f && y < 15.0f) {
            treePositions.push_back(glm::vec3(rX, y, rZ));
        }
        else {
            i--;
        }
    }
}

// --- NUEVO: INICIALIZAR CASTILLO ---
void App::initCastle() {
    // 1. Cargar el modelo
    std::vector<float> castleVertices;
    if (!loadOBJ("../Assets/Models/castle.obj", castleVertices)) {
        std::cout << "AVISO: No se encontro castle.obj\n";
        return;
    }

    castleVertexCount = castleVertices.size() / 8;
    glGenVertexArrays(1, &castleVAO); glGenBuffers(1, &castleVBO);
    glBindVertexArray(castleVAO); glBindBuffer(GL_ARRAY_BUFFER, castleVBO);
    glBufferData(GL_ARRAY_BUFFER, castleVertices.size() * sizeof(float), &castleVertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    // 2. ALGORITMO: BUSCAR EL PUNTO MÁS ALTO (CIMA)
    float maxY = -99999.0f;

    // Recorremos todo el mapa de alturas
    for (int i = 0; i < terrainHeight; i++) {
        for (int j = 0; j < terrainWidth; j++) {
            int index = (i * terrainWidth) + j;
            float h = heightMap[index];

            if (h > maxY) {
                maxY = h;
                // Convertimos índice i,j a coordenadas de mundo X,Z
                float x = (float)j - (terrainWidth / 2.0f);
                float z = (float)i - (terrainHeight / 2.0f);
                castlePosition = glm::vec3(x - 1.2f, maxY - 1.5f, z + 1.0f);
            }
        }
    }
    std::cout << "Cima encontrada en: " << castlePosition.x << ", " << castlePosition.y << ", " << castlePosition.z << "\n";
}

// --- NUEVO: DIBUJAR CASTILLO ---
void App::drawCastle(glm::mat4 view, glm::mat4 projection) {
    if (castleVertexCount == 0) return;

    glm::mat4 model = glm::mat4(1.0f);

    // 1. Colocar en la cima encontrada
    // Ajuste opcional: castlePosition.y - 1.0f si queda flotando
    model = glm::translate(model, castlePosition);

    // 2. Escala (AJUSTA ESTO SI EL CASTILLO ES MUY GRANDE O PEQUEÑO)
    model = glm::scale(model, glm::vec3(0.5f));

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram1, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // 3. Configurar shader para color sólido GRIS
    glUniform1i(glGetUniformLocation(shaderProgram1, "useModelTexture"), false); // No usar textura
    glUniform1i(glGetUniformLocation(shaderProgram1, "useSolidColor"), true);    // Usar color sólido

    // Color Gris (R, G, B)
    glUniform3f(glGetUniformLocation(shaderProgram1, "objectColor"), 0.25f, 0.25f, 0.25f);

    glBindVertexArray(castleVAO);
    glDrawArrays(GL_TRIANGLES, 0, castleVertexCount);
}

// --- UTILIDADES ---
float App::getHeightAt(float x, float z) {
    if (heightMap.empty()) return 0.0f;
    float imgX = x + (terrainWidth / 2.0f);
    float imgZ = z + (terrainHeight / 2.0f);
    int col = (int)imgX; int row = (int)imgZ;
    if (col < 0 || col >= terrainWidth || row < 0 || row >= terrainHeight) return -100.0f;
    return heightMap[(row * terrainWidth) + col];
}

void App::drawModelAt(glm::mat4 view, glm::mat4 projection, glm::vec3 pos) {
    glm::mat4 model = glm::mat4(1.0f);
    // Ajustar altura del árbol
    glm::vec3 posCorregida = pos + glm::vec3(0.0f, 2.0f, 0.0f);

    model = glm::translate(model, posCorregida);
    model = glm::scale(model, glm::vec3(4.0f)); // Tamaño árbol

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram1, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(glGetUniformLocation(shaderProgram1, "useModelTexture"), true);
    glUniform1i(glGetUniformLocation(shaderProgram1, "useSolidColor"), false);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, textureTreeID);
    glUniform1i(glGetUniformLocation(shaderProgram1, "textureModel"), 2);

    glBindVertexArray(modelVAO);
    glDrawArrays(GL_TRIANGLES, 0, modelVertexCount);
}

// --- MAIN LOOP ---
void App::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, deltaTime);

        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram1);
        glm::mat4 view = camera->GetViewMatrix();
        glm::mat4 projection = camera->GetProjectionMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram1, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram1, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // 1. DIBUJAR TERRENO
        glUniform1i(glGetUniformLocation(shaderProgram1, "useModelTexture"), false);
        glUniform1i(glGetUniformLocation(shaderProgram1, "useSolidColor"), false);
        glUniform1f(glGetUniformLocation(shaderProgram1, "globalAlpha"), 1.0f);

        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram1, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, textureRockID);
        glUniform1i(glGetUniformLocation(shaderProgram1, "textureRock"), 0);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, textureGrassID);
        glUniform1i(glGetUniformLocation(shaderProgram1, "textureGrass"), 1);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);

        // 2. DIBUJAR ÁRBOLES
        for (const auto& pos : treePositions) {
            drawModelAt(view, projection, pos);
        }

        // 3. DIBUJAR CASTILLO (NUEVO)
        drawCastle(view, projection);

        // 4. DIBUJAR AGUA (Al final por la transparencia)
        glUniform1i(glGetUniformLocation(shaderProgram1, "useModelTexture"), false);
        glUniform1i(glGetUniformLocation(shaderProgram1, "useSolidColor"), false);
        glUniform1f(glGetUniformLocation(shaderProgram1, "globalAlpha"), 0.6f);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, textureWaterID);
        glUniform1i(glGetUniformLocation(shaderProgram1, "textureGrass"), 1);

        glBindVertexArray(waterVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void App::cleanup() {
    delete camera;
    glDeleteVertexArrays(1, &VAO); glDeleteBuffers(1, &VBO); glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &waterVAO); glDeleteBuffers(1, &waterVBO);
    glDeleteVertexArrays(1, &modelVAO); glDeleteBuffers(1, &modelVBO);
    glDeleteVertexArrays(1, &castleVAO); glDeleteBuffers(1, &castleVBO); // Limpiar
    glDeleteProgram(shaderProgram1);
    glfwDestroyWindow(window); glfwTerminate();
}

// --- INPUT & SETUP (Sin cambios) ---
void App::mouse_callback(double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX; float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    camera->ProcessMouseMovement(xoffset, yoffset);
}
void App::processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera->ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera->ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera->ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera->ProcessKeyboard(RIGHT, deltaTime);
}
std::string App::loadShaderSource(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    std::stringstream buffer; buffer << file.rdbuf(); return buffer.str();
}
void App::initShaders() {
    std::string vC = loadShaderSource("../shaders/basic.vs");
    std::string fC = loadShaderSource("../shaders/basic.fs");
    const char* vCode = vC.c_str(); const char* fCode = fC.c_str();
    unsigned int vShader = glCreateShader(GL_VERTEX_SHADER); glShaderSource(vShader, 1, &vCode, nullptr); glCompileShader(vShader);
    unsigned int fShader = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(fShader, 1, &fCode, nullptr); glCompileShader(fShader);
    shaderProgram1 = glCreateProgram(); glAttachShader(shaderProgram1, vShader); glAttachShader(shaderProgram1, fShader); glLinkProgram(shaderProgram1);
    glDeleteShader(vShader); glDeleteShader(fShader);
}
unsigned int App::loadTexture(const char* path) {
    unsigned int tID; glGenTextures(1, &tID);
    int w, h, c; unsigned char* data = stbi_load(path, &w, &h, &c, 0);
    if (data) {
        GLenum fmt = (c == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, tID); glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    return tID;
}
void App::initTextures() {
    textureRockID = loadTexture("../Assets/Textures/rocky_terrain_02_diff_4k.jpg");
    textureGrassID = loadTexture("../Assets/Textures/grass.jpg");
    textureWaterID = loadTexture("../Assets/Textures/water.jpg");
}
void App::initTerrain() {
    int nrChannels; unsigned char* data = stbi_load("../Assets/Textures/heightmap.png", &terrainWidth, &terrainHeight, &nrChannels, 0);
    if (!data) return;
    heightMap.clear(); heightMap.resize(terrainWidth * terrainHeight);
    std::vector<float> vertices; std::vector<unsigned int> indices;
    for (int i = 0; i < terrainHeight; i++) {
        for (int j = 0; j < terrainWidth; j++) {
            float y = (int)data[(j + terrainWidth * i) * nrChannels] * terrainYScale + terrainYShift;
            heightMap[(i * terrainWidth) + j] = y;
            float x = (float)j - (terrainWidth / 2.0f); float z = (float)i - (terrainHeight / 2.0f);
            float hL = y, hR = y, hD = y, hU = y;
            if (j > 0) hL = (int)data[(j - 1 + terrainWidth * i) * nrChannels] * terrainYScale + terrainYShift;
            if (j < terrainWidth - 1) hR = (int)data[(j + 1 + terrainWidth * i) * nrChannels] * terrainYScale + terrainYShift;
            if (i > 0) hD = (int)data[(j + terrainWidth * (i - 1)) * nrChannels] * terrainYScale + terrainYShift;
            if (i < terrainHeight - 1) hU = (int)data[(j + terrainWidth * (i + 1)) * nrChannels] * terrainYScale + terrainYShift;
            glm::vec3 n = glm::normalize(glm::vec3(hL - hR, 2.0f, hD - hU));
            vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
            vertices.push_back(n.x); vertices.push_back(n.y); vertices.push_back(n.z);
            vertices.push_back((float)j / terrainWidth * 40.0f); vertices.push_back((float)i / terrainHeight * 40.0f);
        }
    }
    stbi_image_free(data);
    for (int i = 0; i < terrainHeight - 1; i++) {
        for (int j = 0; j < terrainWidth - 1; j++) {
            int tl = (i * terrainWidth) + j, tr = tl + 1, bl = ((i + 1) * terrainWidth) + j, br = bl + 1;
            indices.push_back(tl); indices.push_back(bl); indices.push_back(tr);
            indices.push_back(tr); indices.push_back(bl); indices.push_back(br);
        }
    }
    numIndices = indices.size();
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO); glGenBuffers(1, &EBO);
    glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}
void App::initWater() {
    float wh = -100.0f, s = 10000.0f;
    float wv[] = { -s,wh,-s,0,1,0,0,s / 2,-s,wh,s,0,1,0,0,0,s,wh,-s,0,1,0,s / 2,s / 2,s,wh,-s,0,1,0,s / 2,s / 2,-s,wh,s,0,1,0,0,0,s,wh,s,0,1,0,s / 2,0 };
    glGenVertexArrays(1, &waterVAO); glGenBuffers(1, &waterVBO);
    glBindVertexArray(waterVAO); glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wv), wv, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}