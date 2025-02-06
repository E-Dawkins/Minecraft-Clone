#include "Chunk.h"
#include <SOIL2/SOIL2.h>
#include "AssetManager.h"
#include <list>
#include "WorldGenerator.h"
#include "ChunkManager.h"
#include "DebugClock.h"

Chunk::Chunk(glm::vec2 _chunkIndex)
{
	startPos = glm::vec3(_chunkIndex, 0) * chunkSize;
}

Chunk::~Chunk()
{
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);

	glDeleteVertexArrays(1, &vao);
}

void Chunk::init()
{
	DebugClock::recordTime("Start gen chunk");
	generateChunk();
	DebugClock::recordTime("Start gen faces");
	generateFaces();
	DebugClock::recordTime("Start optimize faces");
	optimizeFaces();
	DebugClock::recordTime("Start init shader vars");
	initShaderVars();
	DebugClock::recordTime("Finish gen chunk");
}

void Chunk::render()
{
	// bind the correct texture before rendering
	glBindTexture(GL_TEXTURE_2D, AssetManager::getTextureId("texture-atlas"));

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

void Chunk::optimizeFaces()
{
	// a brute force implementation of greedy meshing
	// could probably optimize this if it becomes a bottle-neck

	std::vector<FaceData> optimizedFaces = {};

	// returns if a pre-optimized face is a part of an optimized one
	auto hasFaceBeenChecked = [&](glm::vec3 queryPos, int refId) {
		for (FaceData& f : optimizedFaces) {
			// same id, now check if the optimized face contains the query face
			if (f.id == refId) {
				glm::vec3 blockPos = getBlockPosFromFace(f.offset, f.id);
				glm::vec3 faceMin = blockPos - f.size * 0.5f;
				glm::vec3 faceMax = blockPos + f.size * 0.5f;
				if (queryPos.x >= faceMin.x && queryPos.x <= faceMax.x &&
					queryPos.y >= faceMin.y && queryPos.y <= faceMax.y &&
					queryPos.z >= faceMin.z && queryPos.z <= faceMax.z) {
					return true;
				}
			}
		}

		return false;
	};

	// returns if a face is valid to be added to an optimized one
	// i.e. within chunk bounds and hasn't already been optimized
	auto isValidFace = [&](glm::vec3 queryPos, BlockType refType, int refId) {
		Block& queryBlock = blocks[(int)queryPos.x][(int)queryPos.y][(int)queryPos.z];
		if (!isPosInChunkSize(queryPos)) {
			return false;
		}

		bool sameType = queryBlock.type == refType;
		bool faceVisible = queryBlock.visibleFaces[refId];
		bool hasBeenChecked = hasFaceBeenChecked(queryPos, refId);
		return (sameType && faceVisible && !hasBeenChecked);
	};

	const glm::vec3 axesToCheck[FACE_COUNT / 2][2]{
		{ { 1, 0, 0 }, { 0, 0, 1 } }, // front & back
		{ { 0, 1, 0 }, { 0, 0, 1 } }, // right & left
		{ { 1, 0, 0 }, { 0, 1, 0 } }, // top & bottom
	};

	// while we have faces to optimize run this logic
	std::list<FaceData> facesToCheck(faceData.begin(), faceData.end());
	while (!facesToCheck.empty()) {
		FaceData currentFace = facesToCheck.front();
		facesToCheck.pop_front();


		glm::vec3 currentPos = getBlockPosFromFace(currentFace.offset, currentFace.id);
		Block& currentBlock = blocks[(int)currentPos.x][(int)currentPos.y][(int)currentPos.z];

		glm::vec3 min = currentPos;
		glm::vec3 max = currentPos;

		// first axis
		glm::vec3 firstAxis = axesToCheck[currentFace.id / 2][0];
		// should we flip the direction?
		firstAxis = (isValidFace(currentPos + firstAxis, currentBlock.type, currentFace.id) ? firstAxis : -firstAxis);
		for (float i = 1; ; i++) {
			glm::vec3 queryPos = currentPos + firstAxis * i;
			
			// if face is valid, update optimized face min/max
			if (isValidFace(queryPos, currentBlock.type, currentFace.id)) {
				min = glm::min(min, queryPos);
				max = glm::max(max, queryPos);
			}
			else {
				currentPos += firstAxis * (i - 1);
				break;
			}
		}

		float firstAxisCount = glm::distance(min, max);

		// second axis
		glm::vec3 secondAxis = axesToCheck[currentFace.id / 2][1];
		// should we flip the direction?
		secondAxis = (isValidFace(currentPos + secondAxis, currentBlock.type, currentFace.id) ? secondAxis : -secondAxis);
		for (float i = 1; ; i++) {
			glm::vec3 queryPos = currentPos + secondAxis * i;

			// check all faces along the -firstAxis
			bool allFacesValid = (firstAxisCount != 0);
			for (float j = 1; j <= firstAxisCount; j++) {
				glm::vec3 checkPos = queryPos - firstAxis * j;

				if (!isValidFace(checkPos, currentBlock.type, currentFace.id)) {
					allFacesValid = false;
					break;
				}
			}

			// if face is valid, update optimized face min/max
			if (allFacesValid && isValidFace(queryPos, currentBlock.type, currentFace.id)) {
				min = glm::min(min, queryPos);
				max = glm::max(max, queryPos);
			}
			else {
				break;
			}
		}

		// we have the min and max pos of the 'region'
		// replace all faces in region with one big face

		// remove all faces in region from list
		glm::vec3 dir = max - min;
		float distX = glm::abs(dir.x);
		float distY = glm::abs(dir.y);
		float distZ = glm::abs(dir.z);
		for (float x = 0; x <= distX; x++) {
			for (float y = 0; y <= distY; y++) {
				for (float z = 0; z <= distZ; z++) {
					glm::vec3 faceOffset = currentFace.offset + glm::vec3(x, y, z);
					auto it = std::find_if(facesToCheck.begin(), facesToCheck.end(),
						[&](const FaceData& face) {
							return face.offset == faceOffset;
						}
					);

					if (it != facesToCheck.end()) {
						facesToCheck.erase(it);
					}
				}
			}
		}

		// replace missing faces with one big face
		glm::vec3 normal = faceNormals[currentFace.id];
		glm::vec3 flipped = glm::vec3(normal.x == 0 ? 1 : 0,
									  normal.y == 0 ? 1 : 0,
									  normal.z == 0 ? 1 : 0);
		currentFace.offset = startPos + min + (dir * 0.5f) + (normal * 0.5f);
		currentFace.size = (glm::vec3(distX, distY, distZ) + glm::vec3(1.0f)) * flipped;
		optimizedFaces.emplace_back(currentFace);
	}

	// and finally replace old face data with new optimized face data
	faceData.clear();
	faceData.shrink_to_fit();
	faceData.swap(optimizedFaces);
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
	
	GLuint faceDataBuffer;
	glGenBuffers(1, &faceDataBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, faceDataBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(FaceData) * faceData.size(), faceData.data(), GL_STATIC_DRAW);

	// Offset (per-instance data)
	glBindBuffer(GL_ARRAY_BUFFER, faceDataBuffer);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(FaceData), (void*)0);
	glVertexAttribDivisor(2, 1); // one offset per-face

	// Id (per-instance data)
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_INT, sizeof(FaceData), (void*)offsetof(FaceData, id));
	glVertexAttribDivisor(3, 1); // one id per-face

	// Texcoord Start (per-instance data)
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(FaceData), (void*)offsetof(FaceData, texcoordStart));
	glVertexAttribDivisor(4, 1); // one texcoord start per-face

	// Size (per-instance data)
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(FaceData), (void*)offsetof(FaceData, size));
	glVertexAttribDivisor(5, 1); // one size per-face
}

