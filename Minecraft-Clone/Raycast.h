#pragma once
#include <glm/glm.hpp>
#include "BlockAttribs.h"
#include "ChunkManager.h"

struct HitResult {
	BlockType hitType = BlockType::AIR;
	glm::ivec3 hitPos = { 0, 0, 0 };
	glm::ivec3 hitNormal = { 0 ,0, 0 };

public:
	std::string to_string() {
		return "HitResult: " + std::string(BlockNames[hitType + 1]) + " | " +
			"p: " + std::to_string(hitPos.x) + "," + std::to_string(hitPos.y) + "," + std::to_string(hitPos.z) + " | " +
			"n: " + std::to_string(hitNormal.x) + "," + std::to_string(hitNormal.y) + "," + std::to_string(hitNormal.z);
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

		glm::vec3 hitPoint = rayStart + rayDirection * distanceTravelled;
		glm::vec3 dirToPt = glm::normalize(hitPoint - glm::vec3(gridPos));

		float largestDot = 0.f;
		glm::vec3 closestNormal = { 0, 0, 0 };

		for (uint8_t i = 0; i < BlockFace::FACE_COUNT; i++) {
			glm::vec3 currentNormal = faceNormals[i];
			float d = glm::dot(dirToPt, currentNormal);
			if (d > largestDot) {
				largestDot = d;
				closestNormal = currentNormal;
			}
		}

		return { ChunkManager::getInstance()->getBlockAtPos(gridPos), gridPos, closestNormal };
	}
};