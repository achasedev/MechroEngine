///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 18th, 2020
/// Description: Template class for custom access to math members
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------

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
template <typename T, typename COMP_TYPE, size_t... OFFSETS>
class Swizzle
{
public:
	//-----Public Methods-----

	// Conversion to type (vec2 val = color.rr)
	// When returning as this type, construct it
	// Type should have a constructor that takes the members required
	operator T() const
	{
		T ret; // Assumes type is tightly packed array of elements
		Get<OFFSETS...>((COMP_TYPE*) &ret);
		return ret;
	}

	// For setting
	void operator=(const T& copy)
	{
		Set<OFFSETS...>((COMP_TYPE*) &copy);
	}


private:
	//-----Private Methods-----

	// - Helpers for accessing parent memory -
	template <size_t OFFSET>
	inline COMP_TYPE* GetPointer()
	{
		COMP_TYPE* ptr = (COMP_TYPE*)this;
		return ptr + OFFSET;
	}

	template <size_t OFFSET>
	inline const COMP_TYPE* GetPointer() const
	{
		const COMP_TYPE* ptr = (const COMP_TYPE*)this;
		return ptr + OFFSET;
	}

	// - Recursive component accessors -
	template <size_t OFFSET>
	inline void Get(COMP_TYPE* out) const
	{
		*out = *GetPointer<OFFSET>();
	}

	template <size_t OFFSET, size_t OFFSET1, size_t... REM_OFFSETS>
	inline void Get(COMP_TYPE* out) const
	{
		Get<OFFSET>(out);
		Get<OFFSET1, REM_OFFSETS...>(out + 1U);
	}

	// - Recursive component mutators -
	template <size_t OFFSET>
	inline void Set(const COMP_TYPE* src)
	{
		COMP_TYPE* dst = GetPointer<OFFSET>();
		*dst = *src;
	}

	template <size_t OFFSET, size_t OFFSET1, size_t ...REM_OFFSETS>
	inline void Set(const COMP_TYPE* src)
	{
		Set<OFFSET>(src);
		Set<OFFSET1, REM_OFFSETS...>(src + 1U);
	}
};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
