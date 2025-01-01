#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "Block.h"
#include "glad/glad.h"

struct Vertex {
    glm::vec3 position;
    glm::vec2 uv;
};

constexpr glm::vec3 chunkSize = {16, 16, 1};
constexpr glm::vec3 extentsMin = { -0.5f, -0.5f, -0.5f };
constexpr glm::vec3 extentsMax = extentsMin + chunkSize;

class Chunk
{
public:
    Chunk(glm::vec2 _chunkIndex);
    ~Chunk();

    void render();

private:
    void generateChunk();
    void initShaderVars();

    void insertVertsAndInds(Block& b);
    bool isFaceVisible(glm::vec3& pos, BlockFace face);

private:
    glm::vec3 startPos = { 0, 0, 0 };

    GLuint vao, vbo, ebo;
    std::vector<Vertex> vertices = {};
    std::vector<GLuint> indices = {};
    Block blocks[(GLuint)chunkSize.x][(GLuint)chunkSize.y][(GLuint)chunkSize.z] = {};
};

