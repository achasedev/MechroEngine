///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 8th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/ConsoleCommand.h"
#include "Engine/Core/DevConsole.h"
#include "Engine/Core/EngineCommon.h"

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
void ConsoleCommand::Register(StringID id, const std::string& description, const std::string& usage, CommandFunction commandFunction, bool isEngine)
{
	bool alreadyExists = s_commands.find(id) != s_commands.end();
	ASSERT_RETURN(!alreadyExists, NO_RETURN_VAL, "Duplicate command registered!");

	s_commands[id] = new ConsoleCommand(id, description, usage, commandFunction, isEngine);
}


//-------------------------------------------------------------------------------------------------
void ConsoleCommand::Run(const std::string& commandLine)
{
	// Extract the command id
	size_t firstSpaceIndex = commandLine.find_first_of(' ');
	StringID commandID = (firstSpaceIndex != std::string::npos ? SID(commandLine.substr(0, firstSpaceIndex)) : SID(commandLine));

	if (s_commands.find(commandID) != s_commands.end())
	{
		std::string argsList = (firstSpaceIndex != std::string::npos ? (commandLine.substr(firstSpaceIndex)) : "");

		CommandArgs args = CommandArgs(argsList);
		s_commands[commandID]->m_function(args);
	}
	else
	{
		ConsoleWarningf("Unknown command: %s", commandID.ToString());
	}
}


//-------------------------------------------------------------------------------------------------
void ConsoleCommand::GetAllCommands(std::vector<const ConsoleCommand*>& out_commands)
{
	std::map<StringID, const ConsoleCommand*>::const_iterator itr = s_commands.begin();

	for (itr; itr != s_commands.end(); itr++)
	{
		out_commands.push_back(itr->second);	
	}
}


//-------------------------------------------------------------------------------------------------
void ConsoleCommand::GetAllCommandsWithIDPrefix(const std::string& prefix, std::vector<const ConsoleCommand*>& out_commands)
{
	std::map<StringID, const ConsoleCommand*>::const_iterator itr = s_commands.begin();

	for (itr; itr != s_commands.end(); itr++)
	{
		std::string currName = itr->first.ToString();
		if (currName.find(prefix) == 0)
		{
			out_commands.push_back(itr->second);
		}
	}
}


//-------------------------------------------------------------------------------------------------
std::string ConsoleCommand::GetIDWithDescription() const
{
	std::string fullDescription(">");
	fullDescription += m_id.ToString();
	fullDescription += " - " + m_description;

	return fullDescription;
}


//-------------------------------------------------------------------------------------------------
bool CommandArgs::GetNextFloat(float& out_float)
{
	std::string nextToken = GetNextToken(true);

	if (nextToken.size() == 0)
	{
		return false;
	}

	out_float = StringToFloat(nextToken);
	return true;
}


//-------------------------------------------------------------------------------------------------
bool CommandArgs::GetNextFloat(float& out_float, float defaultValue)
{
	std::string nextToken = GetNextToken(false);

	if (nextToken.size() == 0)
	{
		out_float = defaultValue;
		return false;
	}

	out_float = StringToFloat(nextToken);
	return true;
}


//-------------------------------------------------------------------------------------------------
std::string CommandArgs::GetNextString(bool printError /*= true*/)
{
	size_t openingQuoteIndex = m_argsLine.find_first_of("\"", m_readHead);

	if (openingQuoteIndex != std::string::npos)
	{
		size_t closingQuoteIndex = m_argsLine.find_first_of("\"", m_readHead + 1);

		if (closingQuoteIndex == std::string::npos)
		{
			ConsoleErrorf("Quote in command line wasn't closed!");
			return "";
		}

		std::string fullString = m_argsLine.substr(openingQuoteIndex + 1, closingQuoteIndex - openingQuoteIndex);
		m_readHead = m_argsLine.find_first_not_of(' ', closingQuoteIndex + 1);

		return fullString;
	}

	// No quotes, so just get the next token
	return GetNextToken(printError);
}


//-------------------------------------------------------------------------------------------------
CommandArgs::CommandArgs(const std::string& argsLine)
	: m_argsLine(argsLine), m_readHead(0)
{
	TrimWhitespace(m_argsLine);
}


//-------------------------------------------------------------------------------------------------
std::string CommandArgs::GetNextToken(bool printError)
{
	if (m_readHead >= m_argsLine.size())
	{
		if (printError)
		{
			ConsoleErrorf("Not enough arguments!");
		}

		return "";
	}

	size_t firstSpaceIndex = m_argsLine.find_first_of(' ', m_readHead);
	std::string nextToken = m_argsLine.substr(m_readHead, firstSpaceIndex - m_readHead);
	m_readHead = m_argsLine.find_first_not_of(' ', firstSpaceIndex);

	return nextToken;
}
