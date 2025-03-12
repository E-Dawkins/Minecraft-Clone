#pragma once
#include <glm/glm.hpp>
#include <string_view>
#include <algorithm>

enum BlockType : int8_t {
	AIR = -1,
	DIRT,
	GRASS,
	STONE,
	COBBLESTONE,

	TYPE_COUNT
};

// +1 to account for AIR being -1
constexpr std::string_view BlockNames[TYPE_COUNT + 1] = {
	"Air",
	"Dirt",
	"Grass",
	"Stone",
	"Cobblestone"
};
static_assert(std::ranges::all_of(BlockNames, [](const std::string_view& s) {return !s.empty(); }), "Not enough block names!");

enum BlockFace : uint8_t {
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

constexpr uint8_t blockTextureIds[TYPE_COUNT][6] = {
	//	FRONT BACK LEFT RIGHT TOP BOTTOM
	{ 0, 0, 0, 0, 0, 0 }, // DIRT
	{ 2, 2, 2, 2, 1, 0 }, // GRASS
	{ 3, 3, 3, 3, 3, 3 }, // STONE
	{ 4, 4, 4, 4, 4, 4 }, // COBBLESTONE
};