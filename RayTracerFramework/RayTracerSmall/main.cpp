// [header]
// A very basic raytracer example.
// [/header]
// [compile]
// c++ -o raytracer -O3 -Wall raytracer.cpp
// [/compile]
// [ignore]
// Copyright (C) 2012  www.scratchapixel.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// [/ignore]

#include <fstream>
#include <iostream>
#include <cassert>
#include <thread>

// Windows only
#include <algorithm>
#include <sstream>
#include <string>

#include "Heap.h"
#include "Timer.h"
#include "Vec3.h"
#include "Sphere.h"
#include "MemoryManager.h"
#include "HeapManager.h"
#include "JSONReader.h"
#include "MemoryPool.h"
#include "ThreadManager.h"

#if defined __linux__ || defined __APPLE__
// "Compiled for Linux
#else

// Windows doesn't define these values by default, Linux does
#define M_PI 3.141592653589793
#define INFINITY 1e8
#endif

ThreadManager* threadManager;
MemoryPool* chunkPool;
MemoryPool* charPool;

//[comment]
// This variable controls the maximum recursion depth
//[/comment]
#define MAX_RAY_DEPTH 5

float mix(const float& a, const float& b, const float& mix)
{
	return b * mix + a * (1 - mix);
}

//[comment]
// This is the main trace function. It takes a ray as argument (defined by its origin
// and direction). We test if this ray intersects any of the geometry in the scene.
// If the ray intersects an object, we compute the intersection point, the normal
// at the intersection point, and shade this point using this information.
// Shading depends on the surface property (is it transparent, reflective, diffuse).
// The function returns a color for the ray. If the ray intersects an object that
// is the color of the object at the intersection point, otherwise it returns
// the background color.
//[/comment]
Vec3f trace(
	const Vec3f& rayorig,
	const Vec3f& raydir,
	const Sphere* spheres,
	const int& depth,
	const int& size)
{
	//if (raydir.length() != 1) std::cerr << "Error " << raydir << std::endl;
	float tnear = INFINITY;
	const Sphere* sphere = nullptr;
	// find intersection of this ray with the sphere in the scene
	for (unsigned i = 0; i < size; ++i) {
		float t0 = INFINITY, t1 = INFINITY;
		if (spheres[i].intersect(rayorig, raydir, t0, t1)) {
			if (t0 < 0) t0 = t1;
			if (t0 < tnear) {
				tnear = t0;
				sphere = &spheres[i];
			}
		}
	}

	// if there's no intersection return black or background color
	if (!sphere) return Vec3f(2);

	Vec3f surfaceColor = 0; // color of the ray/surface of the object intersected by the ray
	Vec3f phit = rayorig + raydir * tnear; // point of intersection
	Vec3f nhit = phit - sphere->_center; // normal at the intersection point
	nhit.normalize(); // normalize normal direction
					  // If the normal and the view direction are not opposite to each other
					  // reverse the normal direction. That also means we are inside the sphere so set
					  // the inside bool to true. Finally reverse the sign of IdotN which we want
					  // positive.
	float bias = 1e-4; // add some bias to the point from which we will be tracing
	bool inside = false;
	if (raydir.dot(nhit) > 0) nhit = -nhit, inside = true;
	if (depth < MAX_RAY_DEPTH && (sphere->_transparency > 0 || sphere->_reflection > 0)) {
		float facingratio = -raydir.dot(nhit);
		// change the mix value to tweak the effect
		float fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1);
		// compute reflection direction (not need to normalize because all vectors
		// are already normalized)
		Vec3f refldir = raydir - nhit * 2 * raydir.dot(nhit);
		refldir.normalize();
		Vec3f reflection = trace(phit + nhit * bias, refldir, spheres, depth + 1, size);
		Vec3f refraction = 0;
		// if the sphere is also transparent compute refraction ray (transmission)
		if (sphere->_transparency) {
			float ior = 1.1, eta = (inside) ? ior : 1 / ior; // are we inside or outside the surface?
			float cosi = -nhit.dot(raydir);
			float k = 1 - eta * eta * (1 - cosi * cosi);
			Vec3f refrdir = raydir * eta + nhit * (eta * cosi - sqrt(k));
			refrdir.normalize();
			refraction = trace(phit - nhit * bias, refrdir, spheres, depth + 1, size);
		}
		// the result is a mix of reflection and refraction (if the sphere is transparent)
		surfaceColor = (
			reflection * fresneleffect +
			refraction * (1 - fresneleffect) * sphere->_transparency) * sphere->_surfaceColor;
	}
	else {
		// it's a diffuse object, no need to ray trace any further
		for (unsigned i = size - 1; i != 0; --i) {
			if (spheres[i]._emissionColor.x > 0) {
				// this is a light
				Vec3f transmission = 1;
				Vec3f lightDirection = spheres[i]._center - phit;
				lightDirection.normalize();
				for (unsigned j = size - 1; j != 0; --j) {
					if (i != j) {
						float t0, t1;
						if (spheres[j].intersect(phit + nhit * bias, lightDirection, t0, t1)) {
							transmission = 0;
							break;
						}
					}
				}
				surfaceColor += sphere->_surfaceColor * transmission *
					std::max(float(0), nhit.dot(lightDirection)) * spheres[i]._emissionColor;
			}
		}
	}

	return surfaceColor + sphere->_emissionColor;
}

