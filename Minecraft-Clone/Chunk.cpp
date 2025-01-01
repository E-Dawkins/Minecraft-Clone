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
	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
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
				insertVertsAndInds(blocks[x][y][z]);
			}
		}
	}
}

void Chunk::initShaderVars()
{
	// Create vertex array object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and fill vertex buffer object
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	// Create and fill element buffer object
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);

	// Specify the layout of the vertex data
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, uv)));
}

void Chunk::insertVertsAndInds(Block& b)
{
	auto insertIndices = [&]() {
		GLuint offset = (GLuint)(vertices.size() - 4);
		indices.emplace_back(offset + 2);
		indices.emplace_back(offset + 1);
		indices.emplace_back(offset + 0);
		indices.emplace_back(offset + 0);
		indices.emplace_back(offset + 3);
		indices.emplace_back(offset + 2);
	};

	// front face
	if (isFaceVisible(b.position, FRONT)) {
		vertices.push_back({ b.position + glm::vec3(-0.5f, -0.5f,  0.5f), b.getTextureCoords(FRONT, 0) });
		vertices.push_back({ b.position + glm::vec3( 0.5f, -0.5f,  0.5f), b.getTextureCoords(FRONT, 1) });
		vertices.push_back({ b.position + glm::vec3( 0.5f, -0.5f, -0.5f), b.getTextureCoords(FRONT, 2) });
		vertices.push_back({ b.position + glm::vec3(-0.5f, -0.5f, -0.5f), b.getTextureCoords(FRONT, 3) });
		insertIndices();
	}

	// back face
	if (isFaceVisible(b.position, BACK)) {
		vertices.push_back({ b.position + glm::vec3( 0.5f,  0.5f,  0.5f), b.getTextureCoords(BACK, 0) });
		vertices.push_back({ b.position + glm::vec3(-0.5f,  0.5f,  0.5f), b.getTextureCoords(BACK, 1) });
		vertices.push_back({ b.position + glm::vec3(-0.5f,  0.5f, -0.5f), b.getTextureCoords(BACK, 2) });
		vertices.push_back({ b.position + glm::vec3( 0.5f,  0.5f, -0.5f), b.getTextureCoords(BACK, 3) });
		insertIndices();
	}

	// left face
	if (isFaceVisible(b.position, LEFT)) {
		vertices.push_back({ b.position + glm::vec3(-0.5f,  0.5f,  0.5f), b.getTextureCoords(LEFT, 0) });
		vertices.push_back({ b.position + glm::vec3(-0.5f, -0.5f,  0.5f), b.getTextureCoords(LEFT, 1) });
		vertices.push_back({ b.position + glm::vec3(-0.5f, -0.5f, -0.5f), b.getTextureCoords(LEFT, 2) });
		vertices.push_back({ b.position + glm::vec3(-0.5f,  0.5f, -0.5f), b.getTextureCoords(LEFT, 3) });
		insertIndices();
	}

	// right face
	if (isFaceVisible(b.position, RIGHT)) {
		vertices.push_back({ b.position + glm::vec3( 0.5f, -0.5f,  0.5f), b.getTextureCoords(RIGHT, 0) });
		vertices.push_back({ b.position + glm::vec3( 0.5f,  0.5f,  0.5f), b.getTextureCoords(RIGHT, 1) });
		vertices.push_back({ b.position + glm::vec3( 0.5f,  0.5f, -0.5f), b.getTextureCoords(RIGHT, 2) });
		vertices.push_back({ b.position + glm::vec3( 0.5f, -0.5f, -0.5f), b.getTextureCoords(RIGHT, 3) });
		insertIndices();
	}

	// top face
	if (isFaceVisible(b.position, TOP)) {
		vertices.push_back({ b.position + glm::vec3(-0.5f,  0.5f,  0.5f), b.getTextureCoords(TOP, 0) });
		vertices.push_back({ b.position + glm::vec3( 0.5f,  0.5f,  0.5f), b.getTextureCoords(TOP, 1) });
		vertices.push_back({ b.position + glm::vec3( 0.5f, -0.5f,  0.5f), b.getTextureCoords(TOP, 2) });
		vertices.push_back({ b.position + glm::vec3(-0.5f, -0.5f,  0.5f), b.getTextureCoords(TOP, 3) });
		insertIndices();
	}

	// bottom face
	if (isFaceVisible(b.position, BOTTOM)) {
		vertices.push_back({ b.position + glm::vec3(-0.5f, -0.5f, -0.5f), b.getTextureCoords(BOTTOM, 0) });
		vertices.push_back({ b.position + glm::vec3( 0.5f, -0.5f, -0.5f), b.getTextureCoords(BOTTOM, 1) });
		vertices.push_back({ b.position + glm::vec3( 0.5f,  0.5f, -0.5f), b.getTextureCoords(BOTTOM, 2) });
		vertices.push_back({ b.position + glm::vec3(-0.5f,  0.5f, -0.5f), b.getTextureCoords(BOTTOM, 3) });
		insertIndices();
	}
}

bool Chunk::isFaceVisible(glm::vec3& pos, BlockFace face)
{
	glm::vec3 offset = faceNormals[face];
	glm::vec3 queryPos = pos + offset;

	if (queryPos.x < 0 || queryPos.x >= chunkSize.x ||
		queryPos.y < 0 || queryPos.y >= chunkSize.y ||
		queryPos.z < 0 || queryPos.z >= chunkSize.z) {
		return true; // default to true if querying outside chunk extents
	}

	// wrap queryPos before indexing into blocks array
	queryPos = glm::mod(queryPos, chunkSize);
	if (blocks[(int)queryPos.x][(int)queryPos.y][(int)queryPos.z].type == AIR) {
		return true;
	}

	return false;
}
