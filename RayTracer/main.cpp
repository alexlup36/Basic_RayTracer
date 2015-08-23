#include <iostream>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <limits>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "Common.h"
#include "Object.h"
#include "Camera.h"
#include "Sphere.h"
#include "Plane.h"
#include "Ray.h"
#include "Scene.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Triangle.h"
#include "Box.h"

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

#include "TGUI/TGUI.hpp"

#include "UI.h"

// ------------------------------------------------------------------------

// Multithreading
#include <thread>
#include <mutex>
#include <condition_variable>
#define MULTITHREADING

#ifdef MULTITHREADING
#include <boost/threadpool.hpp>
#endif // MULTITHREADING

#ifdef MULTITHREADING

std::unique_ptr<boost::threadpool::pool> m_ThreadPool;
typedef boost::function<void()> Task;
std::vector<Task> ImageProcessingTaskList;

unsigned int m_iThreadCount = 24;

void SetupMultithread();

#endif // MULTITHREADING

// ------------------------------------------------------------------------
// Window
const unsigned int iWidth = 640;
const unsigned int iHeight = 480;
const unsigned int iImageSize = iWidth * iHeight;
const unsigned int iColor = 24;
bool bGUIMode;

// ------------------------------------------------------------------------
// Modifiable values from the UI
float moveSpeed = 1.0f;
unsigned int MAX_REFLECTION_DEPTH = 5;
unsigned int MAX_REFRACTION_DEPTH = 5;

int SquareLength = 5;
int SampleCount = 10;
float SampleDistance = 1.0f / SampleCount;

const float AmbientRefractiveIndex = 1.0003f;

bool UpdateRequired = true;
bool Realtime = false;
bool ShadowsEnabled = false;
bool SoftShadowsEnabled = false;
bool SuperSamplingEnabled = false;
bool PlaneTexturingEnabled = false;
bool ReflectionEnabled = false;
bool RefractionEnabled = false;

UI::LightingModel eLightModel = UI::LightingModel::BlinnPhong;

// ------------------------------------------------------------------------

sf::RenderWindow window(sf::VideoMode(iWidth, iHeight, iColor), "RayTracer"/*, sf::Style::Fullscreen*/);
sf::Vector2i windowPosition = window.getPosition();
sf::Vector2i screenCenter(static_cast<int>(windowPosition.x + iWidth * 0.5f), 
	static_cast<int>(windowPosition.y + iHeight * 0.5f));

// -----------------------------------------------------------------------------

Scene scene;
std::shared_ptr<Camera> pCam;
sf::Uint8* pixels = new sf::Uint8[iWidth * iHeight * 4];

// -----------------------------------------------------------------------------
// Forward declarations

void Draw(int iStartLineIndex, int iEndLineIndex);
void Render(int iStartLineIndex, int iEndLineIndex);
void Update(float dt);
void UpdateInput(glm::vec3& moveVector);

IntersectionInfo RaySceneIntersection(const Ray& ray, Scene& scene);
sf::Color FindColor(const IntersectionInfo& intersect, const Material& hitObjectMaterial, Scene& scene, float fShade, float fSoftShade);
void CalculateSquareCoord(int intersectionX, int intersectionZ, int& coordX, int& coordZ);

// -----------------------------------------------------------------------------

sf::Color WhiteColor = sf::Color(255, 255, 255, 255);
sf::Color BlackColor = sf::Color(0, 0, 0, 255);
sf::Color RedColor = sf::Color(255, 0, 0, 255);
sf::Color GreenColor = sf::Color(0, 255, 0, 255);
sf::Color BlueColor = sf::Color(0, 0, 255, 255);

// -----------------------------------------------------------------------------

float mix(const float& t1, const float& t2, const float& mix)
{
	return t2 * mix + t1 * (1.0f - mix);
}

// -----------------------------------------------------------------------------

void SetPixelColor(int iCurrentPixel, const sf::Color color)
{
	pixels[iCurrentPixel]		= color.r;
	pixels[iCurrentPixel + 1]	= color.g;
	pixels[iCurrentPixel + 2]	= color.b;
	pixels[iCurrentPixel + 3]	= color.a;
}

// -----------------------------------------------------------------------------

sf::Color PhongLighting(DirectionalLight& currentLight, 
	const Material& material,
	const glm::vec3& position,
	const glm::vec3& normal,
	float fShade)
{
	sf::Color ambientComponent, diffuseComponent, specularComponent;

	// Ambient component
	ambientComponent = currentLight.AmbientLight * material.Ambient;

	if (fShade == 0.0f)
	{
		// Return the final color
		return ambientComponent;
	}
	else
	{
		// Diffuse component
		glm::vec3 lightDirection = glm::normalize(currentLight.Direction);
		float fNormalDotLight = glm::max(glm::dot(normal, lightDirection), 0.0f);
		sf::Color diffuseResult = material.Diffuse * currentLight.DiffuseLight;
		diffuseComponent = sf::Color((sf::Uint8)(diffuseResult.r * fNormalDotLight * fShade),
			(sf::Uint8)(diffuseResult.g * fNormalDotLight * fShade),
			(sf::Uint8)(diffuseResult.b * fNormalDotLight * fShade));

		// Specular component
		vec3& viewDirection = glm::normalize(pCam->GetCameraPosition() - position);
		vec3& reflectionDirection = glm::reflect<vec3>(-lightDirection, normal);
		float specular = std::pow(std::max(glm::dot(viewDirection, reflectionDirection), 0.0f), material.Shininess) * fShade;
		sf::Color specularResult = material.Specular * currentLight.SpecularLight;
		specularComponent = sf::Color((sf::Uint8)(specularResult.r * specular),
			(sf::Uint8)(specularResult.g * specular),
			(sf::Uint8)(specularResult.b * specular));

		// Return the final color
		return ambientComponent + diffuseComponent + specularComponent;
	}
}

// -----------------------------------------------------------------------------

sf::Color PhongLighting(PointLight& currentLight,
	const Material& material,
	const glm::vec3& position,
	const glm::vec3& normal,
	float fShade)
{
	sf::Color ambientComponent, diffuseComponent, specularComponent;

	// Ambient component
	ambientComponent = currentLight.AmbientLight * material.Ambient;

	if (fShade == 0.0f)
	{
		// Return the final color
		return ambientComponent;
	}
	else
	{
		// Diffuse component

		// Calculate the light vector
		glm::vec3 lightVector = currentLight.Position - position;
		// Calculate the distance from the point light to the pixel position
		float distance = glm::length(lightVector);
		//// Calculate the attenuation factor
		//float attenuation = 1.0f / (currentLight.ConstantAttenuation +
		//	currentLight.LinearAttenuation * distance +
		//	currentLight.QuadraticAttenuation * (distance * distance));
		glm::vec3 lightDirection = glm::normalize(lightVector);
		float fNormalDotLight = glm::dot(normal, lightDirection);
		if (fNormalDotLight > 0.0f)
		{
			sf::Color diffuseResult = material.Diffuse * currentLight.DiffuseLight;

			diffuseComponent = sf::Color((sf::Uint8)(diffuseResult.r * fNormalDotLight * fShade),
				(sf::Uint8)(diffuseResult.g * fNormalDotLight * fShade),
				(sf::Uint8)(diffuseResult.b * fNormalDotLight * fShade));
		}

		// Specular component
		vec3& viewDirection = glm::normalize(pCam->GetCameraPosition() - position);
		vec3& reflectionDirection = glm::reflect<vec3>(-lightDirection, normal);
		float specular = std::pow(std::max(glm::dot(viewDirection, reflectionDirection), 0.0f), material.Shininess) * fShade;
		sf::Color specularResult = material.Specular * currentLight.SpecularLight;
		specularComponent = sf::Color((sf::Uint8)(specularResult.r * specular),
			(sf::Uint8)(specularResult.g * specular),
			(sf::Uint8)(specularResult.b * specular));

		// Return the final color
		return (ambientComponent + diffuseComponent + specularComponent);// *attenuation;
	}
}

