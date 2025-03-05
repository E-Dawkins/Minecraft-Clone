#pragma once
#include "BlockAttribs.h"
#include <glad/glad.h>

class WorldGenerator
{
public:
	static BlockType getBlockTypeAtPos(glm::vec3& pos);

private:
	static GLuint getSurfaceHeightAtPos(glm::vec3& pos);
	static GLuint genRandomValFromPos(glm::vec3& pos, GLuint range);
};
