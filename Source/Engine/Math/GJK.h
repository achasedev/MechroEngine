///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Nov 12th, 2021
/// Description: File for all GJK computations
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Polygon2;
class Polygon3;
class Polyhedron;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

#pragma warning(disable : 4201) // Keep the structs anonymous

//-------------------------------------------------------------------------------------------------
class GJKSolver2D
{
public:
	//-----Public Methods-----

	GJKSolver2D();
	float Solve(const Vector2& point, const Polygon2* poly, Vector2& out_closestPt);
	float Solve(const Vector3& point, const Polygon3* poly, Vector3& out_closestPt);


private:
	//-----Private Methods-----

	void StartEvolution();
	bool EvolveFromPoint();
	bool EvolveFromSegment();
	bool EvolveFromTriangle();
	void CleanUpVertices();


private:
	//-----Private Data-----

	// Input
	Vector2			m_point = Vector2::ZERO;
	const Polygon2* m_poly = nullptr;

	// Simplex vertices - as index into poly vertices
	union
	{
		int	m_iVert[3];
		struct
		{
			int m_iA;
			int m_iB;
			int m_iC;
		};
	};
	int m_numVerts = 0;

	// Results
	Vector2			m_closestPt = Vector2::ZERO;
	float			m_minDist = -1.0f;

};


//-------------------------------------------------------------------------------------------------
class GJKSolver3D
{
public:
	//-----Public Methods-----

	GJKSolver3D();
	float Solve(const Vector3& point, const Polyhedron* poly, Vector3& out_closestPt);


private:
	//-----Private Methods-----

	void StartEvolution();
	bool EvolveFromPoint();
	bool EvolveFromSegment();
	bool EvolveFromTriangle();
	bool EvolveFromTetrahedron();
	void CleanUpVertices();


private:
	//-----Private Data-----

	// Input
	Vector3				m_point = Vector3::ZERO;
	const Polyhedron*	m_poly = nullptr;

	// Simplex vertices - as index into poly vertices
	union
	{
		int	m_iVert[4];
		struct
		{
			int m_iA;
			int m_iB;
			int m_iC;
			int m_iD;
		};
	};
	int m_numVerts = 0;

	// Results
	Vector3			m_closestPt = Vector3::ZERO;
	float			m_minDist = -1.0f;

};

#pragma warning(default : 4201)

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------