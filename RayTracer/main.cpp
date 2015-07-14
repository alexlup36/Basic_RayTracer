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

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

// ------------------------------------------------------------------------
// Window
const unsigned int iWidth = 800;
const unsigned int iHeight = 600;
const unsigned int iImageSize = iWidth * iHeight;
const unsigned int iColor = 24;
const float moveSpeed = 5.0f;

sf::RenderWindow window(sf::VideoMode(iWidth, iHeight, iColor), "RayTracer"/*, sf::Style::Fullscreen*/);
sf::Vector2i windowPosition = window.getPosition();
sf::Vector2i screenCenter(static_cast<int>(windowPosition.x + iWidth * 0.5f), 
	static_cast<int>(windowPosition.y + iHeight * 0.5f));

// -----------------------------------------------------------------------------

std::vector<DirectionalLight*> dirLightSources;
std::vector<PointLight*> pointLightSources;
DirectionalLight dirLight;
PointLight pointLight;
std::shared_ptr<Camera> pCam;
sf::Uint8* pixels = new sf::Uint8[iWidth * iHeight * 4];

// -----------------------------------------------------------------------------

void Draw(Scene& scene);
void Update(float dt);
void UpdateInput(glm::vec3& moveVector);

// -----------------------------------------------------------------------------

void SetPixelColor(int iCurrentPixel, sf::Uint8 r, sf::Uint8 g, sf::Uint8 b, sf::Uint8 a)
{
	pixels[iCurrentPixel]		= r;
	pixels[iCurrentPixel + 1]	= g;
	pixels[iCurrentPixel + 2]	= b;
	pixels[iCurrentPixel + 3]	= a;
}

// -----------------------------------------------------------------------------

Color PhongLighting(DirectionalLight& currentLight, 
	Material& material,
	const glm::vec3& position,
	const glm::vec3& normal)
{
	Color ambientComponent, diffuseComponent, specularComponent;

	// Ambient component
	ambientComponent = currentLight.AmbientLight * material.Ambient;

	// Diffuse component
	glm::vec3 lightDirection = glm::normalize(currentLight.Direction);
	float fNormalDotLight = glm::max(glm::dot(normal, lightDirection), 0.0f);
	diffuseComponent = material.Diffuse * currentLight.DiffuseLight * fNormalDotLight;

	// Specular component
	vec3& viewDirection = glm::normalize(pCam->GetCameraPosition() - position);
	vec3& reflectionDirection = glm::reflect<vec3>(-lightDirection, normal);
	float specular = std::pow(std::max(glm::dot(viewDirection, reflectionDirection), 0.0f), material.Shininess);
	specularComponent = material.Specular * currentLight.SpecularLight * specular;

	// Return the final color
	return ambientComponent + diffuseComponent + specularComponent;
}

// -----------------------------------------------------------------------------

Color PhongLightingPoint(PointLight& currentLight,
	Material& material,
	const glm::vec3& position,
	const glm::vec3& normal)
{
	Color ambientComponent, diffuseComponent, specularComponent;

	// Ambient component
	ambientComponent = currentLight.AmbientLight * material.Ambient;

	// Diffuse component

	// Calculate the light vector
	glm::vec3 lightVector = currentLight.Position - position;
	// Calculate the distance from the point light to the pixel position
	float distance = glm::length(lightVector);
	// Calculate the attenuation factor
	float attenuation = 1.0f / (currentLight.ConstantAttenuation +
		currentLight.LinearAttenuation * distance +
		currentLight.QuadraticAttenuation * (distance * distance));
	glm::vec3 lightDirection = glm::normalize(lightVector);
	float fNormalDotLight = glm::max(glm::dot(normal, lightDirection), 0.0f);
	diffuseComponent = material.Diffuse * currentLight.DiffuseLight * fNormalDotLight;

	// Specular component
	vec3& viewDirection = glm::normalize(pCam->GetCameraPosition() - position);
	vec3& reflectionDirection = glm::reflect<vec3>(-lightDirection, normal);
	float specular = std::pow(std::max(glm::dot(viewDirection, reflectionDirection), 0.0f), material.Shininess);
	specularComponent = material.Specular * currentLight.SpecularLight * specular;

	// Return the final color
	return (ambientComponent + diffuseComponent + specularComponent);// *attenuation;
}

// -----------------------------------------------------------------------------

Color BlinnPhongLighting(DirectionalLight& currentLight,
	Material& material,
	const glm::vec3& position,
	const glm::vec3& normal)
{
	Color ambientComponent, diffuseComponent, specularComponent;

	// Ambient component
	ambientComponent = currentLight.AmbientLight * material.Ambient;

	// Diffuse component
	glm::vec3 lightDirection = glm::normalize(currentLight.Direction);
	float fNormalDotLight = glm::max(glm::dot(normal, lightDirection), 0.0f);
	diffuseComponent = material.Diffuse * currentLight.DiffuseLight * fNormalDotLight;

	// Specular component
	vec3& viewDirection = glm::normalize(pCam->GetCameraPosition() - position);
	vec3& halfVector = glm::normalize(lightDirection + viewDirection);
	float specular = std::pow(std::max(glm::dot(normal, halfVector), 0.0f), material.Shininess);
	specularComponent = material.Specular * currentLight.SpecularLight * specular;

	// Return the final color
	return ambientComponent + diffuseComponent + specularComponent;
}

// -----------------------------------------------------------------------------

