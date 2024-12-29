#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "Block.h"
#include "glad/glad.h"

struct Vertex {
    glm::vec3 position;
    glm::vec2 uv;
};

constexpr GLuint CHUNK_X = 2,
                 CHUNK_Y = 2,
                 CHUNK_Z = 1;

class Chunk
{
public:
    Chunk(glm::vec3 _coords);
    ~Chunk();

    void render();

private:
    void generateChunk();
    void initShaderVars();

    void insertVertsAndInds(Block& b);
    bool isFaceVisible(glm::vec3& pos, BlockFace face);

public:
    glm::vec3 coords = { 0, 0, 0 };
    glm::vec3 extentsMax = { 0, 0, 0 };

private:
    GLuint vao, vbo, ebo;
    std::vector<Vertex> vertices = {};
    std::vector<GLuint> indices = {};
    Block blocks[CHUNK_X][CHUNK_Y][CHUNK_Z] = {};
};