// -----------------------------------------------------------------------------

sf::Color PhongLighting(AreaLight& currentLight,
	const Material& material,
	const glm::vec3& position,
	const glm::vec3& normal,
	float fShade)
{
	float rAcc = 0.0;
	float gAcc = 0.0f;
	float bAcc = 0.0f;
	float aAcc = 0.0f;

	// Ambient component
	sf::Color ambientComponent = currentLight.AmbientLight * material.Ambient;

	if (fShade == 0.0f)
	{
		// Return the final color
		return ambientComponent;
	}
	else
	{
		unsigned int sampleCountX = currentLight.GetSampleCountX();
		unsigned int sampleCountZ = currentLight.GetSampleCountZ();
		float sampleSizeX = currentLight.GetSampleSizeX();
		float sampleSizeZ = currentLight.GetSampleSizeZ();

		for (unsigned int row = 0; row < sampleCountZ; row++)
		{
			for (unsigned int col = 0; col < sampleCountX; col++)
			{
				// Find the current position for the current sample rectangle
				float currentX = currentLight.GetLowerLayerPosition().x + col * sampleSizeX;
				float currentZ = currentLight.GetLowerLayerPosition().z + row * sampleSizeZ;

				// Generate a random offset within the current sample square
				float xOffset = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / sampleSizeX));
				float zOffset = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / sampleSizeZ));

				// Calculate the position of the next sample
				glm::vec3 currentSamplePoint = glm::vec3(currentX + xOffset,
					currentLight.GetLowerLayerPosition().y - Constants::EPS,
					currentZ + zOffset);

				sf::Color diffuseComponent, specularComponent;

				// Diffuse component

				// Calculate the light vector
				glm::vec3 lightVector = currentSamplePoint - position;
				// Calculate the distance from the point light to the pixel position
				float distance = glm::length(lightVector);

				glm::vec3 lightDirection = glm::normalize(lightVector);
				float fNormalDotLight = glm::dot(normal, lightDirection);
				if (fNormalDotLight > 0.0f)
				{
					sf::Color diffuseResult = material.Diffuse * currentLight.DiffuseLight;

					diffuseComponent = sf::Color((sf::Uint8)(diffuseResult.r * fNormalDotLight * fShade),
						(sf::Uint8)(diffuseResult.g * fNormalDotLight * fShade),
						(sf::Uint8)(diffuseResult.b * fNormalDotLight * fShade));
				}

				// Specular component
				vec3& viewDirection = glm::normalize(pCam->GetCameraPosition() - position);
				vec3& reflectionDirection = glm::reflect<vec3>(-lightDirection, normal);
				float specular = std::pow(std::max(glm::dot(viewDirection, reflectionDirection), 0.0f), material.Shininess) * fShade;
				sf::Color specularResult = material.Specular * currentLight.SpecularLight;
				specularComponent = sf::Color((sf::Uint8)(specularResult.r * specular),
					(sf::Uint8)(specularResult.g * specular),
					(sf::Uint8)(specularResult.b * specular));

				rAcc += (float)(ambientComponent.r + diffuseComponent.r + specularComponent.r);
				gAcc += (float)(ambientComponent.g + diffuseComponent.g + specularComponent.g);
				bAcc += (float)(ambientComponent.b + diffuseComponent.b + specularComponent.b);
				aAcc += (float)(ambientComponent.a + diffuseComponent.a + specularComponent.a);
			}
		}

		float r = glm::clamp(rAcc / (sampleCountX * sampleCountZ), 0.0f, 255.0f);
		float g = glm::clamp(gAcc / (sampleCountX * sampleCountZ), 0.0f, 255.0f);
		float b = glm::clamp(bAcc / (sampleCountX * sampleCountZ), 0.0f, 255.0f);
		float a = glm::clamp(aAcc / (sampleCountX * sampleCountZ), 0.0f, 255.0f);

		// Return the final color
		return sf::Color((sf::Uint8)r,
			(sf::Uint8)g,
			(sf::Uint8)b,
			(sf::Uint8)a);
	}
}

// -----------------------------------------------------------------------------

sf::Color BlinnPhongLighting(DirectionalLight& currentLight,
	const Material& material,
	const glm::vec3& position,
	const glm::vec3& normal,
	float fShade)
{
	sf::Color ambientComponent, diffuseComponent, specularComponent;

	// Ambient component
	ambientComponent = currentLight.AmbientLight * material.Ambient;

	if (fShade == 0.0f)
	{
		// Return the final color
		return ambientComponent;
	}
	else
	{
		// Diffuse component
		glm::vec3 lightDirection = glm::normalize(currentLight.Direction);
		float fNormalDotLight = glm::max(glm::dot(normal, lightDirection), 0.0f);
		sf::Color diffuseResult = material.Diffuse * currentLight.DiffuseLight;
		diffuseComponent = sf::Color((sf::Uint8)(diffuseResult.r * fNormalDotLight * fShade),
			(sf::Uint8)(diffuseResult.g * fNormalDotLight * fShade),
			(sf::Uint8)(diffuseResult.b * fNormalDotLight * fShade));

		// Specular component
		vec3& viewDirection = glm::normalize(pCam->GetCameraPosition() - position);
		vec3& halfVector = glm::normalize(lightDirection + viewDirection);
		float specular = std::pow(std::max(glm::dot(normal, halfVector), 0.0f), material.Shininess) * fShade;
		sf::Color specularResult = material.Specular * currentLight.SpecularLight;
		specularComponent = sf::Color((sf::Uint8)(specularResult.r * specular),
			(sf::Uint8)(specularResult.g * specular),
			(sf::Uint8)(specularResult.b * specular));

		// Return the final color
		return ambientComponent + diffuseComponent + specularComponent;
	}
}

// -----------------------------------------------------------------------------

