///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 28th, 2019
/// Description: 
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
class Vector2;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class IntVector2
{

public:

	IntVector2(int initialValue);
	IntVector2() {}
	IntVector2(const IntVector2& copyFrom);	
	explicit IntVector2(int initialX, int initialY);
	explicit IntVector2(unsigned int initialX, unsigned int initialY);
	explicit IntVector2(float initialX, float initialY);
	explicit IntVector2(const Vector2& floatVector);
	~IntVector2() {}

	const IntVector2 operator+(const IntVector2& addVector) const;
	const IntVector2 operator-(const IntVector2& subVector) const;
	const IntVector2 operator*(int uniformScaler) const;
	const IntVector2 operator/(int uniformDivisor) const;
	void operator+=(const IntVector2& vecToAdd);							
	void operator-=(const IntVector2& vecToSubtract);
	void operator*=(const int uniformScale);
	void operator=(const IntVector2& copyFrom);		
	bool operator==(const IntVector2& compare) const;
	bool operator!=(const IntVector2& compare) const;
	friend const IntVector2 operator*(int uniformScale, const IntVector2& vecToScale);

public:

	const static IntVector2 ZERO;
	const static IntVector2 ONES;

public:

	int x;
	int y;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
