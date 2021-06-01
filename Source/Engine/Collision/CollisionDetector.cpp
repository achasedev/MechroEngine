///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 8th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Collision/CollisionDetector.h"
#include "Engine/Collision/Collider.h"
#include "Engine/Collision/Contact.h"
#include "Engine/Core/DevConsole.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/Entity.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Matrix3.h"

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

//-------------------------------------------------------------------------------------------------
static float CalculateFrictionBetween(const Collider& a, const Collider&b)
{
	return Sqrt(a.friction + b.friction);
}


//-------------------------------------------------------------------------------------------------
static float CalculateRestitutionBetween(const Collider& a, const Collider& b)
{
	return a.restitution * b.restitution;
}


//-------------------------------------------------------------------------------------------------
static void FillOutColliderInfo(Contact* contact, const Collider& a, const Collider& b)
{
	if (a.GetOwnerRigidBody() == nullptr)
	{
		contact->bodies[0] = b.GetOwnerRigidBody();
		contact->bodies[1] = a.GetOwnerRigidBody();
		contact->normal *= -1.f;
	}
	else
	{
		contact->bodies[0] = a.GetOwnerRigidBody();
		contact->bodies[1] = b.GetOwnerRigidBody();
	}

	contact->restitution = CalculateRestitutionBetween(a, b);
	contact->friction = CalculateFrictionBetween(a, b);
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const SphereCollider& a, const SphereCollider& b, Contact* out_contacts, int limit)
{
	if (limit <= 0)
		return 0;

	Sphere3D aSphere = a.GetDataInWorldSpace();
	Sphere3D bSphere = b.GetDataInWorldSpace();

	Vector3 bToA = aSphere.center - bSphere.center;
	float distanceSquared = bToA.GetLengthSquared();

	if (distanceSquared >= (aSphere.radius + bSphere.radius) * (aSphere.radius + bSphere.radius))
	{
		return 0;
	}

	float distance = bToA.Normalize();

	out_contacts->position = bSphere.center + 0.5f * distance * bToA;			// Contact position is the midpoint between the centers
	out_contacts->normal = bToA;												// Orientation is set up s.t adding the normal to A would resolve the collision, -normal to B
	out_contacts->penetration = (aSphere.radius + bSphere.radius) - distance;	// Pen is the overlap
	FillOutColliderInfo(out_contacts, a, b);

	out_contacts->CheckValuesAreReasonable();
	return 1;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const SphereCollider& sphere, const HalfSpaceCollider& halfSpace, Contact* out_contacts, int limit)
{
	if (limit <= 0)
		return 0;

	Sphere3D sphereWs = sphere.GetDataInWorldSpace();
	Plane3 planeWs = halfSpace.GetDataInWorldSpace();

	float distance = planeWs.GetDistanceFromPlane(sphereWs.center) - sphereWs.radius;
	
	// Sphere too far in front of plane; its radius isn't enough to intersect the plane
	if (distance >= 0)
		return 0;

	out_contacts->normal = planeWs.GetNormal();
	out_contacts->penetration = -distance;
	out_contacts->position = planeWs.GetProjectedPointOntoPlane(sphereWs.center);
	FillOutColliderInfo(out_contacts, sphere, halfSpace);

	out_contacts->CheckValuesAreReasonable();
	return 1;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const BoxCollider& box, const HalfSpaceCollider& halfSpace, Contact* out_contacts, int limit)
{
	if (limit <= 0)
		return 0;

	OBB3 boxWs = box.GetDataInWorldSpace();
	Plane3 planeWs = halfSpace.GetDataInWorldSpace();

	Vector3 boxVertsWs[8];
	boxWs.GetPoints(boxVertsWs);

	int numContactsAdded = 0;
	Contact* contactToFill = out_contacts;

	for (int i = 0; i < 8; ++i)
	{
		float distance = planeWs.GetDistanceFromPlane(boxVertsWs[i]);

		if (distance < 0.f)
		{
			//contactToFill->position = 0.5f * (boxVertsWs[i] + planeWs.GetProjectedPointOntoPlane(boxVertsWs[i])); // Position is half way between the box vertex and the plane
			contactToFill->position = boxVertsWs[i]; // Position is half way between the box vertex and the plane
			contactToFill->normal = planeWs.m_normal;
			contactToFill->penetration = Abs(distance);
			FillOutColliderInfo(contactToFill, box, halfSpace);
			
			contactToFill->CheckValuesAreReasonable();
			numContactsAdded++;

			if (numContactsAdded < limit)
			{
				contactToFill = &out_contacts[numContactsAdded];
			}
			else
			{
				break;
			}
		}
	}

	return numContactsAdded;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const BoxCollider& box, const SphereCollider& sphere, Contact* out_contacts, int limit)
{
	if (limit <= 0)
		return 0;

	Sphere3D sphereWs = sphere.GetDataInWorldSpace();
	OBB3 boxWs = box.GetDataInWorldSpace();

	Vector3 sphereCenterRel = boxWs.TransformPositionIntoSpace(sphereWs.center);

	// Early out check
	if (Abs(sphereCenterRel.x) - sphereWs.radius >= boxWs.extents.x ||
		Abs(sphereCenterRel.y) - sphereWs.radius >= boxWs.extents.y ||
		Abs(sphereCenterRel.z) - sphereWs.radius >= boxWs.extents.z)
	{
		return 0;
	}

	// Get the closest point on the box to the sphere
	Vector3 closestPointRel = Clamp(sphereCenterRel, -1.0f * boxWs.extents, boxWs.extents);
	Vector3 closestPointWs = boxWs.TransformPositionOutOfSpace(closestPointRel);

	// Get the distance from the sphere to the box point
	Vector3 sphereToBox = closestPointWs - sphereWs.center;
	float distanceSquared = sphereToBox.GetLengthSquared();
	float radiusSquared = sphereWs.radius * sphereWs.radius;

	if (AreMostlyEqual(distanceSquared, radiusSquared))
		return 0;

	// Create the contact
	out_contacts->position = closestPointWs;
	out_contacts->normal = sphereToBox;
	float distance = out_contacts->normal.SafeNormalize(Vector3::Y_AXIS);
	out_contacts->penetration = sphereWs.radius - distance;
	FillOutColliderInfo(out_contacts, box, sphere);

	out_contacts->CheckValuesAreReasonable();
	return 1;
}


