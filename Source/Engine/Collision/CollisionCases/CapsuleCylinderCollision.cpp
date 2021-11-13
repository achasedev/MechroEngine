///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Nov 11th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Collision/Contact.h"
#include "Engine/Collision/Collider.h"
#include "Engine/Collision/CollisionCases/CapsuleCylinderCollision.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.h"

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
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
CapsuleCylinderCollision::CapsuleCylinderCollision(const CapsuleCollider* capsuleCollider, const CylinderCollider* cylinderCollider, Contact* out_contacts, int contactLimit)
	: m_capsuleCollider(capsuleCollider), m_cylinderCollider(cylinderCollider), m_contacts(out_contacts), m_contactLimit(contactLimit)
{
	m_capsule = m_capsuleCollider->GetDataInWorldSpace();
	m_cylinder = m_cylinderCollider->GetDataInWorldSpace();

	InitializeValues();
}


//-------------------------------------------------------------------------------------------------
void CapsuleCylinderCollision::Solve()
{
	SolveVertical();
	SolveHorizontal();
	SolveEdge();
	MakeContacts();
}


//-------------------------------------------------------------------------------------------------
void CapsuleCylinderCollision::InitializeValues()
{
	m_verticalPens[0] = FLT_MAX;
	m_verticalPens[1] = FLT_MAX;
	m_verticalPositions[0] = Vector3::ZERO;
	m_verticalPositions[1] = Vector3::ZERO;

	m_capSpineDir = m_capsule.end - m_capsule.start;
	m_capHeight = m_capSpineDir.Normalize();

	m_cylSpineDir = m_cylinder.m_top - m_cylinder.m_bottom;
	m_cylHeight = m_cylSpineDir.Normalize();

	m_capRadius = m_capsule.radius;
	m_cylRadius = m_cylinder.m_radius;

	m_cylTopPlane = Plane3(m_cylSpineDir, m_cylinder.m_top);
	m_cylBottomPlane = Plane3(-1.0f * m_cylSpineDir, m_cylinder.m_bottom);

	m_distBetweenSegs = FindClosestPointsOnLineSegments(m_capsule.start, m_capsule.end, m_cylinder.m_bottom, m_cylinder.m_top, m_capClosestSegPt, m_cylClosestSegPt);
}


