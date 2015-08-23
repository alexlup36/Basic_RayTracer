#include "Object.h"

unsigned int Object::iGlobalIndex = 0;

Object::Object()
{
	m_uIndex = ++iGlobalIndex;
	m_sName = "Object" + std::to_string(m_uIndex);
}

Object::Object(const std::string& objectname)
{
	m_uIndex = ++iGlobalIndex;
	m_sName = objectname + std::to_string(m_uIndex);
}

Object::Object(const Material& material,
	const std::string& objectname)
	: m_Material(material)
{
	m_uIndex = ++iGlobalIndex;
	m_sName = objectname;
}