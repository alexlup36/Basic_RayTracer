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
	}

	Sphere(const Material& mat,
		const vec3& pos,
		float radius)
		: m_vCenter(pos), m_fRadius(radius)
	{
		m_Material = mat;

		m_fSqRadius = m_fRadius * m_fRadius;
	}

	// Getter functions
	inline vec3 GetCenter() { return m_vCenter; }
	inline float GetRadius() { return m_fRadius; }
	
	// Find distance from the Camera to the intersection point
	inline IntersectionInfo FindIntersection(const Ray& ray)
	{
		vec3 m = ray.GetOrigin() - m_vCenter;
		float b = glm::dot(m, ray.GetDirection());
		float c = glm::dot(m, m) - m_fSqRadius;
		
		if (c > 0.0f && b > 0.0f)
		{
			// r's origin is outside s and r is pointing away from s
			return IntersectionInfo(vec3(0.0f), -1.0f, vec3(0.0f), NULL);
		}
		
		float fDiscr = b * b - c;
		if (fDiscr < 0.0f)
		{
			return IntersectionInfo(vec3(0.0f), -1.0f, vec3(0.0f), NULL);
		}
		
		float sqrtDiscr = sqrt(fDiscr);
		
		float t1 = -b - sqrtDiscr + fEPS;
		float t2 = -b + sqrtDiscr + fEPS;
		
		if (t1 >= 0)
		{
			if (t2 >= 0)
			{
				if (t1 < t2)
				{
					vec3 IntersectionPoint = ray.GetOrigin() + t1 * ray.GetDirection();

					return IntersectionInfo(IntersectionPoint,
						t1,
						glm::normalize(IntersectionPoint - m_vCenter), 
						this);
				}
				else
				{
					vec3 IntersectionPoint = ray.GetOrigin() + t1 * ray.GetDirection();

					return IntersectionInfo(IntersectionPoint,
						t2,
						glm::normalize(IntersectionPoint - m_vCenter),
						this);
				}
			}
			else
			{
				vec3 IntersectionPoint = ray.GetOrigin() + t1 * ray.GetDirection();

				return IntersectionInfo(IntersectionPoint,
					t1,
					glm::normalize(IntersectionPoint - m_vCenter),
					this);
			}
		}
		else
		{
			vec3 IntersectionPoint = ray.GetOrigin() + t1 * ray.GetDirection();

			return IntersectionInfo(IntersectionPoint,
				t2,
				glm::normalize(IntersectionPoint - m_vCenter),
				this);
		}
	}
	
private:
	vec3 m_vCenter;
	float m_fRadius;
	float m_fSqRadius;
};

#endif // __SPHERE_H__