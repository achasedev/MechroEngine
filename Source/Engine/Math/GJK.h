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
class Point;
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
template <class A, class B>
class GJKSolver3D
{
public:
	//-----Public Methods-----

	GJKSolver3D(const A& a, const B& b);
	GJKSolver3D(const GJKSolver3D<A, B>& copy);

	bool Solve();

	Vector3 GetSeparationNormal() const { return m_separationNormal; }
	float	GetSeparationDistance() const { return m_separation; }
	Vector3 GetClosestPointOnA() const { return m_closestPtA; }
	Vector3 GetClosestPointOnB() const { return m_closestPtB; }


private:
	//-----Private Methods-----

	bool CheckSimplexLineSegment();
	bool CheckSimplexTriangle();
	bool CheckSimplexTetrahedron();
	void CleanUpSimplexPoints();

	void	ExpandSimplex();
	Vector3 GetMinkowskiSupportPoint(const Vector3& direction);
	bool	IsSimplexDegenerate() const;
	void	ComputeClosestPoints();

	void	PerformEPA();


private:
	//-----Private Data-----

	const A& m_a;
	const B& m_b;

	// Simplex vertices - as index into poly vertices
	union
	{
		Maybe<Vector3> m_simplexPts[4];
		struct
		{
			Maybe<Vector3> m_simplexA;
			Maybe<Vector3> m_simplexB;
			Maybe<Vector3> m_simplexC;
			Maybe<Vector3> m_simplexD;
		};
	};
	int m_numVerts = 0;

	// Minkowski inputs for simplexes
	Vector3 m_minkowskiInputs[8];

	// Results
	Vector3 m_separationNormal = Vector3::ZERO;
	float	m_separation = 0.f;
	Vector3 m_closestPtA = Vector3::ZERO;
	Vector3 m_closestPtB = Vector3::ZERO;

};


#pragma warning(default : 4201)

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------