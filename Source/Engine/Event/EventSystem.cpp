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
bool FireEvent(const char* eventName)
{
	NamedProperties args;
	return FireEvent(SID(eventName), args);
}


//-------------------------------------------------------------------------------------------------
bool FireEvent(const std::string& eventName)
{
	NamedProperties args;
	return FireEvent(SID(eventName), args);
}


//-------------------------------------------------------------------------------------------------
bool FireEvent(const StringID& eventSID)
{
	NamedProperties args;
	return FireEvent(eventSID, args);
}


//-------------------------------------------------------------------------------------------------
bool FireEvent(const char* eventName, NamedProperties& args)
{
	return FireEvent(SID(eventName), args);
}


//-------------------------------------------------------------------------------------------------
bool FireEvent(const std::string& eventName, NamedProperties& args)
{
	return FireEvent(SID(eventName), args);
}


//-------------------------------------------------------------------------------------------------
bool FireEvent(const StringID& eventSID, NamedProperties& args)
{
	return g_eventSystem->FireEvent(eventSID, args);
}


//-------------------------------------------------------------------------------------------------
bool QueueDelayedEvent(const char* eventName, float delaySeconds)
{
	NamedProperties args;
	return QueueDelayedEvent(SID(eventName), args, delaySeconds);
}


//-------------------------------------------------------------------------------------------------
bool QueueDelayedEvent(const std::string& eventName, float delaySeconds)
{
	NamedProperties args;
	return QueueDelayedEvent(SID(eventName), args, delaySeconds);
}


//-------------------------------------------------------------------------------------------------
bool QueueDelayedEvent(const StringID& eventSID, float delaySeconds)
{
	NamedProperties args;
	return QueueDelayedEvent(eventSID, args, delaySeconds);
}


//-------------------------------------------------------------------------------------------------
bool QueueDelayedEvent(const char* eventName, NamedProperties& args, float delaySeconds)
{
	return QueueDelayedEvent(SID(eventName), args, delaySeconds);
}


//-------------------------------------------------------------------------------------------------
bool QueueDelayedEvent(const std::string& eventName, NamedProperties& args, float delaySeconds)
{
	return QueueDelayedEvent(SID(eventName), args, delaySeconds);
}


//-------------------------------------------------------------------------------------------------
bool QueueDelayedEvent(const StringID& eventSID, NamedProperties& args, float delaySeconds)
{
	return g_eventSystem->QueueDelayedEvent(eventSID, args, delaySeconds);
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
			SAFE_DELETE(subsToEvent[subIndex]);
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
	SAFE_DELETE(g_eventSystem);
}


//-------------------------------------------------------------------------------------------------
void EventSystem::BeginFrame()
{
	for (int eventIndex = (int)m_delayedEvents.size() - 1; eventIndex >= 0; --eventIndex)
	{
		if (m_delayedEvents[eventIndex].m_timer.HasIntervalElapsed())
		{
			FireEvent(m_delayedEvents[eventIndex].m_eventSID, m_delayedEvents[eventIndex].m_args);
			m_delayedEvents.erase(m_delayedEvents.begin() + eventIndex);
		}
	}
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
				SAFE_DELETE(currSub);

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
bool EventSystem::FireEvent(const StringID& eventSID, NamedProperties& args)
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

	return eventHasSubscribers;
}


//-------------------------------------------------------------------------------------------------
bool EventSystem::QueueDelayedEvent(const StringID& eventSID, NamedProperties& args, float delaySeconds)
{
	DelayedEvent delayedEvent;
	delayedEvent.m_eventSID = eventSID;
	delayedEvent.m_args = args;
	delayedEvent.m_timer.SetInterval(delaySeconds);

	m_delayedEvents.push_back(delayedEvent);

	return m_subscriptions.find(eventSID) != m_subscriptions.end();
}


//-------------------------------------------------------------------------------------------------
void EventSystem::GetAllEventNames(std::vector<std::string>& out_eventNames) const
{
	std::map<StringID, std::vector<EventSubscription*>>::const_iterator itr = m_subscriptions.begin();

	for (itr; itr != m_subscriptions.end(); itr++)
	{
		out_eventNames.push_back(itr->first.ToString());
	}
}
