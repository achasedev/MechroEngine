///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 8th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Utility/StringID.h"
#include <map>
#include <string>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Vector2;
class Vector3;
class Vector4;

class CommandArgs;
typedef void(*CommandFunction)(CommandArgs& args);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class CommandArgs
{
	friend class ConsoleCommand;

public:
	//-----Public Methods-----

	// Getters
	float		GetNextFloat();
	int			GetNextInt();
	uint32		GetNextUInt();
	AABB2		GetNextAABB2();
	Vector2		GetNextVector2();
	Vector3		GetNextVector3();
	Vector4		GetNextVector4();
	IntVector2	GetNextIntVector2();
	IntVector3	GetNextIntVector3();
	std::string GetNextString();
	StringID	GetNextStringID();


private:
	//-----Private Methods-----

	CommandArgs() {}
	CommandArgs(const std::string& commandLine);


private:
	//-----Private Data-----

	std::string m_commandLine;
	int			m_readHead = 0;

};

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class ConsoleCommand
{
public:
	//-----Public Methods-----

	static void Register(StringID id, const std::string& description, const std::string& usage, CommandFunction commandFunction);
	static void Run(const std::string& commandLine);
	static void GetAllCommandsThatHavePrefix(const std::string& prefix, std::vector<std::string>& out_names);


private:
	//-----Private Methods-----

	ConsoleCommand(StringID id, const std::string& description, const std::string& usage, CommandFunction commandFunction)
		: m_id(id), m_description(description), m_usage(usage), m_function(commandFunction) {}

	~ConsoleCommand() {}


private:
	//-----Private Data-----

	StringID		m_id;
	std::string		m_description;
	std::string		m_usage;
	CommandFunction m_function = nullptr;

	static std::map<StringID, const ConsoleCommand*> s_commands;

};
