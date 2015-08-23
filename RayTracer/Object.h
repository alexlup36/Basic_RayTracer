#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "Common.h"
#include "Ray.h"

// ----------------------------------------------------------------------------

class Object;

// ----------------------------------------------------------------------------

enum ObjectType
{
	kePLANE,
	keSPHERE,
	keDIRECTIONALLIGHT,
	kePOINTLIGHT,
	keAREALIGHT,
	keBOX,
};

// ----------------------------------------------------------------------------

struct IntersectionInfo
{
	IntersectionInfo()
		: RayLength(-1.0f), NormalAtIntersection(glm::vec3(0.0f)), HitObject(NULL)
	{ }

	IntersectionInfo(const vec3& intersectionPoint,
		float rayIntersectionLength,
		const Normal& normal,
		Object* hitObject)
		: RayLength(rayIntersectionLength),
		IntersectionPoint(intersectionPoint),
		NormalAtIntersection(normal),
		HitObject(hitObject)
	{ }

	vec3 IntersectionPoint;
	Normal NormalAtIntersection;
	float RayLength;
	Object* HitObject;
};

// ----------------------------------------------------------------------------

class Object 
{
public:

	Object();
	Object(const std::string& objectname);
	Object(const Material& material,
		const std::string& objectname);
	
	virtual ~Object() { }
	
	virtual IntersectionInfo FindIntersection(const Ray& ray) { return IntersectionInfo(); }

	virtual glm::vec3 GetPosition() = 0;
	virtual void SetPosition(const glm::vec3& newPosition) = 0;

	inline Material& GetMaterial() { return m_Material; }

	inline unsigned GetIndex() const { return m_uIndex; }

	inline std::string GetName() const { return m_sName; }

	inline const ObjectType Type() const { return m_Type; }

protected:

	unsigned m_uIndex;

	ObjectType m_Type;

	std::string m_sName;

	Material m_Material;

	static unsigned int iGlobalIndex;
};

// ----------------------------------------------------------------------------

#endif // __OBJECT_H__