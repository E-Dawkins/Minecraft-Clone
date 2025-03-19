// Include imgui first...
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

// ...then glad
#include <glad/glad.h>

// ...then GLFW
#include <GLFW/glfw3.h>

// ...and everything else that we need
#include <iostream>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>
#include <fstream>
#include <sstream>

#include "Chunk.h"
#include "AssetManager.h"
#include "Camera.h"
#include "ChunkManager.h"
#include "DebugClock.h"
#include "Raycast.h"
#include "Config.h"

int WINDOW_WIDTH = 0, WINDOW_HEIGHT = 0;
Camera cam = Camera({ chunkSize.x / 2, chunkSize.y / 2, 12 }, { 1, 1, 0 });
glm::vec2 camChunkIndex = Chunk::posToChunkIndex(cam.getPosition());
GLuint renderingMode = 0;
GLuint numRenderingModes = 2; // normal, wire-frame
GLuint renderDistance = 0;
BlockType currentBlockType = DIRT;
bool drawImGui = false;

void setupConfig();
void processInput(GLFWwindow* window);
void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void setRenderingMode(GLuint newMode);
void reloadChunks();

int main(void)
{
    setupConfig();

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
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Minecraft Clone", NULL, NULL);
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
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    // setup GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to load OpenGL function pointers!" << std::endl;
        glfwTerminate();
        return -1;
    }

    AssetManager::loadShader("generic", "./assets/generic.vert", "./assets/generic.frag");
    AssetManager::loadShader("line", "./assets/line.vert", "./assets/line.frag");
    GLuint shaderProgram = AssetManager::getAssetHandle("generic");

    // Bind shader uniforms
    // Set up projection
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cam.getView()));

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.0f);
    GLint projLoc = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

    GLint renderDistLoc = glGetUniformLocation(shaderProgram, "renderDist");
    glUniform1ui(renderDistLoc, renderDistance);

    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    setRenderingMode(0); // set to default rendering mode

    DebugClock::setEnabled(false);
    DebugClock::recordTime("Chunk gen start");

    ChunkManager::getInstance()->initChunks((uint8_t)renderDistance);

    DebugClock::recordTime("Chunk gen end");
    DebugClock::printTimePoints();

    AssetManager::loadTexture("./assets/texture-atlas.png");

    const int targetFPS = 60;
    const double frameDuration = 1.0 / targetFPS;

    auto t_previous = std::chrono::high_resolution_clock::now();

    // Check ImGui version and initialise ImGui specific variables
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); /*(void)io;*/
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    float minFPS = FLT_MAX;
    float maxFPS = FLT_MIN;

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

            reloadChunks();
        }

        ChunkManager::getInstance()->checkForLoadedChunks();
        ChunkManager::getInstance()->updateChunks();

        /* Render here */
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Tell ImGui we are working with a new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ChunkManager::getInstance()->renderChunks();

        float currentFPS = io.Framerate;
        if (currentFPS < minFPS) minFPS = currentFPS;
        if (currentFPS > maxFPS) maxFPS = currentFPS;

        size_t faceCount = ChunkManager::getInstance()->getFaceCount();

        if (drawImGui) {
            // Setup ImGui window/s here
            ImGui::SetNextWindowSize(ImVec2(0, 0)); // set next window to auto-fit its' content
            ImGui::SetNextWindowPos(ImVec2(50, 50));
            ImGui::Begin("FPS Counter");
            ImGui::Text("Target: %.1f", (float)targetFPS);
            ImGui::Text("Current: %.1f", currentFPS);
            ImGui::Text("Range: %.1f-%.1f", minFPS, maxFPS);
            ImGui::End();

            ImGui::SetNextWindowSize(ImVec2(0, 0)); // set next window to auto-fit its' content
            ImGui::SetNextWindowPos(ImVec2(50, 150));
            ImGui::Begin("Graphic Info.");
            ImGui::Text("Faces: %i", faceCount);
            ImGui::Text("Face Data: %.2f kb", (sizeof(FaceData) * faceCount) / 1'024.f);
            ImGui::End();

            ImGui::SetNextWindowSize(ImVec2(0, 0)); // set next window to auto-fit its' content
            ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH - 50.f, 50), 0, ImVec2(1, 0));
            ImGui::Begin("Block Info.");
            ImGui::Text("Placable Block: %s", BlockNames[currentBlockType + 1].data());
            ImGui::End();
        }

        // Draw ImGui window/s here
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete ChunkManager::getInstance();

    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

