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

MemoryPool* chunkPool;
MemoryPool* charPool;

//[comment]
// This variable controls the maximum recursion depth
//[/comment]
#define MAX_RAY_DEPTH 5
#define MAX_THREADS 20

//#define NO_MUTEX
#define MUTEX

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
	const Sphere* hit = nullptr;
	// find intersection of this ray with the sphere in the scene
	for (size_t i = 0; i < size; ++i) {
		const Sphere& object = spheres[i];
		float t0 = INFINITY, t1 = INFINITY;
		if (object.intersect(rayorig, raydir, t0, t1)) {
			if (t0 < 0) t0 = t1;
			if (t0 < tnear) {
				tnear = t0;
				hit = &object;
			}
		}
	}

	// if there's no intersection return black or background color
	if (!hit) return Vec3f(2);

	Vec3f surfaceColor = 0; // color of the ray/surface of the object intersected by the ray
	Vec3f phit = rayorig + raydir * tnear; // point of intersection
	Vec3f nhit = phit - hit->_center; // normal at the intersection point
	nhit.normalize(); // normalize normal direction
					  // If the normal and the view direction are not opposite to each other
					  // reverse the normal direction. That also means we are inside the sphere so set
					  // the inside bool to true. Finally reverse the sign of IdotN which we want
					  // positive.
	float bias = 1e-4; // add some bias to the point from which we will be tracing
	bool inside = false;
	if (raydir.dot(nhit) > 0) nhit = -nhit, inside = true;
	if (depth < MAX_RAY_DEPTH && (hit->_transparency > 0.0f || hit->_reflection > 0.0f)) {
		float facingratio = -raydir.dot(nhit);
		// change the mix value to tweak the effect
		float fresneleffect = mix(pow(1.0f - facingratio, 3.0f), 1.0f, 0.1f);
		// compute reflection direction (not need to normalize because all vectors
		// are already normalized)
		Vec3f refldir = raydir - nhit * 2 * raydir.dot(nhit);
		refldir.normalize();
		Vec3f reflection = trace(phit + nhit * bias, refldir, spheres, depth + 1, size);
		Vec3f refraction = 0;
		// if the sphere is also transparent compute refraction ray (transmission)
		if (hit->_transparency) {
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
			refraction * (1 - fresneleffect) * hit->_transparency) * hit->_surfaceColor;
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
				surfaceColor += hit->_surfaceColor * transmission *
					std::max(float(0), nhit.dot(lightDirection)) * spheres[i]._emissionColor;
			}
		}
	}

	return surfaceColor + hit->_emissionColor;
}

float fov = 30;
float angle = tan(M_PI * 0.5 * fov / 180.0f);

struct RenderConfig {
	unsigned width;
	unsigned height;
	unsigned chunkHeight;
	unsigned chunkSize;
	size_t charSize;
	size_t vec3Size;
	float invWidth;
	float invHeight;
	float aspectRatio;
	char buffer[32];

