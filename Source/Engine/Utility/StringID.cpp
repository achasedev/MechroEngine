///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 8th, 2020
/// Description: Hashed c-string class
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Utility/StringId.h"
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
StringIdSystem* g_sidSystem = nullptr;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void StringIdSystem::Initialize()
{
	ASSERT_OR_DIE(g_sidSystem == nullptr, "DebugSIDSystem being reinitialized!");
	g_sidSystem = new StringIdSystem();
}


//-------------------------------------------------------------------------------------------------
void StringIdSystem::Shutdown()
{
	SAFE_DELETE(g_sidSystem);
}


//-------------------------------------------------------------------------------------------------
// Debug-only function for storing StringID -> strings and checking for collisions
StringId StringIdSystem::InternString(const char* str)
{
	uint32 strHash = HashData((void*)str, strlen(str));
	std::map<uint32, const char*>::iterator itr = m_internedStrings.find(strHash);	
	if (itr == m_internedStrings.end())
	{
		// Allocate and add the string
		size_t bufferSize = strlen(str) + 1U;
		char* internBuffer = (char*) malloc(bufferSize);
		strncpy_s(internBuffer, bufferSize, str, strlen(str));

		m_internedStrings[strHash] = internBuffer;
		return StringId(strHash, internBuffer);
	}
	else
	{
		// Check for hash collisions
		ASSERT_OR_DIE(strcmp(str, itr->second) == 0, "Hash collision on strings %s and %s", str, itr->second);
		return StringId(strHash, itr->second);
	}
}


//-------------------------------------------------------------------------------------------------
StringId StringIdSystem::CreateOrGetStringId(const char* str)
{
	return InternString(str);
}


//-------------------------------------------------------------------------------------------------
StringId StringIdSystem::CreateOrGetStringId(const std::string& str)
{
	return InternString(str.c_str());
}


//-------------------------------------------------------------------------------------------------
const char* StringIdSystem::GetStringForStringID(const StringId& stringID) const
{
	std::map<uint32, const char*>::const_iterator itr = m_internedStrings.find(stringID.m_hash);
	if (itr != m_internedStrings.end())
	{
		return itr->second;
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
StringIdSystem::~StringIdSystem()
{
	// Free up any id's in the map
	std::map<uint32, const char*>::iterator itr = m_internedStrings.begin();

	for (itr; itr != m_internedStrings.end(); itr++)
	{
		free((void*)itr->second);
	}

	m_internedStrings.clear();
}
