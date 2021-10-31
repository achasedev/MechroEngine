///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 28th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/Rgba.h"
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

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
const std::string Stringf(const char* format, ...)
{
	char textLiteral[VARIABLE_ARG_STACK_LOCAL_TEMP_LENGTH];
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	vsnprintf_s(textLiteral, VARIABLE_ARG_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList);
	va_end(variableArgumentList);
	textLiteral[VARIABLE_ARG_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string(textLiteral);
}

//-------------------------------------------------------------------------------------------------
const std::string Stringf(const int maxLength, const char* format, ...)
{
	char textLiteralSmall[VARIABLE_ARG_STACK_LOCAL_TEMP_LENGTH];
	char* textLiteral = textLiteralSmall;
	if (maxLength > VARIABLE_ARG_STACK_LOCAL_TEMP_LENGTH)
		textLiteral = new char[maxLength];

	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	vsnprintf_s(textLiteral, maxLength, _TRUNCATE, format, variableArgumentList);
	va_end(variableArgumentList);
	textLiteral[maxLength - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue(textLiteral);
	if (maxLength > VARIABLE_ARG_STACK_LOCAL_TEMP_LENGTH)
		delete[] textLiteral;

	return returnValue;
}


//-------------------------------------------------------------------------------------------------
int GetStringLength(const char* str)
{
	return static_cast<int>(strlen(str));
}


//-------------------------------------------------------------------------------------------------
// Returns the number of times the character appears in the string
int GetCharCountInText(const char character, const std::string& str)
{
	int num = 0;
	size_t index = str.find(character);
	while (index != std::string::npos)
	{
		num++;
		index = str.find(character, index + 1);
	}

	return num;
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
std::string GetFilePathExtension(const char* filepath)
{
	return GetFilePathExtension(std::string(filepath));
}


//-------------------------------------------------------------------------------------------------
std::string GetFilePathExtension(const std::string& filepath)
{
	size_t dotIndex = filepath.find_last_of('.');
	std::string extension;

	if (dotIndex != std::string::npos)
	{
		extension = filepath.substr(dotIndex + 1);
	}

	return extension;
}


//-------------------------------------------------------------------------------------------------
bool DoesFilePathHaveExtension(const std::string& filepath, const std::string& extension)
{
	return AreEqualCaseInsensitive(GetFilePathExtension(filepath), extension);
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
// Attempts to parse the given text as a single bool; returns invalid if it cannot
Maybe<bool> TryParseAsBool(const std::string& str)
{
	std::string strLocal = str;
	TrimWhitespace(strLocal);

	Maybe<bool> result = Maybe<bool>::INVALID;

	if (AreEqualCaseInsensitive(strLocal, "true") || AreEqualCaseInsensitive(strLocal, "yes"))
	{
		result.Set(true);
	}
	else if (AreEqualCaseInsensitive(strLocal, "false") || AreEqualCaseInsensitive(strLocal, "no"))
	{
		result.Set(false);
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
// Attempts to parse text as a boolean, returning defaultValue if it fails
bool ParseAsBool(const std::string& str, bool defaultValue)
{
	Maybe<bool> result = TryParseAsBool(str);

	if (result.IsValid())
		return result.Get();

	return defaultValue;
}


//-------------------------------------------------------------------------------------------------
// Attempts to parse the given text as a single float; returns invalid if it cannot
Maybe<float> TryParseAsFloat(const std::string& str)
{
	std::string strLocal = str;
	TrimWhitespace(strLocal);

	// Must contain something
	if (strLocal.size() == 0)
		return Maybe<float>::INVALID;;

	// Make sure it's a single token, I don't want to deal with spaces in the middle
	std::vector<std::string> tokens;
	Tokenize(strLocal, ' ', tokens);
	if (tokens.size() > 1)
		return Maybe<float>::INVALID;

	// Can't contain letters outside 'f' and 'F'
	if (strLocal.find_first_of("abcdeghijklmnopqrstuvwxyzABCDEGHIJKLMNOPQRSTUVWXYZ") != std::string::npos)
		return Maybe<float>::INVALID;;

	// Can't contain symbols other than - and +
	if (strLocal.find_first_of("!@#$%^&*(),//'\"=_[]{}`~:;?") != std::string::npos)
		return Maybe<float>::INVALID;;

	// Can only have 1 '-' or '+', and it needs to be at the start
	size_t minusIndex = strLocal.find_first_of('-');
	if (minusIndex != std::string::npos)
	{
		if (GetCharCountInText('-', strLocal) > 1)
			return Maybe<float>::INVALID;;

		if (minusIndex != 0)
			return Maybe<float>::INVALID;;
	}

	size_t plusIndex = strLocal.find_first_of('+');
	if (plusIndex != std::string::npos)
	{
		if (GetCharCountInText('+', strLocal) > 1)
			return Maybe<float>::INVALID;;

		if (plusIndex != 0)
			return Maybe<float>::INVALID;;
	}

	// If it contains an 'f' or 'F', there better be only one, and at the end
	size_t fLocation = strLocal.find_first_of('f');
	if (fLocation != std::string::npos)
	{
		if (strLocal.size() == 1)
			return Maybe<float>::INVALID;;

		if (fLocation != strLocal.size() - 1)
			return Maybe<float>::INVALID;;
	}

	size_t capitalFLocation = strLocal.find_first_of('F');
	if (capitalFLocation != std::string::npos)
	{
		if (strLocal.size() == 1)
			return Maybe<float>::INVALID;;

		if (capitalFLocation != strLocal.size() - 1)
			return Maybe<float>::INVALID;;
	}

	// Make sure there's only one decimal point, but it can exist anywhere
	if (GetCharCountInText('.', strLocal) > 1)
		return Maybe<float>::INVALID;;

	float value = static_cast<float>(atof(strLocal.c_str()));
	Maybe<float> result = Maybe<float>(value);

	return result;
}


//-------------------------------------------------------------------------------------------------
// Attempts to parse text as an int, returning defaultValue if it fails
int ParseAsInt(const std::string& str, int defaultValue)
{
	Maybe<int> result = TryParseAsInt(str);

	if (result.IsValid())
		return result.Get();

	return defaultValue;
}


//-------------------------------------------------------------------------------------------------
// Attempts to parse text as a Vector2, returning defaultValue if it fails
Vector2 ParseAsVector2(const std::string& str, const Vector2& defaultValue)
{
	Maybe<Vector2> result = TryParseAsVector2(str);

	if (result.IsValid())
		return result.Get();

	return defaultValue;
}


//-------------------------------------------------------------------------------------------------
// Attempts to parse text as a Vector3, returning defaultValue if it fails
Vector3 ParseAsVector3(const std::string& str, const Vector3& defaultValue)
{
	Maybe<Vector3> result = TryParseAsVector3(str);

	if (result.IsValid())
		return result.Get();

	return defaultValue;
}


//-------------------------------------------------------------------------------------------------
// Attempts to parse text as a Vector2, returning defaultValue if it fails
Vector4 ParseAsVector4(const std::string& str, const Vector4& defaultValue)
{
	Maybe<Vector4> result = TryParseAsVector4(str);

	if (result.IsValid())
		return result.Get();

	return defaultValue;
}


//-------------------------------------------------------------------------------------------------
// Attempts to parse text as an IntVector2, returning defaultValue if it fails
IntVector2 ParseAsIntVector2(const std::string& str, const IntVector2& defaultValue)
{
	Maybe<IntVector2> result = TryParseAsIntVector2(str);

	if (result.IsValid())
		return result.Get();

	return defaultValue;
}


//-------------------------------------------------------------------------------------------------
// Attempts to parse text as an IntVector3, returning defaultValue if it fails
IntVector3 ParseAsIntVector3(const std::string& str, const IntVector3& defaultValue)
{
	Maybe<IntVector3> result = TryParseAsIntVector3(str);

	if (result.IsValid())
		return result.Get();

	return defaultValue;
}


//-------------------------------------------------------------------------------------------------
// Attempts to parse the given text as a single integer; returns invalid if it cannot
Maybe<int> TryParseAsInt(const std::string& str)
{
	std::string strLocal = str;
	TrimWhitespace(strLocal);

	// Must contain something
	if (strLocal.size() == 0)
		return Maybe<int>::INVALID;;

	// Make sure it's a single token, I don't want to deal with spaces in the middle
	std::vector<std::string> tokens;
	Tokenize(strLocal, ' ', tokens);
	if (tokens.size() > 1)
		return Maybe<int>::INVALID;

	// Can't contain letters
	if (strLocal.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos)
		return Maybe<int>::INVALID;;

	// Can't contain symbols other than - and +
	if (strLocal.find_first_of("!@#$%^.&*(),//'\"=_[]{}`~:;?") != std::string::npos)
		return Maybe<int>::INVALID;

	// Can only have 1 '-' or '+', and it needs to be at the start
	size_t minusIndex = strLocal.find_first_of('-');
	if (minusIndex != std::string::npos)
	{
		if (GetCharCountInText('-', strLocal) > 1)
			return Maybe<int>::INVALID;;

		if (minusIndex != 0)
			return Maybe<int>::INVALID;;
	}

	size_t plusIndex = strLocal.find_first_of('+');
	if (plusIndex != std::string::npos)
	{
		if (GetCharCountInText('+', strLocal) > 1)
			return Maybe<int>::INVALID;;

		if (plusIndex != 0)
			return Maybe<int>::INVALID;;
	}

	int value = atoi(strLocal.c_str());
	Maybe<int> result = Maybe<int>(value);

	return result;
}


//-------------------------------------------------------------------------------------------------
// Attempts to parse text as a float, returning defaultValue if it fails
float ParseAsFloat(const std::string& str, float defaultValue)
{
	Maybe<float> result = TryParseAsFloat(str);

	if (result.IsValid())
		return result.Get();

	return defaultValue;
}


//-------------------------------------------------------------------------------------------------
// Attempts to pull out the component values of the given vector
static bool TryTokenizeVector(const std::string& str, int numComponents, std::vector<std::string>& out_tokens)
{
	std::string strLocal = str;
	TrimWhitespace(strLocal);
	
	// Parentheses are optional, but must be at the start and end of the string and matching
	if (strLocal.find_first_of('(') != std::string::npos || strLocal.find_first_of(')') != std::string::npos)
	{
		if (strLocal.front() != '(')
			return false;

		if (strLocal.back() != ')')
			return false;

		if (GetCharCountInText('(', strLocal) > 1)
			return false;

		if (GetCharCountInText(')', strLocal) > 1)
			return false;

		// Parentheses are valid, so trim them off
		strLocal = strLocal.substr(1, strLocal.size() - 2);
	}

	// Vector values can be separated by spaces or ','s, so check for both
	if (strLocal.find_first_of(',') != std::string::npos)
	{
		// There needs to be the right amount of commas to delimit the components
		if (GetCharCountInText(',', strLocal) != numComponents - 1)
			return false;

		Tokenize(strLocal, ',', out_tokens);

		// If any of the commas were adjacent or at the start/end, call that invalid
		if (out_tokens.size() != numComponents)
			return false;	

		return true;
	}
	else if (strLocal.find_first_of(' ') != std::string::npos) // Separate case for spaces as I allow multiple spaces between tokens, but not multiple ','
	{
		// Get tokens
		Tokenize(strLocal, ' ', out_tokens);

		// Ensure we have a token for each component exactly
		if (out_tokens.size() != numComponents)
			return false;

		return true;
	}

	// No spaces or commas, so consider that invalid
	return false;
}


//-------------------------------------------------------------------------------------------------
// Converts all the tokens to floats, stopping and returning false if one cannot be converted
static bool GetTokensAsFloats(const std::vector<std::string>& tokens, std::vector<float>& out_floats)
{
	// Ensure each token is a valid float
	for (int i = 0; i < (int)tokens.size(); ++i)
	{
		Maybe<float> value = TryParseAsFloat(tokens[i].c_str());
		if (!value.IsValid())
			return false;

		out_floats.push_back(value.Get());
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
// Converts all the tokens to ints, stopping and returning false if one cannot be converted
static bool GetTokensAsInts(const std::vector<std::string>& tokens, std::vector<int>& out_ints)
{
	// Ensure each token is a valid float
	for (int i = 0; i < (int)tokens.size(); ++i)
	{
		Maybe<int> value = TryParseAsInt(tokens[i].c_str());
		if (!value.IsValid())
			return false;

		out_ints.push_back(value.Get());
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
// Attempts to parse the text as a Vector2; returns invalid if it fails
Maybe<Vector2> TryParseAsVector2(const std::string& str)
{
	std::vector<std::string> tokens;
	bool success = TryTokenizeVector(str, 2, tokens);

	if (!success)
		return Maybe<Vector2>::INVALID;

	std::vector<float> values;
	success = GetTokensAsFloats(tokens, values);

	if (!success)
		return Maybe<Vector2>::INVALID;

	Vector2 result = Vector2(values[0], values[1]);
	return Maybe<Vector2>(result);
}


//-------------------------------------------------------------------------------------------------
// Attempts to parse the text as a Vector3; returns invalid if it fails
Maybe<Vector3> TryParseAsVector3(const std::string& str)
{
	std::vector<std::string> tokens;
	bool success = TryTokenizeVector(str, 3, tokens);

	if (!success)
		return Maybe<Vector3>::INVALID;

	std::vector<float> values;
	success = GetTokensAsFloats(tokens, values);

	if (!success)
		return Maybe<Vector3>::INVALID;

	Vector3 result = Vector3(values[0], values[1], values[2]);
	return Maybe<Vector3>(result);
}


//-------------------------------------------------------------------------------------------------
// Attempts to parse the text as a Vector4; returns invalid if it fails
Maybe<Vector4> TryParseAsVector4(const std::string& str)
{
	std::vector<std::string> tokens;
	bool success = TryTokenizeVector(str, 4, tokens);

	if (!success)
		return Maybe<Vector4>::INVALID;

	std::vector<float> values;
	success = GetTokensAsFloats(tokens, values);

	if (!success)
		return Maybe<Vector4>::INVALID;

	Vector4 result = Vector4(values[0], values[1], values[2], values[3]);
	return Maybe<Vector4>(result);
}


//-------------------------------------------------------------------------------------------------
// Attempts to parse the text as an IntVector2; returns invalid if it fails
Maybe<IntVector2> TryParseAsIntVector2(const std::string& str)
{
	std::vector<std::string> tokens;
	bool success = TryTokenizeVector(str, 2, tokens);

	if (!success)
		return Maybe<IntVector2>::INVALID;

	std::vector<int> values;
	success = GetTokensAsInts(tokens, values);

	if (!success)
		return Maybe<IntVector2>::INVALID;

	IntVector2 result = IntVector2(values[0], values[1]);
	return Maybe<IntVector2>(result);
}


//-------------------------------------------------------------------------------------------------
// Attempts to parse the text as an IntVector3; returns invalid if it fails
Maybe<IntVector3> TryParseAsIntVector3(const std::string& str)
{
	std::vector<std::string> tokens;
	bool success = TryTokenizeVector(str, 3, tokens);

	if (!success)
		return Maybe<IntVector3>::INVALID;

	std::vector<int> values;
	success = GetTokensAsInts(tokens, values);

	if (!success)
		return Maybe<IntVector3>::INVALID;

	IntVector3 result = IntVector3(values[0], values[1], values[2]);
	return Maybe<IntVector3>(result);
}


//-------------------------------------------------------------------------------------------------
// Attempts to parse text as an Rgba color (bytes or floats), returning defaultValue if it fails
Rgba ParseAsRgba(const std::string& str, const Rgba& defaultValue)
{
	Maybe<Rgba> result = TryParseAsRgba(str);

	if (result.IsValid())
		return result.Get();

	return defaultValue;
}


//-------------------------------------------------------------------------------------------------
// Attempts to parse the text as a Vector4; returns invalid if it fails
Maybe<Rgba> TryParseAsRgba(const std::string& str)
{
	std::vector<std::string> tokens;
	bool tokenizeSuccess = TryTokenizeVector(str, 4, tokens);
	Maybe<Rgba> result = Maybe<Rgba>::INVALID;

	if (tokenizeSuccess)
	{
		// Try to converts the values to integers
		// If it succeeds, then treat the 4 numbers as 4 bytes between 0 and 255
		std::vector<int> ints;
		bool intSuccess = GetTokensAsInts(tokens, ints);

		if (intSuccess)
		{
			Rgba color = Rgba(ints[0], ints[1], ints[2], ints[3]);
			result.Set(color);
		}
		else
		{
			// Try getting them as 4 floats
			std::vector<float> floats;
			bool floatSuccess = GetTokensAsFloats(tokens, floats);

			if (floatSuccess)
			{
				Rgba color = Rgba(floats[0], floats[1], floats[2], floats[3]);
				result.Set(color);
			}
		}
	}
	else
	{
		// Treat the text as a name of a color
		if		(AreEqualCaseInsensitive(str, "white"))		{ result.Set(Rgba::WHITE); }
		else if (AreEqualCaseInsensitive(str, "black"))		{ result.Set(Rgba::BLACK); }
		else if (AreEqualCaseInsensitive(str, "red"))		{ result.Set(Rgba::RED); }
		else if (AreEqualCaseInsensitive(str, "green"))		{ result.Set(Rgba::GREEN); }
		else if (AreEqualCaseInsensitive(str, "blue"))		{ result.Set(Rgba::BLUE); }
		else if (AreEqualCaseInsensitive(str, "cyan"))		{ result.Set(Rgba::CYAN); }
		else if (AreEqualCaseInsensitive(str, "magenta"))	{ result.Set(Rgba::MAGENTA); }
		else if (AreEqualCaseInsensitive(str, "yellow"))	{ result.Set(Rgba::YELLOW); }
		else if (AreEqualCaseInsensitive(str, "gray"))		{ result.Set(Rgba::GRAY); }
		else if (AreEqualCaseInsensitive(str, "grey"))		{ result.Set(Rgba::GRAY); }
	}

	return result;
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
