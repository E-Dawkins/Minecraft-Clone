#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>
#include <fstream>
#include <sstream>
#include <SOIL2/SOIL2.h>
#include <vector>

const int WINDOW_WIDTH = 640, WINDOW_HEIGHT = 480;

void processInput(GLFWwindow* window);
void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
std::string loadShader(std::string path);

struct Vertex {
    glm::vec3 position;
    glm::vec2 uv;
    int faceIndex; // FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM
};

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

    // Create vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create a vertex buffer object and copy the vertex data into it
    GLuint vbo;
    glGenBuffers(1, &vbo);

    Vertex vertices[] = {
        // front face
        { { -0.5f, -0.5f,  0.5f },    { 0.0f, 0.0f },   0 },
        { {  0.5f, -0.5f,  0.5f },    { 1.0f, 0.0f },   0 },
        { {  0.5f, -0.5f, -0.5f },    { 1.0f, 1.0f },   0 },
        { { -0.5f, -0.5f, -0.5f },    { 0.0f, 1.0f },   0 },
        // back face
        { {  0.5f,  0.5f,  0.5f },    { 0.0f, 0.0f },   1 },
        { { -0.5f,  0.5f,  0.5f },    { 1.0f, 0.0f },   1 },
        { { -0.5f,  0.5f, -0.5f },    { 1.0f, 1.0f },   1 },
        { {  0.5f,  0.5f, -0.5f },    { 0.0f, 1.0f },   1 },
        // left face
        { { -0.5f,  0.5f,  0.5f },    { 0.0f, 0.0f },   2 },
        { { -0.5f, -0.5f,  0.5f },    { 1.0f, 0.0f },   2 },
        { { -0.5f, -0.5f, -0.5f },    { 1.0f, 1.0f },   2 },
        { { -0.5f,  0.5f, -0.5f },    { 0.0f, 1.0f },   2 },
        // right face
        { {  0.5f, -0.5f,  0.5f },    { 0.0f, 0.0f },   3 },
        { {  0.5f,  0.5f,  0.5f },    { 1.0f, 0.0f },   3 },
        { {  0.5f,  0.5f, -0.5f },    { 1.0f, 1.0f },   3 },
        { {  0.5f, -0.5f, -0.5f },    { 0.0f, 1.0f },   3 },
        // top face
        { { -0.5f,  0.5f,  0.5f },    { 0.0f, 0.0f },   4 },
        { {  0.5f,  0.5f,  0.5f },    { 1.0f, 0.0f },   4 },
        { {  0.5f, -0.5f,  0.5f },    { 1.0f, 1.0f },   4 },
        { { -0.5f, -0.5f,  0.5f },    { 0.0f, 1.0f },   4 },
        // bottom face
        { { -0.5f, -0.5f, -0.5f },    { 0.0f, 0.0f },   5 },
        { {  0.5f, -0.5f, -0.5f },    { 1.0f, 0.0f },   5 },
        { {  0.5f,  0.5f, -0.5f },    { 1.0f, 1.0f },   5 },
        { { -0.5f,  0.5f, -0.5f },    { 0.0f, 1.0f },   5 },
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create an element array buffer
    GLuint ebo;
    glGenBuffers(1, &ebo);

    GLuint indices[] = {
        0 + 0, 0 + 1, 0 + 2,    0 + 2, 0 + 3, 0 + 0,        // front face
        4 + 0, 4 + 1, 4 + 2,    4 + 2, 4 + 3, 4 + 0,        // back face
        8 + 0, 8 + 1, 8 + 2,    8 + 2, 8 + 3, 8 + 0,        // left face
        12 + 0, 12 + 1, 12 + 2,    12 + 2, 12 + 3, 12 + 0,  // right face
        16 + 0, 16 + 1, 16 + 2,    16 + 2, 16 + 3, 16 + 0,  // top face
        20 + 0, 20 + 1, 20 + 2,    20 + 2, 20 + 3, 20 + 0,  // bottom face
    };

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Create / load texture
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);

    int width, height;
    unsigned char* image = SOIL_load_image("./assets/texture-atlas.png", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

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

    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));

    GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, uv)));

    GLint indexAttrib = glGetAttribLocation(shaderProgram, "faceIndex");
    glEnableVertexAttribArray(indexAttrib);
    glVertexAttribIPointer(indexAttrib, 1, GL_INT, sizeof(Vertex), (void*)(offsetof(Vertex, faceIndex)));

    // Bind shader uniforms
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");

    // Set up projection
    glm::mat4 view = glm::lookAt(
        glm::vec3(3.0f, 3.0f, 3.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 1.0f, 10.0f);
    GLint projLoc = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

    // Pass uv offset to the shader
    std::vector<glm::vec2> offsets = { {2, 0}, {2, 0}, {2, 0}, {2, 0}, {1, 0}, {0, 0} };
    GLint offsetLoc = glGetUniformLocation(shaderProgram, "uvOffsets");
    glUniform2fv(offsetLoc, offsets.size(), &offsets[0][0]);

    auto t_start = std::chrono::high_resolution_clock::now();

    glEnable(GL_DEPTH_TEST);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        /* Render here */
        glClearColor(0.6f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // rotate transform around the z-axis
        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);

    glDeleteVertexArrays(1, &vao);

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