	void CalculateValues() {
		chunkHeight = height / MAX_THREADS;
		chunkSize = (width * height) / MAX_THREADS;
		invWidth = 1 / float(width);
		invHeight = 1 / float(height);
		aspectRatio = width / float(height);
		charSize = chunkSize * 3;
		vec3Size = chunkSize * sizeof(Vec3f);
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
#ifdef NO_MUTEX
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
#else
	int index = endX * startY + startX;
	std::mutex locker;
	for (unsigned y = startY; y < endY; ++y) {
		for (unsigned x = startX; x < endX; ++x) {
			float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
			float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
			Vec3f raydir(xx, yy, -1);
			raydir.normalize();
			locker.lock();
			image[index] = trace(Vec3f(0), raydir, spheres, 0, size);
			locker.unlock();
			index++;
		}
	}
	std::cout << index << std::endl;
#endif // NO_MUTEX
}

void WriteSector(Vec3f* chunk, int size, char* ss) {
#ifdef NO_MUTEX
	int charIndex = 0;
	for (unsigned i = 0; i < size; ++i) {
		ss[charIndex] = (unsigned char)(std::min(1.0f, chunk[i].x) * 255);
		ss[charIndex + 1] = (unsigned char)(std::min(1.0f, chunk[i].y) * 255);
		ss[charIndex + 2] = (unsigned char)(std::min(1.0f, chunk[i].z) * 255);
		charIndex += 3;
	}
#else
	int charIndex = 0;
	std::mutex locker;
	for (unsigned i = 0; i < size; ++i) {
		locker.lock();
		ss[charIndex] = (unsigned char)(std::min(1.0f, chunk[i].x) * 255);
		ss[charIndex + 1] = (unsigned char)(std::min(1.0f, chunk[i].y) * 255);
		ss[charIndex + 2] = (unsigned char)(std::min(1.0f, chunk[i].z) * 255);
		locker.unlock();
		charIndex += 3;
	}
#endif //NO_MUTEX
}

//[comment]
// Main rendering function. We compute a camera ray for each pixel of the image
// trace it and return a color. If the ray hits a sphere, we return the color of the
// sphere at the intersection point, else we return the background color.
//[/comment]
void Render(const RenderConfig& config, const Sphere* spheres, const int& iteration, const int& size)
{
#ifdef NO_MUTEX
	Vec3f** chunkArrs = new Vec3f * [MAX_THREADS];
	char** charArrs = new char* [MAX_THREADS];
	RenderConfig* renderConfigs = new RenderConfig[MAX_THREADS];
	for (int i = 0; i < MAX_THREADS; ++i) {
		renderConfigs[i] = std::ref(config);
		chunkArrs[i] = (Vec3f*)chunkPool->Alloc(config.vec3Size);
		charArrs[i] = (char*)charPool->Alloc(config.charSize);
	}

	int startY = 0;
	int endY = config.chunkHeight;
	for (int i = 0; i < MAX_THREADS; ++i) {
		Vec3f* currentChunk = chunkArrs[i];
		RenderConfig config = renderConfigs[i];
		ThreadManager::CreateTask([config, currentChunk, &spheres, &size, startY, endY] {RenderSector(0, startY, config.width, endY, config.invWidth, config.invHeight, config.aspectRatio, spheres, currentChunk, size); });
		startY += config.chunkHeight;
		endY += config.chunkHeight;
	}

	ThreadManager::WaitForAllThreads();
	for (int i = 0; i < MAX_THREADS; ++i) {
		char* currentArr = charArrs[i];
		Vec3f* currentChunk = chunkArrs[i];
		RenderConfig config = renderConfigs[i];
		ThreadManager::CreateTask([currentChunk, config, currentArr] {WriteSector(currentChunk, config.chunkSize, currentArr); });
	}

	std::string name = "./spheres" + std::to_string(iteration) + ".ppm";
	std::ofstream ofs(name, std::ios::out | std::ios::binary);
	std::string line = "P6\n" + std::to_string(config.width) + " " + std::to_string(config.height) + "\n255\n";
	ofs.write(line.c_str(), line.length());

	ThreadManager::WaitForAllThreads();
	for (int i = 0; i < MAX_THREADS; ++i) {
		ofs.write(charArrs[i], config.charSize);
		chunkPool->Free(chunkArrs[i]);
		charPool->Free(charArrs[i]);
	}

	ofs.close();

	delete[] charArrs;
	delete[] chunkArrs;
	delete[] renderConfigs;

	charArrs = nullptr;
	chunkArrs = nullptr;
	renderConfigs = nullptr;

	name.clear();
	line.clear();
#else
	Vec3f* image = (Vec3f*)chunkPool->Alloc(config.vec3Size * MAX_THREADS);
	char* charArray = (char*)charPool->Alloc(config.charSize * MAX_THREADS);
		
	int startY = 0;
	int endY = config.chunkHeight;
	for (int i = 0; i < MAX_THREADS; ++i) {
		ThreadManager::CreateTask([config, image, &spheres, &size, startY, endY] {RenderSector(0, startY, config.width, endY, config.invWidth, config.invHeight, config.aspectRatio, spheres, image, size); });
		startY += config.chunkHeight;
		endY += config.chunkHeight;
	}
	ThreadManager::WaitForAllThreads();
	//for (int i = 0; i < MAX_THREADS; ++i) {
	//	ThreadManager::CreateTask([image, config, charArray] {WriteSector(image, config.chunkSize * MAX_THREADS, charArray); });
	//}

	int charIndex = 0;
	std::mutex locker;
	for (unsigned i = 0; i < config.chunkSize * MAX_THREADS; ++i) {
		locker.lock();
		charArray[charIndex] = (unsigned char)(std::min(1.0f, image[i].x) * 255);
		charArray[charIndex + 1] = (unsigned char)(std::min(1.0f, image[i].y) * 255);
		charArray[charIndex + 2] = (unsigned char)(std::min(1.0f, image[i].z) * 255);
		charIndex += 3;
		locker.unlock();
		
	}

	std::cout << image[200000] << std::endl;

	std::string name = "./spheres" + std::to_string(iteration) + ".ppm";
	std::ofstream ofs(name, std::ios::out | std::ios::binary);
	std::string line = "P6\n" + std::to_string(config.width) + " " + std::to_string(config.height) + "\n255\n";
	ofs.write(line.c_str(), line.length());

	ThreadManager::WaitForAllThreads();

	ofs.write(charArray, charIndex);
	
	chunkPool->Free(image);
	charPool->Free(charArray);

	ofs.close();

	name.clear();
	line.clear();
#endif // NO_MUTEX
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

	for (int i = 0; i < 4; ++i)
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

	for (float r = 0; r <= 100; ++r)
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
	for (int i = 0; i < info.frameCount; ++i) {
		//Iterate through all spheres
		for (int j = 0; j < info.sphereCount; ++j) {
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

	//setHighestTimerResolution(1);

	Timer timer;

	Heap* chunkHeap = HeapManager::CreateHeap("ChunkHeap");

	Heap* charHeap = HeapManager::CreateHeap("CharHeap");

	//Allocate a memory pool for the four image chunks 
#ifdef NO_MUTEX
	chunkPool = new(chunkHeap) MemoryPool(chunkHeap, MAX_THREADS, config.vec3Size);
	charPool = new(charHeap) MemoryPool(charHeap, MAX_THREADS, config.charSize);
#else
	chunkPool = new(chunkHeap) MemoryPool(chunkHeap, 1, config.vec3Size * MAX_THREADS);
	charPool = new(charHeap) MemoryPool(charHeap, 1, config.charSize * MAX_THREADS);
#endif


	JSONSphereInfo* info = JSONReader::LoadSphereInfoFromFile("Animations/animSample.json");


	std::cout << sizeof(Sphere) << std::endl;
	SmoothScaling(config);
	//BasicRender(config);
	//SimpleShrinking(config);
	//RenderFromJSONFile(info, config);

	float timeToComplete = timer.Mark();
	std::cout << "Time to complete: " << timeToComplete << std::endl;

	delete chunkPool;
	chunkPool = nullptr;

	delete charPool;
	charPool = nullptr;

	info->Cleanup();

	std::cout << "\n\n" << "HEAP DUMP" << "\n\n";
	HeapManager::DebugAll();

	std::cout << "Deleting heaps" << std::endl;
	HeapManager::CleanHeaps();

	chunkHeap = nullptr;
	charHeap = nullptr;

	//system("ffmpeg -framerate 25 -i spheres%d.ppm -vcodec mpeg4 output.mp4");
	return 0;

}