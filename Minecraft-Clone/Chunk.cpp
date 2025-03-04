#include "Chunk.h"
#include <SOIL2/SOIL2.h>
#include "AssetManager.h"
#include <list>
#include "WorldGenerator.h"
#include "ChunkManager.h"
#include "DebugClock.h"

Chunk::Chunk(glm::vec2 _chunkIndex) :
	blocks((size_t)chunkSize.x, std::vector<std::vector<Block>>((size_t)chunkSize.y, std::vector<Block>((size_t)chunkSize.z)))
{
	startPos = glm::vec3(_chunkIndex, 0) * chunkSize;
	chunkIndex = _chunkIndex;

	generateChunk();
}

Chunk::~Chunk()
{
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);

	glDeleteVertexArrays(1, &vao);

	glDeleteBuffers(1, &faceDataBuffer);

	faceData.clear();
	faceData.shrink_to_fit();

	blocks.clear();
	blocks.shrink_to_fit();
}

void Chunk::init()
{
	DebugClock::recordTime("Start gen faces");
	generateFaces();
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

void Chunk::generateChunk()
{
	// for now just generate a platform of blocks
	for (GLuint x = 0; x < chunkSize.x; x++) {
		for (GLuint y = 0; y < chunkSize.y; y++) {
			for (GLuint z = 0; z < chunkSize.z; z++) {
				glm::vec3 position = startPos + glm::vec3(x, y, z);

				blocks[x][y][z] = {
					position,
					WorldGenerator::getBlockTypeAtPos(position)
				};
			}
		}
	}
}

void Chunk::generateFaces()
{
	for (GLuint x = 0; x < chunkSize.x; x++) {
		for (GLuint y = 0; y < chunkSize.y; y++) {
			for (GLuint z = 0; z < chunkSize.z; z++) {
				Block& b = blocks[x][y][z];
				if (b.type == AIR) {
					continue;
				}

				insertFaceData(b);
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
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and fill vertex buffer object
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	// Create and fill element buffer object
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

	// Position (main vbo)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, 0);
	glEnableVertexAttribArray(0);

	// Texcoord (main vbo)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertexSize, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	
	glGenBuffers(1, &faceDataBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, faceDataBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(FaceData) * faceData.size(), faceData.data(), GL_STATIC_DRAW);

	// Position (per-instance data)
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_SHORT, sizeof(FaceData), (void*)0);
	glVertexAttribDivisor(2, 1);

	// Direction-Id (per-instance data)
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, sizeof(FaceData), (void*)offsetof(FaceData, direction_id));
	glVertexAttribDivisor(3, 1);
}

void Chunk::insertFaceData(Block& b)
{
	auto insertData = [&](BlockFace faceDirection) {
		b.visibleFaces[faceDirection] = isFaceVisible(b.position, faceDirection);
		if (b.visibleFaces[faceDirection]) {
			FaceData f;
			f.setPosition(b.position);
			f.setBlockId(blockTextureIds[b.type][faceDirection]);
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

bool Chunk::isFaceVisible(glm::vec3& pos, BlockFace face)
{
	glm::vec3 offset = faceNormals[face];
	glm::vec3 queryPos = pos + offset;

	if (!isPosInChunk(queryPos)) {
		glm::vec2 index = posToChunkIndex(queryPos);
		Chunk* queryChunk = ChunkManager::getInstance()->getChunkAtIndex(index);

		if (!queryChunk || queryPos.z < 0 || queryPos.z >= chunkSize.z) {
			return true; // default to true if no chunk exists at queryPos
						 // or we are querying outside the valid height range
		}
		else {
			return (WorldGenerator::getBlockTypeAtPos(queryPos) == AIR);
		}
	}

	// wrap queryPos before indexing into blocks array
	queryPos = glm::mod(queryPos, chunkSize);
	if (blocks[(int)queryPos.x][(int)queryPos.y][(int)queryPos.z].type == AIR) {
		return true;
	}

	return false;
}

glm::vec3 Chunk::getBlockPosFromFace(glm::vec3& facePos, BlockFace face) {
	// shift facePos to [0-chunkSize], then offset by the face normal
	return facePos - startPos - (faceNormals[face] * 0.5f);
}

bool Chunk::isPosInChunk(glm::vec3 pos) {
	bool inX = pos.x >= startPos.x && pos.x < startPos.x + chunkSize.x;
	bool inY = pos.y >= startPos.y && pos.y < startPos.y + chunkSize.y;
	bool inZ = pos.z >= startPos.z && pos.z < startPos.z + chunkSize.z;

	return (inX && inY && inZ);
}

bool Chunk::isPosInChunkSize(glm::vec3 pos) {
	return pos.x >= 0 && pos.x < chunkSize.x &&
		pos.y >= 0 && pos.y < chunkSize.y &&
		pos.z >= 0 && pos.z < chunkSize.z;
}
