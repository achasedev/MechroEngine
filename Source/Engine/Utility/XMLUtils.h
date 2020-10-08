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
typedef tinyxml2::XMLDocument XMLDoc;
typedef tinyxml2::XMLElement XMLElem;
typedef tinyxml2::XMLAttribute XMLAttr;
typedef tinyxml2::XMLError XMLErr;

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
	int				ParseAttribute(const XMLElem& element, const char* attributeName, int defaultValue);
	char			ParseAttribute(const XMLElem& element, const char* attributeName, char defaultValue);
	bool			ParseAttribute(const XMLElem& element, const char* attributeName, bool defaultValue);
	float			ParseAttribute(const XMLElem& element, const char* attributeName, float defaultValue);
	Rgba			ParseAttribute(const XMLElem& element, const char* attributeName, const Rgba& defaultValue);
	Vector2			ParseAttribute(const XMLElem& element, const char* attributeName, const Vector2& defaultValue);
	Vector3			ParseAttribute(const XMLElem& element, const char* attributeName, const Vector3& defaultValue);
	Vector4			ParseAttribute(const XMLElem& element, const char* attributeName, const Vector4& defaultValue);
	IntVector2		ParseAttribute(const XMLElem& element, const char* attributeName, const IntVector2& defaultValue);
	IntVector3		ParseAttribute(const XMLElem& element, const char* attributeName, const IntVector3& defaultValue);
	AABB2			ParseAttribute(const XMLElem& element, const char* attributeName, const AABB2& defaultValue);
	std::string		ParseAttribute(const XMLElem& element, const char* attributeName, const std::string& defaultValue);
	std::string		ParseAttribute(const XMLElem& element, const char* attributeName, const char* defaultValue = nullptr);
	bool			DoesAttributeExist(const XMLElem& element, const char* attributeName);
}