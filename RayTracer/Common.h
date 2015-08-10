#ifndef __COMMON_H__
#define __COMMON_H__
 
#include <time.h>

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

#include "SFML/Graphics/Color.hpp"

#include <memory>
#include <string>

#include "Constants.h"

// GLM typedefs
typedef glm::vec3 vec3;
typedef glm::vec3 Normal;
typedef glm::vec3 Point;

typedef glm::vec4 vec4;

#define rad(x) (x * glm::pi<float>()) / 180.0f
#define Random() ((float) rand() / (RAND_MAX)) + 1

// -----------------------------------------------------------------------

inline double fastPow(double a, double b) {
	union {
		double d;
		int x[2];
	} u = { a };
	u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
	u.x[0] = 0;
	return u.d;
}

// -----------------------------------------------------------------------

inline bool SolveQuadratic(const float& a,
	const float& b,
	const float& c,
	float& x0,
	float& x1)
{
	// Calculate discriminant
	float delta = b * b - 4.0f * a * c;

	if (delta < 0)
	{
		// No solution found
		return false;
	}
	else if (delta == 0.0f)
	{
		x0 = -0.5f * b / a;
		x1 = x0;
		return true;
	}
	else
	{
		float q = (b > 0.0f) ?
			-0.5f * (b + sqrt(delta)) :
			-0.5f * (b - sqrt(delta));
		x0 = q / a;
		x1 = c / q;
	}
	if (x0 > x1)
	{
		std::swap(x0, x1);
	}

	return true;
}

// -----------------------------------------------------------------------

inline float PolarizedReflection(
	float n1,              // source material's index of refraction
	float n2,              // target material's index of refraction
	float cos_a1,          // incident or outgoing ray angle cosine
	float cos_a2)		    // outgoing or incident ray angle cosine
{
	const float left = n1 * cos_a1;
	const float right = n2 * cos_a2;
	float numer = left - right;
	float denom = left + right;
	denom *= denom;     // square the denominator
	if (denom < Constants::EPS)
	{
		// Assume complete reflection.
		return 1.0f;
	}
	float reflection = (numer * numer) / denom;
	if (reflection > 1.0f)
	{
		// Clamp to actual upper limit.
		return 1.0f;
	}
	return reflection;
}

// -----------------------------------------------------------------------
// Structs define 
// -----------------------------------------------------------------------

// Material
struct Material
{
	sf::Color Ambient;
	sf::Color Emission;
	sf::Color Diffuse;
	sf::Color Specular;
	float Shininess;
	float Reflectivity;
	float Transparency;
	float RefractiveIndex;

	Material()
	{
		Ambient = sf::Color(60, 60, 60, 255);
		Emission = sf::Color(130, 50, 50, 255);
		Diffuse = sf::Color(45, 180, 210, 255);
		Specular = sf::Color(255, 255, 255, 255);
		Shininess = 128.0f;

		Reflectivity = 1.0f;
		Transparency = 0.0f;
		RefractiveIndex = 0.0f;
	}

	Material(const sf::Color& vAmbient,
		const sf::Color& vEmission,
		const sf::Color& vDiffuse,
		const sf::Color& vSpecular,
		const float fShineness,
		const float fReflectivity,
		const float fTransparency,
		const float fRefractiveIndex)
	{
		Ambient = vAmbient;
		Emission = vEmission;
		Diffuse = vDiffuse;
		Specular = vSpecular;
		Shininess = fShineness;

		Reflectivity = fReflectivity;
		Transparency = fTransparency;

		if (Transparency > 0.0f)
		{
			RefractiveIndex = fRefractiveIndex;
		}
	}
};

// -----------------------------------------------------------------------

#endif // __COMMON_H__