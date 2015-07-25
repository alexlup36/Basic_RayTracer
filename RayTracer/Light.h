#ifndef __LIGHT_H__
#define __LIGHT_H__

// -----------------------------------------------------------------------

#include "Object.h"

#include <string>

// -----------------------------------------------------------------------

class Light : public Object
{
public:
	Light(const vec3& pos = glm::vec3(0.0f),
		float radius = 1.0f)
		: m_vPosition(pos), m_fRadius(radius)
	{
		m_fRenderRadius = 0.1f;

		m_fSqRadius = m_fRenderRadius * m_fRenderRadius;
	}

	Light(const vec3& pos,
		float radius,
		const std::string& name)
		: m_vPosition(pos), m_fRadius(radius), Object(name)
	{
		m_fRenderRadius = 0.1f;

		m_fSqRadius = m_fRenderRadius * m_fRenderRadius;
	}

	// Getter functions
	inline vec3 GetPosition() { return m_vPosition; }
	inline float GetRadius() { return m_fRadius; }

	// Find distance from the Camera to the intersection point
	inline IntersectionInfo FindIntersection(const Ray& ray)
	{
		vec3 m = ray.GetOrigin() - m_vPosition;
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

		float t1 = -b - sqrtDiscr;
		float t2 = -b + sqrtDiscr;

		if (t1 >= 0)
		{
			if (t2 >= 0)
			{
				if (t1 < t2)
				{
					vec3 IntersectionPoint = ray.GetOrigin() + t1 * ray.GetDirection();

					return IntersectionInfo(IntersectionPoint,
						t1,
						glm::normalize(IntersectionPoint - m_vPosition),
						this);
				}
				else
				{
					vec3 IntersectionPoint = ray.GetOrigin() + t1 * ray.GetDirection();

					return IntersectionInfo(IntersectionPoint,
						t2,
						glm::normalize(IntersectionPoint - m_vPosition),
						this);
				}
			}
			else
			{
				vec3 IntersectionPoint = ray.GetOrigin() + t1 * ray.GetDirection();

				return IntersectionInfo(IntersectionPoint,
					t1,
					glm::normalize(IntersectionPoint - m_vPosition),
					this);
			}
		}
		else
		{
			vec3 IntersectionPoint = ray.GetOrigin() + t1 * ray.GetDirection();

			return IntersectionInfo(IntersectionPoint,
				t2,
				glm::normalize(IntersectionPoint - m_vPosition),
				this);
		}
	}

protected:

	vec3 m_vPosition;
	float m_fRadius;
	float m_fSqRadius;
	float m_fRenderRadius;
};

// -----------------------------------------------------------------------

#endif // __LIGHT_H__