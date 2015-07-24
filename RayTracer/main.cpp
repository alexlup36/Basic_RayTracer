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

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

#include "TGUI/TGUI.hpp"

// ------------------------------------------------------------------------
// Window
const unsigned int iWidth = 800;
const unsigned int iHeight = 600;
const unsigned int iImageSize = iWidth * iHeight;
const unsigned int iColor = 24;
float moveSpeed = 1.0f;
unsigned int MAX_REFLECTION_DEPTH = 1;
bool bGUIMode;
unsigned int SliderPositionAmplitude = 30;

enum Button
{
	UpdateSettingsID = 0,
	InvalidID,
};

sf::RenderWindow window(sf::VideoMode(iWidth, iHeight, iColor), "RayTracer"/*, sf::Style::Fullscreen*/);
sf::Vector2i windowPosition = window.getPosition();
sf::Vector2i screenCenter(static_cast<int>(windowPosition.x + iWidth * 0.5f), 
	static_cast<int>(windowPosition.y + iHeight * 0.5f));

// Initialize the Gui 
tgui::Gui gui(window);

tgui::Slider::Ptr sliderXPtr = nullptr;
tgui::Slider::Ptr sliderYPtr = nullptr;
tgui::Slider::Ptr sliderZPtr = nullptr;

tgui::ComboBox::Ptr comboBox = nullptr;

// -----------------------------------------------------------------------------

std::vector<DirectionalLight*> dirLightSources;
std::vector<PointLight*> pointLightSources;
std::shared_ptr<Camera> pCam;
sf::Uint8* pixels = new sf::Uint8[iWidth * iHeight * 4];

// -----------------------------------------------------------------------------

void Draw(Scene& scene);
void Update(float dt);
void UpdateInput(glm::vec3& moveVector);

void SetupWidgets(tgui::Gui& gui);
void updateButtonCallback(const tgui::Callback& callback);
void comboBoxSelectionCallback(const tgui::Callback& callback);
IntersectionInfo RaySceneIntersection(const Ray& ray, Scene& scene);
sf::Color FindColor(const IntersectionInfo& intersect, Scene& scene, float fShade);

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
	Material& material,
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
		sf::Color diffuseResult = material.Diffuse * currentLight.DiffuseLight;;
		diffuseComponent = sf::Color((sf::Uint8)(diffuseResult.r * fNormalDotLight),
			(sf::Uint8)(diffuseResult.g * fNormalDotLight),
			(sf::Uint8)(diffuseResult.b * fNormalDotLight));

		// Specular component
		vec3& viewDirection = glm::normalize(pCam->GetCameraPosition() - position);
		vec3& reflectionDirection = glm::reflect<vec3>(-lightDirection, normal);
		float specular = std::pow(std::max(glm::dot(viewDirection, reflectionDirection), 0.0f), material.Shininess);
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
	Material& material,
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
		// Calculate the attenuation factor
		float attenuation = 1.0f / (currentLight.ConstantAttenuation +
			currentLight.LinearAttenuation * distance +
			currentLight.QuadraticAttenuation * (distance * distance));
		glm::vec3 lightDirection = glm::normalize(lightVector);
		float fNormalDotLight = glm::max(glm::dot(normal, lightDirection), 0.0f);
		sf::Color diffuseResult = material.Diffuse * currentLight.DiffuseLight;;
		diffuseComponent = sf::Color((sf::Uint8)(diffuseResult.r * fNormalDotLight),
			(sf::Uint8)(diffuseResult.g * fNormalDotLight),
			(sf::Uint8)(diffuseResult.b * fNormalDotLight));

		// Specular component
		vec3& viewDirection = glm::normalize(pCam->GetCameraPosition() - position);
		vec3& reflectionDirection = glm::reflect<vec3>(-lightDirection, normal);
		float specular = std::pow(std::max(glm::dot(viewDirection, reflectionDirection), 0.0f), material.Shininess);
		sf::Color specularResult = material.Specular * currentLight.SpecularLight;
		specularComponent = sf::Color((sf::Uint8)(specularResult.r * specular),
			(sf::Uint8)(specularResult.g * specular),
			(sf::Uint8)(specularResult.b * specular));

		// Return the final color
		return (ambientComponent + diffuseComponent + specularComponent);// *attenuation;
	}
}

