#pragma once
#include "Vec3.h"

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