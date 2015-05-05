#ifndef __RAY_H__
#define __RAY_H__

#include "Common.h"

class Ray
{
public:
	Ray(const vec3& orig = vec3(0.0f), 
		const vec3& dir = vec3(1.0f, 0.0f, 0.0f))
		: m_vOrigin(orig), m_vDirection(glm::normalize(dir)) { }
		
	inline void SetOrigin(const vec3& origin)
	{
		m_vOrigin = origin;
	}
	
	inline void SetDirection(const vec3& direction)
	{
		m_vDirection = direction;
	}
	
	inline const vec3& GetOrigin() const { return m_vOrigin; }
	inline const vec3& GetDirection() const { return m_vDirection; }
	
private:
	vec3 m_vOrigin;
	vec3 m_vDirection;
};

#endif // __RAY_H__