float fov = 30;
float angle = tan(M_PI * 0.5 * fov / 180.0f);

struct RenderConfig {
	unsigned width;
	unsigned height;
	unsigned quarterHeight;
	unsigned halfHeight;
	unsigned fullSize;
	unsigned chunkSize;
	float invWidth;
	float invHeight;
	float aspectRatio;
	char buffer[28];

	void CalculateValues() {
		halfHeight = height / 2;
		quarterHeight = halfHeight / 2;
		fullSize = width * height;
		chunkSize = width * (height / 4);
		invWidth = 1 / float(width);
		invHeight = 1 / float(height);
		aspectRatio = width / float(height);
	}
};

void RenderSector(
	const unsigned int& startX,
	const unsigned int& startY,
	const unsigned int& endX,
	const unsigned int& endY,
	const float& invWidth,
	const float& invHeight,
	const float& aspectratio,
	const Sphere* spheres, Vec3f* image, const int& size)
{

	int index = 0;
	for (unsigned y = startY; y < endY; ++y) {
		for (unsigned x = startX; x < endX; ++x) {
			float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
			float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
			Vec3f raydir(xx, yy, -1);
			raydir.normalize();
			image[index] = trace(Vec3f(0), raydir, spheres, 0, size);
			index++;
		}
	}
}

void WriteSector(Vec3f* chunk, int size, char* ss) {
	int charIndex = 0;
	for (unsigned i = 0; i < size; i++) {
		ss[charIndex] = (unsigned char)(std::min(1.0f, chunk[i].x) * 255);
		charIndex++;
		ss[charIndex] = (unsigned char)(std::min(1.0f, chunk[i].y) * 255);
		charIndex++;
		ss[charIndex] = (unsigned char)(std::min(1.0f, chunk[i].z) * 255);
		charIndex++;
	}
}

