#include "Chunk.h"
#include <SOIL2/SOIL2.h>
#include "AssetManager.h"
#include <list>
#include "WorldGenerator.h"
#include "ChunkManager.h"
#include "DebugClock.h"
#include <set>

void checkGLError(const char* stmt, const char* fname, int line) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
		exit(1);
	}
}

#define GL_CHECK(stmt) do { \
    stmt; \
    checkGLError(#stmt, __FILE__, __LINE__); \
} while (0)

Chunk::Chunk(glm::vec2 _chunkIndex) :
	blocks((size_t)chunkSize.x, std::vector<std::vector<BlockType>>((size_t)chunkSize.y, std::vector<BlockType>((size_t)chunkSize.z, BlockType::AIR)))
{
	startPos = glm::vec3(_chunkIndex, 0) * chunkSize;
	chunkIndex = _chunkIndex;

	DebugClock::recordTime("Start gen chunk");
	generateChunk();
	DebugClock::recordTime("Start gen faces");
	generateFaces();
}

Chunk::~Chunk()
{
	GLuint buffers[] = {vbo, ebo, faceDataBuffer};
	glDeleteBuffers(3, buffers);

	glDeleteVertexArrays(1, &vao);

	faceData.clear();
	faceData.shrink_to_fit();

	blocks.clear();
	blocks.shrink_to_fit();
}

void Chunk::init()
{
	DebugClock::recordTime("Start init shader vars");
	initShaderVars();
	DebugClock::recordTime("Finish gen chunk");
}

void Chunk::render()
{
	// bind the correct texture before rendering
	glBindTexture(GL_TEXTURE_2D, AssetManager::getAssetHandle("texture-atlas"));

	// bind the correct chunk index
	GLint chunkIndexLoc = glGetUniformLocation(AssetManager::getAssetHandle("generic"), "chunkIndex");
	glUniform2f(chunkIndexLoc, chunkIndex.x, chunkIndex.y);

	// bind vertex array -> draw vertices -> un-bind vertex array
	glBindVertexArray(vao);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, (GLsizei)faceData.size());
	glBindVertexArray(0);

	// un-bind texture
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Chunk::update() {
	if (!indexesToChange.empty()) {
		std::set<Chunk*> otherChunksToUpdate = {};

		for (auto& i : indexesToChange) {
			if (i.blockType == AIR) {
				removeBlock(i);
			}
			else {
				addBlock(i);
			}
		}

		indexesToChange.clear();
		indexesToChange.shrink_to_fit();

		reBindFaceBuffer();
	}
}

void Chunk::changeBlockAtIndex(const IndexChangeData& changeData) {
	indexesToChange.emplace_back(changeData);
}

void Chunk::generateChunk()
{
	// for now just generate a platform of blocks
	for (GLuint x = 0; x < chunkSize.x; x++) {
		for (GLuint y = 0; y < chunkSize.y; y++) {
			for (GLuint z = 0; z < chunkSize.z; z++) {
				glm::vec3 position = startPos + glm::vec3(x, y, z);

				blocks[x][y][z] = WorldGenerator::getBlockTypeAtPos(position);
			}
		}
	}
}

void Chunk::generateFaces()
{
	for (GLuint x = 0; x < chunkSize.x; x++) {
		for (GLuint y = 0; y < chunkSize.y; y++) {
			for (GLuint z = 0; z < chunkSize.z; z++) {
				if (blocks[x][y][z] == AIR) {
					continue;
				}

				glm::vec3 position = {x, y, z};
				insertFaceData(position);
			}
		}
	}
}

void Chunk::initShaderVars()
{
	const GLuint vertexSize = 5 * sizeof(float);
	float quadVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 1.0f,
		 0.5f, -0.5f, 0.0f,		1.0f, 1.0f,
		 0.5f,  0.5f, 0.0f,		1.0f, 0.0f,
		-0.5f,  0.5f, 0.0f,		0.0f, 0.0f
	};

	GLuint quadIndices[] = {
		0, 1, 2,
		2, 3, 0
	};

	// Create vertex array object
	GL_CHECK(glGenVertexArrays(1, &vao));
	GL_CHECK(glBindVertexArray(vao));

	// Create and fill vertex buffer object
	GL_CHECK(glGenBuffers(1, &vbo));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW));

	// Create and fill element buffer object
	GL_CHECK(glGenBuffers(1, &ebo));
	GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
	GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW));

	// Position (main vbo)
	GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, 0));
	GL_CHECK(glEnableVertexAttribArray(0));

	// Texcoord (main vbo)
	GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertexSize, (void*)(3 * sizeof(float))));
	GL_CHECK(glEnableVertexAttribArray(1));
	
	GL_CHECK(glGenBuffers(1, &faceDataBuffer));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, faceDataBuffer));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(FaceData) * faceData.size(), faceData.data(), GL_STATIC_DRAW));

	// Position (per-instance data)
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_SHORT, sizeof(FaceData), (void*)0);
	glVertexAttribDivisor(2, 1);

	// Direction-Id (per-instance data)
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, sizeof(FaceData), (void*)offsetof(FaceData, direction_id));
	glVertexAttribDivisor(3, 1);
}

void Chunk::insertFaceData(glm::vec3& blockIndex)
{
	auto insertData = [&](BlockFace faceDirection) {
		if (isFaceVisible(blockIndex, faceDirection)) {
			FaceData f;
			f.setPosition(blockIndex);
			f.setBlockTexId(getBlockAtIndex(blockIndex), faceDirection);
			f.setDirection(faceDirection);

			faceData.emplace_back(f);
		}
	};

	insertData(FRONT);
	insertData(BACK);
	insertData(LEFT);
	insertData(RIGHT);
	insertData(TOP);
	insertData(BOTTOM);
}

