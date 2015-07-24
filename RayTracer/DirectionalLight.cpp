// -----------------------------------------------------------------------

#include "DirectionalLight.h"

// -----------------------------------------------------------------------

DirectionalLight::DirectionalLight()
	: Light(vec3(1.0f, 1.0f, -1.0f), 1.0f)
{
	Direction = vec3(1.0f, 1.0f, -1.0f);

	DiffuseLight = sf::Color(180, 210, 230, 255);
	SpecularLight = sf::Color(255, 255, 255, 255);
	AmbientLight = sf::Color(25, 25, 25, 255);

	m_Type = ObjectType::keDIRECTIONALLIGHT;
}

// -----------------------------------------------------------------------

DirectionalLight::DirectionalLight(const vec3& vDir,
	const sf::Color& vAmbientLight,
	const sf::Color& vDiffuseLight,
	const sf::Color& vSpecularLight,
	float fRadius,
	const std::string& name)
	: Light(vDir, fRadius, name)
{
	Direction = vDir;

	DiffuseLight = vDiffuseLight;
	SpecularLight = vSpecularLight;
	AmbientLight = vAmbientLight;

	m_Type = ObjectType::keDIRECTIONALLIGHT;
}

// -----------------------------------------------------------------------