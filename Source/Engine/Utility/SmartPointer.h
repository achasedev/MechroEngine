///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 29th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include <map>
#include <mutex>

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
class RefCount
{
public:
	//-----Public Methods-----

	RefCount(void* pointer);

	uint32				AddRef();
	uint32				Release();
	static RefCount*	CreateOrGetRefCount(void* pointer);


private:
	//-----Private Data-----

	std::mutex	m_lock;
	uint32		m_count = 0;
	void*		m_pointer = nullptr;

	// Used to prevent duplicate SmartPointer conflicts
	static std::mutex					s_registryLock;
	static std::map<void*, RefCount*>	s_refCountRegistry;

};


//-------------------------------------------------------------------------------------------------
template <typename T>
class SmartPointer
{
public:
	//-----Public Methods-----

	SmartPointer() {}
	SmartPointer(T* pointer);
	SmartPointer(const SmartPointer<T>& copy);
	~SmartPointer();

	bool	IsValid() const { return m_pointer != nullptr; }

	T&		operator*()		{ return *m_pointer; }
	T*		operator->()	{ return m_pointer; }

	void	operator=(const SmartPointer<T>& copy);
	void	operator=(T* pointer);
	bool	operator==(const SmartPointer<T>& compare) const;
	bool	operator==(const T* compare) const;
	bool	operator!=(const T* compare) const;


private:
	//-----Private Methods-----

	bool	HasReference() const { return m_refCount != nullptr; }
	void	Release();


private:
	//-----Private Data-----

	T*			m_pointer = nullptr;
	RefCount*	m_refCount = nullptr;

};


// For cleaner looking code when using them
template <typename T>
using R = SmartPointer<T>;


//-------------------------------------------------------------------------------------------------
template <typename T>
SmartPointer<T>::SmartPointer(T* pointer)
{
	if (pointer != nullptr)
	{
		m_pointer = pointer;
		m_refCount = RefCount::CreateOrGetRefCount(pointer);
		m_refCount->AddRef();
	}
}


//-------------------------------------------------------------------------------------------------
template <typename T>
SmartPointer<T>::SmartPointer(const SmartPointer<T>& copy)
{
	// Check for self copy
	ASSERT_RETURN(this != &copy, NO_RETURN_VAL, "Smartpointer self assignment!");

	// Check for whether it points to the same thing
	if (m_pointer == copy.m_pointer)
	{
		return;
	}

	// Release my resources
	if (HasReference())
	{
		Release();
	}

	m_pointer = copy.m_pointer;
	m_refCount = copy.m_refCount;

	// Should never happen!
	ASSERT_OR_DIE(m_refCount != nullptr, "SmartPointer had nullptr RefCount!");

	if (m_refCount != nullptr)
	{
		m_refCount->AddRef();
	}
}


//-------------------------------------------------------------------------------------------------
template <typename T>
SmartPointer<T>::~SmartPointer()
{
	Release();
}


//-------------------------------------------------------------------------------------------------
template <typename T>
void SmartPointer<T>::operator=(const SmartPointer<T>& copy)
{
	ASSERT_OR_DIE(this != &copy, "Smartpointer self assignment!");

	// Check for whether it points to the same thing
	if (m_pointer == copy.m_pointer)
	{
		return;
	}

	// Release my resources
	if (HasReference())
	{
		Release();
	}

	m_pointer = copy.m_pointer;
	m_refCount = copy.m_refCount;

	if (m_refCount != nullptr)
	{
		m_refCount->AddRef();
	}
}


//-------------------------------------------------------------------------------------------------
template <typename T>
void SmartPointer<T>::operator=(T* pointer)
{
	// Check for whether it points to the same thing
	if (m_pointer == pointer)
	{
		return;
	}
	
	// Release my resources
	if (HasReference())
	{
		Release();
	}
	
	m_pointer = pointer;

	if (m_pointer != nullptr)
	{
		m_refCount = RefCount::CreateOrGetRefCount(pointer);
		m_refCount->AddRef();
	}
}


//-------------------------------------------------------------------------------------------------
template <typename T>
bool SmartPointer<T>::operator==(const SmartPointer<T>& compare) const
{
	return (m_pointer == compare.m_pointer);
}


//-------------------------------------------------------------------------------------------------
template <typename T>
bool SmartPointer<T>::operator!=(const T* compare) const
{
	return (m_pointer != compare.m_pointer);
}


//-------------------------------------------------------------------------------------------------
template <typename T>
bool SmartPointer<T>::operator==(const T* compare) const
{
	return (m_pointer == compare);
}


//-------------------------------------------------------------------------------------------------
template <typename T>
void SmartPointer<T>::Release()
{
	int count = m_refCount->Release();
	if (count == 0)
	{
		SAFE_DELETE(m_pointer);
		SAFE_DELETE(m_refCount);
	}
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
