#include "Chunk.h"
#include <SOIL2/SOIL2.h>

Chunk::Chunk(glm::vec3 _coords)
{
	coords = _coords;
	extentsMax = _coords + (glm::vec3(CHUNK_X, CHUNK_Y, CHUNK_Z) * 0.5f);

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
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Chunk::generateChunk()
{
	// for now just generate a platform of blocks
	for (GLuint x = 0; x < CHUNK_X; x++) {
		for (GLuint y = 0; y < CHUNK_Y; y++) {
			for (GLuint z = 0; z < CHUNK_Z; z++) {
				Block b = { coords + glm::vec3(x, y, z), GRASS };
				blocks[x][y][z] = b;
			}
		}
	}

	for (GLuint x = 0; x < CHUNK_X; x++) {
		for (GLuint y = 0; y < CHUNK_Y; y++) {
			for (GLuint z = 0; z < CHUNK_Z; z++) {
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

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);

	int width, height;
	unsigned char* image = SOIL_load_image("./assets/texture-atlas.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
}

void Chunk::insertVertsAndInds(Block& b)
{
	auto insertIndices = [&]() {
		GLuint offset = (GLuint)(vertices.size() - 4);
		indices.emplace_back(offset + 0);
		indices.emplace_back(offset + 1);
		indices.emplace_back(offset + 2);
		indices.emplace_back(offset + 2);
		indices.emplace_back(offset + 3);
		indices.emplace_back(offset + 0);
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

	if (queryPos == glm::vec3(0, 0, 1)) {
		int a = 4;
	}

	if (queryPos.x < coords.x || queryPos.x > extentsMax.x ||
		queryPos.y < coords.y || queryPos.y > extentsMax.y ||
		queryPos.z < coords.z || queryPos.z > extentsMax.z) {
		return true; // default to true if querying outside chunk extents
	}

	if (blocks[(int)queryPos.x][(int)queryPos.y][(int)queryPos.z].type == AIR) {
		return true;
	}

	return false;
}
