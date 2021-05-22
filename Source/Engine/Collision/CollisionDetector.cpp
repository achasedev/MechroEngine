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

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const SphereCollider& a, const SphereCollider& b, CollisionData& collisionData)
{
	if (collisionData.numContacts >= MAX_CONTACT_COUNT)
	{
		ConsoleWarningf("CollisionDetector ran out of room for contacts!");
		return 0;
	}

	Sphere3D aSphere = a.GetDataInWorldSpace();
	Sphere3D bSphere = b.GetDataInWorldSpace();

	Vector3 bToA = aSphere.center - bSphere.center;
	float distanceSquared = bToA.GetLengthSquared();

	if (distanceSquared >= (aSphere.radius + bSphere.radius) * (aSphere.radius + bSphere.radius))
	{
		return 0;
	}

	float distance = bToA.Normalize();

	Contact* contact = &collisionData.contacts[collisionData.numContacts];

	contact->position = bSphere.center + 0.5f * distance * bToA;			// Contact position is the midpoint between the centers
	contact->normal = bToA;													// Orientation is set up s.t adding the normal to A would resolve the collision, -normal to B
	contact->penetration = (aSphere.radius + bSphere.radius) - distance;	// Pen is the overlap
	// TODO: Figure out restitution and friction
	contact->bodies[0] = a.GetOwnerRigidBody();
	contact->bodies[1] = b.GetOwnerRigidBody();	
	contact->restitution = collisionData.restitution;
	contact->friction = collisionData.friction;

	collisionData.numContacts++;
	return 1;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const SphereCollider& sphere, const HalfSpaceCollider& halfSpace, CollisionData& collisionData)
{
	if (collisionData.numContacts >= MAX_CONTACT_COUNT)
	{
		ConsoleWarningf("CollisionDetector ran out of room for contacts!");
		return 0;
	}

	Sphere3D sphereWs = sphere.GetDataInWorldSpace();
	Plane3 planeWs = halfSpace.GetDataInWorldSpace();

	float distance = planeWs.GetDistanceFromPlane(sphereWs.center) - sphereWs.radius;
	
	// Sphere too far in front of plane; its radius isn't enough to intersect the plane
	if (distance >= 0)
		return 0;

	Contact* contact = &collisionData.contacts[collisionData.numContacts];

	contact->normal = planeWs.GetNormal();
	contact->penetration = -distance;
	contact->position = planeWs.GetProjectedPointOntoPlane(sphereWs.center);
	contact->bodies[0] = sphere.GetOwnerRigidBody();
	contact->bodies[1] = nullptr;
	contact->friction = collisionData.friction;
	contact->restitution = collisionData.restitution;
	collisionData.numContacts++;
	return 1;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const BoxCollider& box, const HalfSpaceCollider& halfSpace, CollisionData& collisionData)
{
	if (collisionData.numContacts >= MAX_CONTACT_COUNT)
	{
		ConsoleWarningf("CollisionDetector ran out of room for contacts!");
		return 0;
	}

	OBB3 boxWs = box.GetDataInWorldSpace();
	Plane3 planeWs = halfSpace.GetDataInWorldSpace();

	Vector3 boxVertsWs[8];
	boxWs.GetPoints(boxVertsWs);

	int numContactsAdded = 0;
	for (int i = 0; i < 8; ++i)
	{
		float distance = planeWs.GetDistanceFromPlane(boxVertsWs[i]);

		if (distance < 0.f)
		{
			Contact* contact = &collisionData.contacts[collisionData.numContacts];

			contact->position = 0.5f * (boxVertsWs[i] + planeWs.GetProjectedPointOntoPlane(boxVertsWs[i])); // Position is half way between the box vertex and the plane
			contact->normal = planeWs.m_normal;
			contact->penetration = Abs(distance);
			contact->bodies[0] = box.GetOwnerRigidBody();
			contact->bodies[1] = nullptr;
			contact->friction = collisionData.friction;
			contact->restitution = collisionData.restitution;
			collisionData.numContacts++;
			numContactsAdded++;

			if (collisionData.numContacts == MAX_CONTACT_COUNT)
			{
				break;
			}
		}
	}

	return numContactsAdded;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const BoxCollider& box, const SphereCollider& sphere, CollisionData& collisionData)
{
	if (collisionData.numContacts >= MAX_CONTACT_COUNT)
	{
		ConsoleWarningf("CollisionDetector ran out of room for contacts!");
		return 0;
	}

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

	if (distanceSquared >= radiusSquared)
		return 0;

	// Create the contact
	Contact* contact = &collisionData.contacts[collisionData.numContacts];
	contact->position = closestPointWs;
	contact->normal = sphereToBox;
	float distance = contact->normal.Normalize();
	contact->penetration = sphereWs.radius - distance;
	contact->bodies[0] = box.GetOwnerRigidBody();
	contact->bodies[1] = sphere.GetOwnerRigidBody();
	contact->restitution = collisionData.restitution;
	contact->friction = collisionData.friction;
	collisionData.numContacts++;
	return 1;
}

