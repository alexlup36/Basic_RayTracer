#ifndef __DIRECTIONALLIGHT_H__
#define __DIRECTIONALLIGHT_H__

// -----------------------------------------------------------------------

#include "Light.h"

// -----------------------------------------------------------------------

// Directional light
class DirectionalLight : public Light
{
public:

	DirectionalLight();
	DirectionalLight(const glm::vec3& vDir,
		const sf::Color& vAmbientLight,
		const sf::Color& vDiffuseLight,
		const sf::Color& vSpecularLight,
		float fRadius,
		const std::string& name);

	inline glm::vec3 GetPosition() { return Direction; }
	inline void SetPosition(const glm::vec3& newPosition) { Direction = newPosition; }

	glm::vec3 Direction;

	sf::Color AmbientLight;
	sf::Color DiffuseLight;
	sf::Color SpecularLight;
};

// -----------------------------------------------------------------------

#endif // __DIRECTIONALLIGHT_H__