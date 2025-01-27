#include "WorldGenerator.h"
#include <fast-noise/FastNoiseLite.h>
#include <ctime>
#include <sstream>

BlockType WorldGenerator::getBlockTypeAtPos(glm::vec3& pos) {
    GLuint height = getSurfaceHeightAtPos(pos);

    if (pos.z == height) return GRASS;

    // thresholds for dirt and stone
    if (pos.z < height) {
        if (pos.z >= height - genRandomValFromPos(pos, 4)) {
            return DIRT;
        }
        
        return STONE;
    }

    return AIR;
}

FastNoiseLite gen;
GLuint WorldGenerator::getSurfaceHeightAtPos(glm::vec3& pos) {
    // init FastNoiseLite generator with random seed (but only once!)
    static bool isGenInitialized = false;
    if (!isGenInitialized) {
        std::srand((unsigned int)time(0));
        gen = FastNoiseLite(std::rand());
        isGenInitialized = true;
    }

    // Rescale from -1.0:+1.0 to 0.0:1.0
    double n = gen.GetNoise(pos.x, pos.y) / 2.0 + 0.5;
    return (GLuint)(n * 10);
}

GLuint WorldGenerator::genRandomValFromPos(glm::vec3& pos, GLuint range) {
    std::stringstream ss;
    ss << pos.x << "," << pos.y;

    std::string coordStr = ss.str();

    // simple hash
    GLuint hash = 0;
    GLuint primeA = 373;
    GLuint primeB = 49009;
    GLuint primeC = 37;

    for (char c : coordStr) {
        hash = hash * primeA + c;
        hash = (hash ^ primeB) * primeC;
    }

    std::srand(hash);

    return std::rand() % range;
}
