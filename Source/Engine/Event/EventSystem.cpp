///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 27th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Event/EventSystem.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Utility/NamedProperties.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
EventSystem* g_eventSystem = nullptr;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void FireEvent(const char* eventName)
{
	NamedProperties args;
	FireEvent(SID(eventName), args);
}


//-------------------------------------------------------------------------------------------------
void FireEvent(const StringID& eventSID)
{
	NamedProperties args;
	FireEvent(eventSID, args);
}


//-------------------------------------------------------------------------------------------------
void FireEvent(const char* eventName, NamedProperties& args)
{
	FireEvent(SID(eventName), args);
}


//-------------------------------------------------------------------------------------------------
void FireEvent(const StringID& eventSID, NamedProperties& args)
{
	g_eventSystem->FireEvent(eventSID, args);
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
EventSystem::EventSystem()
{
}


//-------------------------------------------------------------------------------------------------
EventSystem::~EventSystem()
{
	std::map<StringID, std::vector<EventSubscription*>>::iterator itr = m_subscriptions.begin();

	for (itr; itr != m_subscriptions.end(); ++itr)
	{
		std::vector<EventSubscription*>& subsToEvent = itr->second;
		uint32 numSubs = (uint32)subsToEvent.size();

		for (uint32 subIndex = 0; subIndex < numSubs; ++subIndex)
		{
			SAFE_DELETE_POINTER(subsToEvent[subIndex]);
		}

		subsToEvent.clear();
	}

	m_subscriptions.clear();
}


//-------------------------------------------------------------------------------------------------
void EventSystem::Initialize()
{
	g_eventSystem = new EventSystem();
}


//-------------------------------------------------------------------------------------------------
void EventSystem::Shutdown()
{
	SAFE_DELETE_POINTER(g_eventSystem);
}


//-------------------------------------------------------------------------------------------------
void EventSystem::SubscribeEventCallbackFunction(const char* eventName, EventFunctionCallback callback)
{
	EventFunctionSubscription* subscription = new EventFunctionSubscription(callback);

	// This creates the entry if there isn't one already
	std::vector<EventSubscription*>& subsToEvent = m_subscriptions[SID(eventName)];
	subsToEvent.push_back(subscription);
}


//-------------------------------------------------------------------------------------------------
void EventSystem::UnsubscribeEventCallbackFunction(const char* eventName, EventFunctionCallback callback)
{
	StringID eventSID = SID(eventName);
	std::vector<EventSubscription*>& subsToEvent = m_subscriptions[eventSID];

	for (int subIndex = 0; subIndex < (int)subsToEvent.size(); ++subIndex)
	{
		EventFunctionSubscription* currSub = dynamic_cast<EventFunctionSubscription*>(subsToEvent[subIndex]);

		if (currSub != nullptr) // currSub is a standalone function subscription
		{
			if (currSub->m_functionCallback == callback) // currSub is the one for the given callback
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


//-------------------------------------------------------------------------------------------------
void EventSystem::FireEvent(const StringID& eventSID, NamedProperties& args)
{
	bool eventHasSubscribers = (m_subscriptions.find(eventSID) != m_subscriptions.end());

	if (eventHasSubscribers)
	{
		std::vector<EventSubscription*>& subsToEvent = m_subscriptions[eventSID];

		int numSubs = (int)subsToEvent.size();
		for (int subIndex = 0; subIndex < numSubs; ++subIndex)
		{
			bool subConsumedEvent = subsToEvent[subIndex]->Execute(args);

			if (subConsumedEvent)
			{
				break;
			}
		}
	}
}
