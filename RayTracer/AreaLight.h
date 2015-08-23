#ifndef __AREALIGHT_H__
#define __AREALIGHT_H__

#include "Light.h"
#include "Triangle.h"
#include "Box.h"

#include <vector>

class AreaLight : public Light
{
public:
	AreaLight()
		: Light(glm::vec3(0.0f), 0.0f, "AreaLight")
	{
		// Set object type
		m_Type = ObjectType::keAREALIGHT;

		// Initialize the box dimensions and positions. Set the triangle components
		m_fDepth = 1.0f;
		m_fHeight = 0.1f;
		m_fLength = 1.0f;
		SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

		// Set the default light properties
		DiffuseLight = sf::Color(180, 210, 230, 255);
		SpecularLight = sf::Color(255, 255, 255, 255);
		AmbientLight = sf::Color(150, 150, 150, 255);
	}

	AreaLight(const glm::vec3& pos,
		float depth,
		float height,
		float length,
		const sf::Color& vAmbientLight,
		const sf::Color& vDiffuseLight,
		const sf::Color& vSpecularLight,
		const std::string& name)
		: Light(pos, 0.0f, name)
	{
		// Set object type
		m_Type = ObjectType::keAREALIGHT;

		// Initialize the box dimensions and positions. Set the triangle components
		m_fDepth = depth;
		m_fHeight = height;
		m_fLength = length;
		SetPosition(pos);

		// Set the light properties
		DiffuseLight = vDiffuseLight;
		SpecularLight = vSpecularLight;
		AmbientLight = vAmbientLight;
	}

	inline void ResetSamplePositions()
	{
		m_vSampleLocations.clear();
	}

	inline void AddSampleLocation(const glm::vec3& samplePoint)
	{
		m_vSampleLocations.push_back(samplePoint);
	}

	inline glm::vec3 GetLowerLayerPosition()
	{
		return glm::vec3(m_fMinX, m_fMinY, m_fMinZ);
	}

	inline const std::vector<glm::vec3>& GetSampleLocations() { return m_vSampleLocations; }

	inline IntersectionInfo FindIntersection(const Ray& ray)
	{
		// Check for collision against all triangles in the area light
		for (Triangle& tri : m_vTriangleList)
		{
			IntersectionInfo intersect;

			if (tri.RayTriangleIntersect(ray,
				intersect.NormalAtIntersection,
				intersect.IntersectionPoint,
				intersect.RayLength))
			{
				intersect.HitObject = this;
				intersect.NormalAtIntersection = -intersect.NormalAtIntersection;

				return intersect;
			}
		}

		// Nothing was hit
		return IntersectionInfo(vec3(0.0f), -1.0f, vec3(0.0f), NULL);
	}

	inline glm::vec3 GetPosition() { return Position; }
	inline void SetPosition(const glm::vec3& newPosition) 
	{
		Position = newPosition;

		GenerateTriangles(Position, m_fLength, m_fHeight, m_fDepth);
	}

	inline const float GetLength() const { return m_fLength; }
	inline const float GetDepth() const { return m_fDepth; }
	inline const float GetHeight() const { return m_fHeight; }
	inline const float GetSampleSizeX() const { return m_fSampleSize_X; }
	inline const float GetSampleSizeZ() const { return m_fSampleSize_Z; }
	inline const unsigned int GetSampleCountX() const { return m_uiSampleCount_X; }
	inline const unsigned int GetSampleCountZ() const { return m_uiSampleCount_Z; }
	inline const float GetSampleScale() const { return m_fSampleScale; }

	// Light color
	sf::Color AmbientLight;
	sf::Color DiffuseLight;
	sf::Color SpecularLight;

private:
	std::vector<Triangle> m_vTriangleList;
	std::vector<glm::vec3> m_vSampleLocations;

	float m_fLength;
	float m_fDepth;
	float m_fHeight;

	float m_fMaxX;
	float m_fMinX;
	float m_fMaxZ;
	float m_fMinZ;
	float m_fMaxY;
	float m_fMinY;

	unsigned int m_uiSampleCount_X = 2;
	unsigned int m_uiSampleCount_Z = 2;

	float m_fSampleScale = 1.0f / (m_uiSampleCount_X * m_uiSampleCount_Z);

	float m_fSampleSize_X;
	float m_fSampleSize_Z;

