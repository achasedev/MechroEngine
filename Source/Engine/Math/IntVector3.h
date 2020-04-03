///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 2nd, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class IntVector2;
class Vector3;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class IntVector3
{

public:
	//-----Public Methods-----

	IntVector3(int initialValue);
	IntVector3() {}
	IntVector3(const IntVector3& copyFrom);
	explicit IntVector3(int initialX, int initialY, int initialZ);
	explicit IntVector3(uint32 initialX, uint32 initialY, uint32 initialZ);
	explicit IntVector3(float initialX, float initialY, float initialZ);
	explicit IntVector3(const Vector3& floatVector);
	explicit IntVector3(const IntVector2& xyVector, int initialZ);
	~IntVector3() {}

	const IntVector3		operator+(const IntVector3& addVector) const;
	const IntVector3		operator-(const IntVector3& subVector) const;
	const IntVector3		operator*(int uniformScaler) const;
	const IntVector3		operator/(int uniformDivisor) const;
	void					operator+=(const IntVector3& vecToAdd);
	void					operator-=(const IntVector3& vecToSubtract);
	void					operator*=(const int uniformScale);
	void					operator=(const IntVector3& copyFrom);
	bool					operator==(const IntVector3& compare) const;
	bool					operator!=(const IntVector3& compare) const;
	friend const IntVector3 operator*(int uniformScale, const IntVector3& vecToScale);

public:
	//-----Public Static Data-----

	const static IntVector3 ZERO;
	const static IntVector3 ONES;


public:
	//-----Public Data-----

	int x;
	int y;
	int z;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
