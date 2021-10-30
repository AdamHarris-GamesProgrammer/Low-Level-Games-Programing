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

#include <stdlib.h>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <vector>
#include <iostream>
#include <cassert>
// Windows only
#include <algorithm>
#include <sstream>
#include <string.h>
#include "Heap.h"

#include "Timer.h"
#include "Vec3.h"
#include "MemoryManager.h"
#include "HeapFactory.h"
#include <thread>

#if defined __linux__ || defined __APPLE__
// "Compiled for Linux
#else
// Windows doesn't define these values by default, Linux does
#define M_PI 3.141592653589793
#define INFINITY 1e8
#endif

class Sphere
{
public:
	Vec3f _center;                           /// position of the sphere
	float _radius, _radiusSqr;                  /// sphere radius and radius^2
	Vec3f _surfaceColor, _emissionColor;      /// surface color and emission (light)
	float _transparency, _reflection;		/// surface transparency and reflectivity
	Sphere() = default;
	Sphere(
		const Vec3f& center,
		const float& radius,
		const Vec3f& surfaceColor,
		const float& reflection = 0,
		const float& transparency = 0,
		const Vec3f& emmisionColor = 0) :
		_center(center), _radius(radius), _radiusSqr(radius* radius), _surfaceColor(surfaceColor), _emissionColor(emmisionColor),
		_transparency(transparency), _reflection(reflection) { }

	//[comment]
	// Compute a ray-sphere intersection using the geometric solution
	//[/comment]
	bool intersect(const Vec3f& rayorig, const Vec3f& raydir, float& t0, float& t1) const
	{
		Vec3f l = _center - rayorig;
		float tca = l.dot(raydir);
		if (tca < 0) return false;
		float d2 = l.dot(l) - tca * tca;
		if (d2 > _radiusSqr) return false;
		float thc = sqrt(_radiusSqr - d2);
		t0 = tca - thc;
		t1 = tca + thc;

		return true;
	}
};

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
	const std::vector<Sphere>& spheres,
	const int& depth)
{
	//if (raydir.length() != 1) std::cerr << "Error " << raydir << std::endl;
	float tnear = INFINITY;
	const Sphere* sphere = NULL;
	// find intersection of this ray with the sphere in the scene
	std::vector<Sphere*>::size_type size = spheres.size();
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

	Vec3f surfaceColor = 0; // color of the ray/surfaceof the object intersected by the ray
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
		Vec3f reflection = trace(phit + nhit * bias, refldir, spheres, depth + 1);
		Vec3f refraction = 0;
		// if the sphere is also transparent compute refraction ray (transmission)
		if (sphere->_transparency) {
			float ior = 1.1, eta = (inside) ? ior : 1 / ior; // are we inside or outside the surface?
			float cosi = -nhit.dot(raydir);
			float k = 1 - eta * eta * (1 - cosi * cosi);
			Vec3f refrdir = raydir * eta + nhit * (eta * cosi - sqrt(k));
			refrdir.normalize();
			refraction = trace(phit - nhit * bias, refrdir, spheres, depth + 1);
		}
		// the result is a mix of reflection and refraction (if the sphere is transparent)
		surfaceColor = (
			reflection * fresneleffect +
			refraction * (1 - fresneleffect) * sphere->_transparency) * sphere->_surfaceColor;
	}
	else {
		// it's a diffuse object, no need to raytrace any further
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
	unsigned halfWidth;
	unsigned halfHeight;
	unsigned fullSize;
	unsigned chunkSize;
	float invWidth;
	float invHeight;
	float aspectRatio;
	char buffer[28];

	void CalculateValues() {
		halfWidth = width / 2;
		halfHeight = height / 2;
		fullSize = width * height;
		chunkSize = halfWidth * halfHeight;
		invWidth = 1 / float(width);
		invHeight = 1 / float(height);
		aspectRatio = width / float(height);
	}
};