sf::Color BlinnPhongLighting(PointLight& currentLight,
	const Material& material,
	const glm::vec3& position,
	const glm::vec3& normal,
	float fShade)
{
	sf::Color ambientComponent, diffuseComponent, specularComponent;

	// Ambient component
	ambientComponent = currentLight.AmbientLight * material.Ambient;

	if (fShade == 0.0f)
	{
		// Return the final color
		return ambientComponent;
	}
	else
	{
		// Diffuse component
		glm::vec3 lightVector = currentLight.Position - position;
		// Calculate the distance from the point light to the pixel position
		float distance = glm::length(lightVector);
		// Calculate the attenuation factor
		//float attenuation = 1.0f / (currentLight.ConstantAttenuation +
		//	currentLight.LinearAttenuation * distance +
		//	currentLight.QuadraticAttenuation * (distance * distance));
		glm::vec3 lightDirection = glm::normalize(lightVector);
		float fNormalDotLight = glm::max(glm::dot(normal, lightDirection), 0.0f);
		sf::Color diffuseResult = material.Diffuse * currentLight.DiffuseLight;
		diffuseComponent = sf::Color((sf::Uint8)(diffuseResult.r * fNormalDotLight * fShade),
			(sf::Uint8)(diffuseResult.g * fNormalDotLight * fShade),
			(sf::Uint8)(diffuseResult.b * fNormalDotLight * fShade));

		// Specular component
		vec3& viewDirection = glm::normalize(pCam->GetCameraPosition() - position);
		vec3& halfVector = glm::normalize(lightDirection + viewDirection);
		float specular = std::pow(std::max(glm::dot(normal, halfVector), 0.0f), material.Shininess) * fShade;
		sf::Color specularResult = material.Specular * currentLight.SpecularLight;
		specularComponent = sf::Color((sf::Uint8)(specularResult.r * specular),
			(sf::Uint8)(specularResult.g * specular),
			(sf::Uint8)(specularResult.b * specular));

		// Return the final color
		return (ambientComponent + diffuseComponent + specularComponent);// *attenuation;
	}
}

// -----------------------------------------------------------------------------

sf::Color BlinnPhongLighting(AreaLight& currentLight,
	const Material& material,
	const glm::vec3& position,
	const glm::vec3& normal,
	float fShade)
{
	float rAcc = 0.0;
	float gAcc = 0.0f;
	float bAcc = 0.0f;
	float aAcc = 0.0f;

	// Ambient component
	sf::Color ambientComponent = currentLight.AmbientLight * material.Ambient;

	if (fShade == 0.0f)
	{
		// Return the final color
		return ambientComponent;
	}
	else
	{
		unsigned int sampleCountX = currentLight.GetSampleCountX();
		unsigned int sampleCountZ = currentLight.GetSampleCountZ();
		float sampleSizeX = currentLight.GetSampleSizeX();
		float sampleSizeZ = currentLight.GetSampleSizeZ();

		for (unsigned int row = 0; row < sampleCountZ; row++)
		{
			for (unsigned int col = 0; col < sampleCountX; col++)
			{
				// Find the current position for the current sample rectangle
				float currentX = currentLight.GetLowerLayerPosition().x + col * sampleSizeX;
				float currentZ = currentLight.GetLowerLayerPosition().z + row * sampleSizeZ;

				// Generate a random offset within the current sample square
				float xOffset = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / sampleSizeX));
				float zOffset = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / sampleSizeZ));

				// Calculate the position of the next sample
				glm::vec3 currentSamplePoint = glm::vec3(currentX + xOffset,
					currentLight.GetLowerLayerPosition().y - Constants::EPS,
					currentZ + zOffset);

				sf::Color diffuseComponent, specularComponent;

				// Diffuse component
				glm::vec3 lightVector = currentSamplePoint - position;
				// Calculate the distance from the point light to the pixel position
				float distance = glm::length(lightVector);
				glm::vec3 lightDirection = glm::normalize(lightVector);
				float fNormalDotLight = glm::max(glm::dot(normal, lightDirection), 0.0f);
				sf::Color diffuseResult = material.Diffuse * currentLight.DiffuseLight;
				diffuseComponent = sf::Color((sf::Uint8)(diffuseResult.r * fNormalDotLight * fShade),
					(sf::Uint8)(diffuseResult.g * fNormalDotLight * fShade),
					(sf::Uint8)(diffuseResult.b * fNormalDotLight * fShade));

				// Specular component
				vec3& viewDirection = glm::normalize(pCam->GetCameraPosition() - position);
				vec3& halfVector = glm::normalize(lightDirection + viewDirection);
				float specular = std::pow(std::max(glm::dot(normal, halfVector), 0.0f), material.Shininess) * fShade;
				sf::Color specularResult = material.Specular * currentLight.SpecularLight;
				specularComponent = sf::Color((sf::Uint8)(specularResult.r * specular),
					(sf::Uint8)(specularResult.g * specular),
					(sf::Uint8)(specularResult.b * specular));

				rAcc += (float)(ambientComponent.r + diffuseComponent.r + specularComponent.r);
				gAcc += (float)(ambientComponent.g + diffuseComponent.g + specularComponent.g);
				bAcc += (float)(ambientComponent.b + diffuseComponent.b + specularComponent.b);
				aAcc += (float)(ambientComponent.a + diffuseComponent.a + specularComponent.a);
			}
		}

		float r = glm::clamp(rAcc / (sampleCountX * sampleCountZ), 0.0f, 255.0f);
		float g = glm::clamp(gAcc / (sampleCountX * sampleCountZ), 0.0f, 255.0f);
		float b = glm::clamp(bAcc / (sampleCountX * sampleCountZ), 0.0f, 255.0f);
		float a = glm::clamp(aAcc / (sampleCountX * sampleCountZ), 0.0f, 255.0f);

		// Return the final color
		return sf::Color((sf::Uint8)r,
			(sf::Uint8)g,
			(sf::Uint8)b,
			(sf::Uint8)a);
	}
}

// -----------------------------------------------------------------------------

void CalculateSquareCoord(int intersectionX, int intersectionZ, int& coordX, int& coordZ)
{
	int xDivSq = intersectionX / SquareLength;
	int xModSq = intersectionX % SquareLength;

	int zDivSq = intersectionZ / SquareLength;
	int zModSq = intersectionZ % SquareLength;

	float extraX = (float)xModSq / SquareLength;
	float extraZ = (float)zModSq / SquareLength;

	coordX = xDivSq;
	coordZ = zDivSq;

	if (intersectionX < 0.0f)
	{
		// Left half
		if (intersectionZ > 0.0f)
		{
			// Top left quadrant
			if (extraX < 0.0f)
			{
				coordX--;
			}
		}
		else
		{
			// Bottom left quadrant
			if (extraX < 0.0f)
			{
				coordX--;
			}
			if (extraZ < 0.0f)
			{
				coordZ--;
			}
		}
	}
	else
	{
		// Right half
		if (intersectionZ > 0.0f)
		{
			// Top right quadrant
		}
		else
		{
			// Bottom right quadrant
			if (extraZ < 0.0f)
			{
				coordZ--;
			}
		}
	}
}

// -----------------------------------------------------------------------------

