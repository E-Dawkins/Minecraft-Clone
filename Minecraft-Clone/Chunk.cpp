#include "Chunk.h"
#include <SOIL2/SOIL2.h>
#include "AssetManager.h"

Chunk::Chunk(glm::vec2 _chunkIndex)
{
	startPos = glm::vec3(_chunkIndex, 0) * chunkSize;

	generateChunk();
	initShaderVars();
}

Chunk::~Chunk()
{
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);

	glDeleteVertexArrays(1, &vao);
}

void Chunk::render()
{
	// bind the correct texture before rendering
	glBindTexture(GL_TEXTURE_2D, AssetManager::getTextureId("texture-atlas"));

	// bind vertex array -> draw vertices -> un-bind vertex array
	glBindVertexArray(vao);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, faceData.size());
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
				Block b = { startPos + glm::vec3(x, y, z), GRASS };
				blocks[x][y][z] = b;
			}
		}
	}

	for (GLuint x = 0; x < chunkSize.x; x++) {
		for (GLuint y = 0; y < chunkSize.y; y++) {
			for (GLuint z = 0; z < chunkSize.z; z++) {
				insertFaceData(blocks[x][y][z]);
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

	// Texcoord Start
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(FaceData), (void*)offsetof(FaceData, texcoordStart));
	glVertexAttribDivisor(4, 1); // one texcoord start per-face
}

void Chunk::insertFaceData(Block& b)
{
	// front face
	if (isFaceVisible(b.position, FRONT)) {
		faceData.push_back({
			b.position + faceNormals[FRONT] * 0.5f,
			FRONT,
			b.getTextureCoords(FRONT, 0)
		});
	}

	// back face
	if (isFaceVisible(b.position, BACK)) {
		faceData.push_back({
			b.position + faceNormals[BACK] * 0.5f,
			BACK,
			b.getTextureCoords(BACK, 0)
		});
	}

	// left face
	if (isFaceVisible(b.position, LEFT)) {
		faceData.push_back({
			b.position + faceNormals[LEFT] * 0.5f,
			LEFT,
			b.getTextureCoords(LEFT, 0)
		});
	}

	// right face
	if (isFaceVisible(b.position, RIGHT)) {
		faceData.push_back({
			b.position + faceNormals[RIGHT] * 0.5f,
			RIGHT,
			b.getTextureCoords(RIGHT, 0)
		});
	}

	// top face
	if (isFaceVisible(b.position, TOP)) {
		faceData.push_back({
			b.position + faceNormals[TOP] * 0.5f,
			TOP,
			b.getTextureCoords(TOP, 0)
		});
	}

	// bottom face
	if (isFaceVisible(b.position, BOTTOM)) {
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

	if (queryPos.x < startPos.x || queryPos.x >= startPos.x + chunkSize.x ||
		queryPos.y < startPos.y || queryPos.y >= startPos.y + chunkSize.y ||
		queryPos.z < startPos.z || queryPos.z >= startPos.z + chunkSize.z) {
		return true; // default to true if querying outside chunk extents
	}

	// wrap queryPos before indexing into blocks array
	queryPos = glm::mod(queryPos, chunkSize);
	if (blocks[(int)queryPos.x][(int)queryPos.y][(int)queryPos.z].type == AIR) {
		return true;
	}

	return false;
}
