///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: September 25th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

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
struct EdgePairID
{
	union
	{
		struct
		{
			// !!! Only two of these will be set! !!!
			// We're labeling the id of the edge into this point and edge out of this point
			// For a given point, both could be from incident poly, or 1 from incident poly and 1 from reference poly

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
			// For example, for a contact point at v3 for the given incident poly above, it will start off with
			// incident in as e2, incident out as e3. After clipping, it's possible one of these two will be cleared
			// and have a reference set instead, such as clearing incident in to 0 and setting reference in to reference edge e7
			int8 m_incidentEdgeIn;
			int8 m_incidentEdgeOut;
			int8 m_referenceEdgeIn;
			int8 m_referenceEdgeOut;
		};
		int m_value = 0;
	};
};

//-------------------------------------------------------------------------------------------------
struct ClipVertex
{
	Vector2		m_position;
	EdgePairID	m_id;
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
	float accumulatedNormalImpulse = 0.f;	// accumulated normal impulse
	float m_accumulatedTangentImpulse = 0.f;	// accumulated tangent impulse
	float m_pNB = 0.f;	// accumulated normal impulse for position bias
	float m_massNormal = 0.f;
	float m_massTangent = 0.f;
	float m_bias = 0.f;

	CollisionFeatureEdge2D m_referenceEdge;
	CollisionFeatureEdge2D m_incidentEdge;

	EdgePairID m_id;
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
void					ClipIncidentEdgeToReferenceEdge(const ClipVertex& incident1, const ClipVertex& incident2, const Vector2& refEdgeDirection, float offset, std::vector<ClipVertex>& clippedPoints);