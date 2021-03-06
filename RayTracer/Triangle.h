#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <limits>

#include "Common.h"
#include "Ray.h"
#include "Constants.h"
#include "Light.h"

// ----------------------------------------------------------------------------

class Triangle
{
public:
	
	Triangle(const glm::vec3& p1,
		const glm::vec3& p2,
		const glm::vec3& p3)
		: m_vP1(p1), m_vP2(p2), m_vP3(p3)
	{

	}

	inline const glm::vec3& GetP1() const { return m_vP1; }
	inline const glm::vec3& GetP2() const { return m_vP2; }
	inline const glm::vec3& GetP3() const { return m_vP3; }

	inline bool RayPlaneIntersect(const Ray& ray,
		const glm::vec3& normal,
		float& t)
	{
		float normalDotDir = glm::dot(normal, ray.GetDirection());
		if (normalDotDir > Constants::EPS)
		{
			glm::vec3 v = m_vP1 - ray.GetOrigin();
			t = glm::dot(v, normal) / normalDotDir;

			return (t >= 0.0f);
		}

		return false;
	}

	inline bool RayTriangleIntersect(const Ray& ray, 
		glm::vec3& normal,
		glm::vec3& intersectionPoint, 
		float& rayLength)
	{
		// Calculate triangle's edges
		glm::vec3 p1p2 = m_vP2 - m_vP1;
		glm::vec3 p1p3 = m_vP3 - m_vP1;

		// Calculate plane's normal
		normal = glm::normalize(glm::cross(p1p2, p1p3));

		if (RayPlaneIntersect(ray, normal, rayLength) == true)
		{
			intersectionPoint = ray.GetOrigin() + rayLength * ray.GetDirection();

			// Check if the intersection point is inside the triangle

			// Edge 1
			glm::vec3 edge1 = m_vP2 - m_vP1;
			glm::vec3 p1IntersectionPoint = intersectionPoint - m_vP1;
			glm::vec3 c1 = glm::cross(edge1, p1IntersectionPoint);
			if (glm::dot(normal, c1) < 0.0f) return false;

			// Edge 2
			glm::vec3 edge2 = m_vP3 - m_vP2;
			glm::vec3 p2IntersectionPoint = intersectionPoint - m_vP2;
			glm::vec3 c2 = glm::cross(edge2, p2IntersectionPoint);
			if (glm::dot(normal, c2) < 0.0f) return false;

			// Edge 3
			glm::vec3 edge3 = m_vP1 - m_vP3;
			glm::vec3 p3IntersectionPoint = intersectionPoint - m_vP3;
			glm::vec3 c3 = glm::cross(edge3, p3IntersectionPoint);
			if (glm::dot(normal, c3) < 0.0f) return false;

			return true;
		}

		return false;
	}

private:

	glm::vec3 m_vP1;
	glm::vec3 m_vP2;
	glm::vec3 m_vP3;
};

// ----------------------------------------------------------------------------

