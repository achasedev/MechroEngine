///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 8th, 2020
/// Description: Hashed c-string class
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Utility/StringID.h"
#include "Engine/Utility/Hash.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
DebugSIDSystem* g_debugSIDSystem = nullptr;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
StringID HashString(const char* str)
{
	uint32 strHash = HashData((void*)str, strlen(str));
	StringID strID = static_cast<StringID>(strHash);

	if (DebugSIDSystem::IsInitialized())
	{
		g_debugSIDSystem->InternString(strID, str);
	}

	return strID;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void DebugSIDSystem::Initialize()
{
	g_debugSIDSystem = new DebugSIDSystem();
}


//-------------------------------------------------------------------------------------------------
void DebugSIDSystem::Shutdown()
{
	SAFE_DELETE(g_debugSIDSystem);
}


//-------------------------------------------------------------------------------------------------
// Debug-only function for storing StringID -> strings and checking for collisions
void DebugSIDSystem::InternString(const StringID& strID, const char* str)
{
	size_t bufferSize = strlen(str) + 1U;

	std::map<StringID, const char*>::iterator itr = m_stringIDs.find(strID);	
	if (itr == m_stringIDs.end())
	{
		// Allocate and add the string
		char* temp = (char*) malloc(bufferSize);
		strncpy_s(temp, bufferSize, str, strlen(str));
		m_stringIDs[strID] = temp;
	}
	else
	{
		// Check for hash collisions
		ASSERT_OR_DIE(strcmp(str, itr->second) == 0, "Hash collision on strings %s and %s", str, itr->second);
	}
}


//-------------------------------------------------------------------------------------------------
const char* DebugSIDSystem::GetStringForStringID(const StringID& stringID)
{
	bool idInterned = m_stringIDs.find(stringID) != m_stringIDs.end();

	if (idInterned)
	{
		return m_stringIDs[stringID];
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
DebugSIDSystem::~DebugSIDSystem()
{
	// Free up any id's in the map
	std::map<StringID, const char*>::iterator itr = m_stringIDs.begin();

	for (itr; itr != m_stringIDs.end(); itr++)
	{
		free((void*)itr->second);
	}

	m_stringIDs.clear();
}
