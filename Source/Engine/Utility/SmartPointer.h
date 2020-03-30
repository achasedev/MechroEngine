///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 29th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
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

	uint32 AddRef();
	uint32 Release();


private:
	//-----Private Data-----

	std::mutex	m_lock;
	uint32		m_count = 0;

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

	T&		operator*()		{ return *m_pointer; }
	T*		operator->()	{ return m_pointer; }
	void	operator=(const SmartPointer<T>& copy);


private:
	//-----Private Methods-----

	bool	HasReference() const { return m_refCount != nullptr; }


private:
	//-----Private Data-----

	T*			m_pointer = nullptr;
	RefCount*	m_refCount = nullptr;

};


//-------------------------------------------------------------------------------------------------
template <typename T>
SmartPointer<T>::SmartPointer(T* pointer)
{
	ASSERT_OR_DIE(pointer != nullptr, "SmartPointer to nullptr!");

	m_pointer = pointer;
	m_refCount = new RefCount();
	m_refCount->AddRef();
}


//-------------------------------------------------------------------------------------------------
template <typename T>
SmartPointer<T>::SmartPointer(const SmartPointer<T>& copy)
{
	// Check for self copy
	ASSERT_OR_DIE(this != &copy, "Smartpointer self assignment!");

	// Check for whether it points to the same thing
	if (m_pointer == copy.m_pointer)
	{
		return;
	}

	// Release my resources
	if (HasReference())
	{
		int count = m_refCount->Release();
		if (count == 0)
		{
			SAFE_DELETE_POINTER(m_pointer);
			SAFE_DELETE_POINTER(m_refCount);
		}
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
SmartPointer<T>::~SmartPointer()
{
	int count = m_refCount->Release();

	if (count == 0)
	{
		SAFE_DELETE_POINTER(m_pointer);
		SAFE_DELETE_POINTER(m_refCount);
	}
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
		int count = m_refCount->Release();
		if (count == 0)
		{
			SAFE_DELETE_POINTER(m_pointer);
			SAFE_DELETE_POINTER(m_refCount);
		}
	}

	m_pointer = copy.m_pointer;
	m_refCount = copy.m_refCount;
	m_refCount->AddRef();
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
