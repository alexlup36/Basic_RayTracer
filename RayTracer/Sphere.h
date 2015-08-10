#ifndef __SPHERE_H__
#define __SPHERE_H__

#include "Common.h"
#include "Object.h"

class Sphere : public Object
{
public:
	Sphere(const vec3& pos = glm::vec3(0.0f),
		float radius = 1.0f)
		: m_vCenter(pos),  m_fRadius(radius) 
	{
		m_fSqRadius = m_fRadius * m_fRadius;

		m_Type = ObjectType::keSPHERE;
	}

	Sphere(const Material& mat,
		const vec3& pos,
		float radius,
		const std::string& name)
		: m_vCenter(pos), m_fRadius(radius), Object(mat, name)
	{
		m_fSqRadius = m_fRadius * m_fRadius;

		m_Type = ObjectType::keSPHERE;
	}

	// Getter functions
	inline vec3 GetCenter() { return m_vCenter; }
	inline float GetRadius() { return m_fRadius; }
	
	//inline IntersectionInfo FindIntersection(const Ray& ray)
	//{
	//	vec3 m = ray.GetOrigin() - m_vCenter;
	//	float b = glm::dot(m, ray.GetDirection());
	//	float c = glm::dot(m, m) - m_fSqRadius;
	//	
	//	if (c > 0.0f && b > 0.0f)
	//	{
	//		// r's origin is outside s and r is pointing away from s
	//		return IntersectionInfo(vec3(0.0f), -1.0f, vec3(0.0f), NULL);
	//	}
	//	
	//	float fDiscr = b * b - c;
	//	if (fDiscr < 0.0f)
	//	{
	//		return IntersectionInfo(vec3(0.0f), -1.0f, vec3(0.0f), NULL);
	//	}
	//	
	//	float sqrtDiscr = sqrt(fDiscr);
	//	
	//	float t1 = -b - sqrtDiscr;
	//	float t2 = -b + sqrtDiscr;
	//	
	//	if (t1 >= 0)
	//	{
	//		if (t2 >= 0)
	//		{
	//			if (t1 < t2)
	//			{
	//				vec3 IntersectionPoint = ray.GetOrigin() + t1 * ray.GetDirection();

	//				return IntersectionInfo(IntersectionPoint,
	//					t1,
	//					glm::normalize(IntersectionPoint - m_vCenter), 
	//					this);
	//			}
	//			else
	//			{
	//				vec3 IntersectionPoint = ray.GetOrigin() + t1 * ray.GetDirection();

	//				return IntersectionInfo(IntersectionPoint,
	//					t2,
	//					glm::normalize(IntersectionPoint - m_vCenter),
	//					this);
	//			}
	//		}
	//		else
	//		{
	//			vec3 IntersectionPoint = ray.GetOrigin() + t1 * ray.GetDirection();

	//			return IntersectionInfo(IntersectionPoint,
	//				t1,
	//				glm::normalize(IntersectionPoint - m_vCenter),
	//				this);
	//		}
	//	}
	//	else
	//	{
	//		vec3 IntersectionPoint = ray.GetOrigin() + t1 * ray.GetDirection();

	//		return IntersectionInfo(IntersectionPoint,
	//			t2,
	//			glm::normalize(IntersectionPoint - m_vCenter),
	//			this);
	//	}
	//}

	inline IntersectionInfo FindIntersection(const Ray& ray)
	{
		// Solutions
		float t1, t2;

		// Calculate quadratic's coefficients
		glm::vec3 L = ray.GetOrigin() - m_vCenter;
		float a = glm::dot(ray.GetDirection(), ray.GetDirection());
		float b = 2.0f * glm::dot(ray.GetDirection(), L);
		float c = glm::dot(L, L) - m_fSqRadius;

		if (SolveQuadratic(a, b, c, t1, t2) == false)
		{
			// No solution for the quadratic eq
			return IntersectionInfo(vec3(0.0f), -1.0f, vec3(0.0f), NULL);
		}

		if (t1 < 0.0f)
		{
			t1 = t2;
			if (t1 < 0.0f)
			{
				// The ray intersects the sphere behind the origin
				return IntersectionInfo(vec3(0.0f), -1.0f, vec3(0.0f), NULL);
			}
		}

		glm::vec3 IntersectionPoint = ray.GetOrigin() + t1 * ray.GetDirection();
		return IntersectionInfo(IntersectionPoint,
						t1,
						glm::normalize(IntersectionPoint - m_vCenter),
						this);
	}
	
private:
	vec3 m_vCenter;
	float m_fRadius;
	float m_fSqRadius;
};

#endif // __SPHERE_H__