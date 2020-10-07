///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 1st, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "ThirdParty/tinyxml2/tinyxml2.h"
#include <string>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
typedef tinyxml2::XMLDocument XMLDocument;
typedef tinyxml2::XMLElement XMLElement;
typedef tinyxml2::XMLAttribute XMLAttribute;
typedef tinyxml2::XMLError XMLError;

class Rgba;
class Vector2;
class IntVector2;
class AABB2;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

namespace XML
{
	int				ParseAttribute(const XMLElement& element, const char* attributeName, int defaultValue);
	char			ParseAttribute(const XMLElement& element, const char* attributeName, char defaultValue);
	bool			ParseAttribute(const XMLElement& element, const char* attributeName, bool defaultValue);
	float			ParseAttribute(const XMLElement& element, const char* attributeName, float defaultValue);
	Rgba			ParseAttribute(const XMLElement& element, const char* attributeName, const Rgba& defaultValue);
	Vector2			ParseAttribute(const XMLElement& element, const char* attributeName, const Vector2& defaultValue);
	Vector3			ParseAttribute(const XMLElement& element, const char* attributeName, const Vector3& defaultValue);
	Vector4			ParseAttribute(const XMLElement& element, const char* attributeName, const Vector4& defaultValue);
	IntVector2		ParseAttribute(const XMLElement& element, const char* attributeName, const IntVector2& defaultValue);
	IntVector3		ParseAttribute(const XMLElement& element, const char* attributeName, const IntVector3& defaultValue);
	AABB2			ParseAttribute(const XMLElement& element, const char* attributeName, const AABB2& defaultValue);
	std::string		ParseAttribute(const XMLElement& element, const char* attributeName, const std::string& defaultValue);
	std::string		ParseAttribute(const XMLElement& element, const char* attributeName, const char* defaultValue = nullptr);
	bool			DoesAttributeExist(const XMLElement& element, const char* attributeName);
}