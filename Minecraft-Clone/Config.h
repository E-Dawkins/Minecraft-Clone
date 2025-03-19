#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

class Config
{
public:
	static void loadConfigFile(const std::string& filePath) {
        if (isLoaded) {
            return;
        }

        isLoaded = true;

        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cout << "Failed to load shader at: " << filePath << std::endl;
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) {
                continue;
            }

            size_t delimPos = line.find("=");
            std::string varName = line.substr(0, delimPos);
            std::string varValue = line.substr(delimPos+1);
            configVars.insert({varName, varValue});
        }
	}

    template <typename T>
    static T getVar(const std::string& varName) {
        std::string name = typeid(T).name();
        if (name == "bool") return getVarAsBool(varName);
        if (name == "int") return getVarAsInt(varName);
        
        return T();
    }

private:
    static bool getVarAsBool(const std::string& varName) {
        std::string valueStr = configVars[varName];

        if (valueStr.empty()) {
            return false;
        }

        return valueStr == "true";
    }

    static int getVarAsInt(const std::string& varName) {
        std::string valueStr = configVars[varName];

        if (valueStr.empty()) {
            return -1;
        }

        return std::stoi(valueStr);
    }

private:
    static std::map<std::string, std::string> configVars;
    static bool isLoaded;
};

std::map<std::string, std::string> Config::configVars = {};
bool Config::isLoaded = false;