//-------------------------------------------------------------------------------------------------
// Returns the float magnitude of how much the extents, defined in the basis' space, project onto the world-space axis
static float ProjectExtentsOntoAxis(const Vector3& extents, const Matrix3& basis, const Vector3& axis)
{
	return extents.x * Abs(DotProduct(basis.iBasis, axis)) + extents.y * Abs(DotProduct(basis.jBasis, axis)) + extents.z * Abs(DotProduct(basis.kBasis, axis));
}


//-------------------------------------------------------------------------------------------------
// Determines how much each box's extent projects onto the axis and how much the distance projects
// If the distance is greater than the half extents, then the result is negative, indicating there must be a gap between the two
static float GetPenetrationOnAxis(const Matrix3& aBasis, const Matrix3& bBasis, const Vector3& aExtents, const Vector3& bExtents, const Vector3& axis, const Vector3& aToB)
{
	float projA = ProjectExtentsOntoAxis(aExtents, aBasis, axis);
	float projB = ProjectExtentsOntoAxis(bExtents, bBasis, axis);
	float projDistance = Abs(DotProduct(aToB, axis));

	return projA + projB - projDistance;
}


//-------------------------------------------------------------------------------------------------
static void CreateVertexFaceContact(const BoxCollider& faceCol, const BoxCollider& vertexCol, const OBB3& vertexBox, const Vector3& faceNormal, const Vector3& faceToVertex, const Matrix3& vertexBoxBasis, float pen, CollisionData& out_collisionData)
{
	Vector3 contactNormal = faceNormal;

	// Make the normal points away from the vertex box
	if (DotProduct(contactNormal, faceToVertex) > 0.f)
	{
		contactNormal *= -1.0f;
	}

	// Find the vertex furthest in this direction
	Vector3 contactPosLs = vertexBox.extents;
	if (DotProduct(vertexBoxBasis.iBasis, contactNormal) < 0.f) { contactPosLs.x *= -1.0; }
	if (DotProduct(vertexBoxBasis.jBasis, contactNormal) < 0.f) { contactPosLs.y *= -1.0; }
	if (DotProduct(vertexBoxBasis.kBasis, contactNormal) < 0.f) { contactPosLs.z *= -1.0; }

	Contact* contact = &out_collisionData.contacts[out_collisionData.numContacts];
	contact->position = (vertexBoxBasis * contactPosLs) + vertexBox.center; // Convert from vertex box space to world space
	contact->normal = contactNormal;
	contact->penetration = pen;
	contact->bodies[0] = faceCol.GetOwnerRigidBody(); // Face box is always A, and the normal always points away from B towards A. Adding this onto A adheres to how we always do it
	contact->bodies[1] = vertexCol.GetOwnerRigidBody();
	contact->restitution = out_collisionData.restitution;
	contact->friction = out_collisionData.friction;
	out_collisionData.numContacts++;
}