Color FindColor(const IntersectionInfo& intersect, std::vector<DirectionalLight*> dirLightSources)
{
	Color finalColor;
	Material& hitObjectMaterial = intersect.HitObject->GetMaterial();

	// Go through all the directional lights in the scene
	//for (unsigned int lightIndex = 0; lightIndex < dirLightSources.size(); lightIndex++)
	//{
	//	// Get the current light source
	//	DirectionalLight& currentLight = *dirLightSources[lightIndex];

	//	// Compute the final color
	//	finalColor += PhongLighting(currentLight,
	//		hitObjectMaterial, 
	//		intersect.IntersectionPoint, 
	//		intersect.NormalAtIntersection);
	//}

	// Go through all the point lights in the scene
	for (unsigned int lightIndex = 0; lightIndex < pointLightSources.size(); lightIndex++)
	{
		// Get the current light source
		PointLight& currentLight = *pointLightSources[lightIndex];

		// Compute the final color
		finalColor += PhongLightingPoint(currentLight,
			hitObjectMaterial,
			intersect.IntersectionPoint,
			intersect.NormalAtIntersection);
	}

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

void Draw(Scene& scene)
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
	for (int iRow = 0; iRow < iHeight; iRow++)
	{
		for (int iColumn = 0; iColumn < iWidth; iColumn++)
		{
			float fNormalizedXPos = ((iColumn - fHalfWidth) / fHalfWidth);
			float fNormalizedYPos = ((fHalfHeight - iRow) / fHalfHeight);

			float fAlpha = fTanHalfHorizFOV * fNormalizedXPos;
			float fBeta = fTanHalfVertFOV * fNormalizedYPos;

			iCurrentPixel = 4 * (iColumn + iRow * iWidth);

			glm::vec3 rayDirection = glm::normalize(fAlpha * u + fBeta * v - w);

			Ray camIJRay(pCam->GetCameraPosition(), rayDirection);

			IntersectionInfo intersect = RaySceneIntersection(
				camIJRay,
				scene);

			if (intersect.HitObject != NULL)
			{
				// Calculate the shadow ray
				Ray shadowRay(intersect.IntersectionPoint, dirLight.Direction);

				// Get the object list in the scene
				std::vector<Object*>& objectList = scene.ObjectList();

				bool bInShadow = false;

				for (Object* obj : objectList)
				{
					if (obj != NULL && obj->GetIndex() != intersect.HitObject->GetIndex())
					{
						IntersectionInfo intersection = obj->FindIntersection(shadowRay);
						if (intersection.HitObject != NULL)
						{
							bInShadow = true;
						}
					}
				}
				Color& color = FindColor(intersect, dirLightSources);

				if (bInShadow == false)
				{
					SetPixelColor(iCurrentPixel,
						sf::Uint8(255 * color.X),
						sf::Uint8(255 * color.Y),
						sf::Uint8(255 * color.Z),
						sf::Uint8(255 * color.W));
				}
				else
				{
					SetPixelColor(iCurrentPixel, 0, 0, 0, 255);
				}
			}
			else
			{
				SetPixelColor(iCurrentPixel, 0, 0, 0, 255);
			}
		}
	}
}

// ------------------------------------------------------------------------

void Update(float dt)
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

// ------------------------------------------------------------------------

void UpdateInput(glm::vec3& moveVector)
{
	// ------------------------------------------------------------------------

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		moveVector += glm::vec3(1.0f, 0.0f, 0.0f) * moveSpeed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		moveVector += glm::vec3(-1.0f, 0.0f, 0.0f) * moveSpeed;
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
}

// -----------------------------------------------------------------------------

int main(int argc, char **argv)
{
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

	// ------------------------------------------------------------------------
	// Clock
	sf::Clock timer;
	float fCurrentTime = 0;

	// ------------------------------------------------------------------------
	// Camera
	pCam = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 3.0f),
		60.0f,
		60.0f);

	// ------------------------------------------------------------------------

	// Light

	dirLightSources.push_back(&dirLight);
	pointLightSources.push_back(&pointLight);

	// ------------------------------------------------------------------------
	// Scene

	Scene scene;

	std::shared_ptr<Sphere> pSphere = std::make_shared<Sphere>(vec3(1.0f, 1.0f, 5.0f), 0.2f);
	std::shared_ptr<Sphere> pSphere2 = std::make_shared<Sphere>(vec3(0.0f, 1.0f, 2.0f), 0.3f);
	std::shared_ptr<Sphere> pSphere3 = std::make_shared<Sphere>(vec3(2.0f, 1.5f, 8.0f), 0.4f);
	std::shared_ptr<Plane> pPlane = std::make_shared<Plane>(Normal(0.0f, 1.0f, 0.0f), Point(0.0f, -3.0f, 0.0f));

	scene.AddObject(pSphere.get());
	scene.AddObject(pSphere2.get());
	scene.AddObject(pSphere3.get());
	scene.AddObject(pPlane.get());

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
				window.close();

			if (event.type == sf::Event::MouseMoved)
			{
				currentMousePosition = sf::Mouse::getPosition(window);

				if (currentMousePosition != originalMousePosition)
				{
					float fXDiff = static_cast<float>(currentMousePosition.x - originalMousePosition.x);
					float fYDiff = static_cast<float>(currentMousePosition.y - originalMousePosition.y);

					pCam->AddYRotation(fCurrentTime, fXDiff);
					pCam->AddXRotation(fCurrentTime, -fYDiff);

					pCam->UpdateViewMatrix();

					sf::Mouse::setPosition(screenCenter);
				}
			}

			if (event.type == sf::Event::KeyReleased)
			{
				switch (event.key.code)
				{
				case sf::Keyboard::Escape:
				{
					window.close();

					break;
				}
				}
			}
		}

		Update(fCurrentTime);
		Draw(scene);

		window.setTitle(std::to_string(fFPS));

		// Update texture and draw
		texture.update(pixels);
		sprite.setTexture(texture);
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