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

		// Check if the ray is parallel to the plane
		float NormalDotDirection = glm::dot(normal, ray.GetDirection());
		if (fabs(NormalDotDirection) < Constants::EPS)
		{
			return false;
		}

		// Calculate the intersection point
		float d = glm::dot(normal, m_vP1);
		rayLength = -(glm::dot(normal, ray.GetOrigin()) + d) / NormalDotDirection;
		if (rayLength < 0.0f)
		{
			// Plane behind the ray
			return false;
		}
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

private:

	glm::vec3 m_vP1;
	glm::vec3 m_vP2;
	glm::vec3 m_vP3;
};

// ----------------------------------------------------------------------------

class AreaLight : public Object
{
public:
	AreaLight(const std::string& name)
		: Object(name)
	{
		m_Type = ObjectType::keAREALIGHT;

		m_fMaxX = std::numeric_limits<float>::lowest();
		m_fMinX = std::numeric_limits<float>::max();
		m_fMaxZ = std::numeric_limits<float>::lowest();
		m_fMinZ = std::numeric_limits<float>::max();
	}

	inline void AddTriangle(const Triangle& tri) 
	{
		m_vTriangleList.push_back(tri); 

		// Get the triangle's points
		glm::vec3 p1 = tri.GetP1();
		glm::vec3 p2 = tri.GetP2();
		glm::vec3 p3 = tri.GetP3();

		// Calculate the extreme points of the area light
		if (p1.x > m_fMaxX) m_fMaxX = p1.x;
		if (p1.x < m_fMinX) m_fMinX = p1.x;
		if (p1.z > m_fMaxZ) m_fMaxZ = p1.z;
		if (p1.z < m_fMinZ) m_fMinZ = p1.z;

		if (p2.x > m_fMaxX) m_fMaxX = p2.x;
		if (p2.x < m_fMinX) m_fMinX = p2.x;
		if (p2.z > m_fMaxZ) m_fMaxZ = p2.z;
		if (p2.z < m_fMinZ) m_fMinZ = p2.z;

		if (p3.x > m_fMaxX) m_fMaxX = p3.x;
		if (p3.x < m_fMinX) m_fMinX = p3.x;
		if (p3.z > m_fMaxZ) m_fMaxZ = p3.z;
		if (p3.z < m_fMinZ) m_fMinZ = p3.z;

		// Update the position, width and depth of the area light
		m_vPosition = glm::vec2(m_fMinX, m_fMinZ);
		m_fWidth = fabs(m_fMaxX - m_fMinX);
		m_fDepth = fabs(m_fMaxZ - m_fMinZ);

		// Update the sample size on the x and z direction
		m_fSampleSize_X = m_fWidth / m_uiSampleCount_X;
		m_fSampleSize_Z = m_fDepth / m_uiSampleCount_Z;
	}

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

				return intersect;
			}
		}

		// Nothing was hit
		return IntersectionInfo(vec3(0.0f), -1.0f, vec3(0.0f), NULL);
	}

	inline const glm::vec2 GetPosition() const { return m_vPosition; }
	inline const float GetWidth() const { return m_fWidth; }
	inline const float GetDepth() const { return m_fDepth; }
	inline const float GetSampleSizeX() const { return m_fSampleSize_X; }
	inline const float GetSampleSizeZ() const { return m_fSampleSize_Z; }
	inline const unsigned int GetSampleCountX() const { return m_uiSampleCount_X; }
	inline const unsigned int GetSampleCountZ() const { return m_uiSampleCount_Z; }
	inline const float GetSampleScale() const { return m_fSampleScale; }

private:
	std::vector<Triangle> m_vTriangleList;

	glm::vec2 m_vPosition;
	float m_fWidth;
	float m_fDepth;

	float m_fMaxX;
	float m_fMinX;
	float m_fMaxZ;
	float m_fMinZ;

	unsigned int m_uiSampleCount_X = 4;
	unsigned int m_uiSampleCount_Z = 4;

	float m_fSampleScale = 1.0f / (m_uiSampleCount_X * m_uiSampleCount_Z);

	float m_fSampleSize_X;
	float m_fSampleSize_Z;
};

// ----------------------------------------------------------------------------

#endif // TRIANGLE_H