void Trace(const Ray& ray, 
	sf::Color& colorAccumulator, 
	Scene& scene, 
	unsigned int iReflectionDepth,
	unsigned int iRefractionDepth,
	float fRefractiveIndex)
{
	// Calculate intersection
	IntersectionInfo intersect = RaySceneIntersection(ray, scene);

	if (intersect.HitObject != NULL)
	{
		// --------------------------------------------------------------------
		// Light source rendering

		// Check if we hit a light source
		if (intersect.HitObject->Type() == ObjectType::keDIRECTIONALLIGHT ||
			intersect.HitObject->Type() == ObjectType::kePOINTLIGHT ||
			intersect.HitObject->Type() == ObjectType::keAREALIGHT)
		{
			colorAccumulator = sf::Color(255, 255, 255, 255);
			return;
		}

		// --------------------------------------------------------------------
		// Get the material of the hit object

		Material hitObjectMaterial = intersect.HitObject->GetMaterial();

		// --------------------------------------------------------------------
		// Procedural plane texturing

		// Object type plane hit => square pattern texturing
		if (PlaneTexturingEnabled == true)
		{
			if (intersect.HitObject->Type() == ObjectType::kePLANE)
			{
				// Get the intersection point between the ray and the plane
				int intersectionX = (int)floor(intersect.IntersectionPoint.x);
				int intersectionZ = (int)floor(intersect.IntersectionPoint.z);

				int xSquareCoordinate = 0;
				int ySquareCoordinate = 0;

				// Calculate the coordinates of the square where the intersection occurred
				CalculateSquareCoord(intersectionX, intersectionZ, xSquareCoordinate, ySquareCoordinate);

				if ((abs(xSquareCoordinate) + abs(ySquareCoordinate)) % 2 == 0)
				{
					hitObjectMaterial.Diffuse = sf::Color(0, 0, 0, 255);
				}
				else
				{
					hitObjectMaterial.Diffuse = sf::Color(255, 255, 255, 255);
				}
			}
		}

		// --------------------------------------------------------------------
		// Shadows

		float fShade = 1.0f;
		float fSoftShade = 0.0f;

		if (SoftShadowsEnabled == true)
		{
			// Get the list of area lights in the scene
			std::vector<AreaLight*>& areaLightSources = scene.AreaLightList();

			// Go through all the area lights in the scene
			for (unsigned int lightIndex = 0; lightIndex < areaLightSources.size(); lightIndex++)
			{
				AreaLight& currentAreaLight = *areaLightSources[lightIndex];

				// Check the area light source
				unsigned int sampleCountX = currentAreaLight.GetSampleCountX();
				unsigned int sampleCountZ = currentAreaLight.GetSampleCountZ();
				float sampleSizeX = currentAreaLight.GetSampleSizeX();
				float sampleSizeZ = currentAreaLight.GetSampleSizeZ();

				IntersectionInfo newIntersect = intersect;

				for (unsigned int row = 0; row < sampleCountZ; row++)
				{
					for (unsigned int col = 0; col < sampleCountX; col++)
					{
						// Find the current position for the current sample rectangle
						float currentX = currentAreaLight.GetLowerLayerPosition().x + col * sampleSizeX;
						float currentZ = currentAreaLight.GetLowerLayerPosition().z + row * sampleSizeZ;

						// Generate a random offset within the current sample square
						float xOffset = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / sampleSizeX));
						float zOffset = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / sampleSizeZ));

						// Calculate the position of the next sample
						glm::vec3 currentSamplePoint = glm::vec3(currentX + xOffset,
							currentAreaLight.GetLowerLayerPosition().y - Constants::EPS,
							currentZ + zOffset);

						// Calculate the direction to the intersection point
						glm::vec3 shadowVector = currentSamplePoint - newIntersect.IntersectionPoint;
						float distance = glm::length(shadowVector);
						glm::vec3 shadowVectorDirection = glm::normalize(shadowVector);
						glm::vec3 test = shadowVectorDirection * Constants::EPS;
						glm::vec3 startPoint = newIntersect.IntersectionPoint + test;
						Ray shadowRay(startPoint, shadowVectorDirection);

						IntersectionInfo intersect = RaySceneIntersection(shadowRay, scene);
						if (intersect.HitObject != NULL)
						{
							if (intersect.HitObject->Type() == ObjectType::keAREALIGHT)
							{
								fSoftShade += currentAreaLight.GetSampleScale();
							}
						}
					}
				}
			}
		}

		if (ShadowsEnabled == true)
		{
			if (intersect.HitObject != NULL)
			{
				// Calculate the shadow ray for each light source in the scene
				std::vector<DirectionalLight*>& dirLightSources = scene.DirectionalLightList();

				// Go through all directional light sources and calculate the shadow rays
				for (unsigned int lightIndex = 0; lightIndex < dirLightSources.size(); lightIndex++)
				{
					// Get the current light source
					DirectionalLight& currentLight = *dirLightSources[lightIndex];

					// Calculate the intersection of the reflected ray
					glm::vec3 lightDirection = glm::normalize(currentLight.Direction);
					glm::vec3 startPoint = intersect.IntersectionPoint + lightDirection * Constants::EPS;

					Ray shadowRay(startPoint, lightDirection);

					// Get the object list
					std::vector<Object*>& objectList = scene.ObjectList();

					// Check all objects in the scene for intersection against the shadow ray
					for (Object* obj : objectList)
					{
						// Don't check for intersections against light sources
						if (obj->Type() == ObjectType::kePOINTLIGHT ||
							obj->Type() == ObjectType::keDIRECTIONALLIGHT ||
							obj->Type() == ObjectType::keAREALIGHT)
						{
							continue;
						}

						if (obj->GetIndex() != intersect.HitObject->GetIndex())
						{
							IntersectionInfo intersection = obj->FindIntersection(shadowRay);
							if (intersection.HitObject != NULL)
							{
								if (intersection.HitObject->Type() != ObjectType::kePOINTLIGHT &&
									intersection.HitObject->Type() != ObjectType::keDIRECTIONALLIGHT &&
									intersection.HitObject->Type() != ObjectType::keAREALIGHT)
								{
									fShade = 0.0f;
									break;
								}
							}
						}
					}
				}

				std::vector<PointLight*>& pointLightSources = scene.PointLightList();

				// Go through all the point lights in the scene
				for (unsigned int lightIndex = 0; lightIndex < pointLightSources.size(); lightIndex++)
				{
					// Get the current light source
					PointLight& currentLight = *pointLightSources[lightIndex];

					// Calculate the intersection of the reflected ray
					glm::vec3 lightVector = currentLight.Position - intersect.IntersectionPoint;
					float distance = glm::length(lightVector);
					glm::vec3 lightDirection = glm::normalize(lightVector);
					glm::vec3 startPoint = intersect.IntersectionPoint + lightDirection * Constants::EPS;
					Ray shadowRay(startPoint, lightDirection);

					// Get the object list
					std::vector<Object*>& objectList = scene.ObjectList();

					// Check all objects in the scene for intersection against the shadow ray
					for (Object* obj : objectList)
					{
						// Don't check for intersections against light sources
						if (obj->Type() == ObjectType::kePOINTLIGHT ||
							obj->Type() == ObjectType::keDIRECTIONALLIGHT ||
							obj->Type() == ObjectType::keAREALIGHT)
						{
							continue;
						}

						if (obj->GetIndex() != intersect.HitObject->GetIndex())
						{
							IntersectionInfo intersection = obj->FindIntersection(shadowRay);
							if (intersection.RayLength <= distance && intersection.HitObject != NULL)
							{
								if (intersection.HitObject->Type() != ObjectType::kePOINTLIGHT &&
									intersection.HitObject->Type() != ObjectType::keDIRECTIONALLIGHT &&
									intersection.HitObject->Type() != ObjectType::keAREALIGHT)
								{
									fShade = 0.0f;
									break;
								}
							}
						}
					}
				}
			}
		}

		// --------------------------------------------------------------------
		// Shading model

		// Calculate the color of the object based on the shading model
		colorAccumulator += FindColor(intersect, hitObjectMaterial, scene, fShade, fSoftShade);

		// --------------------------------------------------------------------
		// Refraction

		// Calculate the direction of the refracted ray
		glm::vec3 refractedDirection = glm::vec3(0.0f);
		
		// Reflection factor
		float fReflectionFactor = 0.0f;

		if (RefractionEnabled == true)
		{
			glm::vec3 direction = glm::normalize(ray.GetDirection());
			float cos_a1 = glm::dot(direction, intersect.NormalAtIntersection);
			float sin_a1 = 0.0f;

			if (cos_a1 <= -1.0f)
			{
				if (cos_a1 < -1.0001f)
				{
					std::cout << "Dot product too small." << std::endl;
				}
				cos_a1 = -1.0f;
				sin_a1 = 0.0f;
			}
			else if (cos_a1 >= 1.0f)
			{
				if (cos_a1 > 1.0001f)
				{
					std::cout << "Dot product too large." << std::endl;
				}
				cos_a1 = 1.0f;
				sin_a1 = 0.0f;
			}
			else
			{
				sin_a1 = sqrt(1.0f - cos_a1 * cos_a1);
			}

			// Calculate the ratio of the two refractive indices
			const float ratio = fRefractiveIndex / hitObjectMaterial.RefractiveIndex;

			// Use Snell's law to calculate the sine of the refracted ray and normal
			const float sin_a2 = ratio * sin_a1;

			if (sin_a2 <= -1.0f || sin_a2 >= 1.0f)
			{
				// There is no refraction, only reflection
				fReflectionFactor = 1.0f;
			}
			else
			{
				// Solve quadratic for k
				float x1, x2;

				float a = 1.0f;
				float b = 2.0f * cos_a1;
				float c = 1.0f - 1.0f / (ratio * ratio);

				float maxAlignment = -0.0001f;

				if (SolveQuadratic(a, b, c, x1, x2) == true)
				{
					// Solution was found => find the correct one and exclude the ghost one

					// ---------------------------------------------------------------------
					// Calculate the direction of the refracted ray using the first solution

					// Calculate the candidate for the refractive ray direction
					glm::vec3 refractCandidate = direction + x1 * intersect.NormalAtIntersection;

					// Calculate the angle between the incident and refracted ray
					float alignment = glm::dot(direction, refractCandidate);
					if (alignment > maxAlignment)
					{
						maxAlignment = alignment;
						refractedDirection = refractCandidate;
					}

					// ---------------------------------------------------------------------
					// Calculate the direction of the refracted ray using the second solution

					refractCandidate = direction + x2 * intersect.NormalAtIntersection;
					alignment = glm::dot(direction, refractCandidate);
					if (alignment > maxAlignment)
					{
						maxAlignment = alignment;
						refractedDirection = refractCandidate;
					}

					// ---------------------------------------------------------------------
				}

				if (maxAlignment <= 0.0f)
				{
					std::cout << "Invalid value for max alignment." << std::endl;
				}

				// Determine the cosine of the refracted ray and normal
				float cos_a2 = sqrt(1.0f - sin_a2 * sin_a2);
				if (cos_a1 < 0.0f)
				{
					// The polarity of cos_a1 must match the polarity of cos_a2
					cos_a2 = -cos_a2;
				}

				// Determine the fraction of the light which is being reflected
				float sPolarized = PolarizedReflection(fRefractiveIndex, hitObjectMaterial.RefractiveIndex, cos_a1, cos_a2);
				float pPolarized = PolarizedReflection(fRefractiveIndex, hitObjectMaterial.RefractiveIndex, cos_a2, cos_a1);
				fReflectionFactor = (sPolarized + pPolarized) * 0.5f;
			}
		}

		// --------------------------------------------------------------------
		// Reflection

		if (ReflectionEnabled == true)
		{
			// If the hit object is reflective or transparent and we
			// haven't reached max reflection depth
			if (hitObjectMaterial.Reflectivity > 0)
			{
				// Go through all the point lights in the scene

				// Calculate the reflected ray
				vec3 reflectionDirection = glm::normalize(glm::reflect<vec3>(ray.GetDirection(), intersect.NormalAtIntersection));

				// Calculate the intersection of the reflected ray
				glm::vec3 startPoint = intersect.IntersectionPoint + reflectionDirection * Constants::EPS;
				Ray reflectionRay(startPoint, reflectionDirection);

				if (iReflectionDepth < MAX_REFLECTION_DEPTH)
				{
					sf::Color reflectionColor = sf::Color(0, 0, 0, 255);
					Trace(reflectionRay,
						reflectionColor,
						scene,
						iReflectionDepth + 1,
						iRefractionDepth + 1,
						AmbientRefractiveIndex);

					if (RefractionEnabled == true)
					{
						// Reflection factor calculated using Snell
						colorAccumulator += sf::Color((sf::Uint8)(reflectionColor.r * hitObjectMaterial.Reflectivity * fReflectionFactor),
							(sf::Uint8)(reflectionColor.g * hitObjectMaterial.Reflectivity * fReflectionFactor),
							(sf::Uint8)(reflectionColor.b * hitObjectMaterial.Reflectivity * fReflectionFactor),
							255);
					}
					else
					{
						// Use the object's material reflectiveness
						colorAccumulator += sf::Color((sf::Uint8)(reflectionColor.r * hitObjectMaterial.Reflectivity),
							(sf::Uint8)(reflectionColor.g * hitObjectMaterial.Reflectivity),
							(sf::Uint8)(reflectionColor.b * hitObjectMaterial.Reflectivity),
							255);
					}
				}
			}
		}
		
		// --------------------------------------------------------------------

		if (RefractionEnabled == true)
		{
			if (hitObjectMaterial.Transparency > 0)
			{
				// Calculate the refracted ray
				refractedDirection = glm::normalize(refractedDirection);

				// Calculate the intersection of the refracted ray
				glm::vec3 startPoint = intersect.IntersectionPoint + refractedDirection * Constants::EPS;
				Ray refractionRay(startPoint, refractedDirection);

				if (iRefractionDepth < MAX_REFRACTION_DEPTH)
				{
					sf::Color refractionColor = sf::Color(0, 0, 0, 255);
					Trace(refractionRay,
						refractionColor,
						scene,
						iReflectionDepth + 1,
						iRefractionDepth + 1,
						AmbientRefractiveIndex);

					colorAccumulator += sf::Color((sf::Uint8)(refractionColor.r * hitObjectMaterial.Transparency * (1.0f - fReflectionFactor)),
						(sf::Uint8)(refractionColor.g * hitObjectMaterial.Transparency * (1.0f - fReflectionFactor)),
						(sf::Uint8)(refractionColor.b * hitObjectMaterial.Transparency * (1.0f - fReflectionFactor)),
						255);
				}
			}
		}

		// --------------------------------------------------------------------
	}
}

