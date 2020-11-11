///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 8th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/ConsoleCommand.h"
#include "Engine/Framework/DevConsole.h"
#include "Engine/Framework/EngineCommon.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
std::map<StringID, const ConsoleCommand*> ConsoleCommand::s_commands;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void ConsoleCommand::Register(StringID id, const std::string& description, const std::string& usage, CommandFunction commandFunction)
{
	bool alreadyExists = s_commands.find(id) != s_commands.end();
	ASSERT_RETURN(!alreadyExists, NO_RETURN_VAL, "Duplicate command registered!");

	s_commands[id] = new ConsoleCommand(id, description, usage, commandFunction);
}


//-------------------------------------------------------------------------------------------------
void ConsoleCommand::Run(const std::string& commandLine)
{
	// Extract the command id
	size_t firstSpaceIndex = commandLine.find_first_of(' ');
	StringID commandID = (firstSpaceIndex != std::string::npos ? SID(commandLine.substr(0, firstSpaceIndex)) : SID(commandLine));

	if (s_commands.find(commandID) != s_commands.end())
	{
		CommandArgs args = (firstSpaceIndex != std::string::npos ? CommandArgs(commandLine.substr(firstSpaceIndex)) : CommandArgs());
		s_commands[commandID]->m_function(args);
	}
	else
	{
		ConsoleWarningf("Unknown command: %s", commandID.ToString());
	}
}


//-------------------------------------------------------------------------------------------------
void ConsoleCommand::GetAllCommandsThatHavePrefix(const std::string& prefix, std::vector<std::string>& out_ids)
{
	std::map<StringID, const ConsoleCommand*>::const_iterator itr = s_commands.begin();

	for (itr; itr != s_commands.end(); itr++)
	{
		std::string currName = itr->first.ToString();
		if (currName.find(prefix) == 0)
		{
			out_ids.push_back(currName);
		}
	}
}


//-------------------------------------------------------------------------------------------------
float CommandArgs::GetNextFloat()
{
	ASSERT_RETURN(m_readHead < (int)m_commandLine.size(), 0.f, "No more arguments!");

	size_t firstSpaceIndex = m_commandLine.find_first_of(' ', m_readHead);
	std::string nextToken = m_commandLine.substr(m_readHead, firstSpaceIndex - m_readHead);

	m_readHead = static_cast<int>(m_commandLine.find_first_not_of(' ', firstSpaceIndex));
	return StringToFloat(nextToken);
}


//-------------------------------------------------------------------------------------------------
CommandArgs::CommandArgs(const std::string& commandLine)
	: m_commandLine(commandLine), m_readHead(0)
{
	TrimWhitespace(m_commandLine);
}

