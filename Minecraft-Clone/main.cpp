#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>
#include <fstream>
#include <sstream>

#include "Chunk.h"
#include "AssetManager.h"
#include "Camera.h"

const int WINDOW_WIDTH = 1280, WINDOW_HEIGHT = 960;
Camera cam = Camera({ 0, 0, 10 }, { 1, 0, 0 });

void processInput(GLFWwindow* window);
void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
std::string loadShader(std::string path);

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    // Tell GLFW that we are going to use OpenGL version 3.3
    // and that we are using modern OpenGL ('core profile')
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello World", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    // setup GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to load OpenGL function pointers!" << std::endl;
        glfwTerminate();
        return -1;
    }

    std::string vertexShaderStr = loadShader("./assets/generic.vert");
    const char* vertexShaderSrc = vertexShaderStr.c_str();
    std::string fragmentShaderStr = loadShader("./assets/generic.frag");
    const char* fragmentShaderSrc = fragmentShaderStr.c_str();

    // Create and compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSrc, 0); // length 0 means opengl will figure it out for us
    glCompileShader(vertexShader);
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::VERTEX_SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }


    // Create and compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, 0); // length 0 means opengl will figure it out for us
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::FRAGMENT_SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }


    // Link vertex and fragment shaders to a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "fragColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Bind shader uniforms
    // Set up projection
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cam.getView()));

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.0f);
    GLint projLoc = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    const int countX = 5, countY = 5;
    Chunk* chunks[countX][countY] = {};
    for (int x = 0; x < countX; x++) {
        for (int y = 0; y < countY; y++) {
            chunks[x][y] = new Chunk({ x, y });
        }
    }

    AssetManager::loadTexture("./assets/texture-atlas.png");

    const int targetFPS = 60;
    const double frameDuration = 1.0 / targetFPS;

    auto t_previous = std::chrono::high_resolution_clock::now();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        auto t_frameStart = std::chrono::high_resolution_clock::now();

        // Calculate delta time from previous frame
        std::chrono::duration<float> t_deltaTime = t_frameStart - t_previous;
        t_previous = t_frameStart;

        float deltaSeconds = t_deltaTime.count();
        processInput(window);
        if (cam.update(deltaSeconds)) {
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cam.getView()));
        }

        /* Render here */
        glClearColor(0.6f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (int x = 0; x < countX; x++) {
            for (int y = 0; y < countY; y++) {
                chunks[x][y]->render();
            }
        }

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        // Calculate the time taken to render & poll events
        auto t_frameEnd = std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> t_frameTime = t_frameEnd - t_frameStart;

        // Wait the rest of the frame to reach target FPS
        while (t_frameTime.count() < frameDuration) {
            t_frameEnd = std::chrono::high_resolution_clock::now();
            t_frameTime = t_frameEnd - t_frameStart;
        }
    }

    for (int x = 0; x < countX; x++) {
        for (int y = 0; y < countY; y++) {
            delete chunks[x][y];
        }
    }

    glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    // exit game
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // camera controls
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cam.addMoveInput({ 0, 1, 0 });
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cam.addMoveInput({ 0, -1, 0 });
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cam.addMoveInput({ 1, 0, 0 });
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cam.addMoveInput({ -1, 0, 0 });
    }
}

void frameBufferSizeCallback(GLFWwindow* window, int width, int height) {
    // tell OpenGL that the new rendering area is at position (0, 0) and extents (width, height)
    glViewport(0, 0, width, height);
}

bool firstMouse = true;
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    constexpr int halfWidth = WINDOW_WIDTH / 2, halfHeight = WINDOW_HEIGHT / 2;
    
    if (!firstMouse) {
        cam.addLookInput({ xpos - halfWidth, halfHeight - ypos });
    } else {
        firstMouse = false;
    }

    glfwSetCursorPos(window, halfWidth, halfHeight);
}

std::string loadShader(std::string path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to load shader at: " << path << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}
