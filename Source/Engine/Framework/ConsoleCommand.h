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
#include "Engine/Utility/StringId.h"
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
	bool		GetNextFloat(float& out_float);
	bool		GetNextFloat(float& out_float, float defaultValue);

	int			GetNextInt();
	uint32		GetNextUInt();
	AABB2		GetNextAABB2();
	Vector2		GetNextVector2();
	Vector3		GetNextVector3();
	Vector4		GetNextVector4();
	IntVector2	GetNextIntVector2();
	IntVector3	GetNextIntVector3();
	std::string GetNextString(bool printError = true);
	StringId	GetNextStringID();


private:
	//-----Private Methods-----

	CommandArgs() {}
	CommandArgs(const std::string& argsLine);

	std::string GetNextToken(bool assert);


private:
	//-----Private Data-----

	std::string m_argsLine;
	size_t		m_readHead = 0;

};

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class ConsoleCommand
{
public:
	//-----Public Methods-----

	static void Register(StringId id, const std::string& description, const std::string& usage, CommandFunction commandFunction, bool isEngine);
	static void Run(const std::string& commandLine);
	static void GetAllCommands(std::vector<const ConsoleCommand*>& out_commands);
	static void GetAllCommandsWithIDPrefix(const std::string& prefix, std::vector<const ConsoleCommand*>& out_commands);

	StringId	GetID() const { return m_id; }
	std::string GetDescription() const { return m_description; }
	std::string GetUsage() const { return m_usage; }
	bool		IsEngineCommand() const { return m_isEngine; }
	bool		IsGameCommand() const { return !m_isEngine; }

	// For pretty printing
	std::string GetIDWithDescription() const;

private:
	//-----Private Methods-----

	ConsoleCommand(StringId id, const std::string& description, const std::string& usage, CommandFunction commandFunction, bool isEngine)
		: m_id(id), m_description(description), m_usage(usage), m_function(commandFunction), m_isEngine(isEngine) {}

	~ConsoleCommand() {}


private:
	//-----Private Data-----

	StringId		m_id;
	std::string		m_description;
	std::string		m_usage;
	bool			m_isEngine = true;
	CommandFunction m_function = nullptr;

	static std::map<StringId, const ConsoleCommand*> s_commands;

};
