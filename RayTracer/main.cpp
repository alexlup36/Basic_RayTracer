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
const unsigned int iWidth = 1024;
const unsigned int iHeight = 768;
const unsigned int iImageSize = iWidth * iHeight;
const unsigned int iColor = 24;

sf::RenderWindow window(sf::VideoMode(iWidth, iHeight, iColor), "RayTracer"/*, sf::Style::Fullscreen*/);

// -----------------------------------------------------------------------------

DirectionalLight light;
Camera* pCam;
sf::Uint8* pixels = new sf::Uint8[iWidth * iHeight * 4];

// -----------------------------------------------------------------------------

void SetPixelColor(int iCurrentPixel, sf::Uint8 r, sf::Uint8 g, sf::Uint8 b, sf::Uint8 a)
{
	pixels[iCurrentPixel]		= r;
	pixels[iCurrentPixel + 1]	= g;
	pixels[iCurrentPixel + 2]	= b;
	pixels[iCurrentPixel + 3]	= a;
}

// -----------------------------------------------------------------------------

Color FindColor(const IntersectionInfo& intersect)
{
	Color finalColor;
	Object* pHitObject = intersect.HitObject;
	Material& hitObjectMaterial = pHitObject->GetMaterial();

	// Compute the diffuse term
	float fNormalDotLight = glm::dot(intersect.NormalAtIntersection, light.Direction);

	Color& vDiffuse = hitObjectMaterial.Diffuse * light.DiffuseLight * glm::max<float>(fNormalDotLight, 0.0f);

	// Compute the specular term
	vec3& vViewVector = glm::normalize(pCam->GetCameraPosition() - intersect.IntersectionPoint);
		
	// Phong Shading 
	vec3& vReflectedLight = glm::normalize(glm::reflect<vec3>(-light.Direction, intersect.NormalAtIntersection));
	float fReflectedDotView = glm::dot(vReflectedLight, vViewVector);
	Color vSpecular = hitObjectMaterial.Specular *
						light.SpecularLight *
						glm::pow(glm::max<float>(fReflectedDotView, 0.0f), hitObjectMaterial.Shininess);

	// Blinn-Phong shading
	/*vec3 vHalfwayVector = glm::normalize(vViewVector + light.Direction);
	float fNormalDotHalf = glm::dot(intersect.NormalAtIntersection, vHalfwayVector);
	Color vSpecular = pHitObject->GetMaterial().Specular *
						light.SpecularLight *
						glm::pow(glm::max<float>(fNormalDotHalf, 0.0f), intersect.HitObject->GetMaterial().Shininess);*/

	finalColor = AmbientColor + vDiffuse + vSpecular;
	
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

// -----------------------------------------------------------------------------

int main(int argc, char **argv) 
{	
	// ------------------------------------------------------------------------

	sf::Vector2i currentMousePosition;
	sf::Vector2i originalMousePosition;

	sf::Vector2i windowPosition = window.getPosition();
	sf::Mouse::setPosition(sf::Vector2i(windowPosition.x + iWidth * 0.5f, windowPosition.y + iHeight * 0.5));
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
	// Scene
	Scene scene;

	// ------------------------------------------------------------------------
	// Camera
	pCam = new Camera(glm::vec3(0.0f, 0.0f, 3.0f),
		60.0f,
		60.0f);

	// ------------------------------------------------------------------------
	// Pre-compute camera values
	float fTanHalfHorizFOV	= glm::tan(rad(pCam->GetHorizontalFOV() / 2.0f));
	float fTanHalfVertFOV	= glm::tan(rad(pCam->GetVerticalFOV() / 2.0f));

	float fHalfWidth	= iWidth * 0.5f;
	float fHalfHeight	= iHeight * 0.5f;

	// ------------------------------------------------------------------------

	std::shared_ptr<Sphere> pSphere		= std::make_shared<Sphere>(vec3(1.0f, 1.0f, 5.0f), 0.2f);
	std::shared_ptr<Sphere> pSphere2	= std::make_shared<Sphere>(vec3(0.0f, 1.0f, 2.0f), 0.3f);
	std::shared_ptr<Sphere> pSphere3	= std::make_shared<Sphere>(vec3(2.0f, 1.5f, 8.0f), 0.4f);
	std::shared_ptr<Plane> pPlane		= std::make_shared<Plane>(Normal(0.0f, 1.0f, 0.0f), Point(0.0f, -3.0f, 0.0f));

	scene.AddObject(pSphere.get());
	scene.AddObject(pSphere2.get());
	scene.AddObject(pSphere3.get());
	scene.AddObject(pPlane.get());

	int iCurrentPixel;

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
					float fXDiff = currentMousePosition.x - originalMousePosition.x;
					float fYDiff = currentMousePosition.y - originalMousePosition.y;

					pCam->AddYRotation(fCurrentTime, fXDiff);
					pCam->AddXRotation(fCurrentTime, -fYDiff);

					pCam->UpdateViewMatrix();

					sf::Vector2i windowPosition = window.getPosition();
					sf::Mouse::setPosition(sf::Vector2i(windowPosition.x + iWidth * 0.5f, windowPosition.y + iHeight * 0.5));
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

		// ------------------------------------------------------------------------
		// Update input
		glm::vec3 moveVector = glm::vec3(0.0f);

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			moveVector += glm::vec3(1.0f, 0.0f, 0.0f);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			moveVector += glm::vec3(-1.0f, 0.0f, 0.0f);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			moveVector += glm::vec3(0.0f, 0.0f, 1.0f);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			moveVector += glm::vec3(0.0f, 0.0f, -1.0f);
		}

		pCam->AddToCameraPosition(moveVector * fCurrentTime);

		// ------------------------------------------------------------------------
		// Build a coordinate frame
		vec3 vEyeTarget = pCam->GetCameraPosition() - pCam->GetCameraTarget();

		vec3 w = glm::normalize(vEyeTarget);
		vec3 u = glm::normalize(glm::cross(pCam->GetCameraUp(), w));
		vec3 v = glm::normalize(glm::cross(w, u));

		// ------------------------------------------------------------------------

		//window.clear(sf::Color::White);

		// ------------------------------------------------------------------------

		// Update pixels
		for (int iRow = 0; iRow < iHeight; iRow++)
		{
			for (int iColumn = 0; iColumn < iWidth; iColumn++)
			{
				float fNormalizedXPos = ((iColumn - fHalfWidth) / fHalfWidth);
				float fNormalizedYPos = ((fHalfHeight - iRow) / fHalfHeight);

				float fAlpha = fTanHalfHorizFOV * fNormalizedXPos;
				float fBeta = fTanHalfVertFOV * fNormalizedYPos;
						
				glm::vec3 rayDirection = glm::normalize(fAlpha * u + fBeta * v - w);

				Ray camIJRay(pCam->GetCameraPosition(), rayDirection);
					
				IntersectionInfo intersect = RaySceneIntersection(
					camIJRay,
					scene);
							
				if (intersect.HitObject != NULL)
				{
					// Calculate the shadow ray
					Ray shadowRay(intersect.IntersectionPoint, light.Direction);

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

					if (bInShadow == false)
					{
						iCurrentPixel = 4 * (iColumn + iRow * iWidth);
						Color& color = FindColor(intersect);
						SetPixelColor(iCurrentPixel, 255 * color.X, 255 * color.Y, 255 * color.Z, 255 * color.W);
					}
					else
					{
						iCurrentPixel = 4 * (iColumn + iRow * iWidth);
						SetPixelColor(iCurrentPixel, 0, 0, 0, 255);
					}
				}
			}
		}

		// Update light
		light.Direction.x += 0.001f;
		light.Direction.y += 0.0001f;
		light.Direction.z += 0.01f;

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