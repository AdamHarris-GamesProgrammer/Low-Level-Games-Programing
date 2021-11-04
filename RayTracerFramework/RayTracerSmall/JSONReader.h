#pragma once
#include "nlohmann/json.hpp"
#include <iostream>
#include <vector>
#include <string>
#include "Vec3.h"
#include "Sphere.h"
#include <fstream>

using json = nlohmann::json;

struct JSONSphereInfo {
	Sphere* sphereArr;
	Vec3f* sphereEndPositions;
	Vec3f* sphereMovementsPerFrame;
	int frameCount;
	int sphereCount;

	JSONSphereInfo(int spherecount, int framecount) {
		sphereArr = new Sphere[spherecount];
		sphereEndPositions = new Vec3f[spherecount];
		sphereMovementsPerFrame = new Vec3f[spherecount];
		frameCount = framecount;
		sphereCount = spherecount;
	}

	void CalculateSphereMovements() {
		for (int i = 0; i < sphereCount; i++) {
			Vec3f diff = sphereEndPositions[i] - sphereArr[i]._center;
			diff.x /= frameCount;
			diff.y /= frameCount;
			diff.z /= frameCount;
			sphereMovementsPerFrame[i] = diff;
		}
	}
};

class JSONReader
{
public:
	static JSONSphereInfo& LoadSphereInfoFromFile(const char* filepath);

private:
	static bool HasAttribute(json* file, std::string key);
	static Vec3f ReadVec3f(std::vector<float> vec);
};

