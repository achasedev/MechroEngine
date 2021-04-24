///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 8th, 2020
/// Description: Hashed c-string class
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include <map>
#include <string>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
class StringID;
#define INVALID_STRING_ID StringID()
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
class StringID
{
	friend class StringIdSystem;

public:
	//-----Public Methods-----

	StringID() {}

	const char* ToString() const { return m_string; }

	bool operator==(const StringID& compare) const { return m_hash == compare.m_hash; } 
	bool operator!=(const StringID& compare) const { return m_hash != compare.m_hash; } 
	bool operator<(const StringID& compare) const { return m_hash < compare.m_hash; } // For std::map
	

private:
	//-----Private Methods-----

	StringID(uint32 hash, const char* string)
	 : m_hash(hash), m_string(string) {}


private:
	//-----Private Data-----

	const char* m_string = nullptr;
	uint32		m_hash = 0;

};


//-------------------------------------------------------------------------------------------------
class StringIdSystem
{
public:
	//-----Public Methods-----

	static void		Initialize();
	static void		Shutdown();
	static bool		IsInitialized() { return g_sidSystem != nullptr; }

	StringID		CreateOrGetStringId(const char* str);
	StringID		CreateOrGetStringId(const std::string& str);
	const char*		GetStringForStringID(const StringID& stringID) const;


private:
	//-----Private Methods-----

	StringIdSystem() {}
	~StringIdSystem();
	StringIdSystem(const StringIdSystem& copy) = delete;

	StringID		InternString(const char* str);


private:
	//-----Private Data-----

	std::map<uint32, const char*> m_internedStrings;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
