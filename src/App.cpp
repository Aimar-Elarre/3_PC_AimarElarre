#include "App.h"
#include <iostream>
#include <fstream>
#include <sstream>

App::App() {
    init();
}

App::~App() {
    cleanup();
}

void App::init() {
    if (!glfwInit()) {
        std::cerr << "Error iniciando GLFW\n";
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, "Triángulo con índices", NULL, NULL);
    if (!window) {
        std::cerr << "Error creando ventana\n";
        glfwTerminate();
        exit(-1);
    }
  /*  void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_E && action == GLFW_PRESS) { currentState = !currentState; }
    }
    glfwSetKeyCallback(window, key_callback);*/
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Error iniciando GLAD\n";
        exit(-1);
    }

    glViewport(0, 0, 800, 800);

    initShaders();
    initTriangle();
}

std::string App::loadShaderSource(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el shader: " << path << "\n";
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void App::initShaders() {
    std::string vertexCode = loadShaderSource("../shaders/basic.vs");
    std::string fragmentCode = loadShaderSource("../shaders/basic.fs");

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, nullptr);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Error compilando Vertex Shader:\n" << infoLog << "\n";
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Error compilando Fragment Shader:\n" << infoLog << "\n";
    }

    shaderProgram1 = glCreateProgram();
    glAttachShader(shaderProgram1, vertexShader);
    glAttachShader(shaderProgram1, fragmentShader);
    glLinkProgram(shaderProgram1);

    glGetProgramiv(shaderProgram1, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram1, 512, nullptr, infoLog);
        std::cerr << "Error linkeando Shader Program:\n" << infoLog << "\n";
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void App::initTriangle() {
    /*float vertices[] = {
         0.0f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 2
    };*/
    float vertices[] = {
     0.5f,  0.5f, 0.0f,  /* top right*/ 1.f, 0.f, 0.f,
     0.5f, -0.5f, 0.0f,  /* bottom right*/ 0.f, 1.f, 0.f,
    -0.5f, -0.5f, 0.0f,  /* bottom left*/ 0.f, 0.f, 1.f,
    -0.5f,  0.5f, 0.0f,   /* top left */ 1.f, 0.f, 1.f
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    /*glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);*/
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void App::run() {
    mainLoop();
}

void App::mainLoop() {
    float velx = 0.75;
    float vely = -0.75;
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram1);

        float timeValue = glfwGetTime();
        
        int vertexColorLocation = glGetUniformLocation(shaderProgram1, "positions");
        void glUniform3f(0, 1, 1, 1);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void App::cleanup() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram1);

    glfwDestroyWindow(window);
    glfwTerminate();
}
