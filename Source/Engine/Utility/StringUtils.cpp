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
#include <cctype>
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
static void TokenizeByCommasOrSpaces(const std::string& text, std::vector<std::string>& out_tokens)
{
	// Prioritize commas
	if (text.find(',') != std::string::npos)
	{
		Tokenize(text, ',', out_tokens);

		// In case we have spaces with commas
		for (size_t tokenIndex = 0; tokenIndex < out_tokens.size(); ++tokenIndex)
		{
			TrimWhitespace(out_tokens[tokenIndex]);
		}
	}
	else
	{
		Tokenize(text, ' ', out_tokens);
	}
}


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
void TrimWhitespace(std::string& stringToTrim)
{
	size_t firstNonSpace = stringToTrim.find_first_not_of(' ');

	if (firstNonSpace != std::string::npos)
	{
		size_t lastNonSpace = stringToTrim.find_last_not_of(' ');
		stringToTrim = stringToTrim.substr(firstNonSpace, lastNonSpace - firstNonSpace + 1);
	}
}


//-------------------------------------------------------------------------------------------------
void BreakStringIntoLines(const std::string& stringToBreak, std::vector<std::string>& out_lines)
{
	std::string runningString = stringToBreak;

	while (runningString.size() > 0)
	{
		size_t newLineIndex = runningString.find_first_of('\n');
		if (newLineIndex != std::string::npos)
		{
			out_lines.push_back(runningString.substr(0, newLineIndex));
			runningString = runningString.substr(newLineIndex + 1);
		}
		else
		{
			out_lines.push_back(runningString);
			break;
		}
	}
}


//-------------------------------------------------------------------------------------------------
bool AreEqualCaseInsensitive(const std::string& first, const std::string& second)
{
	if (first.size() != second.size())
	{
		return false;
	}

	for (size_t index = 0; index < first.size(); ++index)
	{
		const char firstChar = first[index];
		const char secondChar = second[index];

		if (firstChar != secondChar && std::toupper(firstChar) != std::toupper(secondChar))
		{
			return false;
		}
	}

	return true;
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
bool StringToRgba(const std::string& inValue, Rgba& out_color)
{
	ASSERT_RETURN(inValue.size() > 0, false, "Empty string!");

	// Check if the text is a name first
	bool isName = true;
	if		(AreEqualCaseInsensitive(inValue, "white"))		{ out_color = Rgba::WHITE; }
	else if (AreEqualCaseInsensitive(inValue, "black"))		{ out_color = Rgba::BLACK; }
	else if (AreEqualCaseInsensitive(inValue, "red"))		{ out_color = Rgba::RED; }
	else if (AreEqualCaseInsensitive(inValue, "green"))		{ out_color = Rgba::GREEN; }
	else if (AreEqualCaseInsensitive(inValue, "blue"))		{ out_color = Rgba::BLUE; }
	else if (AreEqualCaseInsensitive(inValue, "cyan"))		{ out_color = Rgba::CYAN; }
	else if (AreEqualCaseInsensitive(inValue, "magenta"))	{ out_color = Rgba::MAGENTA; }
	else if (AreEqualCaseInsensitive(inValue, "yellow"))	{ out_color = Rgba::YELLOW; }
	else if (AreEqualCaseInsensitive(inValue, "gray"))		{ out_color = Rgba::GRAY; }
	else if (AreEqualCaseInsensitive(inValue, "grey"))		{ out_color = Rgba::GRAY; }
	else
	{
		isName = false;
	}

	// Specified components
	if (!isName)
	{
		std::vector<std::string> tokens;
		TokenizeByCommasOrSpaces(inValue, tokens);

		ASSERT_RETURN(tokens.size() > 0, false, "No components!");
		ASSERT_RECOVERABLE(tokens.size() < 5, "Too many components for an RGBA, only using the first 4!");

		// Check if the string is in floats or ints
		bool isFloats = inValue.find('.') != std::string::npos;

		for (size_t colorIndex = 0; colorIndex < tokens.size(); ++colorIndex)
		{
			if (isFloats)
			{
				out_color.data[colorIndex] = NormalizedFloatToByte(StringToFloat(tokens[colorIndex]));
			}
			else
			{
				int colorUnclamped = StringToInt(tokens[colorIndex].c_str());
				out_color.data[colorIndex] = static_cast<uint8>(Clamp(colorUnclamped, 0, 255));
			}
		}
	}	

	return true;
}


//-------------------------------------------------------------------------------------------------
static Vector4 StringToVectorInternal(const std::string& inValue, uint32 numComponents)
{
	ASSERT_RETURN(inValue.size() > 0, Vector4::ZERO, "Empty string!");

	std::vector<std::string> tokens;
	TokenizeByCommasOrSpaces(inValue, tokens);

	ASSERT_RETURN(tokens.size() > 0, Vector4::ZERO, "No components!");
	ASSERT_RECOVERABLE(tokens.size() == numComponents, "Wrong number of components, only using the first %u!", Max(numComponents, (uint32)tokens.size()));

	Vector4 returnValue(0.f);
	for (size_t i = 0; i < tokens.size(); ++i)
	{
		returnValue.data[i] = StringToFloat(tokens[i].c_str());
	}

	return returnValue;
}


//-------------------------------------------------------------------------------------------------
static IntVector3 StringToIntVectorInternal(const std::string& inValue, uint32 numComponents)
{
	ASSERT_RETURN(inValue.size() > 0, IntVector3::ZERO, "Empty string!");

	std::vector<std::string> tokens;
	TokenizeByCommasOrSpaces(inValue, tokens);

	ASSERT_RETURN(tokens.size() > 0, IntVector3::ZERO, "No components!");
	ASSERT_RECOVERABLE(tokens.size() == numComponents, "Wrong number of components, only using the first %u!", Max(numComponents, (uint32)tokens.size()));

	IntVector3 returnValue(0);
	for (size_t i = 0; i < tokens.size(); ++i)
	{
		returnValue.data[i] = StringToInt(tokens[i].c_str());
	}

	return returnValue;
}


//-------------------------------------------------------------------------------------------------
Vector2 StringToVector2(const std::string& inValue)
{
	Vector4 result = StringToVectorInternal(inValue, 2U);
	return result.xy();
}


//-------------------------------------------------------------------------------------------------
Vector3 StringToVector3(const std::string& inValue)
{
	Vector4 result = StringToVectorInternal(inValue, 3U);
	return result.xyz();
}


//-------------------------------------------------------------------------------------------------
Vector4 StringToVector4(const std::string& inValue)
{
	return StringToVectorInternal(inValue, 4U);
}


//-------------------------------------------------------------------------------------------------
IntVector2 StringToIntVector2(const std::string& inValue)
{
	IntVector3 result = StringToIntVectorInternal(inValue, 2U);
	return result.xy;
}


//-------------------------------------------------------------------------------------------------
IntVector3 StringToIntVector3(const std::string& inValue)
{
	return StringToIntVectorInternal(inValue, 3U);
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
