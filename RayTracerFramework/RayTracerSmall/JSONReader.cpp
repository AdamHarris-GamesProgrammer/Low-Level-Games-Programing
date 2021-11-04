#include "JSONReader.h"

JSONSphereInfo& JSONReader::LoadSphereInfoFromFile(const char* filepath)
{
	std::fstream file(filepath);
	
	json jsonFile;
	file >> jsonFile;

	int noFrames = 0;
	int noSpheres = 0;

	if (HasAttribute(&jsonFile, "frameCount")) {
		noFrames = jsonFile["frameCount"];
	}
	else {
		std::cout << "[ERROR: JSONReader.cpp: file does not specify number of frames. Use attribute \"frameCount\" to specify" << std::endl;
	}

	if (HasAttribute(&jsonFile, "sphereCount")) {
		noSpheres = jsonFile["sphereCount"];
	}
	else {
		std::cout << "[ERROR: JSONReader.cpp: file does not specify number of spheres. Use attribute \"sphereCount\" to specify" << std::endl;
	}

	JSONSphereInfo animInfo = JSONSphereInfo(noSpheres, noFrames);

	json sphereArr = jsonFile["spheres"];


	for (int i = 0; i < noSpheres; i++) {
		json sphere = sphereArr.at(i);

		if (HasAttribute(&sphere, "startPos")) {
			animInfo.sphereArr[i]._center = ReadVec3f(sphere["startPos"]);
		}

		if (HasAttribute(&sphere, "endPos")) {
			animInfo.sphereEndPositions[i] = ReadVec3f(sphere["endPos"]);
		}

		if (HasAttribute(&sphere, "radius")) {
			float radius = sphere["radius"];
			animInfo.sphereArr[i]._radius = radius;
			animInfo.sphereArr[i]._radiusSqr = radius * radius;
		}

		if (HasAttribute(&sphere, "surfaceColor")) {
			animInfo.sphereArr[i]._surfaceColor = ReadVec3f(sphere["surfaceColor"]);
		}

		if (HasAttribute(&sphere, "reflection")) {
			animInfo.sphereArr[i]._reflection = sphere["reflection"];
		}

		if (HasAttribute(&sphere, "transparency")) {
			animInfo.sphereArr[i]._transparency = sphere["transparency"];
		}

		if (HasAttribute(&sphere, "emmisionColor")) {
			animInfo.sphereArr[i]._emissionColor = ReadVec3f(sphere["emmisionColor"]);
		}
	}

	animInfo.CalculateSphereMovements();
	return animInfo;
}

bool JSONReader::HasAttribute(json* file, std::string key)
{
	if (file->contains(key)) return true;

	return false;
}

Vec3f JSONReader::ReadVec3f(std::vector<float> vec)
{
	return Vec3f(vec[0], vec[1], vec[2]);
}