//-------------------------------------------------------------------------------------------------
void CapsuleCylinderCollision::SolveVertical()
{
	// Check if the endpoints are vertically aligned
	// If both aren't, no vertical contacts
	Vector3 startProjPoint = m_cylTopPlane.GetProjectedPointOntoPlane(m_capsule.start);
	Vector3 startDiscVector = startProjPoint - m_cylinder.m_top;
	bool startVertical = (startDiscVector.GetLengthSquared() < m_cylRadius * m_cylRadius);

	Vector3 endProjPoint = m_cylTopPlane.GetProjectedPointOntoPlane(m_capsule.end);
	Vector3 endDiscVector = endProjPoint - m_cylinder.m_top;
	bool endVertical = (endDiscVector.GetLengthSquared() < m_cylRadius * m_cylRadius);

	if (startVertical && endVertical)
	{
		float topPens[2];
		topPens[0] = m_capsule.radius - m_cylTopPlane.GetDistanceFromPlane(m_capsule.start);
		topPens[1] = m_capsule.radius - m_cylTopPlane.GetDistanceFromPlane(m_capsule.end);
		
		float bottomPens[2];
		bottomPens[0] = m_capsule.radius - m_cylBottomPlane.GetDistanceFromPlane(m_capsule.start);
		bottomPens[1] = m_capsule.radius - m_cylBottomPlane.GetDistanceFromPlane(m_capsule.end);

		bool startWantsTop = topPens[0] < bottomPens[0];
		bool endWantsTop = topPens[1] < bottomPens[1];
		bool endpointsAgree = (startWantsTop == endWantsTop);
		bool bothOverlapping = (Min(topPens[0], bottomPens[0]) > 0.f && Min(topPens[1], bottomPens[1]) > 0.f);

		if (endpointsAgree && bothOverlapping)
		{
			// Make two contacts
			if (startWantsTop)
			{
				m_verticalNormal = m_cylTopPlane.m_normal;
				m_verticalPens[0] = topPens[0];
				m_verticalPens[1] = topPens[1];
			}
			else
			{
				m_verticalNormal = m_cylBottomPlane.m_normal;
				m_verticalPens[0] = bottomPens[0];
				m_verticalPens[1] = bottomPens[1];
			}

			m_verticalPositions[0] = m_capsule.start - m_capRadius * m_verticalNormal;
			m_verticalPositions[1] = m_capsule.end - m_capRadius * m_verticalNormal;
			m_numVerticalContacts = 2;
			m_worstVerticalPen = Max(m_verticalPens[0], m_verticalPens[1]);
			if (m_worstVerticalPen < 0.f) { m_worstVerticalPen = FLT_MAX; }
		}
		else
		{
			// Choose the min pen direction
			float worstTopPen = Max(topPens[0], topPens[1]);
			float worstBottomPen = Max(bottomPens[0], bottomPens[1]);

			if (Min(worstBottomPen, worstTopPen) < 0.f)
				return;

			if (worstTopPen < worstBottomPen)
			{
				m_verticalPens[0] = worstTopPen;
				m_verticalNormal = m_cylTopPlane.m_normal;
				m_verticalPositions[0] = (worstTopPen == topPens[0] ? m_capsule.start : m_capsule.end) - m_capsule.radius * m_verticalNormal;
			}
			else
			{
				m_verticalPens[0] = worstBottomPen;
				m_verticalNormal = m_cylBottomPlane.m_normal;
				m_verticalPositions[0] = (worstBottomPen == bottomPens[0] ? m_capsule.start : m_capsule.end) - m_capsule.radius * m_verticalNormal;
			}

			m_numVerticalContacts = 1;
			m_worstVerticalPen = m_verticalPens[0];
		}

		return;
	}

	Maybe<Vector3> capVertPt;	// Capsule endpoint that is vertical (if any)
	Vector3 capOtherPt;			// Capsule endpoint that isn't vertical (if first is vertical)

	if (startVertical) 
	{
		capVertPt.Set(m_capsule.start); 
		capOtherPt = m_capsule.end;
	}
	else if (endVertical)	
	{ 
		capVertPt.Set(m_capsule.end); 
		capOtherPt = m_capsule.start;
	}

	if (capVertPt.IsValid())
	{
		float distFromTop = m_cylTopPlane.GetDistanceFromPlane(capVertPt.Get());
		float distFromBottom = m_cylBottomPlane.GetDistanceFromPlane(capVertPt.Get());
		float topPen = m_capRadius - distFromTop;
		float bottomPen = m_capRadius - distFromBottom;
		float minPen = FLT_MAX;
		Plane3 plane;

		if (topPen < bottomPen)
		{
			plane = m_cylTopPlane;
			minPen = topPen;
		}
		else
		{
			plane = m_cylBottomPlane;
			minPen = bottomPen;
		}

		if (minPen < 0.f)
			return;

		// Make sure the other point is further away from the plane
		// If not, then it would be better to make an edge point for a vertical correction
		if (plane.GetDistanceFromPlane(capVertPt.Get()) > plane.GetDistanceFromPlane(capOtherPt))
			return;

		m_verticalPens[0] = minPen;
		m_verticalNormal = (minPen == topPen ? m_cylTopPlane.m_normal : m_cylBottomPlane.m_normal);
		m_verticalPositions[0] = capVertPt.Get() - m_capRadius * m_verticalNormal;
		m_numVerticalContacts = 1;
		m_worstVerticalPen = minPen;
	}
}

//-------------------------------------------------------------------------------------------------
void CapsuleCylinderCollision::SolveHorizontal()
{
	if (m_distBetweenSegs >= (m_cylRadius + m_capRadius) || m_distBetweenSegs == 0.f) // if dist == 0.f, spines are perfectly intersecting - not worth going through the cases, just let the vertical case fix
		return;

	bool capSegPtHorizontal = (m_cylTopPlane.GetDistanceFromPlane(m_capClosestSegPt) < 0.f) && (m_cylBottomPlane.GetDistanceFromPlane(m_capClosestSegPt) < 0.f);
	if (!capSegPtHorizontal)
		return;

	Vector3 cylToCap = m_capClosestSegPt - m_cylClosestSegPt;
	cylToCap /= m_distBetweenSegs;
	m_horizontalPen = m_cylRadius + m_capRadius - m_distBetweenSegs;
	m_horizontalNormal = cylToCap;
	m_horizontalPosition = m_capClosestSegPt - m_capRadius * m_horizontalNormal;
}