static int BoxAndBox(
	const BoxCollider* oneCol,
	const BoxCollider* twoCol,
	CollisionData& out_collisionData
);
//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const BoxCollider& a, const BoxCollider& b, CollisionData& collisionData)
{
	return BoxAndBox(&a, &b, collisionData);

	if (collisionData.numContacts >= MAX_CONTACT_COUNT)
	{
		ConsoleWarningf("CollisionDetector ran out of room for contacts!");
		return 0;
	}

	OBB3 boxAWs = a.GetDataInWorldSpace();
	OBB3 boxBWs = b.GetDataInWorldSpace();
	Vector3 aToB = boxBWs.center - boxAWs.center;

	// Separating Axis Theorem
	// Get the 15 axes to check:
	//  3 face axes on A
	//  3 face axes on B
	//  Cross products from edges - 3 unique on A, 3 unique on B, 3 * 3 = 9 axes

	// TODO: Optimize this to calculate axes as we go, so if we early out we don't waste work
	Matrix3 aBasis = Matrix3(boxAWs.rotation);
	Matrix3 bBasis = Matrix3(boxBWs.rotation);

	Vector3 axes[15];
	axes[0] = aBasis.iBasis;
	axes[1] = aBasis.jBasis;
	axes[2] = aBasis.kBasis;
	axes[3] = bBasis.iBasis;
	axes[4] = bBasis.jBasis;
	axes[5] = bBasis.kBasis;
	
	axes[6] = CrossProduct(axes[0], axes[3]);
	axes[7] = CrossProduct(axes[0], axes[4]);
	axes[8] = CrossProduct(axes[0], axes[5]);
	axes[9] = CrossProduct(axes[1], axes[3]);
	axes[10] = CrossProduct(axes[1], axes[4]);
	axes[11] = CrossProduct(axes[1], axes[5]);
	axes[12] = CrossProduct(axes[2], axes[3]);
	axes[13] = CrossProduct(axes[2], axes[4]);
	axes[14] = CrossProduct(axes[2], axes[5]);

	float minPen = FLT_MAX;
	int bestAxisIndex = -1;

	for (int i = 0; i < 15; ++i)
	{
		const Vector3& axis = axes[i];

		// For edge-edge created axes, if they're colinear the result is degenerative
		if (AreMostlyEqual(axis.GetLengthSquared(), 0.f))
			continue;

		float penOnAxis = GetPenetrationOnAxis(aBasis, bBasis, boxAWs.extents, boxBWs.extents, axis, aToB);
		if (penOnAxis < 0.f)
			return 0;

		if (penOnAxis < minPen)
		{
			minPen = penOnAxis;
			bestAxisIndex = i;
		}
	}

	if (bestAxisIndex < 3)
	{
		// Vertex/Face collision
		// A is the face box, B is the vertex box
		CreateVertexFaceContact(a, b, boxBWs, axes[bestAxisIndex], aToB, bBasis, minPen, collisionData);
	}
	else if (bestAxisIndex < 6)
	{
		// Vertex/Face collision
		// B is the face box, A is the vertex box
		CreateVertexFaceContact(b, a, boxAWs, axes[bestAxisIndex], -1.0f * aToB, aBasis, minPen, collisionData);
	}
	else
	{
		// It's an edge-edge contact - find out which two edges it is

		// Get the local index of the basis vectors that created the cross product axis
		// So 0 is i, 1 is j, 2 is k, for each box
		int aAxisIndex = (bestAxisIndex - 6) / 3;
		int bAxisIndex = (bestAxisIndex - 6) % 3;

		// Start with the point at the max extents
		Vector3 edgeAMidpoint = boxAWs.extents;
		Vector3 edgeBMidpoint = boxBWs.extents;

		// Get the axis of min separation
		// Make sure the axis points from B to A
		Vector3 seperatingAxis = axes[bestAxisIndex];

		if (DotProduct(seperatingAxis, aToB) > 0.f)
		{
			seperatingAxis *= -1.0f;
		}

		// Get the direction of the edges while we're at it, to find the end points
		Vector3 edgeADir = Vector3::ZERO;
		Vector3 edgeBDir = Vector3::ZERO;

		// Iterate across the 1st, 2nd, and 3rd basis vector of each box
		for (int i = 0; i < 3; ++i)
		{
			// For the basis vector that contributed to the cross product yielding the separating axis, set that corresponding element to 0.f
			// This places the point in the middle of that edge of the box (instead of +extent or -extent along that basis vector)
			if (i == aAxisIndex)
			{
				edgeAMidpoint.data[i] = 0.f;
				edgeADir.data[i] = boxAWs.extents.data[i]; // going to -value to +value of this component traces the edge
			}
			else if (DotProduct(axes[i], seperatingAxis) > 0.f) // For all other components, choose the ones furthest against the axis (towards B)
			{		
				edgeAMidpoint.data[i] = -1.0f * edgeAMidpoint.data[i];
			}

			// Do the same for B...
			if (i == bAxisIndex)
			{
				edgeBMidpoint.data[i] = 0.f;
				edgeBDir.data[i] = boxBWs.extents.data[i]; // going to -value to +value of this component traces the edge
			}
			else if (DotProduct(axes[i + 3], seperatingAxis) < 0.f) // ...except find the components most in the direction of the axis, towards A
			{

				edgeBMidpoint.data[i] = -1.0f * edgeBMidpoint.data[i];
			}
		}
		
		// Determine the edge endpoints in world space
		Vector3 edgeAStartWs	= aBasis * (edgeAMidpoint - edgeADir) + boxAWs.center;
		Vector3 edgeAEndWs		= aBasis * (edgeAMidpoint + edgeADir) + boxAWs.center;
		Vector3 edgeBStartWs	= bBasis * (edgeBMidpoint - edgeBDir) + boxBWs.center;
		Vector3 edgeBEndWs		= bBasis * (edgeBMidpoint + edgeBDir) + boxBWs.center;

		// Get the closest points between the edges
		Vector3 closestPtOnAWs;
		Vector3 closestPtOnBWs;
		FindClosestPointsOnLineSegments(edgeAStartWs, edgeAEndWs, edgeBStartWs, edgeBEndWs, closestPtOnAWs, closestPtOnBWs);

		// Find the midpoint between the closest points
		Vector3 finalMidpoint = 0.5f * (closestPtOnAWs - closestPtOnBWs) + closestPtOnBWs;

		// Now make the contact
		Contact* contact = &collisionData.contacts[collisionData.numContacts];

		contact->position = finalMidpoint;
		contact->normal = seperatingAxis;
		contact->penetration = minPen;
		contact->bodies[0] = a.GetOwnerRigidBody();
		contact->bodies[1] = b.GetOwnerRigidBody();
		contact->restitution = collisionData.restitution;
		contact->friction = collisionData.friction;
		collisionData.numContacts++;
	}

	return 1;
}