// -----------------------------------------------------------------------------

sf::Color FindColor(const IntersectionInfo& intersect, 
	const Material& hitObjectMaterial,
	Scene& scene,
	float fShade,
	float fSoftShade)
{
	// ---------------------------------------------------------------------------

	sf::Color finalColor;

	// ---------------------------------------------------------------------------

	std::vector<DirectionalLight*>& dirLightSources = scene.DirectionalLightList();
	std::vector<PointLight*>& pointLightSources = scene.PointLightList();
	std::vector<AreaLight*>& areaLightSources = scene.AreaLightList();

	// ---------------------------------------------------------------------------

	// Go through all the area lights in the scene
	for (unsigned int lightIndex = 0; lightIndex < areaLightSources.size(); lightIndex++)
	{
		AreaLight& currentAreaLight = *areaLightSources[lightIndex];

		if (eLightModel == UI::LightingModel::Phong)
		{
			// Compute the final color
			finalColor += PhongLighting(currentAreaLight,
				hitObjectMaterial,
				intersect.IntersectionPoint,
				intersect.NormalAtIntersection,
				fSoftShade);
		}
		else if (eLightModel == UI::LightingModel::BlinnPhong)
		{
			// Compute the final color
			finalColor += BlinnPhongLighting(currentAreaLight,
				hitObjectMaterial,
				intersect.IntersectionPoint,
				intersect.NormalAtIntersection,
				fSoftShade);
		}
	}

	// ---------------------------------------------------------------------------
	
	// Go through all directional light sources in the scene
	for (unsigned int lightIndex = 0; lightIndex < dirLightSources.size(); lightIndex++)
	{
		// Get the current light source
		DirectionalLight& currentLight = *dirLightSources[lightIndex];

		if (eLightModel == UI::LightingModel::Phong)
		{
			// Compute the final color
			finalColor += PhongLighting(currentLight,
				hitObjectMaterial,
				intersect.IntersectionPoint,
				intersect.NormalAtIntersection,
				fShade);
		}
		else if (eLightModel == UI::LightingModel::BlinnPhong)
		{
			// Compute the final color
			finalColor += BlinnPhongLighting(currentLight,
				hitObjectMaterial,
				intersect.IntersectionPoint,
				intersect.NormalAtIntersection,
				fShade);
		}
	}

	// ---------------------------------------------------------------------------

	// Go through all the point lights in the scene
	for (unsigned int lightIndex = 0; lightIndex < pointLightSources.size(); lightIndex++)
	{
		// Get the current light source
		PointLight& currentLight = *pointLightSources[lightIndex];

		if (eLightModel == UI::LightingModel::Phong)
		{
			// Compute the final color
			finalColor += PhongLighting(currentLight,
				hitObjectMaterial,
				intersect.IntersectionPoint,
				intersect.NormalAtIntersection,
				fShade);
		}
		else if (eLightModel == UI::LightingModel::BlinnPhong)
		{
			// Compute the final color
			finalColor += BlinnPhongLighting(currentLight,
				hitObjectMaterial,
				intersect.IntersectionPoint,
				intersect.NormalAtIntersection,
				fShade);
		}
	}

	// ---------------------------------------------------------------------------

	return finalColor;
}

