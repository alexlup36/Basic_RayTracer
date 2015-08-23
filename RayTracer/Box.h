#ifndef __BOX_H__
#define __BOX_H__

#include "Common.h"
#include "Object.h"
#include "Triangle.h"
#include <vector>

class Box : public Object
{
public:
	Box(const glm::vec3& pos = glm::vec3(0.0f),
		float length = 1.0f,
		float depth = 3.0f,
		float height = 2.0f)
		: m_vPosition(pos), m_fLength(length), m_fDepth(depth), m_fHeight(height), Object("Box")
	{
		m_Type = ObjectType::keBOX;

		GenerateTriangles(pos, length, height, depth);
	}

	Box(const Material& mat,
		const glm::vec3& pos,
		float length,
		float depth,
		float height,
		const std::string& name)
		: m_vPosition(pos), m_fLength(length), m_fDepth(depth), m_fHeight(height), Object(mat, name)
	{
		m_Type = ObjectType::keBOX;

		GenerateTriangles(pos, length, height, depth);
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
				intersect.NormalAtIntersection = -intersect.NormalAtIntersection;

				return intersect;
			}
		}

		// Nothing was hit
		return IntersectionInfo(vec3(0.0f), -1.0f, vec3(0.0f), NULL);
	}

	inline glm::vec3 GetPosition() override { return m_vPosition; }
	inline const float GetDepth() const { return m_fDepth; }
	inline const float GetHeight() const { return m_fHeight; }
	inline const float GetLength() const { return m_fLength; }

	inline void SetPosition(const glm::vec3& newPosition) 
	{
		m_vPosition = newPosition;
		GenerateTriangles(m_vPosition, m_fLength, m_fHeight, m_fDepth);
	}
	inline void SetLength(float newLength)
	{
		m_fLength = newLength;
		GenerateTriangles(m_vPosition, m_fLength, m_fHeight, m_fDepth);
	}
	inline void SetDepth(float newDepth)
	{
		m_fDepth = newDepth;
		GenerateTriangles(m_vPosition, m_fLength, m_fHeight, m_fDepth);
	}
	inline void SetHeight(float newHeight)
	{
		m_fHeight = newHeight;
		GenerateTriangles(m_vPosition, m_fLength, m_fHeight, m_fDepth);
	}
	
private:
	std::vector<Triangle> m_vTriangleList;

	glm::vec3 m_vPosition;
	float m_fLength;
	float m_fDepth;
	float m_fHeight;

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

		// Add the triangles to the list of triangles

		//// CCW
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

#endif // __BOX_H__