///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 27th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Event/EventSubscription.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Utility/StringID.h"
#include <map>
#include <vector>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class NamedProperties;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class EventSystem
{
public:
	//-----Public Methods-----

	static void Initialize();
	static void Shutdown();

	// Subscriptions
	void SubscribeEventCallbackFunction(const char* eventName, EventFunctionCallback callback);
	void UnsubscribeEventCallbackFunction(const char* eventName, EventFunctionCallback callback);
	template <typename T, typename T_Method> void SubscribeEventCallbackObjectMethod(const char* eventName, T_Method callback, T& object);
	template <typename T, typename T_Method> void UnsubscribeEventCallbackObjectMethod(const char* eventName, T_Method callback, T& object);

	void FireEvent(const StringID& eventSID, NamedProperties& args);


private:
	//-----Private Methods-----

	EventSystem();
	~EventSystem();
	EventSystem(const EventSystem& copy) = delete;


private:
	//-----Private Data-----

	std::map<StringID, std::vector<EventSubscription*>> m_subscriptions;

};


//-------------------------------------------------------------------------------------------------
template <typename T, typename T_Method>
void EventSystem::SubscribeEventCallbackObjectMethod(const char* eventName, T_Method callback, T& object)
{
	EventObjectMethodSubscription<T>* subscription = new EventObjectMethodSubscription<T>(callback, object);

	// This creates the entry if there isn't one already
	std::vector<EventSubscription*>& subsToEvent = m_subscriptions[SID(eventName)];
	subsToEvent.push_back(subscription);
}


//-------------------------------------------------------------------------------------------------
template <typename T, typename T_Method>
void EventSystem::UnsubscribeEventCallbackObjectMethod(const char* eventName, T_Method callback, T& object)
{
	StringID eventSID = SID(eventName);

	std::vector<EventSubscription*>& subsToEvent = m_subscriptions[eventSID];

	for (int subIndex = 0; subIndex < (int)subsToEvent.size(); ++subIndex)
	{
		EventObjectMethodSubscription<T>* currSub = dynamic_cast<EventObjectMethodSubscription<T>*>(subsToEvent[subIndex]);

		if (currSub != nullptr) // currSub is an object method subscription
		{
			if (currSub->m_methodCallback == callback && &currSub->m_object == &object) // currSub is the one for the given object and callback
			{
				subsToEvent.erase(subsToEvent.begin() + subIndex);
				SAFE_DELETE_POINTER(currSub);

				break;
			}
		}
	}

	if (subsToEvent.size() == 0)
	{
		m_subscriptions.erase(eventSID);
	}
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
void FireEvent(const char* eventName);
void FireEvent(const StringID& eventSID);
void FireEvent(const char* eventName, NamedProperties& args);
void FireEvent(const StringID& eventSID, NamedProperties& args);