// -----------------------------------------------------------------------------

IntersectionInfo RaySceneIntersection(const Ray& ray, Scene& scene)
{
	// To be used when returning the intersection info
	float fMinIntersectionDistance = std::numeric_limits<float>::infinity();
	Object* hitObject = NULL;
	Normal normalAtIntersection;
	vec3 intersectionPoint;
	
	// Temp storage for intersection testing
	IntersectionInfo intersection;

	// Get the object list in the scene
	std::vector<Object*>& objectList = scene.ObjectList();
	
	for (Object* obj : objectList)
	{
		if (obj != NULL)
		{
			intersection = obj->FindIntersection(ray);
			if (intersection.RayLength > 0 && 
				intersection.RayLength < fMinIntersectionDistance)
			{
				intersectionPoint			= intersection.IntersectionPoint;
				fMinIntersectionDistance	= intersection.RayLength;
				normalAtIntersection		= intersection.NormalAtIntersection;
				hitObject					= intersection.HitObject;
			}
		}
	}
	
	return IntersectionInfo(intersectionPoint,
							fMinIntersectionDistance,
							normalAtIntersection,
							hitObject);
}

// ------------------------------------------------------------------------

void Render(int iStartLineIndex, int iEndLineIndex)
{
	if (Realtime == true)
	{
		Draw(iStartLineIndex, iEndLineIndex);
	}
	else
	{
		if (UpdateRequired == true)
		{
			Draw(iStartLineIndex, iEndLineIndex);
		}
	}
}

// ------------------------------------------------------------------------

void Draw(int iStartLineIndex, int iEndLineIndex)
{
	// ------------------------------------------------------------------------
	// Pre-compute camera values
	float fTanHalfHorizFOV = glm::tan(rad(pCam->GetHorizontalFOV() / 2.0f));
	float fTanHalfVertFOV = glm::tan(rad(pCam->GetVerticalFOV() / 2.0f));

	float fHalfWidth = iWidth * 0.5f;
	float fHalfHeight = iHeight * 0.5f;

	// ------------------------------------------------------------------------
	// Build a coordinate frame
	vec3 vEyeTarget = pCam->GetCameraPosition() - pCam->GetCameraTarget();

	vec3 w = glm::normalize(vEyeTarget);
	vec3 u = glm::normalize(glm::cross(pCam->GetCameraUp(), w));
	vec3 v = glm::normalize(glm::cross(w, u));

	// ------------------------------------------------------------------------

	int iCurrentPixel;

	// Update pixels
	for (int iRow = iStartLineIndex; iRow < iEndLineIndex; iRow++)
	{
		for (int iColumn = 0; iColumn < iWidth; iColumn++)
		{
			// Anti-aliasing active ---------------------------------------------------------
			if (SuperSamplingEnabled == true && SampleCount > 1.0f)
			{
				float rAcc = 0.0f;
				float gAcc = 0.0f;
				float bAcc = 0.0f;
				float aAcc = 0.0f;

				float startX = (float)iColumn;
				float startY = (float)iRow;
				float endX = (float)iColumn + 1.0f;
				float endY = (float)iRow + 1.0f;

				for (; startX < endX; startX += SampleDistance)
				{
					for (; startY < endY; startY += SampleDistance)
					{
						// -------------------------------------------------------------------

						float fNormalizedXPos = ((fHalfWidth - startX) / fHalfWidth);
						float fNormalizedYPos = ((fHalfHeight - startY) / fHalfHeight);

						float fAlpha = fTanHalfHorizFOV * fNormalizedXPos;
						float fBeta = fTanHalfVertFOV * fNormalizedYPos;

						glm::vec3 rayDirection = glm::normalize(fAlpha * u + fBeta * v - w);

						Ray camIJRay(pCam->GetCameraPosition(), rayDirection);

						sf::Color surfaceColor = sf::Color(0, 0, 0, 255);
						Trace(camIJRay, surfaceColor, scene, 0, 0, AmbientRefractiveIndex);

						rAcc += surfaceColor.r;
						gAcc += surfaceColor.g;
						bAcc += surfaceColor.b;
						aAcc += surfaceColor.a;

						// -------------------------------------------------------------------
					}
				}

				if (rAcc != 0.0f)
				{
					float r = round(rAcc / SampleCount);
					float g = round(gAcc / SampleCount);
					float b = round(bAcc / SampleCount);

					sf::Uint8 rfinal = (sf::Uint8)r;
					sf::Uint8 gfinal = (sf::Uint8)g;

				}

				// Calculate final color
				sf::Color finalPixelColor = sf::Color(
					(sf::Uint8)(round(rAcc / SampleCount)),
					(sf::Uint8)(round(gAcc / SampleCount)),
					(sf::Uint8)(round(bAcc / SampleCount)),
					(sf::Uint8)(round(aAcc / SampleCount)));

				iCurrentPixel = 4 * (iColumn + iRow * iWidth);
				SetPixelColor(iCurrentPixel, finalPixelColor);
			}
			else // No anti-aliasing ---------------------------------------------------------
			{
				float fNormalizedXPos = ((fHalfWidth - iColumn) / fHalfWidth);
				float fNormalizedYPos = ((fHalfHeight - iRow) / fHalfHeight);

				float fAlpha = fTanHalfHorizFOV * fNormalizedXPos;
				float fBeta = fTanHalfVertFOV * fNormalizedYPos;

				iCurrentPixel = 4 * (iColumn + iRow * iWidth);

				glm::vec3 rayDirection = glm::normalize(fAlpha * u + fBeta * v - w);

				Ray camIJRay(pCam->GetCameraPosition(), rayDirection);

				sf::Color surfaceColor = sf::Color(0, 0, 0, 255);
				Trace(camIJRay, surfaceColor, scene, 0, 0, AmbientRefractiveIndex);

				SetPixelColor(iCurrentPixel, surfaceColor);
			}
		}
	}
}