void RenderSector(
	const unsigned int startX, 
	const unsigned int startY, 
	const unsigned int endX, 
	const unsigned int endY, 
	const float& invWidth, 
	const float& invHeight,
	const float& aspectratio,
	const std::vector<Sphere>& spheres, Vec3f* image) 
{

	int index = 0;
	for (unsigned y = startY; y < endY; ++y) {
		for (unsigned x = startX; x < endX; ++x) {
			float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
			float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
			Vec3f raydir(xx, yy, -1);
			raydir.normalize();
			image[index] = trace(Vec3f(0), raydir, spheres, 0);
			index++;
		}
	}
}


//[comment]
// Main rendering function. We compute a camera ray for each pixel of the image
// trace it and return a color. If the ray hits a sphere, we return the color of the
// sphere at the intersection point, else we return the background color.
//[/comment]
void render(const RenderConfig& config, const std::vector<Sphere>& spheres, int iteration)
{
	Vec3f* image = new Vec3f[config.fullSize];
	Vec3f* firstChunk = new Vec3f[config.chunkSize];
	Vec3f* secondChunk = new Vec3f[config.chunkSize];
	Vec3f* thirdChunk = new Vec3f[config.chunkSize];
	Vec3f* fourthChunk = new Vec3f[config.chunkSize];


	std::thread topLeft = std::thread([&config, &firstChunk, spheres]
		{
			RenderSector(0, 0, config.halfWidth, config.halfHeight, config.invWidth, config.invHeight, config.aspectRatio, spheres, std::ref(firstChunk));
		}
	);


	std::thread topRight = std::thread([&config, &secondChunk, spheres]
		{
			RenderSector(config.halfWidth, 0, config.width , config.halfHeight, config.invWidth, config.invHeight, config.aspectRatio, spheres, std::ref(secondChunk));
		}
	);

	std::thread bottomLeft = std::thread([&config, &thirdChunk, spheres]
		{
			RenderSector(0, config.halfHeight, config.halfWidth, config.height, config.invWidth, config.invHeight, config.aspectRatio, spheres, std::ref(thirdChunk));
		}
	);

	std::thread bottomRight = std::thread([&config, &fourthChunk, spheres]
		{
			RenderSector(config.halfWidth, config.halfHeight, config.width, config.height, config.invWidth, config.invHeight, config.aspectRatio, spheres, std::ref(fourthChunk));
		}
	);

	topLeft.join();
	int index = 0;
	for (unsigned y = 0; y < config.halfHeight; ++y) {
		for (unsigned x = 0; x < config.halfWidth; ++x) {
			image[x + config.width * y] = firstChunk[index];
			index++;
		}
	}

	topRight.join();
	index = 0;
	for (unsigned y = 0; y < config.halfHeight; ++y) {
		for (unsigned x = config.halfWidth; x < config.width; ++x) {
			image[x + config.width * y] = secondChunk[index];
			index++;
		}
	}

	bottomLeft.join();
	index = 0;
	for (unsigned y = config.halfHeight; y < config.height; ++y) {
		for (unsigned x = 0; x < config.halfWidth; ++x) {
			image[x + config.width * y] = thirdChunk[index];
			index++;
		}
	}

	bottomRight.join();
	index = 0;
	for (unsigned y = config.halfHeight; y < config.height; ++y) {
		for (unsigned x = config.halfWidth; x < config.width; ++x) {
			image[x + config.width * y] = fourthChunk[index];
			index++;
		}
	}

	delete[] firstChunk;
	firstChunk = nullptr;
	delete[] secondChunk;
	secondChunk = nullptr;
	delete[] thirdChunk;
	thirdChunk = nullptr;
	delete[] fourthChunk;
	fourthChunk = nullptr;


	// Save result to a PPM image (keep these flags if you compile under Windows)
	std::stringstream ss;
	ss << "./spheres" << iteration << ".ppm";
	std::string tempString = ss.str();
	char* filename = (char*)tempString.c_str();

	std::stringstream fileStream;
	fileStream << "P6\n" << config.width << " " << config.height << "\n255\n";
	for (unsigned i = 0; i < config.fullSize; ++i) {
		fileStream << (unsigned char)(std::min(1.0f, image[i].x) * 255) <<
			(unsigned char)(std::min(1.0f, image[i].y) * 255) <<
			(unsigned char)(std::min(1.0f, image[i].z) * 255);
	}

	std::ofstream ofs(filename, std::ios::out | std::ios::binary);
	ofs << fileStream.str();
	ofs.close();

	delete[] image;
	image = nullptr;
}

