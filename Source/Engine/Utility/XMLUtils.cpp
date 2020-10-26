///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 1st, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/Rgba.h"
#include "Engine/Utility/XMLUtils.h"
#include "Engine/Math/AABB2.h"
#include "Engine/Math/IntVector2.h"
#include "Engine/Math/IntVector3.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
// TODO: Print/log warnings if any of these functions fail, maybe use QueryAttribute to get whether it's missing or a failed conversion

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
int XML::ParseAttribute(const XMLElem& element, const char* attributeName, int defaultValue)
{
	return element.IntAttribute(attributeName, defaultValue);
}


//-------------------------------------------------------------------------------------------------
bool XML::DoesAttributeExist(const XMLElem& element, const char* attributeName)
{
	return (element.Attribute(attributeName) != nullptr);
}


//-------------------------------------------------------------------------------------------------
char XML::ParseAttribute(const XMLElem& element, const char* attributeName, char defaultValue)
{
	const char* attributeText = element.Attribute(attributeName);

	if (attributeText != nullptr)
	{
		return *attributeText;
	}

	return defaultValue;
}


//-------------------------------------------------------------------------------------------------
bool XML::ParseAttribute(const XMLElem& element, const char* attributeName, bool defaultValue)
{
	return element.BoolAttribute(attributeName, defaultValue);
}


//-------------------------------------------------------------------------------------------------
float XML::ParseAttribute(const XMLElem& element, const char* attributeName, float defaultValue)
{
	return element.FloatAttribute(attributeName, defaultValue);
}


//-------------------------------------------------------------------------------------------------
Rgba XML::ParseAttribute(const XMLElem& element, const char* attributeName, const Rgba& defaultValue)
{
	Rgba result = defaultValue;
	const char* attributeText = element.Attribute(attributeName);

	if (attributeText != nullptr)
	{
		StringToRgba(attributeText, result);
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
Vector2 XML::ParseAttribute(const XMLElem& element, const char* attributeName, const Vector2& defaultValue)
{
	Vector2 result = defaultValue;
	const char* attributeText = element.Attribute(attributeName);

	if (attributeText != nullptr)
	{
		result = StringToVector2(attributeText);
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
Vector3 XML::ParseAttribute(const XMLElem& element, const char* attributeName, const Vector3& defaultValue)
{
	Vector3 result = defaultValue;
	const char* attributeText = element.Attribute(attributeName);

	if (attributeText != nullptr)
	{
		result = StringToVector3(attributeText);
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
Vector4 XML::ParseAttribute(const XMLElem& element, const char* attributeName, const Vector4& defaultValue)
{
	Vector4 result = defaultValue;
	const char* attributeText = element.Attribute(attributeName);

	if (attributeText != nullptr)
	{
		result = StringToVector4(attributeText);
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
IntVector2 XML::ParseAttribute(const XMLElem& element, const char* attributeName, const IntVector2& defaultValue)
{
	IntVector2 result = defaultValue;
	const char* attributeText = element.Attribute(attributeName);

	if (attributeText != nullptr)
	{
		result = StringToIntVector2(attributeText);
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
IntVector3 XML::ParseAttribute(const XMLElem& element, const char* attributeName, const IntVector3& defaultValue)
{
	IntVector3 result = defaultValue;
	const char* attributeText = element.Attribute(attributeName);

	if (attributeText != nullptr)
	{
		result = StringToIntVector3(attributeText);
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
AABB2 XML::ParseAttribute(const XMLElem& element, const char* attributeName, const AABB2& defaultValue)
{
	AABB2 result;
	result.vectorData = ParseAttribute(element, attributeName, defaultValue.vectorData);

	return result;
}


//-------------------------------------------------------------------------------------------------
std::string XML::ParseAttribute(const XMLElem& element, const char* attributeName, const std::string& defaultValue)
{
	const char* attributeText = element.Attribute(attributeName);

	if (attributeText == nullptr)
	{
		return defaultValue;
	}
	else
	{
		return std::string(attributeText);
	}
}


//-------------------------------------------------------------------------------------------------
std::string XML::ParseAttribute(const XMLElem& element, const char* attributeName, const char* defaultValue/*=nullptr */)
{
	const char* attributeText = element.Attribute(attributeName);

	if (attributeText == nullptr)
	{
		if (defaultValue != nullptr)
		{
			return std::string(defaultValue);
		}
		else
		{
			return std::string("");
		}
	}
	else
	{
		return std::string(attributeText);
	}
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
