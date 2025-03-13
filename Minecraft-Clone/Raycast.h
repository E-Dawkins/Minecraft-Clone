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

constexpr float rayMarchDistance = 0.05f;

class Raycast 
{
public:
	static HitResult getHitResult(const glm::vec3& rayStart, const glm::vec3& rayDirection, float rayLength) {
		float distanceTravelled = 0.f;

		glm::ivec3 gridPos = glm::round(rayStart);

		while (distanceTravelled < rayLength) {
			gridPos = glm::round(rayStart + rayDirection * distanceTravelled);
			if (ChunkManager::getInstance()->getBlockAtPos(gridPos) != AIR) {
				break;
			}

			distanceTravelled += rayMarchDistance;
		}

		return { ChunkManager::getInstance()->getBlockAtPos(gridPos), gridPos };
	}
};