void setupConfig() {
    Config::loadConfigFile("./assets/config.cfg");

    renderDistance = Config::getVar<int>("renderDistance");
    
    WINDOW_WIDTH = Config::getVar<int>("width");
    WINDOW_HEIGHT = Config::getVar<int>("height");

    drawImGui = Config::getVar<bool>("drawImGui");
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

    // rendering controls
    static bool f1Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS && !f1Pressed) {
        setRenderingMode(renderingMode + 1);
        f1Pressed = true;
    }

    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_RELEASE) {
        f1Pressed = false;
    }

    // block selection
    static bool qPressed = false;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && !qPressed) {
        currentBlockType = (BlockType)(currentBlockType - 1);
        qPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE) {
        qPressed = false;
    }

    static bool ePressed = false;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !ePressed) {
        currentBlockType = (BlockType)(currentBlockType + 1);
        ePressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE) {
        ePressed = false;
    }

    if (currentBlockType < 0) currentBlockType = (BlockType)(BlockType::TYPE_COUNT - 1);
    if (currentBlockType >= BlockType::TYPE_COUNT) currentBlockType = (BlockType)0;
}

#pragma warning(suppress: 4100)
void frameBufferSizeCallback(GLFWwindow* window, int width, int height) {
    // tell OpenGL that the new rendering area is at position (0, 0) and extents (width, height)
    glViewport(0, 0, width, height);
}

bool firstMouse = true;
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    const int halfWidth = WINDOW_WIDTH / 2, halfHeight = WINDOW_HEIGHT / 2;
    
    if (!firstMouse) {
        cam.addLookInput({ xpos - halfWidth, halfHeight - ypos });
    } else {
        firstMouse = false;
    }

    glfwSetCursorPos(window, halfWidth, halfHeight);
}

#pragma warning(suppress: 4100)
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    cam.moveSpeed = std::max(cam.moveSpeed + (float)yoffset, 1.0f);
}

#pragma warning(suppress: 4100)
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    HitResult r = Raycast::getHitResult(cam.getPosition(), cam.getForwardDir(), 10.f);

    // Raycast did not hit a valid block
    if (r.hitType == BlockType::AIR) {
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // r.hitPos = the position of the 'removed' block
        glm::vec2 chunkIndex = Chunk::posToChunkIndex(r.hitPos);
        if (Chunk* c = ChunkManager::getInstance()->getChunkAtIndex(chunkIndex)) {
            c->changeBlockAtIndex({ r.hitPos - glm::ivec3(c->getStartPos()), AIR });
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        // r.hitPos + r.hitNormal = the position of the 'added' block
        glm::vec2 chunkIndex = Chunk::posToChunkIndex(r.hitPos + r.hitNormal);
        if (Chunk* c = ChunkManager::getInstance()->getChunkAtIndex(chunkIndex)) {
            c->changeBlockAtIndex({ r.hitPos - glm::ivec3(c->getStartPos()) + r.hitNormal, currentBlockType });
        }
    }
}

void setRenderingMode(GLuint newMode) {
    renderingMode = newMode % numRenderingModes;

    switch (renderingMode) {
        case 0: {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } break;

        case 1: {
            glDisable(GL_CULL_FACE);

            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } break;
    }
}

void reloadChunks() {
    // check if camera has moved across chunk boundaries
    glm::vec2 chunkIndex = Chunk::posToChunkIndex(cam.getPosition());
    if (chunkIndex == camChunkIndex) {
        return;
    }

    // any chunk outside render distance should:
    // -> be cleared of data (faces, blocks, etc.)
    // -> re-loaded at new position
    std::vector<glm::vec2> newIndexes = {};

    ChunkManager* chunkManager = ChunkManager::getInstance();
    for (int i = (int)chunkManager->chunkCount() - 1; i >= 0; i--) {
        auto& pair = chunkManager->at(i);

        if (pair.second != nullptr) {
            glm::vec2 curChunkIndex = pair.second->getChunkIndex();
            float distX = glm::distance(chunkIndex.x, curChunkIndex.x);
            float distY = glm::distance(chunkIndex.y, curChunkIndex.y);

            if (distX >= renderDistance || distY >= renderDistance) {
                chunkManager->removeChunk(curChunkIndex);
                newIndexes.emplace_back(chunkIndex - (curChunkIndex - camChunkIndex));
            }
        }
    }

    for (glm::vec2& index : newIndexes) {
        chunkManager->addChunk(index);
    }

    newIndexes.clear();

    camChunkIndex = chunkIndex;
}