// -----------------------------------------------------------------------------

sf::Color BlinnPhongLighting(DirectionalLight& currentLight,
	Material& material,
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
		sf::Color diffuseResult = material.Diffuse * currentLight.DiffuseLight;;
		diffuseComponent = sf::Color((sf::Uint8)(diffuseResult.r * fNormalDotLight),
			(sf::Uint8)(diffuseResult.g * fNormalDotLight),
			(sf::Uint8)(diffuseResult.b * fNormalDotLight));

		// Specular component
		vec3& viewDirection = glm::normalize(pCam->GetCameraPosition() - position);
		vec3& halfVector = glm::normalize(lightDirection + viewDirection);
		float specular = std::pow(std::max(glm::dot(normal, halfVector), 0.0f), material.Shininess);
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
	Material& material,
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
		float attenuation = 1.0f / (currentLight.ConstantAttenuation +
			currentLight.LinearAttenuation * distance +
			currentLight.QuadraticAttenuation * (distance * distance));
		glm::vec3 lightDirection = glm::normalize(lightVector);
		float fNormalDotLight = glm::max(glm::dot(normal, lightDirection), 0.0f);
		sf::Color diffuseResult = material.Diffuse * currentLight.DiffuseLight;;
		diffuseComponent = sf::Color((sf::Uint8)(diffuseResult.r * fNormalDotLight),
			(sf::Uint8)(diffuseResult.g * fNormalDotLight),
			(sf::Uint8)(diffuseResult.b * fNormalDotLight));

		// Specular component
		vec3& viewDirection = glm::normalize(pCam->GetCameraPosition() - position);
		vec3& halfVector = glm::normalize(lightDirection + viewDirection);
		float specular = std::pow(std::max(glm::dot(normal, halfVector), 0.0f), material.Shininess);
		sf::Color specularResult = material.Specular * currentLight.SpecularLight;
		specularComponent = sf::Color((sf::Uint8)(specularResult.r * specular),
			(sf::Uint8)(specularResult.g * specular),
			(sf::Uint8)(specularResult.b * specular));

		// Return the final color
		return (ambientComponent + diffuseComponent + specularComponent);// *attenuation;
	}
}

// -----------------------------------------------------------------------------

