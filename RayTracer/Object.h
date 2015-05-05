#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "Common.h"
#include "Ray.h"

class Object 
{
public:
	Object() 
	{
		m_uIndex = ++gObjectIndex;
	}

	Object(const Material& material)
		: m_Material(material) 
	{
		m_uIndex = ++gObjectIndex;
	}
	
	virtual ~Object() {  }
	
	virtual IntersectionInfo FindIntersection(const Ray& ray) { return IntersectionInfo(); }
	inline Material& GetMaterial() { return m_Material; }
	inline unsigned GetIndex() { return m_uIndex; }

protected:
	Material m_Material;
	unsigned m_uIndex;
};

#endif // __OBJECT_H__