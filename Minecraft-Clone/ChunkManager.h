#pragma once
#include <map>
#include <glm/vec2.hpp>
#include <thread>
#include <mutex>
#include <queue>

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

	ChunkManager();
	~ChunkManager();

	void initChunks(uint8_t renderDistance);
	void renderChunks();

	size_t chunkCount();
	const size_t getFaceCount() const;
	Chunk* getChunkAtIndex(glm::vec2& index);
	const std::pair<const glm::vec2, Chunk*>& at(size_t index) const;

	void removeChunk(glm::vec2& chunkIndex);
	void addChunk(glm::vec2& chunkIndex);

	void checkForLoadedChunks();

private:
	void loadingThreadFunc();

private:
	static ChunkManager* instance;
	
	std::map<glm::vec2, Chunk*, Vec2Comparator> worldChunks = {};

	std::thread loadingThread;
	bool shouldLoadChunks = true;
	std::queue<glm::vec2> indexToLoad = {};
	std::queue<Chunk*> loadedChunks = {};
	std::mutex chunkMutex;
};