void Trace(const Ray& ray, sf::Color& colorAccumulator, Scene& scene, unsigned int iReflectionDepth)
{
	// Calculate intersection
	IntersectionInfo intersect = RaySceneIntersection(ray, scene);

	if (intersect.HitObject != NULL)
	{
		// Check if we hit a light source
		if (intersect.HitObject->Type() == ObjectType::keDIRECTIONALLIGHT ||
			intersect.HitObject->Type() == ObjectType::kePOINTLIGHT)
		{
			colorAccumulator = sf::Color(255, 255, 255, 255);
			return;
		}

		// Get the material of the hit object
		Material& hitObjectMaterial = intersect.HitObject->GetMaterial();

		// --------------------------------------------------------------------
		// Shadows

		float fShade = 1.0f;

		if (intersect.HitObject != NULL)
		{
			// Calculate the shadow ray for each light source in the scene

			// Go through all directional light sources and calculate the shadow rays
			for (unsigned int lightIndex = 0; lightIndex < dirLightSources.size(); lightIndex++)
			{
				// Get the current light source
				DirectionalLight& currentLight = *dirLightSources[lightIndex];

				// Calculate the intersection of the reflected ray
				glm::vec3 lightDirection = glm::normalize(currentLight.Direction);
				glm::vec3 startPoint = intersect.NormalAtIntersection * 0.1f;
				Ray shadowRay(startPoint, lightDirection);

				// Get the object list
				std::vector<Object*>& objectList = scene.ObjectList();

				// Check all objects in the scene for intersection against the shadow ray
				for (Object* obj : objectList)
				{
					if (obj != NULL && obj->GetIndex() != intersect.HitObject->GetIndex())
					{
						IntersectionInfo intersection = obj->FindIntersection(shadowRay);
						if (intersection.HitObject != NULL)
						{
							fShade = 0.0f;
						}
					}
				}
			}

			// Go through all the point lights in the scene
			for (unsigned int lightIndex = 0; lightIndex < pointLightSources.size(); lightIndex++)
			{
				// Get the current light source
				PointLight& currentLight = *pointLightSources[lightIndex];

				// Calculate the intersection of the reflected ray
				glm::vec3 lightDirection = glm::normalize(currentLight.Position - intersect.IntersectionPoint);
				glm::vec3 startPoint = intersect.NormalAtIntersection * 0.1f;
				Ray shadowRay(startPoint, lightDirection);

				// Get the object list
				std::vector<Object*>& objectList = scene.ObjectList();

				// Check all objects in the scene for intersection against the shadow ray
				for (Object* obj : objectList)
				{
					if (obj != NULL && obj->GetIndex() != intersect.HitObject->GetIndex())
					{
						IntersectionInfo intersection = obj->FindIntersection(shadowRay);
						if (intersection.HitObject != NULL)
						{
							fShade = 0.0f;
						}
					}
				}
			}
		}

		// --------------------------------------------------------------------
		// Shading model

		// Calculate the color of the object based on the shading model
		sf::Color illuminationModel = FindColor(intersect, scene, fShade);
		colorAccumulator += illuminationModel;

		// --------------------------------------------------------------------
		// Reflection

		//// If the hit object is reflective or transparent and we
		//// haven't reached max reflection depth
		//if ((hitObjectMaterial.Reflectivity > 0 ||
		//	hitObjectMaterial.Transparency > 0) &&
		//	(iReflectionDepth < MAX_REFLECTION_DEPTH))
		//{
		//	// Calculate the reflected ray
		//	glm::vec3 lightDirection = glm::normalize(pointLightSources[0]->Position - intersect.IntersectionPoint);
		//	vec3 reflectionDirection = glm::reflect<vec3>(-lightDirection, intersect.NormalAtIntersection);

		//	// Calculate the intersection of the reflected ray
		//	glm::vec3 startPoint = intersect.NormalAtIntersection * 0.01f;
		//	Ray reflectionRay(startPoint, reflectionDirection);

		//	sf::Color reflectionColor = sf::Color(0, 0, 0, 255);
		//	Trace(reflectionRay, reflectionColor, scene, iReflectionDepth + 1);

		//	colorAccumulator += sf::Color((sf::Uint8)(reflectionColor.r * hitObjectMaterial.Reflectivity),
		//		(sf::Uint8)(reflectionColor.g * hitObjectMaterial.Reflectivity),
		//		(sf::Uint8)(reflectionColor.b * hitObjectMaterial.Reflectivity),
		//		255);
		//}

		// --------------------------------------------------------------------
	}
}

// -----------------------------------------------------------------------------

