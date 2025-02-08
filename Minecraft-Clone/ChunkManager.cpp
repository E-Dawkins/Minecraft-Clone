#include "ChunkManager.h"
#include "Chunk.h"
#include <ranges>

ChunkManager* ChunkManager::instance = nullptr;

void ChunkManager::initChunks(uint8_t renderDistance) {
	int yCount = 0, yOffset = 1;
	for (int x = -renderDistance; x <= renderDistance; x++) {
		for (int y = -yCount; y <= yCount; y++) {
			glm::vec2 chunkIndex = { x, y };
			worldChunks[chunkIndex] = new Chunk(chunkIndex);
		}

		if (x == 0)
			yOffset = -1;

		yCount += yOffset;
	}

	for (auto& c : worldChunks) {
		c.second->init();
	}
}

void ChunkManager::renderChunks() {
	for (auto& c : worldChunks) {
		c.second->render();
	}
}

size_t ChunkManager::chunkCount() {
	return worldChunks.size();
}

Chunk* ChunkManager::getChunkAtIndex(glm::vec2& index) {
	if (worldChunks.find(index) != worldChunks.end()) {
		return worldChunks[index];
	}

	return nullptr;
}

const std::pair<const glm::vec2, Chunk*>& ChunkManager::at(size_t index) const {
	auto itr = worldChunks.begin();
	std::advance(itr, index);
	return *itr;
}

void ChunkManager::removeChunk(glm::vec2& chunkIndex) {
	delete worldChunks[chunkIndex];
	worldChunks.erase(chunkIndex);
}

void ChunkManager::addChunk(glm::vec2& chunkIndex) {
	worldChunks[chunkIndex] = new Chunk(chunkIndex);
	worldChunks[chunkIndex]->init();
}
