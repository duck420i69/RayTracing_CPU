#include "debug.h"

template<class T>
int contain(std::vector<std::pair<std::string, T>> arr, std::string key) {
	for (int i = 0; i < arr.size(); i++) {
		if (arr[i].first == key) {
			return i;
		}
	}
	return -1;
}


void RayStackTrace::addVar(std::string function, std::string varName, std::string varType, std::string value)
{
	std::array<std::string, 3> var;
	var[0] = std::move(varName); var[1] = std::move(varType); var[2] = std::move(value);
	int iter = contain(stackTrace, function);
	if (iter != -1) {
		stackTrace[iter].second.push_back(var);
	}
	else {
		stackTrace.push_back(std::make_pair(function, varInfo()));
		stackTrace.back().second.push_back(var);
	}
}


void RayStackTrace::addRay(Ray r)
{
	rayPath.emplace_back(r);
}


std::string RayStackTrace::toString()
{
	std::string debugString = "Exception throw: " + reason + "\n\n";
	for (auto& func : stackTrace)
	{
		debugString += func.first + " {\n";
		for (auto& var : func.second) {
			debugString += "\t" + var[0] + " = " + var[2] + "\n";
		}
		debugString += "}\n";
	}
	return debugString;
}

StackTrace* RayStackTrace::throwException(std::string why)
{
	reason = why;
	return this;
}