void BasicRender(const RenderConfig& config)
{
	std::vector<Sphere> spheres;
	// Vector structure for Sphere (position, radius, surface color, reflectivity, transparency, emission color)

	spheres.push_back(Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
	spheres.push_back(Sphere(Vec3f(0.0, 0, -20), 4, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // The radius paramter is the value we will change
	spheres.push_back(Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
	spheres.push_back(Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));

	// This creates a file, titled 1.ppm in the current working directory
	render(config, spheres, 1);

}

void SimpleShrinking(const RenderConfig& config)
{
	std::vector<Sphere> spheres;
	// Vector structure for Sphere (position, radius, surface color, reflectivity, transparency, emission color)

	for (int i = 0; i < 4; i++)
	{
		if (i == 0)
		{
			spheres.push_back(Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
			spheres.push_back(Sphere(Vec3f(0.0, 0, -20), 4, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // The radius paramter is the value we will change
			spheres.push_back(Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
			spheres.push_back(Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));

		}
		else if (i == 1)
		{
			spheres.push_back(Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
			spheres.push_back(Sphere(Vec3f(0.0, 0, -20), 3, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // Radius--
			spheres.push_back(Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
			spheres.push_back(Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));
		}
		else if (i == 2)
		{
			spheres.push_back(Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
			spheres.push_back(Sphere(Vec3f(0.0, 0, -20), 2, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // Radius--
			spheres.push_back(Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
			spheres.push_back(Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));
		}
		else if (i == 3)
		{
			spheres.push_back(Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
			spheres.push_back(Sphere(Vec3f(0.0, 0, -20), 1, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // Radius--
			spheres.push_back(Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
			spheres.push_back(Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));
		}

		render(config, spheres, i);
		// Dont forget to clear the Vector holding the spheres.
		spheres.clear();
	}
}

void SmoothScaling(const RenderConfig& config)
{
	std::vector<Sphere> spheres;
	// Vector structure for Sphere (position, radius, surface color, reflectivity, transparency, emission color)

	spheres.resize(4);
	spheres[0] = Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0);
	spheres[1] = Sphere(Vec3f(0.0, 0, -20), 0 / 100, Vec3f(1.00, 0.32, 0.36), 1, 0.5); // Radius++ change here
	spheres[2] = Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0);
	spheres[3] = Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0);

	for (float r = 0; r <= 100; r++)
	{
		float radius = r / 100;
		spheres[1]._radius = radius;
		spheres[1]._radiusSqr = radius * radius;

		render(config, spheres, r);
		std::cout << "Rendered and saved spheres" << r << ".ppm" << std::endl;
	}

	spheres.clear();
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

	float timeToComplete = 0.0f;

	Timer timer;

	RenderConfig configObject;
	configObject.width = 640;
	configObject.height = 480;
	configObject.CalculateValues();

	HeapFactory::Init();

	int* v = new int;

	//SmoothScaling(configObject);
	//BasicRender(configObject);
	//SimpleShrinking(configObject);

	int* arr = new int[1000];

	int* a = new int[2];
	int* b = new int[5];
	int* c = new int[7];

	std::cout << "Allocating" << std::endl;
	std::cout << "Total amount of memory allocated: " << HeapFactory::GetDefaultHeap()->GetAmountAllocated() << std::endl;

	HeapFactory::GetDefaultHeap()->DisplayDebugInformation();

	delete v;
	v = nullptr;

	std::cout << "Deallocating" << std::endl;
	std::cout << "Total amount of memory allocated: " << HeapFactory::GetDefaultHeap()->GetAmountAllocated() << std::endl;

	timeToComplete += timer.Mark();

	std::cout << "Time to complete: " << timeToComplete << std::endl;

	//BasicRender();
	//SimpleShrinking();
	//SmoothScaling();

	return 0;
}

