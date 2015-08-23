#ifndef __POINTLIGHT_H__
#define __POINTLIGHT_H__

// -----------------------------------------------------------------------

#include "Light.h"

// -----------------------------------------------------------------------

// Point light
class PointLight : public Light
{
public:

	PointLight();

	PointLight(const vec3& vPosition,
		const sf::Color& vAmbientLight,
		const sf::Color& vDiffuseLight,
		const sf::Color& vSpecularLight,
		const glm::vec3 vAttenuation,
		float fRadius,
		const std::string& name);

	inline glm::vec3 GetPosition() { return Position; }
	inline void SetPosition(const glm::vec3& newPosition) { Position = newPosition; }

	sf::Color AmbientLight;
	sf::Color DiffuseLight;
	sf::Color SpecularLight;

	float ConstantAttenuation;
	float LinearAttenuation;
	float QuadraticAttenuation;
};

// -----------------------------------------------------------------------

#endif // __POINTLIGHT_H__