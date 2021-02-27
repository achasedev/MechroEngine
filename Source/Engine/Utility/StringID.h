///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 8th, 2020
/// Description: Hashed c-string class
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include <map>
#include <string>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
class StringId;
#define INVALID_STRING_ID 0
#define SID(x) g_sidSystem->CreateOrGetStringId(x)

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
class StringId
{
	friend class StringIdSystem;

public:
	//-----Public Methods-----

	StringId() {}

	const char* ToString() const { return m_string; }

	bool operator==(const StringId& compare) const { return m_hash == compare.m_hash; } 
	bool operator!=(const StringId& compare) const { return m_hash != compare.m_hash; } 
	bool operator<(const StringId& compare) const { return m_hash < compare.m_hash; } // For std::map
	

private:
	//-----Private Methods-----

	StringId(uint32 hash, const char* string)
	 : m_hash(hash), m_string(string) {}


private:
	//-----Private Data-----

	const char* m_string = nullptr;
	uint32		m_hash = INVALID_STRING_ID;

};


//-------------------------------------------------------------------------------------------------
class StringIdSystem
{
public:
	//-----Public Methods-----

	static void		Initialize();
	static void		Shutdown();
	static bool		IsInitialized() { return g_sidSystem != nullptr; }

	StringId		CreateOrGetStringId(const char* str);
	StringId		CreateOrGetStringId(const std::string& str);
	const char*		GetStringForStringID(const StringId& stringID) const;


private:
	//-----Private Methods-----

	StringIdSystem() {}
	~StringIdSystem();
	StringIdSystem(const StringIdSystem& copy) = delete;

	StringId		InternString(const char* str);


private:
	//-----Private Data-----

	std::map<uint32, const char*> m_internedStrings;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
