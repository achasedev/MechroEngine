///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 23th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include <string>
#include "Engine/Core/EngineCommon.h"
#include "Engine/Utility/StringID.h"
#include "Engine/Utility/StringUtils.h"

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
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class BaseProperty 
{
public:
	//-----Public Methods-----

	virtual std::string GetValueAsString() const = 0;
	virtual void const* GetTypeID() const = 0;

};


//-------------------------------------------------------------------------------------------------
template <typename T>
class TypedProperty : public BaseProperty
{
public:
	//-----Public Methods-----

	virtual std::string GetValueAsString() const override	{ return ToString(m_value); }
	void				SetValue(const T& value)			{ m_value = value; }
	T					GetValue()							{ return m_value; }	
	virtual void const* GetTypeID() const override			{ return &s_typeID; }
	static void const*	GetTypeIDStatic()					{ return &s_typeID; }


private:
	//-----Private Data-----

	T m_value;
	static constexpr uint32 s_typeID = 0;

};


//-------------------------------------------------------------------------------------------------
class NamedProperties
{
public:
	//-----Public Methods-----

	template <typename T> void	Set(const StringID& name, const T& value);
	template <typename T> T		Get(const StringID& name, const T& defaultValue);

	// Helpers
	template <typename T> void	Set(const char* name, const T& value);
	template <typename T> void	Set(const std::string& name, const T& value);
	template <typename T> T		Get(const char* name, const T& defaultValue);
	template <typename T> T		Get(const std::string& name, const T& defaultValue);

	std::string					ToString() const;

	// const char* specializations
	void						Set(const StringID& name, const char* value);
	void						Set(const char* name, const char* value);
	void						Set(const std::string& name, const char* value);
	std::string					Get(const StringID& name, const char* defaultValue);
	std::string					Get(const char* name, const char* defaultValue);
	std::string					Get(const std::string& name, const char* defaultValue);


private:
	//-----Private Data-----

	std::map<StringID, BaseProperty*> m_properties;

};


//-------------------------------------------------------------------------------------------------
template <typename T>
void NamedProperties::Set(const StringID& name, const T& value)
{
	bool alreadyExists = m_properties.find(name) != m_properties.end();

	if (alreadyExists)
	{
		BaseProperty* existingProperty = m_properties[name];

		// Ensure the type is the same
		ASSERT_RECOVERABLE(existingProperty->GetTypeID() == TypedProperty<T>::GetTypeIDStatic(), "Named property is being reset with a different type!");

		// Free the old value
		SAFE_DELETE(existingProperty);
	}

	TypedProperty<T>* tp = new TypedProperty<T>();
	tp->SetValue(value);
	m_properties[name] = tp;
}



//-------------------------------------------------------------------------------------------------
template <typename T>
T NamedProperties::Get(const StringID& name, const T& defaultValue)
{
	bool propertyExists = m_properties.find(name) != m_properties.end();

	if (propertyExists)
	{
		BaseProperty* existingProperty = m_properties[name];

		// Ensure the type is the one expected
		bool typesMatch = existingProperty->GetTypeID() == TypedProperty<T>::GetTypeIDStatic();
		ASSERT_RECOVERABLE(typesMatch, "Named property is being Get() as different type, returning default!");

		if (typesMatch)
		{
			TypedProperty<T>* tp = static_cast<TypedProperty<T>*>(existingProperty);
			return tp->GetValue();
		}
	}

	// Property doesn't exist or is mismatched type, return the default
	return defaultValue;
}


//-------------------------------------------------------------------------------------------------
template <typename T>
void NamedProperties::Set(const char* name, const T& value)
{
	Set(SID(name), value);
}


//-------------------------------------------------------------------------------------------------
template <typename T> 
T NamedProperties::Get(const char* name, const T& defaultValue)
{
	return Get(SID(name), defaultValue);
}


//-------------------------------------------------------------------------------------------------
template <typename T> 
void NamedProperties::Set(const std::string& name, const T& value)
{
	Set(SID(name.c_str()), value);
}


//-------------------------------------------------------------------------------------------------
template <typename T>
T NamedProperties::Get(const std::string& name, const T& defaultValue)
{
	return Get(SID(name.c_str()), defaultValue);
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
