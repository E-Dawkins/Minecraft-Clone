#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "Block.h"
#include "glad/glad.h"

constexpr glm::vec3 chunkSize = {16, 16, 256};
constexpr glm::vec3 extentsMin = { -0.5f, -0.5f, -0.5f };
constexpr glm::vec3 extentsMax = extentsMin + chunkSize;

struct FaceData {
    glm::vec3 offset;
    BlockFace id;
    glm::vec2 texcoordStart;
    glm::vec3 size = glm::vec3(1);
};

class Chunk
{
public:
    Chunk(glm::vec2 _chunkIndex);
    ~Chunk();

    void init();
    void render();

    // @returns Whether a position is within the chunk boundaries. [startPos -> startPos + chunkSize)
    bool isPosInChunk(glm::vec3 pos);

    // @returns Whether a position is within the chunk size. [0 -> chunkSize)
    bool isPosInChunkSize(glm::vec3 pos);

    // @returns The chunk index that contains the position
    static glm::vec2 posToChunkIndex(const glm::vec3& pos) {
        return glm::floor(pos / chunkSize);
    }

    const glm::vec2 getChunkIndex() const {
        return chunkIndex;
    }

    const size_t getFaceCount() const {
        return faceData.size();
    }

private:
    void generateChunk();
    void generateFaces();
    void optimizeFaces();
    void initShaderVars();

    void insertFaceData(Block& b);
    bool isFaceVisible(glm::vec3& pos, BlockFace face);

    glm::vec3 getBlockPosFromFace(glm::vec3& facePos, BlockFace face);

private:
    glm::vec3 startPos = { 0, 0, 0 };
    glm::vec2 chunkIndex = { 0, 0 };

    GLuint vao, vbo, ebo;
    GLuint faceDataBuffer;
    std::vector<FaceData> faceData = {};
    std::vector<std::vector<std::vector<Block>>> blocks;
};