// ------------------------------------------------------------------------

void Update(float dt)
{
	if (bGUIMode == false)
	{
		// ------------------------------------------------------------------------

		// Update input
		glm::vec3 moveVector = glm::vec3(0.0f);
		UpdateInput(moveVector);

		// ------------------------------------------------------------------------

		// Update camera position
		pCam->AddToCameraPosition(moveVector * dt);

		// ------------------------------------------------------------------------
	}
}

// ------------------------------------------------------------------------

void UpdateInput(glm::vec3& moveVector)
{
	// ------------------------------------------------------------------------

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		moveVector += glm::vec3(-1.0f, 0.0f, 0.0f) * moveSpeed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		moveVector += glm::vec3(1.0f, 0.0f, 0.0f) * moveSpeed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		moveVector += glm::vec3(0.0f, 0.0f, 1.0f) * moveSpeed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		moveVector += glm::vec3(0.0f, 0.0f, -1.0f) * moveSpeed;
	}

	// ------------------------------------------------------------------------

	// Print camera position
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::C))
	{
		const glm::vec3 cameraPosition = pCam->GetCameraPosition();

		std::cout << "Camera position: " << cameraPosition.x << " " << cameraPosition.y << " " << cameraPosition.z << std::endl;
		std::cout << "Camera x rot: " << pCam->GetXRotation() << std::endl;
		std::cout << "Camera y rot: " << pCam->GetYRotation() << std::endl;
	}

	// ------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