//-------------------------------------------------------------------------------------------------
static inline float TransformToAxis(const BoxCollider& box, const Vector3 &axis)
{
	OBB3 boxWs = box.GetDataInWorldSpace();
	Matrix3 boxBasis = Matrix3(boxWs.rotation);
	ASSERT_REASONABLE(boxBasis);
	ASSERT_REASONABLE(axis);

	return
		boxWs.extents.x * Abs(DotProduct(axis, boxBasis.iBasis)) +
		boxWs.extents.y * Abs(DotProduct(axis, boxBasis.jBasis)) +
		boxWs.extents.z * Abs(DotProduct(axis, boxBasis.kBasis));
}

/*
 * This function checks if the two boxes overlap
 * along the given axis, returning the ammount of overlap.
 * The final parameter toCentre
 * is used to pass in the vector between the boxes centre
 * points, to avoid having to recalculate it each time.
 */
static inline float GetPenetrationOnAxis(const BoxCollider& a, const BoxCollider& b, const Vector3 &axis, const Vector3 &aToB)
{
	ASSERT_REASONABLE(axis);
	ASSERT_REASONABLE(aToB);
	ASSERT_OR_DIE(AreMostlyEqual(axis.GetLength(), 1.0f), "Axis not unit!");

	// Project the half-size of one onto axis
	float oneProject = TransformToAxis(a, axis);
	float twoProject = TransformToAxis(b, axis);

	// Project this onto the axis
	float distance = Abs(DotProduct(aToB, axis));

	// Return the overlap (i.e. positive indicates
	// overlap, negative indicates separation).
	return oneProject + twoProject - distance;
}


