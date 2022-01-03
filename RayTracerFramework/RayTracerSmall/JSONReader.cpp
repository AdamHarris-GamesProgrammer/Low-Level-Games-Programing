#include "JSONReader.h"

JSONSphereInfo* JSONReader::LoadSphereInfoFromFile(const char* filepath)
{
	std::fstream file(filepath);
	
	json jsonFile;
	file >> jsonFile;

	int noFrames = 0;
	int noSpheres = 0;

	//Checks we have a frame count attribute
	if (HasAttribute(&jsonFile, "frameCount")) {
		noFrames = jsonFile["frameCount"];
	}
	else {
		//Outputs an error if we do not
		std::cout << "[ERROR: JSONReader.cpp: file does not specify number of frames. Use attribute \"frameCount\" to specify" << std::endl;
	}

	//Checks we have a sphere count attribute
	if (HasAttribute(&jsonFile, "sphereCount")) {
		noSpheres = jsonFile["sphereCount"];
	}
	else {
		//Outputs an error if we do not
		std::cout << "[ERROR: JSONReader.cpp: file does not specify number of spheres. Use attribute \"sphereCount\" to specify" << std::endl;
	}

	//Creates the animation information object
	JSONSphereInfo* animInfo = new JSONSphereInfo(noSpheres, noFrames);

	//Gets the "spheres" array from the json object
	json sphereArr = jsonFile["spheres"];

	//Cycles through all spheres
	for (int i = 0; i < noSpheres; i++) {
		//Gets the current json object for this sphere
		json sphere = sphereArr.at(i);

		//Sets the starting position
		if (HasAttribute(&sphere, "startPos")) {
			animInfo->sphereArr[i]._center = ReadVec3f(sphere["startPos"]);
		}

		//Sets the ending position
		if (HasAttribute(&sphere, "endPos")) {
			animInfo->sphereEndPositions[i] = ReadVec3f(sphere["endPos"]);
		}

		//Sets the surface color
		if (HasAttribute(&sphere, "surfaceColor")) {
			animInfo->sphereArr[i]._surfaceColor = ReadVec3f(sphere["surfaceColor"]);
		}

		//Sets the end surface color value
		if (HasAttribute(&sphere, "endSurfaceColor")) {
			animInfo->sphereEndColor[i] = ReadVec3f(sphere["endSurfaceColor"]);
		}

		//Sets the radius and radius squared
		if (HasAttribute(&sphere, "radius")) {
			float radius = sphere["radius"];
			animInfo->sphereArr[i]._radius = radius;
			animInfo->sphereArr[i]._radiusSqr = radius * radius;
		}

		//Sets the reflection value
		if (HasAttribute(&sphere, "reflection")) {
			animInfo->sphereArr[i]._reflection = sphere["reflection"];
		}

		//Sets the transparency value
		if (HasAttribute(&sphere, "transparency")) {
			animInfo->sphereArr[i]._transparency = sphere["transparency"];
		}

		//Sets the emmision color value
		if (HasAttribute(&sphere, "emmisionColor")) {
			animInfo->sphereArr[i]._emissionColor = ReadVec3f(sphere["emmisionColor"]);
		}


	}

	//Calculate the sphere movement and color values accross each frame
	animInfo->CalculateSphereMovements();
	animInfo->CalculateSphereColor();
	return animInfo;
}


bool JSONReader::HasAttribute(json* file, std::string key)
{
	//Sees if this json object contains the key
	if (file->contains(key)) return true;

	return false;
}

Vec3f JSONReader::ReadVec3f(std::vector<float> vec)
{
	//Reads a vector of floats into a vec3f object
	return Vec3f(vec[0], vec[1], vec[2]);
}
