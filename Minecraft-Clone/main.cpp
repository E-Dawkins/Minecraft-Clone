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

const int WINDOW_WIDTH = 640, WINDOW_HEIGHT = 480;

void processInput(GLFWwindow* window);
void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
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

    // Create and compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, 0); // length 0 means opengl will figure it out for us
    glCompileShader(fragmentShader);

    // Link vertex and fragment shaders to a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "fragColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Bind shader uniforms
    // Set up projection
    glm::mat4 view = glm::lookAt(
        glm::vec3(3.0f, 3.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 1.0f, 10.0f);
    GLint projLoc = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

    auto t_start = std::chrono::high_resolution_clock::now();

    glEnable(GL_DEPTH_TEST);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    Chunk c = Chunk({ 0, 0, 0 });

    AssetManager::loadTexture("./assets/texture-atlas.png");

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        /* Render here */
        glClearColor(0.6f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        //glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        c.render();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void frameBufferSizeCallback(GLFWwindow* window, int width, int height) {
    // tell OpenGL that the new rendering area is at position (0, 0) and extents (width, height)
    glViewport(0, 0, width, height);
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
