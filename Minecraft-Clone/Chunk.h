#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "Block.h"
#include "glad/glad.h"

constexpr glm::vec3 chunkSize = {16, 16, 1};
constexpr glm::vec3 extentsMin = { -0.5f, -0.5f, -0.5f };
constexpr glm::vec3 extentsMax = extentsMin + chunkSize;

struct FaceData {
    glm::vec3 offset;
    int id;
    glm::vec2 texcoordStart;
    glm::vec3 size;
};

class Chunk
{
public:
    Chunk(glm::vec2 _chunkIndex);
    ~Chunk();

    void render();

    // @returns Whether a position is within the chunk boundaries. [startPos -> startPos + chunkSize)
    bool isPosInChunk(glm::vec3 pos);

    // @returns Whether a position is within the chunk size. [0 -> chunkSize)
    bool isPosInChunkSize(glm::vec3 pos);

private:
    void generateChunk();
    void generateFaces();
    void optimizeFaces();
    void initShaderVars();

    void insertFaceData(Block& b);
    bool isFaceVisible(glm::vec3& pos, BlockFace face);

private:
    glm::vec3 startPos = { 0, 0, 0 };

    GLuint vao, vbo, ebo;
    std::vector<FaceData> faceData = {};
    Block blocks[(GLuint)chunkSize.x][(GLuint)chunkSize.y][(GLuint)chunkSize.z] = {};
};