int main(int argc, char **argv)
{
	// ------------------------------------------------------------------------

	bGUIMode = false;
	srand((unsigned int)time(NULL));

	// ------------------------------------------------------------------------

#ifdef MULTITHREADING
	SetupMultithread();
#endif // MULTITHREADING

	// ------------------------------------------------------------------------

	sf::Vector2i currentMousePosition;
	sf::Vector2i originalMousePosition;
	sf::Mouse::setPosition(screenCenter);
	originalMousePosition = sf::Mouse::getPosition(window);

	// ------------------------------------------------------------------------
	// Pixel data

	sf::Texture texture;
	texture.create(iWidth, iHeight);
	sf::Sprite sprite;
	unsigned int uiPrintIndex = 0;

	// ------------------------------------------------------------------------
	// Clock
	sf::Clock timer;
	float fCurrentTime = 0;

	// ------------------------------------------------------------------------
	// Camera
	pCam = std::make_shared<Camera>(glm::vec3(-1.57641f, 2.33531f, -0.256838f),
		60.0f,
		60.0f * (iWidth / (float)iHeight));
	pCam->SetXRotation(0.49803f);
	pCam->SetYRotation(-5.36572f);

	// ------------------------------------------------------------------------

	// Light
	DirectionalLight* dirLight = new DirectionalLight(
		glm::vec3(-1.0f, 2.0f, 0.5f),
		sf::Color(40, 40, 40, 255),
		sf::Color(40, 40, 40, 255),
		sf::Color(40, 40, 40, 255),
		5.0f, 
		"DirectionalLight1");
	PointLight* pointLight1 = new PointLight(
		glm::vec3(1.0f, 1.0f, 1.0f),
		sf::Color(80, 80, 80, 255),
		WhiteColor,
		WhiteColor,
		glm::vec3(0.0f, 2.0f, 1.0f),
		5.0f,
		"PointLightWhite");
	PointLight* pointLight2 = new PointLight(
		glm::vec3(4.0f, 1.0f, -3.0f),
		sf::Color(10, 10, 10, 255),
		BlueColor,
		WhiteColor,
		glm::vec3(1.0f, 4.0f, 1.0f),
		2.0f,
		"PointLightBlue");
	PointLight* pointLight3 = new PointLight(
		glm::vec3(8.0f, 1.0f, -5.0f),
		sf::Color(10, 10, 10, 255),
		RedColor,
		WhiteColor,
		glm::vec3(1.0f, 4.0f, 1.0f),
		2.0f,
		"PointLightRed");

	// Add area light for soft shadows
	Triangle triangle1(glm::vec3(0.0f, 5.0f, 0.0f),
		glm::vec3(1.0f, 5.0f, 0.0f),
		glm::vec3(0.0f, 5.0f, 1.0f));
	Triangle triangle2(glm::vec3(0.0f, 5.0f, 1.0f),
		glm::vec3(1.0f, 5.0f, 0.0f),
		glm::vec3(1.0f, 5.0f, 1.0f));

	AreaLight* areaLight = new AreaLight(
		glm::vec3(3.0f, 4.0f, 3.0f),
		0.8f, 0.1f, 0.8f,
		WhiteColor, WhiteColor, WhiteColor, "SquareAreaLight");

	// ------------------------------------------------------------------------
	// Scene

	//scene.AddObject(dirLight);
	//scene.AddObject(pointLight1);
	scene.AddObject(areaLight);

	Material sphere1CopperMat;
	memset(&sphere1CopperMat, 0, sizeof(Material));
	sphere1CopperMat.Ambient = sf::Color(49, 19, 6, 255);
	sphere1CopperMat.Diffuse = sf::Color(180, 69, 21, 255);
	sphere1CopperMat.Specular = sf::Color(65, 35, 22, 255);
	sphere1CopperMat.Shininess = 12.8f;
	sphere1CopperMat.Reflectivity = 1.0f;
	sphere1CopperMat.Transparency = 0.0f;

	Material sphere2SilverMat;
	memset(&sphere2SilverMat, 0, sizeof(Material));
	sphere2SilverMat.Ambient = sf::Color(49, 49, 49, 255);
	sphere2SilverMat.Diffuse = sf::Color(129, 129, 129, 255);
	sphere2SilverMat.Specular = sf::Color(130, 130, 130, 255);
	sphere2SilverMat.Shininess = 51.2f;
	sphere2SilverMat.Reflectivity = 0.3f;
	sphere2SilverMat.Transparency = 0.5f;
	sphere2SilverMat.RefractiveIndex = 1.55f;

	Material greenRubberMat;
	memset(&greenRubberMat, 0, sizeof(Material));
	greenRubberMat.Ambient = sf::Color(0, 13, 0, 255);
	greenRubberMat.Diffuse = sf::Color(102, 128, 102, 255);
	greenRubberMat.Specular = sf::Color(10, 179, 10, 255);
	greenRubberMat.Shininess = 10.0f;
	greenRubberMat.Reflectivity = 1.0f;
	greenRubberMat.Transparency = 0.0f;

	Sphere* pSphere = new Sphere(sphere1CopperMat, 
		glm::vec3(1.0f, 0.5f, 2.0f), 
		0.2f,
		"CopperSphere");
	Sphere* pSphere2 = new Sphere(sphere2SilverMat,
		glm::vec3(0.2f, 0.2f, 2.0f),
		0.3f,
		"SilverSphere");
	Sphere* pSphere3 = new Sphere(sphere2SilverMat,
		glm::vec3(2.0f, 1.5f, 4.0f), 
		0.4f,
		"SliverSphere2");
	Plane* pPlaneBottom = new Plane(greenRubberMat,
		Normal(0.0f, 1.0f, 0.0f), 
		Point(0.0f, -3.0f, 0.0f),
		"BottomPlane");
	
	Plane* pPlaneLeft = new Plane(sphere1CopperMat,
		Normal(1.0f, 0.0f, 0.0f), 
		Point(-10.0f, 0.0f, 0.0f),
		"PlaneLeft");
	Plane* pPlaneBack = new Plane(sphere1CopperMat,
		Normal(0.0f, 0.0f, 1.0f), 
		Point(0.0f, 0.0f, 10.0f),
		"PlaneBack");

	Box* pBox1 = new Box(sphere2SilverMat, glm::vec3(2.0f, 0.0f, 3.0f), 1.0f, 1.0f, 1.0f, "FirstBox");

	//scene.AddObject(pSphere);
	//scene.AddObject(pSphere2);
	//scene.AddObject(pSphere3);
	scene.AddObject(pPlaneBottom);
	//scene.AddObject(pPlaneLeft);
	//scene.AddObject(pPlaneBack);
	scene.AddObject(pBox1);

	// ------------------------------------------------------------------------
	// Launch the UI thread

	UI* ui = new UI(&scene,
		&MAX_REFLECTION_DEPTH,
		&MAX_REFRACTION_DEPTH,
		&moveSpeed,
		&SquareLength,
		&SampleCount,
		&SampleDistance,
		&UpdateRequired,
		&Realtime,
		&ShadowsEnabled,
		&SoftShadowsEnabled,
		&SuperSamplingEnabled,
		&PlaneTexturingEnabled,
		&ReflectionEnabled,
		&RefractionEnabled,
		&eLightModel);
	sf::Thread uiThread(&UI::ProcessUI, ui);
	uiThread.launch();

	// ------------------------------------------------------------------------

	while (window.isOpen())
	{
		// Update time
		fCurrentTime = timer.restart().asSeconds();
		float fFPS = 1.0f / fCurrentTime;

		// check all the window's events that were triggered since the last iteration of the loop
		sf::Event event;
		while (window.pollEvent(event))
		{
			// "close requested" event: we close the window
			if (event.type == sf::Event::Closed)
			{
				uiThread.terminate();
				window.close();
			}

			if (event.type == sf::Event::MouseMoved)
			{
				if (bGUIMode == false)
				{
					currentMousePosition = sf::Mouse::getPosition(window);

					if (currentMousePosition != originalMousePosition)
					{
						float fXDiff = static_cast<float>(currentMousePosition.x - originalMousePosition.x);
						float fYDiff = static_cast<float>(currentMousePosition.y - originalMousePosition.y);

						pCam->AddYRotation(fCurrentTime, -fXDiff);
						pCam->AddXRotation(fCurrentTime, -fYDiff);

						pCam->UpdateViewMatrix();

						sf::Mouse::setPosition(screenCenter);
					}
				}
			}

			if (event.type == sf::Event::KeyReleased)
			{
				switch (event.key.code)
				{
					case sf::Keyboard::Escape:
					{
						uiThread.terminate();
						window.close();
						
						break;
					}
					case sf::Keyboard::G:
					{
						if (bGUIMode)
						{
							bGUIMode = false;
							std::cout << "Gui mode de-activated" << std::endl;
						}
						else
						{
							bGUIMode = true;
							std::cout << "Gui mode activated" << std::endl;

							sf::Mouse::setPosition(screenCenter);
							currentMousePosition = originalMousePosition;
						}

						break;
					}
					case sf::Keyboard::P:
					{
						uiPrintIndex++;
						texture.copyToImage().saveToFile("Print" + std::to_string(uiPrintIndex) + ".png");
						std::cout << "Image ""Print" << uiPrintIndex << ".png"" exported" << std::endl;
						break;
					}
				}
			}
		}

		Update(fCurrentTime);

#ifdef MULTITHREADING

		for (unsigned int i = 0; i < ImageProcessingTaskList.size(); i++)
		{
			m_ThreadPool->schedule(ImageProcessingTaskList[i]);
			//m_ThreadPool->wait();
		}

		m_ThreadPool->wait();
#else

		Render(0, iHeight);

#endif // MULTITHREADING

		// Update done
		UpdateRequired = false;

		window.setTitle(std::to_string(fFPS));

		// Update texture and draw
		texture.update(pixels);
		sprite.setTexture(texture);
		texture.copyToImage().saveToFile("raytraced.png");
		window.draw(sprite);

		// end the current frame
		window.display();
	}

	// ------------------------------------------------------------------------

	if (pixels)
	{
		delete[] pixels;
	}

	// ------------------------------------------------------------------------

	return 0;
}

// ------------------------------------------------------------------------
// Multithreading
// ------------------------------------------------------------------------

#ifdef MULTITHREADING

void SetupMultithread()
{
	if (m_ThreadPool == nullptr)
	{
		m_ThreadPool = std::make_unique<boost::threadpool::pool>(m_iThreadCount);
	}
	ImageProcessingTaskList.clear();

	// Create a list of tasks
	for (unsigned int iThreadIndex = 0; iThreadIndex < m_iThreadCount; iThreadIndex++)
	{
		// Calculate the start and end index to process for the current thread
		int iStep = iHeight / m_iThreadCount;

		int iStartIndex = iStep * iThreadIndex;
		int iEndIndex = iStep * (iThreadIndex + 1);

		if (iThreadIndex == m_iThreadCount - 1)
		{
			iEndIndex = iHeight;
		}

		// Initialize task list for lambda calculation
		ImageProcessingTaskList.push_back(boost::bind(&Render,
			iStartIndex,
			iEndIndex));
	}
}

#endif // MULTITHREADING

// ------------------------------------------------------------------------