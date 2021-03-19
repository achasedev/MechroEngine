///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: September 25th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Physics/2D/Collsion2D.h" // TODO: Make a collision common or something

#pragma warning(disable : 4201) // Keep the structs anonymous so we can still do myVector.x even when x is part of a struct

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/Face3.h"
#include "Engine/Math/Polygon3D.h"
#include "Engine/Math/Vector3.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
struct CollisionSeparation3d
{
	CollisionSeparation3d() {}
	CollisionSeparation3d(bool collisionFound)
		: m_collisionFound(collisionFound) {}

	bool	m_collisionFound = false;
	Vector3 m_dirFromFirst = Vector3::ZERO;
	float	m_separation = FLT_MAX;
};


//-------------------------------------------------------------------------------------------------
struct CollisionFace3d
{
	Vector3 m_furthestVertex;
	Face3	m_face;
	Vector3 m_normal;
	int		m_faceIndex = -1;
};


//-------------------------------------------------------------------------------------------------
struct RefClip
{
	bool operator==(const RefClip& other) const
	{
		return (m_referenceFaceIndex == other.m_referenceFaceIndex) && (m_indexOfVertexInReferenceFace == other.m_indexOfVertexInReferenceFace);
	}

	bool operator!=(const RefClip& other) const
	{
		return (m_referenceFaceIndex != other.m_referenceFaceIndex) || (m_indexOfVertexInReferenceFace != other.m_indexOfVertexInReferenceFace);
	}

	bool IsValid() const { return m_referenceFaceIndex >= 0; }

	int m_referenceFaceIndex = -1;
	int m_indexOfVertexInReferenceFace = -1;
};


//-------------------------------------------------------------------------------------------------
class ClipVertexId
{
public:
	//-----Public Methods-----

	ClipVertexId() {}
	ClipVertexId(void* poly, int incidentFaceIndex, int indexOfVertexInIncidentFace)
		: m_poly(poly), m_faceIndex(incidentFaceIndex), m_vertexIndex(indexOfVertexInIncidentFace) {}

	static constexpr int MAX_CLIPS = 8;

	bool operator==(const ClipVertexId& other) const
	{
		if (m_poly != other.m_poly || m_faceIndex != other.m_faceIndex || m_vertexIndex != other.m_vertexIndex)
		{
			return false;
		}

		return true;
	}

	bool IsValid() const { return m_poly != nullptr; }


private:
	//-----Private Data-----

	void*	m_poly = nullptr;
	int		m_faceIndex = -1;
	int		m_vertexIndex = -1;

};


//-------------------------------------------------------------------------------------------------
struct ClipVertex3
{
	Vector3			m_originalPosition;
	Vector3			m_position;
	ClipVertexId	m_id;
};


//-------------------------------------------------------------------------------------------------
struct Contact3D
{
	Contact3D() {}

	Vector3 m_originalPosition = Vector3::ZERO; // For debugging clipping
	Vector3 m_position = Vector3::ZERO;
	Vector3 m_normal = Vector3::ZERO;

	Vector3 m_r1 = Vector3::ZERO;	// From the center of mass of body 1 to the contact
	Vector3 m_r2 = Vector3::ZERO;	// From the center of mass of body 2 to the contact

	float m_separation = 0.f;
	float m_accNormalImpulse = 0.f;		// accumulated normal impulse
	float m_accTangentImpulse = 0.f;	// accumulated tangent impulse
	float m_normalBiasImpulse = 0.f;	// accumulated normal impulse for position bias
	float m_massNormal = 0.f;
	float m_massTangent = 0.f;
	float m_bias = 0.f;

	CollisionFace3d m_referenceFace;
	CollisionFace3d m_incidentFace;

	ClipVertexId m_id;
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


Vector3					GetMinkowskiDiffSupport3D(const Polygon3D* first, const Polygon3D* second, const Vector3& direction);
bool					SetupSimplex3D(const Polygon3D* first, const Polygon3D* second, std::vector <Vector3>& simplex);
EvolveSimplexResult		EvolveSimplex3D(const Polygon3D* first, const Polygon3D* second, std::vector<Vector3>& evolvingSimplex);
uint32					GetSimplexSeparation3D(const std::vector<Vector3>& simplex, CollisionSeparation3d& out_separation);
CollisionSeparation3d	PerformEPA3D(const Polygon3D* first, const Polygon3D* second, std::vector<Vector2>& simplex);
CollisionSeparation3d	CalculateSeparation3D(const Polygon3D* first, const Polygon3D* second);
CollisionFace3d	GetFeatureFace3D(const Polygon3D* polygon, const Vector3& outwardSeparationNormal);
void					ClipIncidentFaceToReferenceFace(const std::vector<ClipVertex3>& incidentVertices, const Vector2& refEdgeDirection, float offset, std::vector<ClipVertex2D>& clippedPoints);

#pragma warning(default : 4201)