sf::Color FindColor(const IntersectionInfo& intersect, 
	Scene& scene,
	float fShade)
{
	sf::Color finalColor;
	Material& hitObjectMaterial = intersect.HitObject->GetMaterial();

	// Go through all directional light sources in the scene
	for (unsigned int lightIndex = 0; lightIndex < dirLightSources.size(); lightIndex++)
	{
		// Get the current light source
		DirectionalLight& currentLight = *dirLightSources[lightIndex];

		// Compute the final color
		finalColor += PhongLighting(currentLight,
			hitObjectMaterial,
			intersect.IntersectionPoint,
			intersect.NormalAtIntersection,
			fShade);
	}

	// Go through all the point lights in the scene
	for (unsigned int lightIndex = 0; lightIndex < pointLightSources.size(); lightIndex++)
	{
		// Get the current light source
		PointLight& currentLight = *pointLightSources[lightIndex];

		// Compute the final color
		finalColor += PhongLighting(currentLight,
			hitObjectMaterial,
			intersect.IntersectionPoint,
			intersect.NormalAtIntersection,
			fShade);
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

			sf::Color surfaceColor = sf::Color(0, 0, 0, 255);
			Trace(camIJRay, surfaceColor, scene, 0);
			SetPixelColor(iCurrentPixel, surfaceColor);
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
	else
	{
		// Retrieve the values from the position sliders
		int xPos, yPos, zPos;

		if (sliderXPtr != nullptr)
		{
			xPos = (int)(sliderXPtr->getValue() - SliderPositionAmplitude * 0.5f);
		}

		if (sliderYPtr != nullptr)
		{
			yPos = (int)(sliderYPtr->getValue() - SliderPositionAmplitude * 0.5f);
		}

		if (sliderZPtr != nullptr)
		{
			zPos = (int)(sliderZPtr->getValue() - SliderPositionAmplitude * 0.5f);
		}

		//std::cout << "XPos: " << xPos << "YPos: " << yPos << "ZPos: " << zPos << std::endl;

		// Get a reference to the selected point light
		int iItemIndex = comboBox->getSelectedItemIndex();
		if (iItemIndex != -1)
		{
			// Update the position of the selected point light
			PointLight* pSelectedPointLight = pointLightSources[iItemIndex];
			if (pSelectedPointLight != NULL)
			{
				pSelectedPointLight->Position = glm::vec3(xPos, yPos, zPos);
			}
		}
	}

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

	// Print camera position
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::C))
	{
		const glm::vec3 cameraPosition = pCam->GetCameraPosition();

		std::cout << "Camera position: " << cameraPosition.x << " " << cameraPosition.y << " " << cameraPosition.z << std::endl;
	}

	// ------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

int main(int argc, char **argv)
{
	// ------------------------------------------------------------------------
	bool bSuccess = gui.setGlobalFont("Lib\\TGUI\\fonts\\DejaVuSans.ttf");
	if (!bSuccess)
	{
		std::cout << "Error loading TGUI font." << std::endl;
	}

	bGUIMode = false;

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
	pCam = std::make_shared<Camera>(glm::vec3(0.0f, 3.0f, 2.0f),
		60.0f,
		90.0f);

	// ------------------------------------------------------------------------

	// Light
	DirectionalLight dirLight(glm::vec3(-1.0f, 2.0f, 0.5f),
		sf::Color(40, 40, 40, 255),
		sf::Color(40, 40, 40, 255),
		sf::Color(40, 40, 40, 255),
		5.0f, 
		"DirectionalLight1");
	PointLight pointLight1(glm::vec3(1.0f, 1.0f, 1.0f),
		sf::Color(10, 10, 10, 255),
		WhiteColor,
		WhiteColor,
		glm::vec3(0.0f, 2.0f, 1.0f),
		5.0f,
		"PointLightWhite");
	PointLight pointLight2(glm::vec3(4.0f, 1.0f, -3.0f),
		sf::Color(10, 10, 10, 255),
		BlueColor,
		WhiteColor,
		glm::vec3(1.0f, 4.0f, 1.0f),
		2.0f,
		"PointLightBlue");
	PointLight pointLight3(glm::vec3(8.0f, 1.0f, -5.0f),
		sf::Color(10, 10, 10, 255),
		RedColor,
		WhiteColor,
		glm::vec3(1.0f, 4.0f, 1.0f),
		2.0f,
		"PointLightRed");

	//dirLightSources.push_back(&dirLight);
	pointLightSources.push_back(&pointLight1);
	//pointLightSources.push_back(&pointLight2);
	//pointLightSources.push_back(&pointLight3);

	// ------------------------------------------------------------------------

	SetupWidgets(gui);

	// ------------------------------------------------------------------------
	// Scene

	Scene scene;

	//scene.AddObject(&dirLight);
	//scene.AddObject(&pointLight);

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
	sphere2SilverMat.Reflectivity = 1.0f;
	sphere2SilverMat.Transparency = 0.0f;

	Material greenRubberMat;
	memset(&greenRubberMat, 0, sizeof(Material));
	greenRubberMat.Ambient = sf::Color(0, 13, 0, 255);
	greenRubberMat.Diffuse = sf::Color(102, 128, 102, 255);
	greenRubberMat.Specular = sf::Color(10, 179, 10, 255);
	greenRubberMat.Shininess = 10.0f;
	greenRubberMat.Reflectivity = 0.01f;
	greenRubberMat.Transparency = 0.0f;

	std::shared_ptr<Sphere> pSphere = std::make_shared<Sphere>(sphere1CopperMat, 
		glm::vec3(1.0f, 1.0f, 2.0f), 
		0.2f,
		"CopperSphere");
	std::shared_ptr<Sphere> pSphere2 = std::make_shared<Sphere>(sphere2SilverMat,
		glm::vec3(0.0f, 1.0f, 2.0f),
		0.3f,
		"SilverSphere");
	std::shared_ptr<Sphere> pSphere3 = std::make_shared<Sphere>(sphere2SilverMat,
		glm::vec3(2.0f, 1.5f, 4.0f), 
		0.4f,
		"SliverSphere2");
	std::shared_ptr<Plane> pPlaneBottom = std::make_shared<Plane>(greenRubberMat,
		Normal(0.0f, 1.0f, 0.0f), 
		Point(0.0f, -3.0f, 0.0f),
		"BottomPlane");
	
	std::shared_ptr<Plane> pPlaneLeft = std::make_shared<Plane>(Normal(1.0f, 0.0f, 0.0f), Point(-10.0f, 0.0f, 0.0f));
	std::shared_ptr<Plane> pPlaneBack = std::make_shared<Plane>(Normal(0.0f, 0.0f, 1.0f), Point(0.0f, 0.0f, 10.0f));

	scene.AddObject(pSphere.get());
	//scene.AddObject(pSphere2.get());
	//scene.AddObject(pSphere3.get());
	scene.AddObject(pPlaneBottom.get());
	//scene.AddObject(pPlaneLeft.get());
	//scene.AddObject(pPlaneBack.get());

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
				if (bGUIMode == false)
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
				}
			}

			// Send the events to the GUI object
			gui.handleEvent(event);
		}

		Update(fCurrentTime);
		Draw(scene);

		window.setTitle(std::to_string(fFPS));

		// Update texture and draw
		texture.update(pixels);
		sprite.setTexture(texture);
		window.draw(sprite);

		gui.draw();

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

