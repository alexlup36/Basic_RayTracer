#ifndef __COMMON_H__
#define __COMMON_H__
 
#include <time.h>

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

#include <memory>

// Forward declaration
class Object;

// GLM typedefs
typedef glm::vec3 vec3;
typedef glm::vec3 Normal;
typedef glm::vec3 Point;

typedef glm::vec4 vec4;

#define rad(x) (x * glm::pi<float>()) / 180.0f
#define AmbientColor Color(0.3f, 0.2f, 0.25f, 1.0f)
#define Random() ((float) rand() / (RAND_MAX)) + 1

unsigned gObjectIndex = 0;

float fEPS = 0.01f;

inline void PrintVector(const glm::vec3& v)
{
	std::cout << "X = " << v.x << "; Y = " << v.y << "; Z = " << v.z << std::endl;
}

// -----------------------------------------------------------------------
// Structs define 
// -----------------------------------------------------------------------

struct Color
{
	Color()
		: X(0.0f), Y(0.0f), Z(0.0f), W(0.0f)
	{
	}

	Color(float x, float y, float z, float alpha)
	{
		X = glm::clamp(x, 0.0f, 1.0f);
		Y = glm::clamp(y, 0.0f, 1.0f);
		Z = glm::clamp(z, 0.0f, 1.0f);
		W = glm::clamp(alpha, 0.0f, 1.0f);
	}

	Color operator*(float scalar)
	{
		float x = glm::clamp(X * scalar, 0.0f, 1.0f);
		float y = glm::clamp(Y * scalar, 0.0f, 1.0f);
		float z = glm::clamp(Z * scalar, 0.0f, 1.0f);
		float w = glm::clamp(W * scalar, 0.0f, 1.0f);

		return Color(x, y, z, w);
	}

	Color operator*(const Color& other)
	{
		float x = glm::clamp(X * other.X, 0.0f, 1.0f);
		float y = glm::clamp(Y * other.Y, 0.0f, 1.0f);
		float z = glm::clamp(Z * other.Z, 0.0f, 1.0f);
		float w = glm::clamp(W * other.W, 0.0f, 1.0f);

		return Color(x, y, z, w);
	}

	Color operator+(const Color& other)
	{
		float x = glm::clamp(X + other.X, 0.0f, 1.0f);
		float y = glm::clamp(Y + other.Y, 0.0f, 1.0f);
		float z = glm::clamp(Z + other.Z, 0.0f, 1.0f);
		float w = glm::clamp(W + other.W, 0.0f, 1.0f);

		return Color(x, y, z, w);
	}

	float X, Y, Z, W;
};

// Material
struct Material
{
	Color Emission;
	Color Diffuse;
	Color Specular;
	float Shininess;

	Material()
	{
		Emission = Color(0.4f, 0.2f, 0.5f, 1.0f);
		Diffuse = Color(0.2f, 0.7f, 0.8f, 1.0f);
		Specular = Color(1.0f, 1.0f, 1.0f, 1.0f);
		Shininess = 128.0f;
	}

	Material(const Color& vEmission,
		const Color& vDiffuse,
		const Color& vSpecular,
		const float fShineness)
	{
		Emission = vEmission;
		Diffuse = vDiffuse;
		Specular = vSpecular;
		Shininess = fShineness;
	}

	void RandomMaterial()
	{
		srand((unsigned)time(NULL));

		Emission = Color(Random(), Random(), Random(), 1.0f);
		Diffuse = Color(Random(), Random(), Random(), 1.0f);
		Specular = Color(Random(), Random(), Random(), 1.0f);
		Shininess = glm::pow<float>(2.0f, (float)(rand() % 10));
	}
};

// -----------------------------------------------------------------------

// Directional light
struct DirectionalLight
{
	vec3 Direction;

	Color DiffuseLight;
	Color SpecularLight;

	DirectionalLight()
	{
		Direction = vec3(1.0f, 1.0f, -1.0f);

		DiffuseLight = Color(0.7f, 0.8f, 0.9f, 1.0f); 
		SpecularLight = Color(1.0f, 1.0f, 1.0f, 1.0f);
	}

	DirectionalLight(const vec3& vDir,
		const Color& vDiffuseLight,
		const Color& vSpecularLight)
	{
		Direction = vDir;

		DiffuseLight = vDiffuseLight;
		SpecularLight = vSpecularLight;
	}
};

// -----------------------------------------------------------------------

struct IntersectionInfo
{
	IntersectionInfo() 
		: RayLength(-1.0f), NormalAtIntersection(glm::vec3(0.0f)), HitObject(NULL)
	{ }
	
	IntersectionInfo(const vec3& intersectionPoint,
					float rayIntersectionLength, 
					const Normal& normal,
					Object* hitObject)
		: RayLength(rayIntersectionLength), 
		IntersectionPoint(intersectionPoint),
		NormalAtIntersection(normal), 
		HitObject(hitObject)
	{ }
	
	vec3 IntersectionPoint;
	Normal NormalAtIntersection;
	float RayLength;
	Object* HitObject;
};

#endif // __COMMON_H__