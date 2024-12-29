#pragma once
#include <glm/glm.hpp>

enum BlockType {
	AIR = -1,
	DIRT,
	GRASS,
	STONE,
	COBBLESTONE,

	TYPE_COUNT
};

constexpr float blockOffset = 16.f / 128.f;

#define BLOCK_OFFSET(x, y) glm::vec2(blockOffset * x, blockOffset * y)

constexpr glm::vec2 blockTextureOffsets[TYPE_COUNT][6] = {
	//	FRONT				BACK				LEFT				RIGHT				TOP					BOTTOM
	{BLOCK_OFFSET(0, 0), BLOCK_OFFSET(0, 0), BLOCK_OFFSET(0, 0), BLOCK_OFFSET(0, 0), BLOCK_OFFSET(0, 0), BLOCK_OFFSET(0, 0) }, // DIRT
	{BLOCK_OFFSET(2, 0), BLOCK_OFFSET(2, 0), BLOCK_OFFSET(2, 0), BLOCK_OFFSET(2, 0), BLOCK_OFFSET(1, 0), BLOCK_OFFSET(0, 0) }, // GRASS
	{BLOCK_OFFSET(3, 0), BLOCK_OFFSET(3, 0), BLOCK_OFFSET(3, 0), BLOCK_OFFSET(3, 0), BLOCK_OFFSET(3, 0), BLOCK_OFFSET(3, 0) }, // STONE
	{BLOCK_OFFSET(4, 0), BLOCK_OFFSET(4, 0), BLOCK_OFFSET(4, 0), BLOCK_OFFSET(4, 0), BLOCK_OFFSET(4, 0), BLOCK_OFFSET(4, 0) }, // COBBLESTONE
};