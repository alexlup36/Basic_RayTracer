#ifndef __PLANE_H__
#define __PLANE_H__

#include "Common.h"
#include "Object.h"
#include "Ray.h"
#include "Constants.h"

class Plane : public Object
{
public:
	Plane(const Normal& normal = Normal(0.0f, 1.0f, 0.0f),
		const Point& point = Point(0.0f, 0.0f, 0.0f))
		: m_vNormal(glm::normalize(normal)), m_vPointOnPlane(point) 
	{
		m_Type = ObjectType::kePLANE;
	}

	Plane(const Material& mat,
		const Normal& normal,
		const Point& point,
		const std::string& name)
		: m_vNormal(glm::normalize(normal)), m_vPointOnPlane(point), Object(mat, name) 
	{
		m_Type = ObjectType::keSPHERE;
	}

	// Getter functions
	inline Normal GetNormal() { return m_vNormal; }
	inline Point GetPointOnPlane() { return m_vPointOnPlane;}
	
	// Find distance from the Camera to the intersection point
	inline IntersectionInfo FindIntersection(const Ray& ray)
	{
		float fNormalDotRay = glm::dot(m_vNormal, ray.GetDirection());
		
		if (fNormalDotRay == 0.0f)
		{
			// The ray doesn't intersect the plane
			return IntersectionInfo(vec3(0.0f), -1.0f, vec3(0.0f), NULL);
		}
		else
		{
			float d = -glm::dot(m_vPointOnPlane, m_vNormal);
			float fRayOrigDotNormal = glm::dot(ray.GetOrigin(), m_vNormal);
			float fRayDirDotNormal = glm::dot(ray.GetDirection(), m_vNormal);
			float t = -(fRayOrigDotNormal + d) / fRayDirDotNormal;

			if (t > Constants::EPS)
			{
				vec3 IntersectionPoint = ray.GetOrigin() + t * ray.GetDirection();

				return IntersectionInfo(IntersectionPoint,
					glm::length(t * ray.GetDirection()),
					m_vNormal,
					this);
			}
			else
			{
				return IntersectionInfo(vec3(0.0f), -1.0f, vec3(0.0f), NULL);
			}
			
		}
	}

private:
	Normal m_vNormal;
	Point m_vPointOnPlane;
};

#endif // __PLANE_H__