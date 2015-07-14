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
#define Random() ((float) rand() / (RAND_MAX)) + 1

unsigned gObjectIndex = 0;

float fEPS = 0.01f;

inline double fastPow(double a, double b) {
	union {
		double d;
		int x[2];
	} u = { a };
	u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
	u.x[0] = 0;
	return u.d;
}

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

	Color operator+=(const Color& other)
	{
		this->X = glm::clamp(this->X + other.X, 0.0f, 1.0f);
		this->Y = glm::clamp(this->Y + other.Y, 0.0f, 1.0f);
		this->Z = glm::clamp(this->Z + other.Z, 0.0f, 1.0f);
		this->W = glm::clamp(this->W + other.W, 0.0f, 1.0f);

		return *this;
	}

	float X, Y, Z, W;
};

// Material
struct Material
{
	Color Ambient;
	Color Emission;
	Color Diffuse;
	Color Specular;
	float Shininess;

	Material()
	{
		Ambient = Color(0.3f, 0.3f, 0.3f, 1.0f);
		Emission = Color(0.4f, 0.2f, 0.5f, 1.0f);
		Diffuse = Color(0.2f, 0.7f, 0.8f, 1.0f);
		Specular = Color(1.0f, 1.0f, 1.0f, 1.0f);
		Shininess = 128.0f;
	}

	Material(const Color& vAmbient,
		const Color& vEmission,
		const Color& vDiffuse,
		const Color& vSpecular,
		const float fShineness)
	{
		Ambient = vAmbient;
		Emission = vEmission;
		Diffuse = vDiffuse;
		Specular = vSpecular;
		Shininess = fShineness;
	}

	void RandomMaterial()
	{
		srand((unsigned)time(NULL));

		Ambient = Color(Random(), Random(), Random(), 1.0f);
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

	Color AmbientLight;
	Color DiffuseLight;
	Color SpecularLight;

	DirectionalLight()
	{
		Direction = vec3(1.0f, 1.0f, -1.0f);

		DiffuseLight = Color(0.7f, 0.8f, 0.9f, 1.0f); 
		SpecularLight = Color(1.0f, 1.0f, 1.0f, 1.0f);
		AmbientLight = Color(0.1f, 0.1f, 0.1f, 1.0f);
	}

	DirectionalLight(const vec3& vDir,
		const Color& vAmbientLight,
		const Color& vDiffuseLight,
		const Color& vSpecularLight)
	{
		Direction = vDir;

		DiffuseLight = vDiffuseLight;
		SpecularLight = vSpecularLight;
		AmbientLight = vAmbientLight;
	}
};

// -----------------------------------------------------------------------

// Point light
struct PointLight
{
	vec3 Position;

	Color AmbientLight;
	Color DiffuseLight;
	Color SpecularLight;

	float ConstantAttenuation;
	float LinearAttenuation;
	float QuadraticAttenuation;

	float Radius;

	PointLight()
	{
		Position = vec3(0.0f, 0.0f, 0.0f);

		DiffuseLight = Color(0.7f, 0.8f, 0.9f, 1.0f);
		SpecularLight = Color(1.0f, 1.0f, 1.0f, 1.0f);
		AmbientLight = Color(0.1f, 0.1f, 0.1f, 1.0f);

		Radius = 50.0f;

		ConstantAttenuation = 1.0f;
		LinearAttenuation = 2.0f / Radius;
		QuadraticAttenuation = 1.0f / (Radius * Radius);
	}

	PointLight(const vec3& vPosition,
		const Color& vAmbientLight,
		const Color& vDiffuseLight,
		const Color& vSpecularLight,
		const glm::vec3 vAttenuation)
	{
		Position = vPosition;

		DiffuseLight = vDiffuseLight;
		SpecularLight = vSpecularLight;
		AmbientLight = vAmbientLight;

		ConstantAttenuation = vAttenuation.x;
		LinearAttenuation = vAttenuation.y;
		QuadraticAttenuation = vAttenuation.z;
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