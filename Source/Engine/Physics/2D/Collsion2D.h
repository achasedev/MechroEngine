///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: September 25th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once
#pragma warning(disable : 4201) // Keep the structs anonymous so we can still do myVector.x even when x is part of a struct

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Polygon2D.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
enum EvolveSimplexResult
{
	NO_INTERSECTION,
	INTERSECTION_FOUND,
	SIMPLEX_STILL_EVOLVING
};

//-------------------------------------------------------------------------------------------------
struct CollisionSeparation2D
{
	CollisionSeparation2D() {}
	CollisionSeparation2D(bool collisionFound)
		: m_collisionFound(collisionFound) {}

	bool	m_collisionFound = false;
	Vector2 m_dirFromFirst;
	float	m_separation = FLT_MAX;
};

//-------------------------------------------------------------------------------------------------
struct CollisionFeatureEdge2D
{
	Vector2 m_furthestVertex;
	Vector2 m_vertex1;
	Vector2 m_vertex2;
	Vector2 m_normal;
	int		m_edgeId = -1;
};

//-------------------------------------------------------------------------------------------------
struct EdgePairID2D
{
	union
	{
		struct
		{
			// Edge IDs - always labeled after the index of their endpoint, since we need 0 to be a cleared/invalid ID
			//
			//        ^ y
			//        |
			//        e2
			//   v1 ------ v2
			//    |        |
			// e1 |		   | e3  --> x
			//    |        |
			//   v0 ------ v3
			//        e4
			//
			int8 m_incidentEdgeIn;
			int8 m_incidentEdgeOut;
			int8 m_minRefEdgeClipped;
			int8 m_maxRefEdgeClipped;
		};
		int m_value = 0;
	};
};

//-------------------------------------------------------------------------------------------------
struct ClipVertex2D
{
	Vector2			m_position;
	EdgePairID2D	m_id;
};

//-------------------------------------------------------------------------------------------------
struct Contact2D
{
	Contact2D() {}

	Vector2 m_position = Vector2::ZERO;
	Vector2 m_normal = Vector2::ZERO;

	Vector2 m_r1 = Vector2::ZERO;	// From the center of mass of body 1 to the contact
	Vector2 m_r2 = Vector2::ZERO;	// From the center of mass of body 2 to the contact

	float m_separation = 0.f;
	float m_accNormalImpulse = 0.f;	// accumulated normal impulse
	float m_accTangentImpulse = 0.f;	// accumulated tangent impulse
	float m_normalBiasImpulse = 0.f;	// accumulated normal impulse for position bias
	float m_massNormal = 0.f;
	float m_massTangent = 0.f;
	float m_bias = 0.f;

	CollisionFeatureEdge2D m_referenceEdge;
	CollisionFeatureEdge2D m_incidentEdge;

	EdgePairID2D m_id;
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------


Vector2					GetMinkowskiDiffSupport2D(const Polygon2D* first, const Polygon2D* second, const Vector2& direction);
void					SetupSimplex2D(const Polygon2D* first, const Polygon2D* second, std::vector <Vector2>& simplex);
EvolveSimplexResult		EvolveSimplex2D(const Polygon2D* first, const Polygon2D* second, std::vector<Vector2>& evolvingSimplex);
uint32					GetSimplexSeparation2D(const std::vector<Vector2>& simplex, CollisionSeparation2D& out_separation);
CollisionSeparation2D	PerformEPA(const Polygon2D* first, const Polygon2D* second, std::vector<Vector2>& simplex);
CollisionSeparation2D	CalculateSeparation2D(const Polygon2D* first, const Polygon2D* second);
CollisionFeatureEdge2D	GetFeatureEdge2D(const Polygon2D* polygon, const Vector2& outwardSeparationNormal);
void					ClipIncidentEdgeToReferenceEdge(const ClipVertex2D& incident1, const ClipVertex2D& incident2, const Vector2& refEdgeDirection, float offset, std::vector<ClipVertex2D>& clippedPoints);

#pragma warning(default : 4201)
