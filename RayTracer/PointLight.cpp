// -----------------------------------------------------------------------

#include "PointLight.h"

// -----------------------------------------------------------------------

PointLight::PointLight()
	: Light(vec3(0.0f, 0.0f, 0.0f), 50.0f, "PointLight")
{
	Position = vec3(0.0f, 0.0f, 0.0f);

	DiffuseLight = sf::Color(180, 210, 230, 255);
	SpecularLight = sf::Color(255, 255, 255, 255);
	AmbientLight = sf::Color(150, 150, 150, 255);

	ConstantAttenuation = 1.0f;
	LinearAttenuation = 2.0f / Radius;
	QuadraticAttenuation = 1.0f / (Radius * Radius);

	m_Type = ObjectType::kePOINTLIGHT;
}

// -----------------------------------------------------------------------

PointLight::PointLight(const vec3& vPosition,
	const sf::Color& vAmbientLight,
	const sf::Color& vDiffuseLight,
	const sf::Color& vSpecularLight,
	const glm::vec3 vAttenuation,
	float fRadius,
	const std::string& name)
	: Light(vPosition, fRadius, name)
{
	DiffuseLight = vDiffuseLight;
	SpecularLight = vSpecularLight;
	AmbientLight = vAmbientLight;

	ConstantAttenuation = vAttenuation.x;
	LinearAttenuation = vAttenuation.y;
	QuadraticAttenuation = vAttenuation.z;

	m_Type = ObjectType::kePOINTLIGHT;
}

// -----------------------------------------------------------------------