//[comment]
// Main rendering function. We compute a camera ray for each pixel of the image
// trace it and return a color. If the ray hits a sphere, we return the color of the
// sphere at the intersection point, else we return the background color.
//[/comment]
void Render(const RenderConfig& config, const Sphere* spheres, const int& iteration, const int& size)
{
	Vec3f* firstChunk = (Vec3f*)chunkPool->Alloc(config.chunkSize * sizeof(Vec3f));
	Vec3f* secondChunk = (Vec3f*)chunkPool->Alloc(config.chunkSize * sizeof(Vec3f));
	Vec3f* thirdChunk = (Vec3f*)chunkPool->Alloc(config.chunkSize * sizeof(Vec3f));
	Vec3f* fourthChunk = (Vec3f*)chunkPool->Alloc(config.chunkSize * sizeof(Vec3f));

	threadManager->CreateTask([&config, &firstChunk, spheres, size] {RenderSector(0, 0, config.width, config.quarterHeight, config.invWidth, config.invHeight, config.aspectRatio, spheres, std::ref(firstChunk), size); });
	threadManager->CreateTask([&config, &secondChunk, spheres, size] {RenderSector(0, config.quarterHeight, config.width, config.halfHeight, config.invWidth, config.invHeight, config.aspectRatio, spheres, std::ref(secondChunk), size); });
	threadManager->CreateTask([&config, &thirdChunk, spheres, size] {RenderSector(0, config.halfHeight, config.width, config.halfHeight + config.quarterHeight, config.invWidth, config.invHeight, config.aspectRatio, spheres, std::ref(thirdChunk), size); });
	threadManager->CreateTask([&config, &fourthChunk, spheres, size] {RenderSector(0, config.halfHeight + config.quarterHeight, config.width, config.height, config.invWidth, config.invHeight, config.aspectRatio, spheres, std::ref(fourthChunk), size); });
	threadManager->WaitForAllThreads();

	size_t strSize = config.width * config.quarterHeight * 3;
	char* s1 = (char*)charPool->Alloc(strSize);
	char* s2 = (char*)charPool->Alloc(strSize);
	char* s3 = (char*)charPool->Alloc(strSize);
	char* s4 = (char*)charPool->Alloc(strSize);
	threadManager->CreateTask([firstChunk, &config, &s1] {WriteSector(firstChunk, config.chunkSize, s1); });
	threadManager->CreateTask([secondChunk, &config, &s2] {WriteSector(secondChunk, config.chunkSize, s2); });
	threadManager->CreateTask([thirdChunk, &config, &s3] {WriteSector(thirdChunk, config.chunkSize, s3); });
	threadManager->CreateTask([fourthChunk, &config, &s4] {WriteSector(fourthChunk, config.chunkSize, s4); });
	threadManager->WaitForAllThreads();

	std::string name = "./spheres" + std::to_string(iteration) + ".ppm";
	std::ofstream ofs(name, std::ios::out | std::ios::binary);
	std::string line = "P6\n" + std::to_string(config.width) + " " + std::to_string(config.height) + "\n255\n";
	ofs.write(line.c_str(), line.length());
	ofs.write(s1, strSize);
	ofs.write(s2, strSize);
	ofs.write(s3, strSize);
	ofs.write(s4, strSize);
	ofs.close();

	chunkPool->Free(firstChunk);
	chunkPool->Free(secondChunk);
	chunkPool->Free(thirdChunk);
	chunkPool->Free(fourthChunk);
	charPool->Free(s1);
	charPool->Free(s2);
	charPool->Free(s3);
	charPool->Free(s4);
}

void BasicRender(const RenderConfig& config)
{
	//Create dynamic array for spheres, more efficient than creating vector
	Sphere* spheres = new Sphere[4];

	spheres[0] = Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0);
	spheres[1] = Sphere(Vec3f(0.0, 0, -20), 4, Vec3f(1.00, 0.32, 0.36), 1, 0.5); // The radius paramter is the value we will change
	spheres[2] = Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0);
	spheres[3] = Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0);

	// This creates a file, titled 1.ppm in the current working directory
	Render(config, spheres, 1, 4);

	delete[] spheres;
	spheres = nullptr;

}