void SetupWidgets(tgui::Gui& gui)
{
	// ------------------------------------------------------------------------

	// Setup the background 
	tgui::Picture::Ptr background(gui);
	bool bSuccess = background->load("background.png");
	background->setSize(300, 400);
	background->setPosition(10, 10);

	// ------------------------------------------------------------------------

	// Setup the reflection level label
	tgui::Label::Ptr reflectionLevelLabel(gui);
	reflectionLevelLabel->setText("ReflectionLevel");
	reflectionLevelLabel->setPosition(12, 15);

	// ------------------------------------------------------------------------

	// Setup the reflection level edit box
	tgui::EditBox::Ptr reflectionLevelEditBox(gui, "ReflectionLevel");
	reflectionLevelEditBox->load("lib//TGUI//widgets//Black.conf");
	reflectionLevelEditBox->setSize(180, 40);
	reflectionLevelEditBox->setPosition(12, 55);

	// ------------------------------------------------------------------------

	// Setup the move speed label
	tgui::Label::Ptr moveSpeedLabel(gui);
	moveSpeedLabel->setText("MoveSpeed");
	moveSpeedLabel->setPosition(12, 95);

	// ------------------------------------------------------------------------

	// Setup the move speed edit box
	tgui::EditBox::Ptr moveSpeedEditBox(gui, "MoveSpeed");
	moveSpeedEditBox->load("lib//TGUI//widgets//Black.conf");
	moveSpeedEditBox->setSize(180, 40);
	moveSpeedEditBox->setPosition(12, 135);

	// ------------------------------------------------------------------------

	// Setup the update settings button
	tgui::Button::Ptr button(gui);
	button->load("lib//TGUI//widgets//Black.conf");
	button->setSize(100, 60);
	button->setPosition(12, 300);
	button->setText("Update");
	button->setCallbackId((int)Button::UpdateSettingsID);
	button->bindCallbackEx(updateButtonCallback, tgui::Button::LeftMouseClicked);

	// ------------------------------------------------------------------------

	comboBox = tgui::ComboBox::Ptr(gui, "PointLightList");
	comboBox->load("lib//TGUI//widgets//Black.conf");
	comboBox->setBorders(4, 4, 4, 4);
	comboBox->setSize(300, 30);
	comboBox->setPosition(12, 380);
	comboBox->bindCallbackEx(comboBoxSelectionCallback, tgui::ComboBox::ItemSelected);

	// Add point light sources
	for (unsigned int lightIndex = 0; lightIndex < pointLightSources.size(); lightIndex++)
	{
		// Get the current light source
		PointLight& currentLight = *pointLightSources[lightIndex];

		comboBox->addItem(currentLight.GetName());
	}

	comboBox->setSelectedItem(-1);

	// ------------------------------------------------------------------------

	sliderXPtr = tgui::Slider::Ptr(gui, "PointLightPosX");
	sliderXPtr->load("lib//TGUI//widgets//Black.conf");
	sliderXPtr->setVerticalScroll(false);
	sliderXPtr->setPosition(30, 200);
	sliderXPtr->setMinimum(0);
	sliderXPtr->setMaximum(SliderPositionAmplitude);

	sliderYPtr = tgui::Slider::Ptr(gui, "PointLightPosY");
	sliderYPtr->load("lib//TGUI//widgets//Black.conf");
	sliderYPtr->setVerticalScroll(false);
	sliderYPtr->setPosition(30, 240);
	sliderYPtr->setMinimum(0);
	sliderYPtr->setMaximum(SliderPositionAmplitude);

	sliderZPtr = tgui::Slider::Ptr(gui, "PointLightPosZ");
	sliderZPtr->load("lib//TGUI//widgets//Black.conf");
	sliderZPtr->setVerticalScroll(false);
	sliderZPtr->setPosition(30, 280);
	sliderZPtr->setMinimum(0);
	sliderZPtr->setMaximum(SliderPositionAmplitude);

	// ------------------------------------------------------------------------
}

