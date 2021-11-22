#pragma once
#include "json.hpp"
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
	Vec3f* sphereEndColor;
	Vec3f* sphereColorPerFrame;
	int frameCount;
	int sphereCount;

	JSONSphereInfo(int spherecount, int framecount) {
		frameCount = framecount;
		sphereCount = spherecount;
		sphereArr = new Sphere[spherecount];
		sphereEndPositions = new Vec3f[spherecount];
		sphereMovementsPerFrame = new Vec3f[spherecount];
		sphereEndColor = new Vec3f[spherecount];
		sphereColorPerFrame = new Vec3f[spherecount];
	}

	void Cleanup() {
		delete sphereArr;
		delete sphereEndPositions;
		delete sphereMovementsPerFrame;
		delete sphereEndColor;
		delete sphereColorPerFrame;

		sphereArr = nullptr;
		sphereEndPositions = nullptr;
		sphereMovementsPerFrame = nullptr;
		sphereEndColor = nullptr;
		sphereColorPerFrame = nullptr;
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

	void CalculateSphereColor() {
		for (int i = 0; i < sphereCount; i++) {
			Vec3f diff = sphereEndColor[i] - sphereArr[i]._surfaceColor;
			Vec3f color = sphereArr[i]._surfaceColor;
			Vec3f endColor = sphereEndColor[i];
			diff.x /= frameCount;
			diff.y /= frameCount;
			diff.z /= frameCount;
			sphereColorPerFrame[i] = diff;
		}
	}
};

class JSONReader
{
public:
	static JSONSphereInfo* LoadSphereInfoFromFile(const char* filepath);

private:
	static bool HasAttribute(json* file, std::string key);
	static Vec3f ReadVec3f(std::vector<float> vec);
};