void SimpleShrinking(const RenderConfig& config)
{
	//Create dynamic array for spheres, more efficient than creating vector
	Sphere* spheres = new Sphere[4];


	spheres[0] = Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0);
	spheres[1] = Sphere(Vec3f(0.0, 0, -20), 4, Vec3f(1.00, 0.32, 0.36), 1, 0.5); // The radius paramter is the value we will change
	spheres[2] = Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0);
	spheres[3] = Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0);

	for (int i = 0; i < 4; i++)
	{
		if (i == 0)
		{
			spheres[1]._radius = 4;
			spheres[1]._radiusSqr = 4 * 4;
		}
		else if (i == 1)
		{
			spheres[1]._radius = 3;
			spheres[1]._radiusSqr = 3 * 3;
		}
		else if (i == 2)
		{
			spheres[1]._radius = 2;
			spheres[1]._radiusSqr = 2 * 2;
		}
		else if (i == 3)
		{
			spheres[1]._radius = 1;
			spheres[1]._radiusSqr = 1 * 1;
		}

		Render(config, spheres, i, 4);
		// Dont forget to clear the Vector holding the spheres.
	}

	delete[] spheres;
	spheres = nullptr;
}

void SmoothScaling(const RenderConfig& config)
{
	//Create dynamic array for spheres, more efficient than creating vector
	Sphere* spheres = new Sphere[4];

	spheres[0] = Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0);
	spheres[1] = Sphere(Vec3f(0.0, 0, -20), 0 / 100, Vec3f(1.00, 0.32, 0.36), 1, 0.5); // Radius++ change here
	spheres[2] = Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0);
	spheres[3] = Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0);

	for (float r = 0; r <= 100; r++)
	{
		float radius = r / 100;
		spheres[1]._radius = radius;
		spheres[1]._radiusSqr = radius * radius;

		Render(config, spheres, r, 4);
		std::cout << "Rendered and saved spheres" << r << ".ppm" << std::endl;
	}

	delete[] spheres;
	spheres = nullptr;
}

void RenderFromJSONFile(const JSONSphereInfo& info, const RenderConfig& config) {

	//Iterate through all the frames
	for (int i = 0; i < info.frameCount; i++) {
		//Iterate through all spheres
		for (int j = 0; j < info.sphereCount; j++) {
			//Change sphere position and surface color
			info.sphereArr[j]._center += info.sphereMovementsPerFrame[j];
			info.sphereArr[j]._surfaceColor += info.sphereColorPerFrame[j];
		}

		//Call render function
		Render(config, info.sphereArr, i, info.sphereCount);
		std::cout << "Rendered and saved spheres" << i << ".ppm" << std::endl;
	}
}

//[comment]
// In the main function, we will create the scene which is composed of 5 spheres
// and 1 light (which is also a sphere). Then, once the scene description is complete
// we render that scene, by calling the render() function.
//[/comment]
int main(int argc, char** argv)
{
	// This sample only allows one choice per program execution. Feel free to improve upon this
	srand(13);

	RenderConfig config;
	config.width = 640;
	config.height = 480;
	config.CalculateValues();

	Timer timer;

	Heap* chunkHeap = HeapManager::CreateHeap("ChunkHeap");
	threadManager = new ThreadManager();

	Heap* charHeap = HeapManager::CreateHeap("CharHeap");

	//Allocate a memory pool for the four image chunks 
	chunkPool = new(chunkHeap) MemoryPool(chunkHeap, 4, sizeof(Vec3f) * config.chunkSize);
	charPool = new(charHeap) MemoryPool(charHeap, 4, config.width * config.quarterHeight * 3);

	JSONSphereInfo info = JSONReader::LoadSphereInfoFromFile("Animations/animSample.json");

	//SmoothScaling(config);
	BasicRender(config);
	//SimpleShrinking(config);
	//RenderFromJSONFile(info, config);

	float timeToComplete = timer.Mark();
	std::cout << "Time to complete: " << timeToComplete << std::endl;

	delete chunkPool;
	chunkPool = nullptr;

	delete threadManager;
	threadManager = nullptr;

	delete charPool;
	charPool = nullptr;

	info.Cleanup();

	std::cout << "\n\n" << "HEAP DUMP" << "\n\n";
	HeapManager::DebugAll();

	std::cout << "Deleting heaps" << std::endl;
	HeapManager::CleanHeaps();

	system("ffmpeg -framerate 25 -i spheres%d.ppm -vcodec mpeg4 output.mp4");
	return 0;

}