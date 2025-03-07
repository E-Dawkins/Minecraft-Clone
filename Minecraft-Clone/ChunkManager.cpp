#include "ChunkManager.h"
#include "Chunk.h"

ChunkManager* ChunkManager::instance = nullptr;

ChunkManager::ChunkManager() {
	loadingThread = std::thread(&ChunkManager::loadingThreadFunc, this);
}

ChunkManager::~ChunkManager() {
	shouldLoadChunks = false;
	loadingThread.join();
}

void ChunkManager::initChunks(uint8_t renderDistance) {
	if (renderDistance == 0) {
		printf("Render distance 0 not allowed!\n");
		return;
	}

	auto topEdge = [&](int dist, int count, bool flip) {
		for (int i = 0; i < count; i++) {
			int multi = (flip ? -1 : 1);
			addChunk({ (-dist + i) * multi, dist * multi });
		}
	};

	auto rightEdge = [&](int dist, int count, bool flip) {
		for (int i = 0; i < count; i++) {
			int multi = (flip ? -1 : 1);
			addChunk({ dist * multi, (dist - 1 - i) * multi });
		}
	};
	
	// always add the center chunk
	topEdge(0, 1, false);

	// then add 'rings' of chunks around the center
	for (int i = 1; i < renderDistance; i++) {
		int topCount = (2 * i) + 1;
		int rightCount = (2 * (i - 1)) + 1;

		topEdge(i, topCount, false);
		rightEdge(i, rightCount, false);
		topEdge(i, topCount, true);
		rightEdge(i, rightCount, true);
	}
}

void ChunkManager::renderChunks() {
	std::lock_guard<std::mutex> lock(chunkMutex);

	std::vector<glm::vec2> nullIndexes = {};

	for (auto& c : worldChunks) {
		if (c.second == nullptr) {
			nullIndexes.emplace_back(c.first);
			continue;
		}

		c.second->render();
	}

	for (auto& index : nullIndexes) {
		worldChunks.erase(index);
	}

	nullIndexes.clear();
}

size_t ChunkManager::chunkCount() {
	return worldChunks.size();
}

const size_t ChunkManager::getFaceCount() const {
	size_t count = 0;
	for (auto& c : worldChunks) {
		count += c.second->getFaceCount();
	}
	return count;
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
	std::lock_guard<std::mutex> lock(chunkMutex);

	delete worldChunks[chunkIndex];
	worldChunks.erase(chunkIndex);
}

void ChunkManager::addChunk(const glm::vec2& chunkIndex) {
	std::lock_guard<std::mutex> lock(chunkMutex);
	
	indexToLoad.push(chunkIndex);
}

void ChunkManager::checkForLoadedChunks() {
	std::lock_guard<std::mutex> lock(chunkMutex);

	while (!loadedChunks.empty()) {
		Chunk* c = loadedChunks.front();

		loadedChunks.pop();

		c->init();
		worldChunks[c->getChunkIndex()] = c;
	}
}

void ChunkManager::loadingThreadFunc() {
	while (shouldLoadChunks) {
		glm::vec2 index;
		{
			std::lock_guard<std::mutex> lock(chunkMutex);

			if (indexToLoad.empty()) {
				continue;
			}

			index = indexToLoad.front();
			indexToLoad.pop();
		}

		Chunk* c = new Chunk(index);

		{
			std::lock_guard<std::mutex> lock(chunkMutex);

			loadedChunks.push(c);
		}
	}
}
