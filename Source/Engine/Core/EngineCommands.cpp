///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 8th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommands.h"
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

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// COMMANDS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Clear the scroll log
void Command_ClearLog(CommandArgs& args)
{
	UNUSED(args);
	g_devConsole->ClearLog();
}

//-------------------------------------------------------------------------------------------------
void Command_Add(CommandArgs& args)
{
	float a, b;
	
	bool aSpecified = args.GetNextFloat(a);
	bool bSpecified = args.GetNextFloat(b);

	if (aSpecified && bSpecified)
	{
		ConsolePrintf("%.2f + %.2f = %.2f", a, b, a + b);
	}
}


//-------------------------------------------------------------------------------------------------
static bool CompareConsoleCommands(const ConsoleCommand* first, const ConsoleCommand* second)
{
	return (first->GetID().ToString() < second->GetID().ToString());
}


//-------------------------------------------------------------------------------------------------
void Command_Help(CommandArgs& args)
{
	std::string type = args.GetNextString();

	bool printGameCommands = type.size() == 0 || AreEqualCaseInsensitive(type, "game");
	bool printEngineCommands = type.size() == 0 || AreEqualCaseInsensitive(type, "engine");

	if (!printEngineCommands && !printGameCommands)
	{
		ConsoleWarningf("Unrecognized type of command: %s", type.c_str());
		return;
	}

	std::vector<const ConsoleCommand*> commands;
	ConsoleCommand::GetAllCommands(commands);
	std::sort(commands.begin(), commands.end(), CompareConsoleCommands);

	int numEngineCommands = 0;
	if (printEngineCommands)
	{
		ConsolePrintf(Rgba::CYAN, "-----Engine Commands-----");

		for (int commandIndex = 0; commandIndex < static_cast<int>(commands.size()); ++commandIndex)
		{
			const ConsoleCommand* currCommand = commands[commandIndex];

			if (currCommand->IsEngineCommand())
			{
				ConsolePrintf(commands[commandIndex]->GetIDWithDescription());
				numEngineCommands++;
			}
		}

		ConsolePrintf(Rgba::CYAN, "-----End Engine Commands, %i total-----", numEngineCommands);
	}

	int numGameCommands = 0;
	if (printGameCommands)
	{
		ConsolePrintf(Rgba::CYAN, "-----Game Commands-----");

		for (int commandIndex = 0; commandIndex < static_cast<int>(commands.size()); ++commandIndex)
		{
			const ConsoleCommand* currCommand = commands[commandIndex];

			if (currCommand->IsGameCommand())
			{
				ConsolePrintf(commands[commandIndex]->GetIDWithDescription());
				numGameCommands++;
			}
		}

		ConsolePrintf(Rgba::CYAN, "-----End Game Commands, %i total-----", numGameCommands);
	}
}
