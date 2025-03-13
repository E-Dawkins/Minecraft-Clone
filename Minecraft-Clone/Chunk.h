#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "BlockAttribs.h"
#include "glad/glad.h"

constexpr glm::vec3 chunkSize = {16, 16, 128};
constexpr glm::vec3 extentsMin = { -0.5f, -0.5f, -0.5f };
constexpr glm::vec3 extentsMax = extentsMin + chunkSize;

struct FaceData {
    // position     x: 4 bits   y: 4 bits   z: 8 bits
    // direction     : 3 bits
    // block id      : 4 bits
    // TOTAL         : 23 bits
    
    uint16_t position = 0;
    uint8_t direction_id = 0;

    void setPosition(const glm::ivec3& p) {
        // p => xxxx yyyy zzzz zzzz
        position = ((p.x & 15) << 4 | (p.y & 15)) << 8 | (p.z & 255);
    }

    void setDirection(const BlockFace d) {
        // d => 0ddd ----
        direction_id = ((d & 7) << 4) | (direction_id & 15);
    }

    void setBlockId(const uint16_t b) {
        // b => ---- bbbb
        direction_id = (direction_id & 240) | (b & 15);
    }

    const BlockType getBlockId() const {
        return (BlockType)(direction_id & 15);
    }
};

class Chunk
{
public:
    Chunk(glm::vec2 _chunkIndex);
    ~Chunk();

    void init();
    void render();
    void update();

    void deleteBlockAtIndex(const glm::ivec3 index);

    // @returns The chunk index that contains the position
    static glm::vec2 posToChunkIndex(const glm::vec3& pos) {
        return glm::floor(pos / chunkSize);
    }

    const BlockType getBlockAtIndex(const glm::ivec3& index) const {
        if (!isValidBlockIndex(index)) {
            return AIR;
        }

        return blocks[index.x][index.y][index.z];
    }
    
    const glm::vec3 getStartPos() const {
        return startPos;
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
    void initShaderVars();

    void insertFaceData(glm::vec3& blockIndex);
    bool isFaceVisible(glm::vec3& pos, BlockFace face);
    bool isValidBlockIndex(const glm::ivec3 index) const;

private:
    glm::vec3 startPos = { 0, 0, 0 };
    glm::vec2 chunkIndex = { 0, 0 };

    GLuint vao, vbo, ebo;
    GLuint faceDataBuffer;
    std::vector<FaceData> faceData = {};
    std::vector<std::vector<std::vector<BlockType>>> blocks;

    std::vector<glm::ivec3> indexesToDelete = {};
};