static inline float TransformToAxis(
	const BoxCollider* box,
	const Vector3 &axis
)
{
	OBB3 boxWs = box->GetDataInWorldSpace();

	return
		boxWs.extents.x * Abs(DotProduct(axis, boxWs.GetRightVector())) +
		boxWs.extents.y * Abs(DotProduct(axis, boxWs.GetUpVector())) +
		boxWs.extents.z * Abs(DotProduct(axis, boxWs.GetForwardVector()));
}

/*
 * This function checks if the two boxes overlap
 * along the given axis, returning the ammount of overlap.
 * The final parameter toCentre
 * is used to pass in the vector between the boxes centre
 * points, to avoid having to recalculate it each time.
 */
static inline float GetPenetrationOnAxis(
	const BoxCollider* colOne,
	const BoxCollider* colTwo,
	const Vector3 &axis,
	const Vector3 &aToB
)
{
	// Project the half-size of one onto axis
	float oneProject = TransformToAxis(colOne, axis);
	float twoProject = TransformToAxis(colTwo, axis);

	// Project this onto the axis
	float distance = Abs(DotProduct(aToB, axis));

	// Return the overlap (i.e. positive indicates
	// overlap, negative indicates separation).
	return oneProject + twoProject - distance;
}


static inline bool tryAxis(
	const BoxCollider* one,
	const BoxCollider* two,
	const Vector3& axisP,
	const Vector3& toCentre,
	unsigned index,

	// These values may be updated
	float& smallestPenetration,
	unsigned &smallestCase
)
{
	// Make sure we have a normalized axis, and don't check almost parallel axes
	if (axisP.GetLengthSquared() < 0.0001) return true;
	Vector3 axis = axisP.GetNormalized();

	float penetration = GetPenetrationOnAxis(one, two, axis, toCentre);

	if (penetration < 0.f) return false;
	if (penetration < smallestPenetration) {
		smallestPenetration = penetration;
		smallestCase = index;
	}
	return true;
}

