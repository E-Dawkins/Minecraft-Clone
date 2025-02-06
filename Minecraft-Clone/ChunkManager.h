#pragma once
#include <map>
#include <glm/vec2.hpp>

class Chunk;

struct Vec2Comparator {
	bool operator()(const glm::vec2& a, const glm::vec2& b) const {
		return std::tie(a.x, a.y) < std::tie(b.x, b.y);
	}
};

class ChunkManager {
public:
	static ChunkManager* getInstance() {
		if (instance == nullptr) {
			instance = new ChunkManager();
		}

		return instance;
	}

	void initChunks(uint8_t renderDistance);
	void renderChunks();

	size_t chunkCount();
	Chunk* getChunkAtIndex(glm::vec2& index);

private:
	static ChunkManager* instance;
	
	std::map<glm::vec2, Chunk*, Vec2Comparator> worldChunks = {};
};