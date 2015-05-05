#ifndef __SCENE_H__
#define __SCENE_H__

#include <vector>

#include "Object.h"

class Scene
{
public:
	Scene() {}
	~Scene() { }
	
	inline std::vector<Object*>& ObjectList() { return m_ObjectList; }
	inline void AddObject(Object* newObject)
	{
		m_ObjectList.push_back(newObject);
	}
	
private:
	std::vector<Object*> m_ObjectList;
};

#endif // __SCENE_H__