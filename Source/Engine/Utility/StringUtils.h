///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 28th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
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
class NamedProperties;
class Rgba;
class Vector2;
class Vector3;
class Vector4;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
const std::string	Stringf(const char* format, ...);
const std::string	Stringf(const int maxLength, const char* format, ...);
int					GetStringLength(const char* str);
void				Tokenize(const std::string& stringToTokenize, const char delimiter, std::vector<std::string>& out_tokens);

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

int					StringToInt(const std::string& inValue);
float				StringToFloat(const std::string& inValue);
Rgba				StringToRgba(const std::string& inValue);