void FillPointFaceBoxBox(
	const BoxCollider* oneCol,
	const BoxCollider* twoCol,
	const Vector3 &aToB,
	CollisionData& out_collisionData,
	const int& bestAxisIndex,
	float pen
)
{
	// This method is called when we know that a vertex from
	// box two is in contact with box one.

	Contact* contact = &out_collisionData.contacts[out_collisionData.numContacts];

	OBB3 one = oneCol->GetDataInWorldSpace();
	OBB3 two = twoCol->GetDataInWorldSpace();

	// We know which axis the collision is on (i.e. best),
	// but we need to work out which of the two faces on
	// this axis.
	Vector3 normal = Matrix3(one.rotation).columnVectors[bestAxisIndex];
	if (DotProduct(normal, aToB) > 0.f)
	{
		normal = normal * -1.0f;
	}

	// Work out which vertex of box two we're colliding with.
	// Using toCentre doesn't work!
	Vector3 vertex = two.extents;
	if (DotProduct(two.GetRightVector(), normal) < 0.f) vertex.x = -vertex.x;
	if (DotProduct(two.GetUpVector(), normal) < 0.f) vertex.y = -vertex.y;
	if (DotProduct(two.GetForwardVector(), normal) < 0.f) vertex.z = -vertex.z;


	// Create the contact data
	contact->normal = normal;
	contact->penetration = pen;
	contact->position = (Matrix3(two.rotation) * vertex) + two.center; // Convert from vertex box space to world space
	contact->friction = out_collisionData.friction;
	contact->restitution = out_collisionData.restitution;
	contact->bodies[0] = oneCol->GetOwnerRigidBody();
	contact->bodies[1] = twoCol->GetOwnerRigidBody();
	out_collisionData.numContacts++;
}

static inline Vector3 contactPoint(
	const Vector3 &pOne,
	const Vector3 &dOne,
	float oneSize,
	const Vector3 &pTwo,
	const Vector3 &dTwo,
	float twoSize,

	// If this is true, and the contact point is outside
	// the edge (in the case of an edge-face contact) then
	// we use one's midpoint, otherwise we use two's.
	bool useOne)
{
	Vector3 toSt, cOne, cTwo;
	float dpStaOne, dpStaTwo, dpOneTwo, smOne, smTwo;
	float denom, mua, mub;

	smOne = dOne.GetLengthSquared();
	smTwo = dTwo.GetLengthSquared();
	dpOneTwo = DotProduct(dTwo, dOne);

	toSt = pOne - pTwo;
	dpStaOne = DotProduct(dOne, toSt);
	dpStaTwo = DotProduct(dTwo, toSt);

	denom = smOne * smTwo - dpOneTwo * dpOneTwo;

	// Zero denominator indicates parrallel lines
	if (Abs(denom) < 0.0001f) {
		return useOne ? pOne : pTwo;
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
		return useOne ? pOne : pTwo;
	}
	else
	{
		cOne = pOne + dOne * mua;
		cTwo = pTwo + dTwo * mub;

		return cOne * 0.5 + cTwo * 0.5;
	}
}

// This preprocessor definition is only used as a convenience
// in the boxAndBox contact generation method.
#define CHECK_OVERLAP(axis, index) \
    if (!tryAxis(oneCol, twoCol, (axis), toCentre, (index), pen, best)) return 0;

