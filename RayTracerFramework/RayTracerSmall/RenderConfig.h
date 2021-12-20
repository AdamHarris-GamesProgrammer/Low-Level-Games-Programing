#pragma once
#include "Vec3.h"
#define M_PI 3.141592653589793

struct RenderConfig {
	//Width of the frames produced
	unsigned width;
	//Height of the frames produced
	unsigned height;
	//Height of each chunk of the frame
	unsigned chunkHeight;
	//Size of each chunk. (Width * chunkHeight)
	unsigned chunkSize;
	//Size required to store each chunk in char form
	size_t charSize;
	//Size required to store each chunk in Vec3 form (chunkSize * sizeof(Vec3f) (12))
	size_t vec3Size;
	//Inverse Width (1 / width)
	float invWidth;
	//Inverse Height (1 / height)
	float invHeight;
	//Aspect Ratio (width / height)
	float aspectRatio;
	//Fov (Field of View) for the camera
	float fov;
	//Angle of the camera. (tan(pi * 0.5 * fov / 180)
	float angle;

	unsigned singularChunkSize;
	unsigned singularCharSize;

	RenderConfig(unsigned width, unsigned height, unsigned threadCount, float fov = 30) {
		this->width = width;
		this->height = height;
		this->fov = fov;
		CalculateValues(threadCount);
	}

	RenderConfig() = default;
private:
	//Buffer data. Keeps the struct 64 bit aligned 
	char buffer[16];

	//Calculates all the values of the struct based on the width and height of the frames.
	void CalculateValues(unsigned threadCount) {
		chunkHeight = height / threadCount;
		chunkSize = (width * height) / threadCount;
		invWidth = 1 / float(width);
		invHeight = 1 / float(height);
		aspectRatio = width / float(height);
		charSize = chunkSize * 3;
		vec3Size = chunkSize * sizeof(Vec3f);
		angle = tan(M_PI * 0.5 * fov / 180.0f);

		singularChunkSize = chunkSize * threadCount;
		singularCharSize = charSize * threadCount;
	}
};