void Chunk::insertFaceData(Block& b)
{
	// front face
	b.visibleFaces[FRONT] = isFaceVisible(b.position, FRONT);
	if (b.visibleFaces[FRONT]) {
		faceData.push_back({
			b.position + faceNormals[FRONT] * 0.5f,
			FRONT,
			b.getTextureCoords(FRONT, 0)
		});
	}

	// back face
	b.visibleFaces[BACK] = isFaceVisible(b.position, BACK);
	if (b.visibleFaces[BACK]) {
		faceData.push_back({
			b.position + faceNormals[BACK] * 0.5f,
			BACK,
			b.getTextureCoords(BACK, 0)
		});
	}

	// left face
	b.visibleFaces[LEFT] = isFaceVisible(b.position, LEFT);
	if (b.visibleFaces[LEFT]) {
		faceData.push_back({
			b.position + faceNormals[LEFT] * 0.5f,
			LEFT,
			b.getTextureCoords(LEFT, 0)
		});
	}

	// right face
	b.visibleFaces[RIGHT] = isFaceVisible(b.position, RIGHT);
	if (b.visibleFaces[RIGHT]) {
		faceData.push_back({
			b.position + faceNormals[RIGHT] * 0.5f,
			RIGHT,
			b.getTextureCoords(RIGHT, 0)
		});
	}

	// top face
	b.visibleFaces[TOP] = isFaceVisible(b.position, TOP);
	if (b.visibleFaces[TOP]) {
		faceData.push_back({
			b.position + faceNormals[TOP] * 0.5f,
			TOP,
			b.getTextureCoords(TOP, 0)
		});
	}

	// bottom face
	b.visibleFaces[BOTTOM] = isFaceVisible(b.position, BOTTOM);
	if (b.visibleFaces[BOTTOM]) {
		faceData.push_back({
			b.position + faceNormals[BOTTOM] * 0.5f,
			BOTTOM,
			b.getTextureCoords(BOTTOM, 0)
		});
	}
}

bool Chunk::isFaceVisible(glm::vec3& pos, BlockFace face)
{
	glm::vec3 offset = faceNormals[face];
	glm::vec3 queryPos = pos + offset;

	if (!isPosInChunk(queryPos)) {
		glm::vec2 chunkIndex = glm::floor(queryPos / chunkSize);
		Chunk* queryChunk = ChunkManager::getInstance()->getChunkAtIndex(chunkIndex);

		if (!queryChunk) {
			return true; // default to true if no chunk exists at queryPos
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
