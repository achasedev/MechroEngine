///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Oct 24th, 2021
/// Description: Class to describe a single variable in a constant buffer
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Utility/StringID.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
enum PropertyDataType
{
	PROPERTY_TYPE_INVALID = -1,
	PROPERTY_TYPE_FLOAT,
	PROPERTY_TYPE_INT,
	PROPERTY_TYPE_VECTOR2,
	PROPERTY_TYPE_VECTOR3,
	PROPERTY_TYPE_VECTOR4,
	PROPERTY_TYPE_INTVECTOR2,
	PROPERTY_TYPE_INTVECTOR3,
	PROPERTY_TYPE_MATRIX3,
	PROPERTY_TYPE_MATRIX4,
	PROPERTY_TYPE_STRUCT,
	NUM_PROPERTY_TYPES
};
class PropertyBlockDescription;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class PropertyDescription
{
public:
	//-----Public Methods-----

	PropertyDescription(const StringID& name, const PropertyBlockDescription* owningBlockDescription, int byteOffset, int byteSize, PropertyDataType type);

	StringID						GetName() const { return m_name; }
	int								GetByteOffset() const { return m_byteOffset; }
	int								GetByteSize() const { return m_byteSize; }
	PropertyDataType				GetDataType() const { return m_type; }
	const PropertyBlockDescription*	GetOwningBlockDescription() const { return m_owningBlockDesc; }

private:
	//-----Private Data-----

	StringID						m_name;
	int								m_byteOffset = -1;
	int								m_byteSize = -1;
	PropertyDataType				m_type = PROPERTY_TYPE_INVALID;
	const PropertyBlockDescription* m_owningBlockDesc = nullptr;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------