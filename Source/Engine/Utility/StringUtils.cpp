///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 28th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/AABB2.h"
#include "Engine/Math/AABB3.h"
#include "Engine/Math/IntVector2.h"
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


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
