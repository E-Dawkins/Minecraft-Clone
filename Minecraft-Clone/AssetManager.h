#pragma once
#include <SOIL2/SOIL2.h>
#include <map>
#include <string>
#include <glad/glad.h>

class AssetManager {
private:
	static std::string getFileName(std::string path) {
		if (path.back() == '/') {
			path.pop_back();
		}
		path.erase(0, path.find_last_of('/'));
		path.erase(path.find_last_of('.'));

		return path;
	}

public:
	static void loadTexture(std::string path) {
		std::string fileName = getFileName(path);

		// check if we have already loaded this texture
		if (textures.find(fileName) != textures.end()) {
			return;
		}

		GLuint texId;
		glGenTextures(1, &texId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glGenerateMipmap(GL_TEXTURE_2D);

		int width, height;
		unsigned char* image = SOIL_load_image(path.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

		textures.insert(std::make_pair(fileName, texId));
	}

	static GLuint getTextureId(std::string fileName) {
		return textures[fileName];
	}

private:
	static std::map<std::string, GLuint> textures;
};