	// ---------------------------------------------------------------------------

	void GenerateTriangles(const glm::vec3& pos, float length, float height, float depth)
	{
		m_vTriangleList.clear();

		// Calculate the coordinates of the box
		glm::vec3 p1 = glm::vec3(pos.x - length / 2.0f, pos.y - height / 2.0f, pos.z - depth / 2.0f);
		glm::vec3 p2 = glm::vec3(pos.x + length / 2.0f, pos.y - height / 2.0f, pos.z - depth / 2.0f);
		glm::vec3 p3 = glm::vec3(pos.x + length / 2.0f, pos.y + height / 2.0f, pos.z - depth / 2.0f);
		glm::vec3 p4 = glm::vec3(pos.x - length / 2.0f, pos.y + height / 2.0f, pos.z - depth / 2.0f);
		glm::vec3 p5 = glm::vec3(pos.x - length / 2.0f, pos.y - height / 2.0f, pos.z + depth / 2.0f);
		glm::vec3 p6 = glm::vec3(pos.x + length / 2.0f, pos.y - height / 2.0f, pos.z + depth / 2.0f);
		glm::vec3 p7 = glm::vec3(pos.x + length / 2.0f, pos.y + height / 2.0f, pos.z + depth / 2.0f);
		glm::vec3 p8 = glm::vec3(pos.x - length / 2.0f, pos.y + height / 2.0f, pos.z + depth / 2.0f);

		m_fMinX = pos.x - length / 2.0f;
		m_fMaxX = pos.x + length / 2.0f;

		m_fMinY = pos.y - height / 2.0f;
		m_fMaxY = pos.y + height / 2.0f;

		m_fMinZ = pos.z - depth / 2.0f;
		m_fMaxZ = pos.z + depth / 2.0f;

		// Update the position, width and depth of the area light
		m_fLength = fabs(m_fMaxX - m_fMinX);
		m_fDepth = fabs(m_fMaxZ - m_fMinZ);
		m_fHeight = fabs(m_fMaxY - m_fMinY);

		// Update the sample size on the x and z direction
		m_fSampleSize_X = m_fLength / m_uiSampleCount_X;
		m_fSampleSize_Z = m_fDepth / m_uiSampleCount_Z;

		// Add the triangles to the list of triangles

		// CCW
		// Front face
		m_vTriangleList.push_back(Triangle(p1, p2, p4)); m_vTriangleList.push_back(Triangle(p2, p3, p4));

		// Right face
		m_vTriangleList.push_back(Triangle(p2, p6, p3)); m_vTriangleList.push_back(Triangle(p6, p7, p3));

		// Back face
		m_vTriangleList.push_back(Triangle(p6, p5, p7)); m_vTriangleList.push_back(Triangle(p5, p8, p7));

		// Left face
		m_vTriangleList.push_back(Triangle(p5, p1, p8)); m_vTriangleList.push_back(Triangle(p1, p4, p8));

		// Top face
		m_vTriangleList.push_back(Triangle(p4, p3, p8)); m_vTriangleList.push_back(Triangle(p3, p7, p8));

		// Bottom face
		m_vTriangleList.push_back(Triangle(p5, p6, p1)); m_vTriangleList.push_back(Triangle(p6, p2, p1));

		// CW
		//// Front face
		//m_vTriangleList.push_back(Triangle(p1, p4, p2)); m_vTriangleList.push_back(Triangle(p2, p4, p3));

		//// Right face
		//m_vTriangleList.push_back(Triangle(p2, p3, p6)); m_vTriangleList.push_back(Triangle(p6, p3, p7));

		//// Back face
		//m_vTriangleList.push_back(Triangle(p6, p7, p5)); m_vTriangleList.push_back(Triangle(p5, p7, p8));

		//// Left face
		//m_vTriangleList.push_back(Triangle(p5, p8, p1)); m_vTriangleList.push_back(Triangle(p1, p8, p4));

		//// Top face
		//m_vTriangleList.push_back(Triangle(p4, p8, p3)); m_vTriangleList.push_back(Triangle(p3, p8, p7));

		//// Bottom face
		//m_vTriangleList.push_back(Triangle(p5, p1, p6)); m_vTriangleList.push_back(Triangle(p6, p1, p2));
	}
};

#endif // __AREALIGHT_H__