// ------------------------------------------------------------------------

void updateButtonCallback(const tgui::Callback& callback)
{
	switch (callback.id)
	{
	case Button::UpdateSettingsID:
	{
		std::cout << "Settings updated." << std::endl;

		// Get the number of reflections
		tgui::EditBox::Ptr reflectionLevelEditBox = gui.get("ReflectionLevel");
		if (reflectionLevelEditBox != nullptr)
		{
			sf::String value = reflectionLevelEditBox->getText();
			if (value != "")
			{
				MAX_REFLECTION_DEPTH = std::stoi(value.toAnsiString());
				std::cout << "Reflections level count: " << MAX_REFLECTION_DEPTH << std::endl;
			}
		}

		// Get the move speed
		tgui::EditBox::Ptr moveSpeedEditBox = gui.get("MoveSpeed");
		if (moveSpeedEditBox != nullptr)
		{
			sf::String value = moveSpeedEditBox->getText();
			if (value != "")
			{
				moveSpeed = std::stof(value.toAnsiString());
				std::cout << "Move speed: " << moveSpeed << std::endl;
			}
		}

		break;
	}
	default:
		break;
	}
}

// ------------------------------------------------------------------------

void comboBoxSelectionCallback(const tgui::Callback& callback)
{
	// Get a reference to the selected point light
	int iItemIndex = comboBox->getSelectedItemIndex();
	if (iItemIndex != -1)
	{
		// Update the position of the selected point light
		PointLight* pSelectedPointLight = pointLightSources[iItemIndex];
		if (pSelectedPointLight != NULL)
		{
			glm::vec3 pos = pSelectedPointLight->Position;

			std::cout << "Point light " << iItemIndex << " selected." << std::endl;
			std::cout << "Position: " << pos.x << " " << pos.y << " " << pos.z << std::endl;

			sliderXPtr->setValue((unsigned int)(pos.x + SliderPositionAmplitude * 0.5f));
			sliderYPtr->setValue((unsigned int)(pos.y + SliderPositionAmplitude * 0.5f));
			sliderZPtr->setValue((unsigned int)(pos.z + SliderPositionAmplitude * 0.5f));
		}
	}
}

// ------------------------------------------------------------------------