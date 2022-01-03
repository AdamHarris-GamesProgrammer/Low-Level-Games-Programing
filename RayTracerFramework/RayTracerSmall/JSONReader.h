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
	//Arrays of information for an animation
	Sphere* sphereArr;
	Vec3f* sphereEndPositions;
	Vec3f* sphereMovementsPerFrame;
	Vec3f* sphereEndColor;
	Vec3f* sphereColorPerFrame;

	//Frame and sphere count. Needed to create arrays for each sphere
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
		//Cycles through each sphere
		for (int i = 0; i < sphereCount; i++) {
			//Calculates the amount of movement each frame based on lerping
			Vec3f diff = sphereEndPositions[i] - sphereArr[i]._center;
			diff.x /= frameCount;
			diff.y /= frameCount;
			diff.z /= frameCount;
			sphereMovementsPerFrame[i] = diff;
		}
	}

	void CalculateSphereColor() {
		//Cycles through each sphere
		for (int i = 0; i < sphereCount; i++) {
			//Lerps between the start and end colors
			Vec3f diff = sphereEndColor[i] - sphereArr[i]._surfaceColor;
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
	//Loads animation information from a json file
	static JSONSphereInfo* LoadSphereInfoFromFile(const char* filepath);

private:
	//helper methods for loading a file
	static bool HasAttribute(json* file, std::string key);
	static Vec3f ReadVec3f(std::vector<float> vec);
};

