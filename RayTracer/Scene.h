#ifndef __SCENE_H__
#define __SCENE_H__

#include <vector>

#include "Object.h"

#include "PointLight.h"
#include "DirectionalLight.h"
#include "Triangle.h"
#include "AreaLight.h"

class Scene
{
public:
	Scene() {}
	~Scene() 
	{
		for (size_t index = 0; index < m_ObjectList.size(); index++)
		{
			if (m_ObjectList[index] != nullptr)
			{
				delete m_ObjectList[index];
				m_ObjectList[index] = nullptr;
			}
		}
	}
	
	// ---------------------------------------------------------------------------

	inline std::vector<Object*>& ObjectList() { return m_ObjectList; }
	inline std::vector<PointLight*>& PointLightList() { return m_PointLightList; }
	inline std::vector<DirectionalLight*>& DirectionalLightList() { return m_DirectionalLightList; }
	inline std::vector<AreaLight*>& AreaLightList() { return m_AreaLightList; }

	// ---------------------------------------------------------------------------

	inline void AddObject(Object* newObject)
	{
		// Add object to the general list
		m_ObjectList.push_back(newObject);

		// Add point light reference
		if (newObject->Type() == ObjectType::kePOINTLIGHT)
		{
			// Cast object to point light object
			PointLight* pPointLight = dynamic_cast<PointLight*>(newObject);

			// Add object to the point light list
			if (pPointLight != nullptr)
			{
				m_PointLightList.push_back(pPointLight);
			}
		}

		// Add directional light reference
		if (newObject->Type() == ObjectType::keDIRECTIONALLIGHT)
		{
			// Cast object to directional light object
			DirectionalLight* pDirectionalLight = dynamic_cast<DirectionalLight*>(newObject);

			// Add object to the directional light list
			if (pDirectionalLight != nullptr)
			{
				m_DirectionalLightList.push_back(pDirectionalLight);
			}
		}

		// Add area light reference
		if (newObject->Type() == ObjectType::keAREALIGHT)
		{
			// Cast object to area light object
			AreaLight* pAreaLight = dynamic_cast<AreaLight*>(newObject);

			// Add object to the area light list
			if (pAreaLight != nullptr)
			{
				m_AreaLightList.push_back(pAreaLight);
			}
		}
	}

	// ---------------------------------------------------------------------------
	
private:
	std::vector<Object*> m_ObjectList;

	std::vector<PointLight*>		m_PointLightList;
	std::vector<DirectionalLight*>	m_DirectionalLightList;
	std::vector<AreaLight*>			m_AreaLightList;
};

#endif // __SCENE_H__