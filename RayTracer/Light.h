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
		: Position(pos), Radius(radius)
	{
		RenderRadius = 0.1f;

		m_fSqRadius = RenderRadius * RenderRadius;
	}

	Light(const vec3& pos,
		float radius,
		const std::string& name)
		: Position(pos), Radius(radius), Object(name)
	{
		RenderRadius = 0.1f;

		m_fSqRadius = RenderRadius * RenderRadius;
	}

	// Find distance from the Camera to the intersection point
	inline IntersectionInfo FindIntersection(const Ray& ray)
	{
		vec3 m = ray.GetOrigin() - Position;
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
						glm::normalize(IntersectionPoint - Position),
						this);
				}
				else
				{
					vec3 IntersectionPoint = ray.GetOrigin() + t1 * ray.GetDirection();

					return IntersectionInfo(IntersectionPoint,
						t2,
						glm::normalize(IntersectionPoint - Position),
						this);
				}
			}
			else
			{
				vec3 IntersectionPoint = ray.GetOrigin() + t1 * ray.GetDirection();

				return IntersectionInfo(IntersectionPoint,
					t1,
					glm::normalize(IntersectionPoint - Position),
					this);
			}
		}
		else
		{
			vec3 IntersectionPoint = ray.GetOrigin() + t1 * ray.GetDirection();

			return IntersectionInfo(IntersectionPoint,
				t2,
				glm::normalize(IntersectionPoint - Position),
				this);
		}
	}

protected:

	float m_fSqRadius;

public:

	glm::vec3 Position;
	float Radius;
	float RenderRadius;
};

// -----------------------------------------------------------------------

#endif // __LIGHT_H__