bool Chunk::isFaceVisible(const glm::vec3& pos, BlockFace face)
{
	glm::vec3 offset = faceNormals[face];
	glm::vec3 queryPos = startPos + pos + offset;

	return (WorldGenerator::getBlockTypeAtPos(queryPos) == AIR);
}

bool Chunk::isValidBlockIndex(const glm::ivec3 index) const {
	bool withinMin = glm::all(glm::greaterThanEqual(index, glm::ivec3(0)));
	bool withinMax = glm::all(glm::lessThan(index, glm::ivec3(chunkSize)));

	return withinMin && withinMax;
}

void Chunk::reBindFaceBuffer() {
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, faceDataBuffer));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(FaceData) * faceData.size(), faceData.data(), GL_STATIC_DRAW));
}

void Chunk::removeBlock(const IndexChangeData& data) {
	// can't remove air, so early return
	if (getBlockAtIndex(data.blockIndex) == AIR) {
		return;
	}

	// for each face
	// -> if visible, delete from faceData
	// -> if not visible
	//    -> if facing inside chunk, add to this chunks' faceData
	//    -> if facing outside chunk, add to other chunks' faceData

	for (uint8_t i = 0; i < BlockFace::FACE_COUNT; i++) {
		FaceData ref;
		ref.setPosition(data.blockIndex);
		ref.setDirection((BlockFace)i);
		ref.setBlockTexId(getBlockAtIndex(data.blockIndex), (BlockFace)i);

		// if ref FaceData is found, then the face is 'visible' and should be deleted
		auto itr = std::find(faceData.begin(), faceData.end(), ref);
		if (itr != faceData.end()) {
			faceData.erase(itr);
		}
		// otherwise it is not 'visible' and should be added
		else {
			glm::ivec3 queryIndex = data.blockIndex + faceNormals[i];
			if (queryIndex.z < 0 || queryIndex.z >= chunkSize.z) { // outside z-axis so skip adding this face
				continue;
			}
			else if (isValidBlockIndex(queryIndex)) {
				ref.setPosition(queryIndex);
				ref.setDirection(inverseFace[i]);
				ref.setBlockTexId(getBlockAtIndex(queryIndex), inverseFace[i]);

				faceData.emplace_back(ref);
			}
			else {
				glm::vec2 index = Chunk::posToChunkIndex(queryIndex + glm::ivec3(startPos));
				if (Chunk* c = ChunkManager::getInstance()->getChunkAtIndex(index)) {
					glm::vec3 wrappedIndex = glm::mod(glm::vec3(queryIndex), chunkSize);
					ref.setPosition(wrappedIndex);
					ref.setDirection(inverseFace[i]);
					ref.setBlockTexId(c->getBlockAtIndex(wrappedIndex), inverseFace[i]);

					c->faceData.emplace_back(ref);
					c->reBindFaceBuffer();
				}
			}
		}
	}

	blocks[data.blockIndex.x][data.blockIndex.y][data.blockIndex.z] = BlockType::AIR;
	reBindFaceBuffer();
}

void Chunk::addBlock(const IndexChangeData& data) {
	// can't place blocks inside eachother
	if (getBlockAtIndex(data.blockIndex) != AIR) {
		return;
	}
	
	// loop over all faces of new block
	// -> if a face exists already, delete it
	// -> otherwise, add new face

	for (uint8_t i = 0; i < BlockFace::FACE_COUNT; i++) {
		glm::ivec3 offsetIndex = data.blockIndex + faceNormals[i];

		if (isValidBlockIndex(offsetIndex)) {
			FaceData ref;
			ref.setPosition(offsetIndex);
			ref.setDirection(inverseFace[i]);
			ref.setBlockTexId(getBlockAtIndex(offsetIndex), inverseFace[i]);

			// face exists, delete it
			auto itr = std::find(faceData.begin(), faceData.end(), ref);
			if (itr != faceData.end()) {
				faceData.erase(itr);
			}
			else { // add new face
				ref.setPosition(data.blockIndex);
				ref.setDirection((BlockFace)i);
				ref.setBlockTexId(data.blockType, (BlockFace)i);
				faceData.emplace_back(ref);
			}
		}
		else {
			glm::vec2 index = posToChunkIndex((glm::vec3)offsetIndex + startPos);
			if (Chunk* c = ChunkManager::getInstance()->getChunkAtIndex(index)) {
				glm::ivec3 wrappedOffsetIndex = glm::mod((glm::vec3)offsetIndex, chunkSize);

				FaceData ref;
				ref.setPosition(wrappedOffsetIndex);
				ref.setDirection(inverseFace[i]);
				ref.setBlockTexId(c->getBlockAtIndex(wrappedOffsetIndex), inverseFace[i]);

				// we delete the face in the adjacent chunk if it exists
				auto itr = std::find(c->faceData.begin(), c->faceData.end(), ref);
				if (itr != c->faceData.end()) {
					c->faceData.erase(itr);
					c->reBindFaceBuffer();
				}
				else { // but we only ever add faces to this chunk
					ref.setPosition(data.blockIndex);
					ref.setDirection((BlockFace)i);
					ref.setBlockTexId(data.blockType, (BlockFace)i);
					faceData.emplace_back(ref);
				}
			}
		}
	}

	blocks[data.blockIndex.x][data.blockIndex.y][data.blockIndex.z] = data.blockType;
	reBindFaceBuffer();
}