//-------------------------------------------------------------------------------------------------
// Returns true to signal we haven't found a gap yet and to keep checking axes
static inline bool CheckAxis(const BoxCollider& one, const BoxCollider& two, Vector3 axis, const Vector3& toCentre, unsigned index, float& out_smallestPen, unsigned &out_smallestIndex)
{
	// Make sure we have a normalized axis, and don't check almost parallel axes
	if (AreMostlyEqual(axis.GetLengthSquared(), 0.f)) 
		return true;

	axis.Normalize();
	float penetration = GetPenetrationOnAxis(one, two, axis, toCentre);

	if (penetration < 0.f) 
		return false;

	if (penetration < out_smallestPen) 
	{
		out_smallestPen = penetration;
		out_smallestIndex = index;
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
static int CreateFaceVertexContact(const BoxCollider& faceCol, const BoxCollider& vertexCol, const Vector3 &aToB, Contact* out_contact, const int bestAxisIndex, int limit)
{
	// This method is called when we know that a vertex from
	// box two is in contact with box one.
	OBB3 one = faceCol.GetDataInWorldSpace();
	OBB3 two = vertexCol.GetDataInWorldSpace();
	ASSERT_REASONABLE(one);
	ASSERT_REASONABLE(two);

	// We know which axis the collision is on (i.e. best),
	// but we need to work out which of the two faces on
	// this axis.
	Vector3 normal = Matrix3(one.rotation).columnVectors[bestAxisIndex];
	if (DotProduct(normal, aToB) > 0.f)
	{
		normal = normal * -1.0f;
	}

	ASSERT_OR_DIE(AreMostlyEqual(normal.GetLength(), 1.0f), "Normal not unit!");


	Vector3 vertexOffsetOne = one.extents;
	if (DotProduct(one.GetRightVector(), normal) > 0.f) { vertexOffsetOne.x = -vertexOffsetOne.x; }
	if (DotProduct(one.GetUpVector(), normal) > 0.f) { vertexOffsetOne.y = -vertexOffsetOne.y; }
	if (DotProduct(one.GetForwardVector(), normal) > 0.f) { vertexOffsetOne.z = -vertexOffsetOne.z; }

	Plane3 plane(normal, Matrix3(one.rotation) * vertexOffsetOne + one.center);

	Vector3 points[8];
	two.GetPoints(points);

	int numContactsAdded = 0;
	for (int i = 0; i < 8; ++i)
	{
		float distance = plane.GetDistanceFromPlane(points[i]);

		if (distance > 0.f && one.ContainsWorldSpacePoint(points[i]))
		{
			out_contact->normal = normal;
			out_contact->penetration = distance;
			out_contact->position = points[i]; 
			FillOutColliderInfo(out_contact, faceCol, vertexCol);

			out_contact->CheckValuesAreReasonable();
			out_contact++;

			numContactsAdded++;

			if (numContactsAdded >= limit)
				break;
		}
	}

	return numContactsAdded;

	// Single vertex version - just finds the vertex most in the direction of the normal, and uses the axis pen as the amount of pen

	/*Vector3 vertexOffset = two.extents;
	if (DotProduct(two.GetRightVector(), normal) < 0.f)	{ vertexOffset.x = -vertexOffset.x; }
	if (DotProduct(two.GetUpVector(), normal) < 0.f)		{ vertexOffset.y = -vertexOffset.y; }
	if (DotProduct(two.GetForwardVector(), normal) < 0.f)	{ vertexOffset.z = -vertexOffset.z; }

	// Create the contact data
	out_contact->normal = normal;
	out_contact->penetration = pen;
	out_contact->position = (Matrix3(two.rotation) * vertexOffset) + two.center; // Convert from vertex box space to world space
	FillOutColliderInfo(out_contact, faceCol, vertexCol);*/

}


//-------------------------------------------------------------------------------------------------
// Regarding useOne param: if this is true, and the contact point is outside
// the edge (in the case of an edge-face contact) then
// we use one's midpoint, otherwise we use two's.
static inline Vector3 CalculateEdgeEdgeContactPosition(const Vector3& ptOnEdgeOne, const Vector3& oneAxis, float oneSize, const Vector3& ptOnEdgeTwo, const Vector3& twoAxis, float twoSize, bool useOne)
{
	ASSERT_REASONABLE(ptOnEdgeOne);

	Vector3 toSt, cOne, cTwo;
	float dpStaOne, dpStaTwo, dpOneTwo, smOne, smTwo;
	float denom, mua, mub;

	smOne = oneAxis.GetLengthSquared();
	smTwo = twoAxis.GetLengthSquared();
	dpOneTwo = DotProduct(twoAxis, oneAxis);

	toSt = ptOnEdgeOne - ptOnEdgeTwo;
	dpStaOne = DotProduct(oneAxis, toSt);
	dpStaTwo = DotProduct(twoAxis, toSt);

	denom = smOne * smTwo - dpOneTwo * dpOneTwo;

	// Zero denominator indicates parallel lines
	if (Abs(denom) < 0.0001f) {
		return useOne ? ptOnEdgeOne : ptOnEdgeTwo;
	}

	mua = (dpOneTwo * dpStaTwo - smTwo * dpStaOne) / denom;
	mub = (smOne * dpStaTwo - dpOneTwo * dpStaOne) / denom;

	// If either of the edges has the nearest point out
	// of bounds, then the edges aren't crossed, we have
	// an edge-face contact. Our point is on the edge, which
	// we know from the useOne parameter.
	if (mua > oneSize ||
		mua < -oneSize ||
		mub > twoSize ||
		mub < -twoSize)
	{
		return useOne ? ptOnEdgeOne : ptOnEdgeTwo;
	}
	else
	{
		cOne = ptOnEdgeOne + oneAxis * mua;
		cTwo = ptOnEdgeTwo + twoAxis * mub;

		return cOne * 0.5 + cTwo * 0.5;
	}
}

// This preprocessor definition is only used as a convenience
// in the boxAndBox contact generation method.
#define CHECK_OVERLAP(axis, index) \
    if (!CheckAxis(a, b, (axis), aToB, (index), pen, best)) return 0;

int CollisionDetector::GenerateContacts(const BoxCollider& a, const BoxCollider& b, Contact* out_contacts, int limit)
{
	if (limit <= 0)
		return 0;

	OBB3 aBox = a.GetDataInWorldSpace();
	OBB3 bBox = b.GetDataInWorldSpace();
	ASSERT_REASONABLE(aBox);
	ASSERT_REASONABLE(bBox);

	Matrix3 aBasis(aBox.GetRightVector(), aBox.GetUpVector(), aBox.GetForwardVector());
	Matrix3 bBasis(bBox.GetRightVector(), bBox.GetUpVector(), bBox.GetForwardVector());
	ASSERT_REASONABLE(aBasis);
	ASSERT_REASONABLE(bBasis);

	Vector3 aToB = bBox.center - aBox.center;
	ASSERT_REASONABLE(aToB);

	// We start assuming there is no contact
	float pen = FLT_MAX;
	unsigned best = 0xffffff;

	// Now we check each axes, returning if it gives us
	// a separating axis, and keeping track of the axis with
	// the smallest penetration otherwise.
	CHECK_OVERLAP(aBasis.columnVectors[0], 0);
	CHECK_OVERLAP(aBasis.columnVectors[1], 1);
	CHECK_OVERLAP(aBasis.columnVectors[2], 2);

	CHECK_OVERLAP(bBasis.columnVectors[0], 3);
	CHECK_OVERLAP(bBasis.columnVectors[1], 4);
	CHECK_OVERLAP(bBasis.columnVectors[2], 5);

	// Store the best axis-major, in case we run into almost
	// parallel edge collisions later
	unsigned int bestSingleAxis = best;

	CHECK_OVERLAP(CrossProduct(aBasis.columnVectors[0], bBasis.columnVectors[0]), 6);
	CHECK_OVERLAP(CrossProduct(aBasis.columnVectors[0], bBasis.columnVectors[1]), 7);
	CHECK_OVERLAP(CrossProduct(aBasis.columnVectors[0], bBasis.columnVectors[2]), 8);
	CHECK_OVERLAP(CrossProduct(aBasis.columnVectors[1], bBasis.columnVectors[0]), 9);
	CHECK_OVERLAP(CrossProduct(aBasis.columnVectors[1], bBasis.columnVectors[1]), 10);
	CHECK_OVERLAP(CrossProduct(aBasis.columnVectors[1], bBasis.columnVectors[2]), 11);
	CHECK_OVERLAP(CrossProduct(aBasis.columnVectors[2], bBasis.columnVectors[0]), 12);
	CHECK_OVERLAP(CrossProduct(aBasis.columnVectors[2], bBasis.columnVectors[1]), 13);
	CHECK_OVERLAP(CrossProduct(aBasis.columnVectors[2], bBasis.columnVectors[2]), 14);

	// Make sure we've got a result.
	ASSERT_OR_DIE(best != 0xffffff, "No best index found!");

	// We now know there's a collision, and we know which
	// of the axes gave the smallest penetration. We now
	// can deal with it in different ways depending on
	// the case.
	if (best < 3)
	{
		// We've vertices of box two on a face of box one.
		return CreateFaceVertexContact(a, b, aToB, out_contacts, best, limit);
	}
	else if (best < 6)
	{
		// We've got vertices of box one on a face of box two.
		// We use the same algorithm as above, but swap around
		// one and two (and therefore also the vector between their
		// centers).
		return CreateFaceVertexContact(b, a, aToB*-1.0f, out_contacts, best - 3, limit);
	}
	else
	{
		// We've got an edge-edge contact. Find out which axes
		best -= 6;
		int oneAxisIndex = best / 3;
		int twoAxisIndex = best % 3;
		Vector3 oneAxis = aBasis.columnVectors[oneAxisIndex];
		Vector3 twoAxis = bBasis.columnVectors[twoAxisIndex];
		Vector3 axis = CrossProduct(oneAxis, twoAxis);
		axis.Normalize();

		// The axis should point from box one to box two.
		bool flippedAxis = false;
		if (DotProduct(axis, aToB) > 0.f)
		{
			axis = axis * -1.0f;
			flippedAxis = true;
		}

		// We have the axes, but not the edges: each axis has 4 edges parallel
		// to it, we need to find which of the 4 for each object. We do
		// that by finding the point in the centers of the edge. We know
		// its component in the direction of the box's collision axis is zero
		// (its a mid-point) and we determine which of the extremes in each
		// of the other axes is closest.
		Vector3 ptOnOneEdgeLs = aBox.extents;
		Vector3 ptOnTwoEdgeLs = bBox.extents;
		for (int i = 0; i < 3; i++)
		{
			if (i == oneAxisIndex)
			{
				ptOnOneEdgeLs.data[i] = 0.f;
			}
			else if (DotProduct(aBasis.columnVectors[i], axis) > 0.f)
			{
				ptOnOneEdgeLs.data[i] = -ptOnOneEdgeLs.data[i];
			}

			if (i == twoAxisIndex)
			{
				ptOnTwoEdgeLs.data[i] = 0.f;
			}
			else if (DotProduct(bBasis.columnVectors[i], axis) < 0.f)
			{
				ptOnTwoEdgeLs.data[i] = -ptOnTwoEdgeLs.data[i];
			}
		}

		// Move them into world coordinates (they are already oriented
		// correctly, since they have been derived from the axes).
		Vector3 ptOnOneEdgeWs = (aBasis * ptOnOneEdgeLs) + aBox.center;
		Vector3 ptOnTwoEdgeWs = (bBasis * ptOnTwoEdgeLs) + bBox.center;

		// So we have a point and a direction for the colliding edges.
		// We need to find out point of closest approach of the two
		// line-segments.
		Vector3 vertex = CalculateEdgeEdgeContactPosition(
			ptOnOneEdgeWs, oneAxis, aBox.extents.data[oneAxisIndex],
			ptOnTwoEdgeWs, twoAxis, bBox.extents.data[twoAxisIndex],
			bestSingleAxis > 2
		);

		// We can fill the contact.
		out_contacts->penetration = pen;
		out_contacts->normal = axis;
		out_contacts->position = vertex;
		FillOutColliderInfo(out_contacts, a, b);

		out_contacts->CheckValuesAreReasonable();
		return 1;
	}

	return 0;
}
#undef CHECK_OVERLAP


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const CapsuleCollider& capsule, const HalfSpaceCollider& halfSpace, Contact* out_contacts, int limit)
{
	if (limit <= 0)
		return 0;

	Capsule3D capsuleWs = capsule.GetDataInWorldSpace();
	Plane3 planeWs = halfSpace.GetDataInWorldSpace();

	float startDistance = planeWs.GetDistanceFromPlane(capsuleWs.start) - capsuleWs.radius;
	float endDistance = planeWs.GetDistanceFromPlane(capsuleWs.end) - capsuleWs.radius;

	if (startDistance >= 0.f && endDistance >= 0.f)
		return 0;

	int numAdded = 0;
	if (startDistance < 0.f)
	{
		out_contacts->normal = planeWs.GetNormal();
		out_contacts->penetration = -startDistance;
		out_contacts->position = capsuleWs.start - out_contacts->normal * capsuleWs.radius;
		FillOutColliderInfo(out_contacts, capsule, halfSpace);
		out_contacts->CheckValuesAreReasonable();
		numAdded++;
		out_contacts++;

		if (limit == 1)
			return 1;
	}

	if (endDistance < 0.f)
	{
		out_contacts->normal = planeWs.GetNormal();
		out_contacts->penetration = -endDistance;
		out_contacts->position = capsuleWs.end - out_contacts->normal * capsuleWs.radius;
		FillOutColliderInfo(out_contacts, capsule, halfSpace);
		out_contacts->CheckValuesAreReasonable();
		numAdded++;
	}

	return numAdded;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const SphereCollider& sphere, const CapsuleCollider& capsule, Contact* out_contacts, int limit)
{
	if (limit <= 0)
		return 0;

	Sphere3D sphereWs = sphere.GetDataInWorldSpace();
	Capsule3D capsuleWs = capsule.GetDataInWorldSpace();

	Vector3 closestCapsulePointWs;
	float distance = GetClosestPointOnLineSegment(capsuleWs.start, capsuleWs.end, sphereWs.center, closestCapsulePointWs);
	float overlap = (sphereWs.radius + capsuleWs.radius) - distance;

	if (overlap > 0.f)
	{
		out_contacts->normal = (sphereWs.center - closestCapsulePointWs) / distance;
		out_contacts->penetration = overlap;
		out_contacts->position = 0.5f * (closestCapsulePointWs + sphereWs.center);
		FillOutColliderInfo(out_contacts, sphere, capsule);
		out_contacts->CheckValuesAreReasonable();

		return 1;
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const CapsuleCollider& a, const CapsuleCollider& b, Contact* out_contacts, int limit)
{
	if (limit <= 0)
		return 0;

	Capsule3D capsuleA = a.GetDataInWorldSpace();
	Capsule3D capsuleB = b.GetDataInWorldSpace();

	Vector3 ptOnA, ptOnB;
	float distance = FindClosestPointsOnLineSegments(capsuleA.start, capsuleA.end, capsuleB.start, capsuleB.end, ptOnA, ptOnB);
	float overlap = (capsuleA.radius + capsuleB.radius) - distance;

	if (overlap > 0.f)
	{
		out_contacts->normal = (ptOnA - ptOnB) / distance;
		out_contacts->penetration = overlap;
		out_contacts->position = 0.5f * (ptOnA + ptOnB);
		FillOutColliderInfo(out_contacts, a, b);
		out_contacts->CheckValuesAreReasonable();

		return 1;
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
static bool GetMinPlanePen(const OBB3& box, const Capsule3D& capsule, float* out_pens, Vector3* out_positions, Vector3& out_normal)
{
	out_pens[0] = FLT_MAX;
	out_pens[1] = FLT_MAX;

	Vector3 startLs = box.TransformPositionIntoSpace(capsule.start);
	Vector3 endLs = box.TransformPositionIntoSpace(capsule.end);

	float radius = capsule.radius;
	Vector3 extents = box.extents;

	Vector3 pens[2];
	Vector3 signs[2];

	for (int i = 0; i < 2; ++i)
	{
		Vector3 endPoint = (i == 0 ? startLs : endLs);

		for (int j = 0; j < 3; ++j)
		{
			bool inLateralBounds = true;
			for (int k = 0; k < 3; ++k)
			{
				if (k == j)
					continue;

				if (endPoint.data[k] > extents.data[k] || endPoint.data[k] < -extents.data[k])
				{
					inLateralBounds = false;
					break;
				}
			}

			if (inLateralBounds)
			{
				float posPen = extents.data[j] - endPoint.data[j] + radius;
				float negPen = extents.data[j] + endPoint.data[j] + radius;

				if (posPen < negPen)
				{
					pens[i].data[j] = posPen;
					signs[i].data[j] = 1.f;
				}
				else
				{
					pens[i].data[j] = negPen;
					signs[i].data[j] = -1.f;
				}
			}
			else
			{
				pens[i].data[j] = FLT_MAX;
			}
		}
	}

	// Determine the best normal for each end point
	Vector3 normals[2];
	for (int i = 0; i < 2; ++i)
	{
		normals[i] = Vector3::ZERO;

		const Vector3& axisPens = pens[i];
		float minPen = Min(axisPens.x, axisPens.y, axisPens.z);

		// If the min pen is negative, then this capsule's "sphere" is completely outside a face
		// so it has no overlap
		if (minPen < 0.f)
			continue;

		for (int j = 0; j < 3; ++j)
		{
			if (minPen == axisPens.data[j])
			{
				normals[i].data[j] = 1.f * signs[i].data[j];
				out_pens[i] = minPen;

				Vector3 endPoint = (i == 0 ? capsule.start : capsule.end);
				out_positions[i] = endPoint - normals[i] * radius;
				break;
			}
		}
	}

	// If the endpoints find 2 different best faces, the capsule must be intersecting an edge
	// Instead, just generate an edgepoint
	float normalDot = DotProduct(normals[0], normals[1]);
	if ((out_pens[0] < FLT_MAX && out_pens[1] < FLT_MAX) && AreMostlyEqual(normalDot, 0.f))
	{
		return false;
	}
	else if (AreMostlyEqual(normalDot, -1.f))
	{
		ERROR_AND_DIE("Ha, capsule is stuck isn't it");
	}


	if (out_pens[0] < FLT_MAX)
	{
		Matrix3 basis(box.rotation);
		out_normal = normals[0].x * basis.columnVectors[0] + normals[0].y * basis.columnVectors[1] + normals[0].z * basis.columnVectors[2];
	}
	else if (out_pens[1] < FLT_MAX)
	{
		Matrix3 basis(box.rotation);
		out_normal = normals[1].x * basis.columnVectors[0] + normals[1].y * basis.columnVectors[1] + normals[1].z * basis.columnVectors[2];
	}

	return (out_pens[0] < FLT_MAX || out_pens[1] < FLT_MAX);
}


//-------------------------------------------------------------------------------------------------
static bool GetMinEdgePen(const OBB3& box, const Capsule3D& capsule, float& out_pen, Vector3& out_normal, Vector3& out_position)
{
	out_pen = FLT_MAX;

	Edge3 edges[12];
	box.GetEdges(edges);

	for (int i = 0; i < 12; ++i)
	{
		const Edge3& edge = edges[i];

		// Get the distance between the closest points on the box edge and the capsule's spine
		Vector3 capsulePt, boxPt;
		float distance = FindClosestPointsOnLineSegments(capsule.start, capsule.end, edge.m_start, edge.m_end, capsulePt, boxPt);

		// Get these in the box's local space to check Voronoi regions
		// Only consider edge overlap if the capsule closest point either
		// 1. Is inside the box
		// 2. Is in the voronoi region adjacent to the edge
		// This prevents bad cases of overlapping edges across the box and pushing the wrong way
		Vector3 capsulePtLs = box.TransformPositionIntoSpace(capsulePt);
		Vector3 boxPtLs = box.TransformPositionIntoSpace(boxPt);
		const Vector3& extents = box.extents;
		bool boxContainsCapsulePt = (Abs(capsulePtLs.x) < extents.x) && (Abs(capsulePtLs.y) < extents.y) && (Abs(capsulePtLs.z) < extents.z);

		bool isInCorrectVoronoiRegion = true;
		if (!boxContainsCapsulePt)
		{
			for (int j = 0; j < 3; ++j)
			{
				// If the box point is at an extent...
				if (AreMostlyEqual(Abs(boxPtLs.data[j]), extents.data[j]))
				{
					// Ensure we're outside of them in that direction
					// i.e. "If the point is on the max x, make sure we're farther out on x, if the box point is on negative z, make sure we're below that"
					if ((boxPtLs.data[j] > 0.f && boxPtLs.data[j] > capsulePtLs.data[j]) || (boxPtLs.data[j] < 0.f && boxPtLs.data[j] < capsulePtLs.data[j]))
					{
						isInCorrectVoronoiRegion = false;
						break;
					}
				}
				else
				{
					// Technically the below should never fail....due to how the distance calculation finds the end points
					if (Abs(capsulePtLs.data[j] > extents.data[j]))
					{
						isInCorrectVoronoiRegion = false;
						break;
					}
				}
			}
		}

		if (boxContainsCapsulePt || isInCorrectVoronoiRegion)
		{
			float sign = (box.ContainsWorldSpacePoint(capsulePt) ? 1.0f : -1.0f);
			float pen = sign * distance + capsule.radius;
			if (pen > 0.f && pen < out_pen)
			{
				out_pen = pen;
				out_normal = (-1.f * sign) * (capsulePt - boxPt) / distance;
				out_position = capsulePt - out_normal * distance;
			}
		}
	}

	return (out_pen < FLT_MAX);
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const BoxCollider& box, const CapsuleCollider& capsule, Contact* out_contacts, int limit)
{
	if (limit <= 0)
		return 0;

	OBB3 boxWs = box.GetDataInWorldSpace();
	Capsule3D capsuleWs = capsule.GetDataInWorldSpace();

	float facePens[2];
	Vector3 faceNormal;
	Vector3 faceContactPos[2];
	bool hasFaceOverlap = GetMinPlanePen(boxWs, capsuleWs, facePens, faceContactPos, faceNormal);

	float edgePen = FLT_MAX;
	Vector3 edgeNormal = Vector3::ZERO;
	Vector3 edgePos = Vector3::ZERO;
	bool hasEdgeOverlap = GetMinEdgePen(boxWs, capsuleWs, edgePen, edgeNormal, edgePos);

	float worstFacePen = FLT_MAX;

	if (facePens[0] < FLT_MAX && facePens[1] < FLT_MAX)
	{
		worstFacePen = Max(facePens[0], facePens[1]);
	}
	else if (facePens[0] < FLT_MAX)
	{
		worstFacePen = facePens[0];
	}
	else if (facePens[1] < FLT_MAX)
	{
		worstFacePen = facePens[1];
	}

	// If we have face overlap but no edge overlap *or* the face overlap has less pen than the edge, make face contacts
	int numContactsAdded = 0;

	if (hasFaceOverlap)
	{
		if (facePens[0] < FLT_MAX)
		{
			out_contacts->normal = faceNormal;
			out_contacts->penetration = facePens[0];
			out_contacts->position = faceContactPos[0];
			FillOutColliderInfo(out_contacts, capsule, box);
			out_contacts->CheckValuesAreReasonable();
			out_contacts++;
			numContactsAdded++;
		}

		if (facePens[1] < FLT_MAX && numContactsAdded < limit)
		{
			out_contacts->normal = faceNormal;
			out_contacts->penetration = facePens[1];
			out_contacts->position = faceContactPos[1];
			FillOutColliderInfo(out_contacts, capsule, box);
			out_contacts->CheckValuesAreReasonable();
			out_contacts++;
			numContactsAdded++;
		}
	}

	if (hasEdgeOverlap)
	{
		out_contacts->normal = edgeNormal;
		out_contacts->penetration = edgePen;
		out_contacts->position = edgePos;
		FillOutColliderInfo(out_contacts, capsule, box);
		out_contacts->CheckValuesAreReasonable();

		numContactsAdded++;
	}

	return numContactsAdded;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const SphereCollider& sphere, const PlaneCollider& plane, Contact* out_contacts, int limit)
{
	if (limit <= 0)
		return 0;

	Sphere3D sphereWs = sphere.GetDataInWorldSpace();
	Plane3 planeWs = plane.GetDataInWorldSpace();

	float distance = planeWs.GetDistanceFromPlane(sphereWs.center);

	// Sphere too far in front of plane; its radius isn't enough to intersect the plane
	if (Abs(distance) >= sphereWs.radius)
		return 0;

	// Find the direction to push the sphere
	if (distance > 0.f)
	{
		out_contacts->normal = planeWs.GetNormal();
		out_contacts->penetration = sphereWs.radius - distance;
	}
	else
	{
		out_contacts->normal = -1.f * planeWs.GetNormal();
		out_contacts->penetration = sphereWs.radius - Abs(distance);
	}

	out_contacts->position = planeWs.GetProjectedPointOntoPlane(sphereWs.center);
	FillOutColliderInfo(out_contacts, sphere, plane);

	out_contacts->CheckValuesAreReasonable();
	return 1;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const CapsuleCollider& capsule, const PlaneCollider& plane, Contact* out_contacts, int limit)
{
	if (limit <= 0)
		return 0;

	Capsule3D capsuleWs = capsule.GetDataInWorldSpace();
	Plane3 planeWs = plane.GetDataInWorldSpace();

	float startDistance = planeWs.GetDistanceFromPlane(capsuleWs.start);
	float endDistance = planeWs.GetDistanceFromPlane(capsuleWs.end);

	if ((startDistance >= capsuleWs.radius && endDistance >= capsuleWs.radius) || (startDistance < -capsuleWs.radius && endDistance < -capsuleWs.radius))
		return 0;

	bool startIsPen = Abs(startDistance) < capsuleWs.radius;
	bool endIsPen = Abs(endDistance) < capsuleWs.radius;
	bool bisected = (startDistance > 0.f && endDistance < 0.f) || (startDistance < 0.f && endDistance > 0.f); // Plane is between start and end; Fixes edge case where plane is outside one radus from end points
	bool chooseStart = false;

	Vector3 normal = planeWs.m_normal;

	if ((startIsPen && endIsPen) || bisected)
	{
		// Take the min direction if both are in contact but on opposite sides
		if (startDistance < 0.f && endDistance > 0.f)
		{
			if (Abs(endDistance) < Abs(startDistance))
			{
				normal *= -1.0f;
				chooseStart = false;
			}
		}
		else if (endDistance < 0.f && startDistance > 0.f)
		{
			if (Abs(startDistance) < Abs(endDistance))
			{
				normal *= -1.0f;
				chooseStart = true;
			}
		}
	}
	else if ((startIsPen && startDistance < 0.f) || (endIsPen && endDistance < 0.f))
	{
		normal *= -1.0f;
	}

	int numAdded = 0;
	if (startIsPen || (bisected && chooseStart))
	{
		out_contacts->normal = normal;
		out_contacts->penetration = capsuleWs.radius - Abs(startDistance);
		out_contacts->position = capsuleWs.start - normal * capsuleWs.radius;
		FillOutColliderInfo(out_contacts, capsule, plane);
		out_contacts->CheckValuesAreReasonable();
		numAdded++;
		out_contacts++;

		if (limit == 1)
			return 1;
	}

	if (endIsPen || (bisected && !chooseStart))
	{
		out_contacts->normal = normal;
		out_contacts->penetration = capsuleWs.radius - Abs(endDistance);
		out_contacts->position = capsuleWs.end - normal * capsuleWs.radius;
		FillOutColliderInfo(out_contacts, capsule, plane);
		out_contacts->CheckValuesAreReasonable();
		numAdded++;
	}

	return numAdded;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const BoxCollider& box, const PlaneCollider& plane, Contact* out_contacts, int limit)
{
	if (limit <= 0)
		return 0;

	OBB3 boxWs = box.GetDataInWorldSpace();
	Plane3 planeWs = plane.GetDataInWorldSpace();

	Vector3 boxVertsWs[8];
	boxWs.GetPoints(boxVertsWs);

	int numContactsAdded = 0;
	Contact* contactToFill = out_contacts;

	std::vector<int> pointsBehind;
	std::vector<int> pointsInFront;
	float maxFrontDistance = 0.f;
	float maxBehindDistance = 0.f;

	for (int i = 0; i < 8; ++i)
	{
		float distance = planeWs.GetDistanceFromPlane(boxVertsWs[i]);

		if (distance < 0.f)
		{
			pointsBehind.push_back(i);
			maxBehindDistance = Max(Abs(distance), maxBehindDistance);
		}
		else if (distance > 0.f)
		{
			pointsInFront.push_back(i);
			maxFrontDistance = Max(Abs(distance), maxFrontDistance);
		}
	}

	// If all the points are on one side, there's no collision
	if (pointsBehind.size() == 0 || pointsInFront.size() == 0)
	{
		return 0;
	}

	std::vector<int>* penPoints;
	float normalSign = 1.0f;

	if (maxFrontDistance < maxBehindDistance)
	{
		penPoints = &pointsInFront;
		normalSign *= -1.0f;
	}
	else
	{
		penPoints = &pointsBehind;
	}

	for (int pointIndex : *penPoints)
	{
		Vector3 point = boxVertsWs[pointIndex];

		contactToFill->position = point;
		contactToFill->normal = normalSign * planeWs.m_normal;
		contactToFill->penetration = Abs(planeWs.GetDistanceFromPlane(point));
		FillOutColliderInfo(contactToFill, box, plane);

		contactToFill->CheckValuesAreReasonable();
		numContactsAdded++;

		if (numContactsAdded < limit)
		{
			contactToFill = &out_contacts[numContactsAdded];
		}
		else
		{
			break;
		}
	}

	return numContactsAdded;
}
