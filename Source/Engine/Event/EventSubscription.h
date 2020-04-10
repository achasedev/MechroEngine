///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 27th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class NamedProperties;
typedef bool(*EventFunctionCallback)(NamedProperties& args);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class EventSubscription
{
public:
	//-----Public Methods-----

	EventSubscription() {}
	virtual ~EventSubscription() {}
	virtual bool Execute(NamedProperties& args) = 0;

};

//-------------------------------------------------------------------------------------------------
// For C functions and class static functions
class EventFunctionSubscription : public EventSubscription
{
	friend class EventSystem;

public:
	//-----Public Methods-----

	EventFunctionSubscription(EventFunctionCallback callback);
	virtual ~EventFunctionSubscription();
	virtual bool Execute(NamedProperties& args) override;


private:
	//-----Private Data-----

	EventFunctionCallback m_functionCallback = nullptr;

};


//-------------------------------------------------------------------------------------------------
// For object methods
template <typename T>
class EventObjectMethodSubscription : public EventSubscription
{
	friend class EventSystem;

public:

	// For saving off the function callback for each class used
	typedef bool(T::*EventObjectMethodCallback)(NamedProperties& args);


public:
	//-----Public Methods-----

	EventObjectMethodSubscription(EventObjectMethodCallback callback, T& object);
	virtual ~EventObjectMethodSubscription();
	virtual bool Execute(NamedProperties& args) override;


private:
	//-----Private Data-----

	T& m_object;
	EventObjectMethodCallback m_methodCallback = nullptr;

};


//-------------------------------------------------------------------------------------------------
template <typename T>
EventObjectMethodSubscription<T>::EventObjectMethodSubscription(EventObjectMethodCallback callback, T& object)
	: m_methodCallback(callback)
	, m_object(object)
{
}


//-------------------------------------------------------------------------------------------------
template <typename T>
EventObjectMethodSubscription<T>::~EventObjectMethodSubscription()
{
	m_methodCallback = nullptr;
}


//-------------------------------------------------------------------------------------------------
template <typename T>
bool EventObjectMethodSubscription<T>::Execute(NamedProperties& args)
{
	return (m_object.*m_methodCallback)(args);
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
