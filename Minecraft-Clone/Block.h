#pragma once
#include <glm/glm.hpp>
#include <bitset>
#include "BlockTypes.h"

enum BlockFace : int {
	FRONT = 0,
	BACK,
	LEFT,
	RIGHT,
	TOP,
	BOTTOM,

	FACE_COUNT
};

constexpr glm::vec3 faceNormals[FACE_COUNT] = {
	{ 0,  -1,  0},		{ 0,  1,  0}, // FRONT-BACK
	{-1,  0,  0},		{ 1,  0,  0}, // LEFT-RIGHT
	{ 0,  0,  1},		{ 0,  0, -1}, // TOP-BOTTOM
};

class Block
{
public:
	glm::vec2 getTextureCoords(BlockFace face, int vert) {
		switch (vert) {
			case 0: return blockTextureOffsets[type][face];
			case 1: return blockTextureOffsets[type][face] + glm::vec2(blockOffset, 0);
			case 2: return blockTextureOffsets[type][face] + glm::vec2(blockOffset, blockOffset);
			default: return blockTextureOffsets[type][face] + glm::vec2(0, blockOffset);
		}
	}

public:
	glm::vec3 position = {0, 0, 0};
	BlockType type = AIR;

	std::bitset<6> visibleFaces;
};

