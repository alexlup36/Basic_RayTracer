#ifndef __COMMON_H__
#define __COMMON_H__
 
#include <time.h>

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

#include "SFML/Graphics/Color.hpp"

#include <memory>
#include <string>

// GLM typedefs
typedef glm::vec3 vec3;
typedef glm::vec3 Normal;
typedef glm::vec3 Point;

typedef glm::vec4 vec4;

#define rad(x) (x * glm::pi<float>()) / 180.0f
#define Random() ((float) rand() / (RAND_MAX)) + 1

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

	Material()
	{
		Ambient = sf::Color(60, 60, 60, 255);
		Emission = sf::Color(130, 50, 50, 255);
		Diffuse = sf::Color(45, 180, 210, 255);
		Specular = sf::Color(255, 255, 255, 255);
		Shininess = 128.0f;

		Reflectivity = 1.0f;
		Transparency = 0.0f;
	}

	Material(const sf::Color& vAmbient,
		const sf::Color& vEmission,
		const sf::Color& vDiffuse,
		const sf::Color& vSpecular,
		const float fShineness,
		const float fReflectivity,
		const float fTransparency)
	{
		Ambient = vAmbient;
		Emission = vEmission;
		Diffuse = vDiffuse;
		Specular = vSpecular;
		Shininess = fShineness;

		Reflectivity = fReflectivity;
		Transparency = fTransparency;
	}
};

// -----------------------------------------------------------------------

#endif // __COMMON_H__