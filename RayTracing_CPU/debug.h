#pragma once

#include <stdexcept>
#include <string>
#include <array>
#include <vector>
#include <typeinfo>
#include "Ray.h"

#define LOG_VAR(var) addVar(__FUNCTION__, #var, typeid(var).name(), std::to_string(var))
#define LOG(varName, content) addVar(__FUNCTION__, varName, typeid(content).name(), std::to_string(content))
#define LOG_STR(varName, content) addVar(__FUNCTION__, varName, typeid(content).name(), content)

typedef std::vector<std::array<std::string, 3>> varInfo;

class StackTrace {
public:
	virtual void addVar(std::string function, std::string varName, std::string varType, std::string value) = 0;
	virtual void addRay(Ray r) = 0;
	virtual std::string toString() = 0;
	virtual StackTrace* throwException(std::string why) = 0;
protected:
	std::string reason;
	std::vector<std::pair<std::string, varInfo>> stackTrace;
};

class DummyStackTrace : StackTrace {
public:
	void addVar(std::string function, std::string varName, std::string varType, std::string value) {}
	void addRay(Ray r) {}
	std::string toString() { return ""; }
	StackTrace* throwException(std::string why) { return this; }
};

class RayStackTrace : StackTrace {
public:
	void addVar(std::string function, std::string varName, std::string varType, std::string value);
	void addRay(Ray r);
	std::string toString();
	StackTrace* throwException(std::string why);

	std::vector<Ray> rayPath;
};