static int BoxAndBox(
	const BoxCollider* oneCol,
	const BoxCollider* twoCol,
	CollisionData& out_collisionData
)
{
	//if (!IntersectionTests::boxAndBox(one, two)) return 0;

	OBB3 one = oneCol->GetDataInWorldSpace();
	OBB3 two = twoCol->GetDataInWorldSpace();

	Matrix3 oneBasis(one.GetRightVector(), one.GetUpVector(), one.GetForwardVector());
	Matrix3 twoBasis(two.GetRightVector(), two.GetUpVector(), two.GetForwardVector());

	// Find the vector between the two centres
	Vector3 toCentre = two.center - one.center;

	// We start assuming there is no contact
	float pen = FLT_MAX;
	unsigned best = 0xffffff;

	// Now we check each axes, returning if it gives us
	// a separating axis, and keeping track of the axis with
	// the smallest penetration otherwise.
	CHECK_OVERLAP(oneBasis.columnVectors[0], 0);
	CHECK_OVERLAP(oneBasis.columnVectors[1], 1);
	CHECK_OVERLAP(oneBasis.columnVectors[2], 2);

	CHECK_OVERLAP(twoBasis.columnVectors[0], 3);
	CHECK_OVERLAP(twoBasis.columnVectors[1], 4);
	CHECK_OVERLAP(twoBasis.columnVectors[2], 5);

	// Store the best axis-major, in case we run into almost
	// parallel edge collisions later
	unsigned bestSingleAxis = best;

	CHECK_OVERLAP(CrossProduct(oneBasis.columnVectors[0], twoBasis.columnVectors[0]), 6);
	CHECK_OVERLAP(CrossProduct(oneBasis.columnVectors[0], twoBasis.columnVectors[1]), 7);
	CHECK_OVERLAP(CrossProduct(oneBasis.columnVectors[0], twoBasis.columnVectors[2]), 8);
	CHECK_OVERLAP(CrossProduct(oneBasis.columnVectors[1], twoBasis.columnVectors[0]), 9);
	CHECK_OVERLAP(CrossProduct(oneBasis.columnVectors[1], twoBasis.columnVectors[1]), 10);
	CHECK_OVERLAP(CrossProduct(oneBasis.columnVectors[1], twoBasis.columnVectors[2]), 11);
	CHECK_OVERLAP(CrossProduct(oneBasis.columnVectors[2], twoBasis.columnVectors[0]), 12);
	CHECK_OVERLAP(CrossProduct(oneBasis.columnVectors[2], twoBasis.columnVectors[1]), 13);
	CHECK_OVERLAP(CrossProduct(oneBasis.columnVectors[2], twoBasis.columnVectors[2]), 14);

	// Make sure we've got a result.
	ASSERT_OR_DIE(best != 0xffffff, "oof ouch my bones we failed the thing");

	// We now know there's a collision, and we know which
	// of the axes gave the smallest penetration. We now
	// can deal with it in different ways depending on
	// the case.
	if (best < 3)
	{
		// We've got a vertex of box two on a face of box one.
		FillPointFaceBoxBox(oneCol, twoCol, toCentre, out_collisionData, best, pen);
		return 1;
	}
	else if (best < 6)
	{
		// We've got a vertex of box one on a face of box two.
		// We use the same algorithm as above, but swap around
		// one and two (and therefore also the vector between their
		// centres).
		FillPointFaceBoxBox(twoCol, oneCol, toCentre*-1.0f, out_collisionData, best - 3, pen);
		return 1;
	}
	else
	{
		// We've got an edge-edge contact. Find out which axes
		best -= 6;
		unsigned oneAxisIndex = best / 3;
		unsigned twoAxisIndex = best % 3;
		Vector3 oneAxis = oneBasis.columnVectors[oneAxisIndex];
		Vector3 twoAxis = twoBasis.columnVectors[twoAxisIndex];
		Vector3 axis = CrossProduct(oneAxis, twoAxis);
		axis.Normalize();

		// The axis should point from box one to box two.
		if (DotProduct(axis, toCentre) > 0.f) axis = axis * -1.0f;

		// We have the axes, but not the edges: each axis has 4 edges parallel
		// to it, we need to find which of the 4 for each object. We do
		// that by finding the point in the centre of the edge. We know
		// its component in the direction of the box's collision axis is zero
		// (its a mid-point) and we determine which of the extremes in each
		// of the other axes is closest.
		Vector3 ptOnOneEdge = one.extents;
		Vector3 ptOnTwoEdge = two.extents;
		for (unsigned i = 0; i < 3; i++)
		{
			if (i == oneAxisIndex) ptOnOneEdge.data[i] = 0.f;
			else if (DotProduct(oneBasis.columnVectors[i], axis) > 0.f) ptOnOneEdge.data[i] = -ptOnOneEdge.data[i];

			if (i == twoAxisIndex) ptOnTwoEdge.data[i] = 0.f;
			else if (DotProduct(twoBasis.columnVectors[i], axis) < 0.f) ptOnTwoEdge.data[i] = -ptOnTwoEdge.data[i];
		}

		// Move them into world coordinates (they are already oriented
		// correctly, since they have been derived from the axes).
		ptOnOneEdge = (oneBasis * ptOnOneEdge) + one.center;
		ptOnTwoEdge = (twoBasis * ptOnTwoEdge) + two.center;

		// So we have a point and a direction for the colliding edges.
		// We need to find out point of closest approach of the two
		// line-segments.
		Vector3 vertex = contactPoint(
			ptOnOneEdge, oneAxis, one.extents.data[oneAxisIndex],
			ptOnTwoEdge, twoAxis, two.extents.data[twoAxisIndex],
			bestSingleAxis > 2
		);

		// We can fill the contact.
		Contact* contact = &out_collisionData.contacts[out_collisionData.numContacts];

		contact->penetration = pen;
		contact->normal = axis;
		contact->position = vertex;
		contact->bodies[0] = oneCol->GetOwnerRigidBody();
		contact->bodies[1] = twoCol->GetOwnerRigidBody();
		contact->restitution = out_collisionData.restitution;
		contact->friction = out_collisionData.friction;
		out_collisionData.numContacts++;
		return 1;
	}
	return 0;
}
#undef CHECK_OVERLAP