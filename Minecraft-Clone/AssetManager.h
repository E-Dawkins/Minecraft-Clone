#pragma once
#include <SOIL2/SOIL2.h>
#include <map>
#include <string>
#include <glad/glad.h>

class AssetManager {
public:
	static GLuint getAssetHandle(std::string fileName);

	static void loadTexture(std::string path);
	static void loadShader(std::string handleName, std::string vertShader, std::string fragShader);

private:
	static std::string getFileName(std::string path, bool removeExtension = true);
	static std::string loadFile(std::string path);

private:
	static std::map<std::string, GLuint> assetHandles;
};