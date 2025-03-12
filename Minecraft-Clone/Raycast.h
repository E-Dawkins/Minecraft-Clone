#pragma once
#include <glm/glm.hpp>
#include "BlockAttribs.h"
#include "ChunkManager.h"

struct HitResult {
	BlockType hitType = BlockType::AIR;
	glm::ivec3 hitPos = { 0, 0, 0 };

public:
	std::string to_string() {
		return "HitResult: " + std::string(BlockNames[hitType + 1]) + " | " +
			std::to_string(hitPos.x) + "," + std::to_string(hitPos.y) + "," + std::to_string(hitPos.z);
	}
};

class Raycast 
{
public:
	static HitResult getHitResult(const glm::vec3& rayStart, const glm::vec3& rayDirection, float rayLength) {
		// 1. determine distance to next X, Y, Z boundaries
		// 3. step the smallest distance
		// 4. is the block non-air?
		// 5. if not go back to 1.
		// 6. else return HitResult using block data

		glm::ivec3 gridPos = rayStart;

		glm::ivec3 step = {
			(rayDirection.x > 0 ? 1 : -1),
			(rayDirection.y > 0 ? 1 : -1),
			(rayDirection.z > 0 ? 1 : -1)
		};

		glm::vec3 tMax = {
			(step.x > 0) ? (gridPos.x + 1 - rayStart.x) / rayDirection.x : (gridPos.x - rayStart.x) / rayDirection.x,
			(step.y > 0) ? (gridPos.y + 1 - rayStart.y) / rayDirection.y : (gridPos.y - rayStart.y) / rayDirection.y,
			(step.z > 0) ? (gridPos.z + 1 - rayStart.z) / rayDirection.z : (gridPos.z - rayStart.z) / rayDirection.z
		};

		glm::vec3 tDelta = {
			abs(1 / rayDirection.x),
			abs(1 / rayDirection.y),
			abs(1 / rayDirection.z)
		};

		while (glm::distance(rayStart, glm::vec3(gridPos)) < rayLength && 
			ChunkManager::getInstance()->getBlockAtPos(gridPos) == AIR)
		{
			float min = fminf(fminf(tMax.x, tMax.y), tMax.z);
			if (min == tMax.x) {
				tMax.x += tDelta.x;
				gridPos.x += step.x;
			}
			else if (min == tMax.y) {
				tMax.y += tDelta.y;
				gridPos.y += step.y;
			}
			else {
				tMax.z += tDelta.z;
				gridPos.z += step.z;
			}
		}

		return { ChunkManager::getInstance()->getBlockAtPos(gridPos), gridPos };
	}
};