///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 28th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Utility/Maybe.h"
#include <string>
#include <vector>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class AABB2;
class AABB3;
class IntVector2;
class IntVector3;
class NamedProperties;
class Rgba;
class Vector2;
class Vector3;
class Vector4;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
const int VARIABLE_ARG_STACK_LOCAL_TEMP_LENGTH = 2048;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
const std::string	Stringf(const char* format, ...);
const std::string	Stringf(const int maxLength, const char* format, ...);
int					GetStringLength(const char* str);
int					GetCharCountInText(const char character, const char* text);
void				Tokenize(const std::string& stringToTokenize, const char delimiter, std::vector<std::string>& out_tokens);
void				TrimWhitespace(std::string& stringToTrim);
void				BreakStringIntoLines(const std::string& stringToBreak, std::vector<std::string>& out_lines);
bool				AreEqualCaseInsensitive(const std::string& first, const std::string& second);
std::string			GetFilePathExtension(const char* filepath);
std::string			GetFilePathExtension(const std::string& filepath);
bool				DoesFilePathHaveExtension(const std::string& filepath, const std::string& extension);

std::string			ToString(float inValue);
std::string			ToString(int inValue);
std::string			ToString(std::string inValue);
std::string			ToString(const Vector2& inValue);
std::string			ToString(const Vector3& inValue);
std::string			ToString(const Vector4& inValue);
std::string			ToString(const IntVector2& inValue);
std::string			ToString(const AABB2& inValue);
std::string			ToString(const AABB3& inValue);
std::string			ToString(const NamedProperties& inValue);
std::string			ToString(void* inValue); // For allowing pointers to be stored in NamedProperties

Maybe<bool>			TryParseAsBool(const char* text);
Maybe<float>		TryParseAsFloat(const char* text);
Maybe<int>			TryParseAsInt(const char* text);
Maybe<Vector2>		TryParseAsVector2(const char* text);
Maybe<Vector3>		TryParseAsVector3(const char* text);
Maybe<Vector4>		TryParseAsVector4(const char* text);
Maybe<IntVector2>	TryParseAsIntVector2(const char* text);
Maybe<IntVector3>	TryParseAsIntVector3(const char* text);
Maybe<Rgba>			TryParseAsRgba(const char* text);

bool				ParseAsBool(const char* text, bool defaultValue);
float				ParseAsFloat(const char* text, float defaultValue);
int					ParseAsInt(const char* text, int defaultValue);
Vector2				ParseAsVector2(const char* text, const Vector2& defaultValue);
Vector3				ParseAsVector3(const char* text, const Vector3& defaultValue);
Vector4				ParseAsVector4(const char* text, const Vector4& defaultValue);
IntVector2			ParseAsIntVector2(const char* text, const IntVector2& defaultValue);
IntVector3			ParseAsIntVector3(const char* text, const IntVector3& defaultValue);
Rgba				ParseAsRgba(const char* text, const Rgba& defaultValue);
