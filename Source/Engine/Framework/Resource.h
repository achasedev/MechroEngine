///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 25th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Utility/SmartPointer.h"
#include "Engine/Utility/StringId.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
template <typename T>
class Resource
{
public:
	//----Public Methods-----

	virtual void	Load(const char* filepath) = 0;
	virtual void	Refresh() = 0;
	virtual R<T>	Copy() = 0;
	virtual void	Clear() = 0;

	StringId		GetStringID() const { return m_id; }
	const char*		GetNameOrFilepath() const { return m_id.ToString(); }
	
	static R<T>		GetResource(const char* filepathOrName);
	static R<T>		GetResource(const StringId& id);
	static R<T>		CreateOrGetResource(const char* filepathOrName);
	static bool		AddResource(R<T> resource);


private:
	//-----Private Data-----

	StringId m_id;

	static std::map<StringId, R<T>> s_resources;

};


//-------------------------------------------------------------------------------------------------
template <typename T>
std::map<StringId, R<T>> Resource<T>::s_resources;


//-------------------------------------------------------------------------------------------------
template <typename T>
R<T> Resource<T>::GetResource(const char* filepathOrName)
{
	StringId id = SID(filepathOrName);
	return GetResource<T>(id);
}


//-------------------------------------------------------------------------------------------------
template <typename T>
R<T> Resource<T>::GetResource(const StringId& id)
{
	bool resourceExists = (s_resources.find(id) != s_resources.end());

	if (resourceExists)
	{
		return s_resources[id];
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
template <typename T>
R<T> Resource<T>::CreateOrGetResource(const char* nameOrFilepath)
{
	T* resource = Resource<T>::GetResource(nameOrFilepath);

	if (resource == nullptr)
	{
		resource = new T();
		resource->Load(nameOrFilepath);

		Resource<T>::AddResource(nameOrFilepath, resource);
	}

	return resource;
}


//-------------------------------------------------------------------------------------------------
template <typename T>
bool Resource<T>::AddResource(R<T> resource)
{
	bool resourceAlreadyExists = s_resources.find(resource->m_id) != s_resources.end();

	if (!resourceAlreadyExists)
	{
		s_resources[resource->m_id] = resource;
		return true;
	}

	return false;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