#include "Engine/Render/Debug/DebugRenderSystem.h"

//-------------------------------------------------------------------------------------------------
void CapsuleCylinderCollision::SolveEdge()
{
	if (m_distBetweenSegs == 0.f)
		return;

	// Find edge point - solve in 2D using Quadratic Formula
	Vector3 projStart = m_cylTopPlane.GetProjectedPointOntoPlane(m_capsule.start);
	Vector3 projEnd = m_cylTopPlane.GetProjectedPointOntoPlane(m_capsule.end);
	Vector3 dir = (projEnd - projStart);
	float dirLength = dir.GetLength();
	if (dirLength == 0.f)
		return;
	dir /= dirLength;

	Vector2 ts;
	bool hasSolution = SolveLineCircleIntersection(projStart, dir, m_cylinder.m_top, m_cylRadius, ts);
	if (!hasSolution)
		return;




	if (!m_capsule.ContainsPoint(cylEdgePt))
		return;

	Vector3 capSpinePt;
	float edgeToCapDist = GetClosestPointOnLineSegment(m_capsule.start, m_capsule.end, cylEdgePt, capSpinePt);

	Vector3 cylEdgeToCapSpine = (capSpinePt - cylEdgePt) / edgeToCapDist;

	Vector3 cylToCap = (m_capClosestSegPt - m_cylClosestSegPt) / m_distBetweenSegs;
	if (DotProduct(cylEdgeToCapSpine, cylToCap) > 0.f)
	{
		m_edgeNormal = cylEdgeToCapSpine;
		m_edgePen = m_capRadius - edgeToCapDist;
	}
	else
	{
		m_edgeNormal = -1.0f * cylEdgeToCapSpine;
		m_edgePen = m_capRadius + edgeToCapDist;
	}

	m_edgePosition = capSpinePt - m_edgeNormal * m_capRadius;
}


//-------------------------------------------------------------------------------------------------
void CapsuleCylinderCollision::MakeContacts()
{
	float minPen = Min(m_worstVerticalPen, m_horizontalPen, m_edgePen);

	if (minPen < FLT_MAX)
	{
		if (minPen == m_worstVerticalPen)
		{
			MakeVerticalContacts();
		}
		else if (minPen == m_horizontalPen)
		{
			MakeHorizontalContacts();
		}
		else if (minPen == m_edgePen)
		{
			MakeEdgeContacts();
		}
	}
}


//-------------------------------------------------------------------------------------------------
void CapsuleCylinderCollision::MakeVerticalContacts()
{
	m_numContacts = Min(m_numVerticalContacts, m_contactLimit);
	for (int i = 0; i < m_numContacts; ++i)
	{
		m_contacts[i].position = m_verticalPositions[i];
		m_contacts[i].normal = m_verticalNormal;
		m_contacts[i].penetration = m_verticalPens[i];
		m_contacts[i].CheckValuesAreReasonable();
	}
}


//-------------------------------------------------------------------------------------------------
void CapsuleCylinderCollision::MakeHorizontalContacts()
{
	m_contacts[0].position = m_horizontalPosition;
	m_contacts[0].normal = m_horizontalNormal;
	m_contacts[0].penetration = m_horizontalPen;
	m_contacts[0].CheckValuesAreReasonable();
	m_numContacts = 1;
}


//-------------------------------------------------------------------------------------------------
void CapsuleCylinderCollision::MakeEdgeContacts()
{
	m_contacts[0].position = m_edgePosition;
	m_contacts[0].normal = m_edgeNormal;
	m_contacts[0].penetration = m_edgePen;
	m_contacts[0].CheckValuesAreReasonable();
	m_numContacts = 1;
}
