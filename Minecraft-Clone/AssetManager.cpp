#include "AssetManager.h"
#include <fstream>
#include <iostream>
#include <sstream>

std::map<std::string, GLuint> AssetManager::assetHandles = {};

GLuint AssetManager::getAssetHandle(std::string fileName) {
	return assetHandles[fileName];
}

void AssetManager::loadTexture(std::string path) {
	std::string fileName = getFileName(path);

	// check if we have already loaded this texture
	if (assetHandles.find(fileName) != assetHandles.end()) {
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

	assetHandles.insert(std::make_pair(fileName, texId));
}

void AssetManager::loadShader(std::string handleName, std::string vertShader, std::string fragShader) {
    std::string vertexShaderStr = loadFile("./assets/generic.vert");
    const char* vertexShaderSrc = vertexShaderStr.c_str();
    std::string fragmentShaderStr = loadFile("./assets/generic.frag");
    const char* fragmentShaderSrc = fragmentShaderStr.c_str();

    // Create and compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSrc, 0); // length 0 means opengl will figure it out for us
    glCompileShader(vertexShader);
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::VERTEX_SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }


    // Create and compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, 0); // length 0 means opengl will figure it out for us
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::FRAGMENT_SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }


    // Link vertex and fragment shaders to a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "fragColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
    
    assetHandles.insert(std::make_pair(handleName, shaderProgram));
}

std::string AssetManager::getFileName(std::string path, bool removeExtension) {
    if (path.back() == '/') {
        path.pop_back();
    }

    path.erase(0, path.find_last_of('/'));

    if (removeExtension) {
        path.erase(path.find_last_of('.'));
    }

    return path;
}

std::string AssetManager::loadFile(std::string path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to load shader at: " << path << std::endl;
        return std::string();
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}
