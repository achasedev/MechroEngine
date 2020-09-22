///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 28th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/Rgba.h"
#include "Engine/Math/AABB2.h"
#include "Engine/Math/AABB3.h"
#include "Engine/Math/IntVector2.h"
#include "Engine/Math/IntVector3.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"
#include "Engine/Utility/NamedProperties.h"
#include "Engine/Utility/StringUtils.h"
#include <cstring>
#include <stdarg.h>


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
const std::string Stringf(const char* format, ...)
{
	char textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH];
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	vsnprintf_s(textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList);
	va_end(variableArgumentList);
	textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string(textLiteral);
}

//-------------------------------------------------------------------------------------------------
const std::string Stringf(const int maxLength, const char* format, ...)
{
	char textLiteralSmall[STRINGF_STACK_LOCAL_TEMP_LENGTH];
	char* textLiteral = textLiteralSmall;
	if (maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH)
		textLiteral = new char[maxLength];

	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	vsnprintf_s(textLiteral, maxLength, _TRUNCATE, format, variableArgumentList);
	va_end(variableArgumentList);
	textLiteral[maxLength - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue(textLiteral);
	if (maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH)
		delete[] textLiteral;

	return returnValue;
}


//-------------------------------------------------------------------------------------------------
int GetStringLength(const char* str)
{
	return static_cast<int>(strlen(str));
}


//-------------------------------------------------------------------------------------------------
void Tokenize(const std::string& stringToTokenize, const char delimiter, std::vector<std::string>& out_tokens)
{
	// Set up the substring indices
	size_t subStringStartIndex = stringToTokenize.find_first_not_of(delimiter);
	if (subStringStartIndex == std::string::npos) { return; }	// Return if the entire string is just delimiters
	size_t subStringEndPostion = stringToTokenize.find(delimiter, subStringStartIndex + 1);

	// Iterate across the entire string
	while (subStringEndPostion != std::string::npos)
	{
		// Create the substring
		size_t substringLength = (subStringEndPostion - subStringStartIndex);
		out_tokens.push_back(std::string(stringToTokenize, subStringStartIndex, substringLength));

		// Update the indices
		subStringStartIndex = stringToTokenize.find_first_not_of(delimiter, subStringEndPostion + 1);
		if (subStringStartIndex == std::string::npos) { return; } // Return if the rest of the string is just delimiters
		subStringEndPostion = stringToTokenize.find(delimiter, subStringStartIndex + 1);
	}

	// Add the rest of the string
	out_tokens.push_back(std::string(stringToTokenize, subStringStartIndex));
}


//-------------------------------------------------------------------------------------------------
std::string ToString(float inValue)
{
	return Stringf("%f", inValue);
}


//-------------------------------------------------------------------------------------------------
std::string ToString(int inValue)
{
	return Stringf("%i", inValue);
}


//-------------------------------------------------------------------------------------------------
std::string ToString(const Vector2& inValue)
{
	return Stringf("(%f, %f)", inValue.x, inValue.y);
}


//-------------------------------------------------------------------------------------------------
std::string ToString(const Vector3& inValue)
{
	return Stringf("(%f, %f, %f)", inValue.x, inValue.y, inValue.z);
}


//-------------------------------------------------------------------------------------------------
std::string ToString(const Vector4& inValue)
{
	return Stringf("(%f, %f, %f, %f)", inValue.x, inValue.y, inValue.z, inValue.w);
}


//-------------------------------------------------------------------------------------------------
std::string ToString(const IntVector2& inValue)
{
	return Stringf("(%i, %i)", inValue.x, inValue.y);
}


//-------------------------------------------------------------------------------------------------
std::string ToString(const AABB2& inValue)
{
	return Stringf("((%f, %f), (%f, %f))", inValue.mins.x, inValue.mins.y, inValue.maxs.x, inValue.maxs.y);
}


//-------------------------------------------------------------------------------------------------
std::string ToString(const AABB3& inValue)
{
	return Stringf("((%f, %f, %f), (%f, %f, %f))", inValue.mins.x, inValue.mins.y, inValue.mins.z, inValue.maxs.x, inValue.maxs.y, inValue.maxs.z);
}


//-------------------------------------------------------------------------------------------------
std::string ToString(const NamedProperties& inValue)
{
	return inValue.ToString();
}


//-------------------------------------------------------------------------------------------------
std::string ToString(std::string inValue)
{
	return inValue;
}


//-------------------------------------------------------------------------------------------------
std::string ToString(void* inValue)
{
	return Stringf("Pointer at address: %x", inValue);
}


//-------------------------------------------------------------------------------------------------
int StringToInt(const std::string& inValue)
{
	return atoi(inValue.c_str());
}


//-------------------------------------------------------------------------------------------------
float StringToFloat(const std::string& inValue)
{
	return static_cast<float>(atof(inValue.c_str()));
}


//-------------------------------------------------------------------------------------------------
Rgba StringToRgba(const std::string& inValue)
{
	// TODO: Check for ',' as a delimiter first
	ASSERT_RETURN(inValue.size() > 0, Rgba::BLACK, "Emtpy string!");

	std::vector<std::string> colorTokens;
	Tokenize(inValue, ' ', colorTokens);

	ASSERT_RECOVERABLE(colorTokens.size() < 5, "Too many components for an RGBA, only using the first 4!");

	// Check if the string is in floats or ints
	bool isFloats = inValue.find('.') != std::string::npos;

	Rgba color;
	for (uint32 colorIndex = 0; colorIndex < colorTokens.size(); ++colorIndex)
	{
		if (isFloats)
		{
			color.data[colorIndex] = NormalizedFloatToByte(StringToFloat(colorTokens[colorIndex]));
		}
		else
		{
			int colorUnclamped = StringToInt(colorTokens[colorIndex].c_str());
			color.data[colorIndex] = static_cast<uint8>(Clamp(colorUnclamped, 0, 255));
		}
	}

	return color;
}


//-------------------------------------------------------------------------------------------------
IntVector3 StringToIntVector3(const std::string& inValue)
{
	// TODO: Check for ',' as a delimiter first
	ASSERT_RETURN(inValue.size() > 0, IntVector3::ZERO, "Emtpy string!");

	std::vector<std::string> tokens;
	Tokenize(inValue, ' ', tokens);

	ASSERT_RECOVERABLE(tokens.size() < 4, "Too many components for an IntVector3, only using the first 3!");

	IntVector3 returnValue(0);
	for (uint32 i = 0; i < tokens.size(); ++i)
	{
		returnValue.data[i] = StringToInt(tokens[i].c_str());
	}

	return returnValue;
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