//class AreaLight : public Light
//{
//public:
//	AreaLight()
//		: Light(glm::vec3(0.0f), 0.0f, "AreaLight")
//	{
//		Position = vec3(0.0f, 0.0f, 0.0f);
//
//		DiffuseLight = sf::Color(180, 210, 230, 255);
//		SpecularLight = sf::Color(255, 255, 255, 255);
//		AmbientLight = sf::Color(150, 150, 150, 255);
//
//		m_Type = ObjectType::keAREALIGHT;
//	}
//
//	AreaLight(const sf::Color& vAmbientLight,
//		const sf::Color& vDiffuseLight,
//		const sf::Color& vSpecularLight, 
//		const std::string& name)
//		: Light(glm::vec3(0.0f), 0.0f, name)
//	{
//		m_Type = ObjectType::keAREALIGHT;
//
//		DiffuseLight = vDiffuseLight;
//		SpecularLight = vSpecularLight;
//		AmbientLight = vAmbientLight;
//
//		m_fMaxX = std::numeric_limits<float>::lowest();
//		m_fMinX = std::numeric_limits<float>::max();
//		m_fMaxZ = std::numeric_limits<float>::lowest();
//		m_fMinZ = std::numeric_limits<float>::max();
//	}
//
//	inline void AddTriangle(const Triangle& tri) 
//	{
//		m_vTriangleList.push_back(tri); 
//
//		// Get the triangle's points
//		glm::vec3 p1 = tri.GetP1();
//		glm::vec3 p2 = tri.GetP2();
//		glm::vec3 p3 = tri.GetP3();
//
//		// Calculate the extreme points of the area light
//		if (p1.x > m_fMaxX) m_fMaxX = p1.x;
//		if (p1.x < m_fMinX) m_fMinX = p1.x;
//		if (p1.z > m_fMaxZ) m_fMaxZ = p1.z;
//		if (p1.z < m_fMinZ) m_fMinZ = p1.z;
//
//		if (p2.x > m_fMaxX) m_fMaxX = p2.x;
//		if (p2.x < m_fMinX) m_fMinX = p2.x;
//		if (p2.z > m_fMaxZ) m_fMaxZ = p2.z;
//		if (p2.z < m_fMinZ) m_fMinZ = p2.z;
//
//		if (p3.x > m_fMaxX) m_fMaxX = p3.x;
//		if (p3.x < m_fMinX) m_fMinX = p3.x;
//		if (p3.z > m_fMaxZ) m_fMaxZ = p3.z;
//		if (p3.z < m_fMinZ) m_fMinZ = p3.z;
//
//		// Update the position, width and depth of the area light
//		m_fWidth = fabs(m_fMaxX - m_fMinX);
//		m_fDepth = fabs(m_fMaxZ - m_fMinZ);
//
//		// Update the sample size on the x and z direction
//		m_fSampleSize_X = m_fWidth / m_uiSampleCount_X;
//		m_fSampleSize_Z = m_fDepth / m_uiSampleCount_Z;
//
//		m_fHeight = p1.y;
//
//		Position = glm::vec3(m_fMinX, m_fHeight, m_fMinZ);
//	}
//
//	inline void ResetSamplePositions()
//	{
//		m_vSampleLocations.clear();
//	}
//
//	inline void AddSampleLocation(const glm::vec3& samplePoint)
//	{
//		m_vSampleLocations.push_back(samplePoint);
//	}
//
//	inline const std::vector<glm::vec3>& GetSampleLocations() { return m_vSampleLocations; }
//
//	inline IntersectionInfo FindIntersection(const Ray& ray)
//	{
//		// Check for collision against all triangles in the area light
//		for (Triangle& tri : m_vTriangleList)
//		{
//			IntersectionInfo intersect;
//
//			if (tri.RayTriangleIntersect(ray, 
//				intersect.NormalAtIntersection, 
//				intersect.IntersectionPoint,
//				intersect.RayLength))
//			{
//				intersect.HitObject = this;
//
//				return intersect;
//			}
//		}
//
//		// Nothing was hit
//		return IntersectionInfo(vec3(0.0f), -1.0f, vec3(0.0f), NULL);
//	}
//
//	inline glm::vec3 GetPosition() { return Position; }
//	inline void SetPosition(const glm::vec3& newPosition) {}
//
//	inline const float GetWidth() const { return m_fWidth; }
//	inline const float GetDepth() const { return m_fDepth; }
//	inline const float GetSampleSizeX() const { return m_fSampleSize_X; }
//	inline const float GetSampleSizeZ() const { return m_fSampleSize_Z; }
//	inline const unsigned int GetSampleCountX() const { return m_uiSampleCount_X; }
//	inline const unsigned int GetSampleCountZ() const { return m_uiSampleCount_Z; }
//	inline const float GetSampleScale() const { return m_fSampleScale; }
//
//	sf::Color AmbientLight;
//	sf::Color DiffuseLight;
//	sf::Color SpecularLight;
//
//private:
//	std::vector<Triangle> m_vTriangleList;
//	std::vector<glm::vec3> m_vSampleLocations;
//
//	float m_fWidth;
//	float m_fDepth;
//	float m_fHeight;
//
//	float m_fMaxX;
//	float m_fMinX;
//	float m_fMaxZ;
//	float m_fMinZ;
//
//	unsigned int m_uiSampleCount_X = 1;
//	unsigned int m_uiSampleCount_Z = 1;
//
//	float m_fSampleScale = 1.0f / (m_uiSampleCount_X * m_uiSampleCount_Z);
//
//	float m_fSampleSize_X;
//	float m_fSampleSize_Z;
//};

// ----------------------------------------------------------------------------

#endif // TRIANGLE_H