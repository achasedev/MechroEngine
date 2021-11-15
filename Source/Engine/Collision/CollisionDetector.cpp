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
#include "Engine/Collision/CollisionCases/CapsuleCylinderCollision.h"
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
GenerateContactsFunction CollisionDetector::s_colliderMatrix[NUM_COLLIDER_TYPES][NUM_COLLIDER_TYPES] =
{
	{ nullptr, nullptr, &CollisionDetector::GenerateContacts_HalfSpaceSphere,	&CollisionDetector::GenerateContacts_HalfSpaceCapsule,	&CollisionDetector::GenerateContacts_HalfSpaceBox,	&CollisionDetector::GenerateContacts_HalfSpaceCylinder, nullptr },
	{ nullptr, nullptr, &CollisionDetector::GenerateContacts_PlaneSphere,		&CollisionDetector::GenerateContacts_PlaneCapsule,		&CollisionDetector::GenerateContacts_PlaneBox,		&CollisionDetector::GenerateContacts_PlaneCylinder,		nullptr },
	{ nullptr, nullptr,	&CollisionDetector::GenerateContacts_SphereSphere,		&CollisionDetector::GenerateContacts_SphereCapsule,		&CollisionDetector::GenerateContacts_SphereBox,		&CollisionDetector::GenerateContacts_SphereCylinder,	nullptr },
	{ nullptr, nullptr, nullptr,												&CollisionDetector::GenerateContacts_CapsuleCapsule,	&CollisionDetector::GenerateContacts_CapsuleBox,	&CollisionDetector::GenerateContacts_CapsuleCylinder,	nullptr },
	{ nullptr, nullptr, nullptr,												nullptr,												&CollisionDetector::GenerateContacts_BoxBox,		nullptr,												nullptr },
	{ nullptr, nullptr, nullptr,												nullptr,												nullptr,											nullptr,												nullptr },
	{ nullptr, nullptr, nullptr,												nullptr,												nullptr,											nullptr,												nullptr }
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
static float CalculateFrictionBetween(const Collider* a, const Collider* b)
{
	if (a->m_ignoreFriction || b->m_ignoreFriction)
	{
		return 0.f;
	}

	return Sqrt(a->m_friction + b->m_friction);
}


//-------------------------------------------------------------------------------------------------
static float CalculateRestitutionBetween(const Collider* a, const Collider* b)
{
	return a->m_restitution * b->m_restitution;
}


//-------------------------------------------------------------------------------------------------
static void FillOutColliderInfo(Contact* contact, const Collider* a, const Collider* b)
{
	if (a->GetOwnerRigidBody() == nullptr)
	{
		contact->bodies[0] = b->GetOwnerRigidBody();
		contact->bodies[1] = a->GetOwnerRigidBody();
		contact->normal *= -1.f;
	}
	else
	{
		contact->bodies[0] = a->GetOwnerRigidBody();
		contact->bodies[1] = b->GetOwnerRigidBody();
	}

	contact->restitution = CalculateRestitutionBetween(a, b);
	contact->friction = CalculateFrictionBetween(a, b);
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts_SphereSphere(const Collider* a, const Collider* b, Contact* out_contacts, int limit)
{
	const SphereCollider* aSphereCol = a->GetAsType<SphereCollider>();
	const SphereCollider* bSphereCol = b->GetAsType<SphereCollider>();

	ASSERT_OR_DIE(aSphereCol != nullptr && bSphereCol != nullptr, "Colliders not the right type!");

	if (limit <= 0)
		return 0;

	Sphere3D aSphere = aSphereCol->GetDataInWorldSpace();
	Sphere3D bSphere = bSphereCol->GetDataInWorldSpace();

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
	FillOutColliderInfo(out_contacts, aSphereCol, bSphereCol);

	out_contacts->CheckValuesAreReasonable();
	return 1;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const Collider* a, const Collider* b, Contact* out_contacts, int limit)
{
	if (limit <= 0)
		return 0;

	int firstIndex = a->GetTypeIndex();
	int secondIndex = b->GetTypeIndex();

	if (firstIndex > secondIndex)
	{
		int temp = firstIndex;
		firstIndex = secondIndex;
		secondIndex = temp;

		const Collider* tempPtr = a;
		a = b;
		b = tempPtr;
	}

	//ASSERT_OR_DIE(s_colliderMatrix[firstIndex][secondIndex] != nullptr, "Collision matrix missing an entry!");

	if (s_colliderMatrix[firstIndex][secondIndex] != nullptr)
	{
		return (this->*s_colliderMatrix[firstIndex][secondIndex])(a, b, out_contacts, limit);
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts_HalfSpaceSphere(const Collider* a, const Collider* b, Contact* out_contacts, int limit)
{
	const HalfSpaceCollider* aHalfspaceCol = a->GetAsType<HalfSpaceCollider>();
	const SphereCollider* bSphereCol = b->GetAsType<SphereCollider>();

	ASSERT_OR_DIE(aHalfspaceCol != nullptr && bSphereCol != nullptr, "Colliders not the right type!");

	if (limit <= 0)
		return 0;

	Plane3 planeWs = aHalfspaceCol->GetDataInWorldSpace();
	Sphere3D sphereWs = bSphereCol->GetDataInWorldSpace();

	float distance = planeWs.GetDistanceFromPlane(sphereWs.center) - sphereWs.radius;
	
	// Sphere too far in front of plane; its radius isn't enough to intersect the plane
	if (distance >= 0)
		return 0;

	out_contacts->normal = planeWs.GetNormal();
	out_contacts->penetration = -distance;
	out_contacts->position = planeWs.GetProjectedPointOntoPlane(sphereWs.center);
	FillOutColliderInfo(out_contacts, bSphereCol, aHalfspaceCol);

	out_contacts->CheckValuesAreReasonable();
	return 1;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts_HalfSpaceBox(const Collider* a, const Collider* b, Contact* out_contacts, int limit)
{
	const HalfSpaceCollider* aHalfSpaceCol = a->GetAsType<HalfSpaceCollider>();
	const BoxCollider* bBoxCollider = b->GetAsType<BoxCollider>();
	ASSERT_OR_DIE(aHalfSpaceCol != nullptr && bBoxCollider != nullptr, "Colliders are of wrong type!");

	if (limit <= 0)
		return 0;

	Plane3 planeWs = aHalfSpaceCol->GetDataInWorldSpace();
	OBB3 boxWs = bBoxCollider->GetDataInWorldSpace();

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
			FillOutColliderInfo(contactToFill, bBoxCollider, aHalfSpaceCol);
			
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
int CollisionDetector::GenerateContacts_HalfSpaceCylinder(const Collider* a, const Collider* b, Contact* out_contacts, int limit)
{
	const HalfSpaceCollider* aHalfSpaceCol = a->GetAsType<HalfSpaceCollider>();
	const CylinderCollider* bCylinderCol = b->GetAsType<CylinderCollider>();
	ASSERT_OR_DIE(aHalfSpaceCol != nullptr && bCylinderCol != nullptr, "Colliders are of wrong type!");

	if (limit <= 0)
		return 0;

	const Plane3 planeWs = aHalfSpaceCol->GetDataInWorldSpace();
	const Cylinder3D cylinderWs = bCylinderCol->GetDataInWorldSpace();

	int numContactsAdded = 0;
	Contact* contactToFill = out_contacts;

	for (int i = 0; i < 2; ++i)
	{
		Vector3 endPoint = (i == 0 ? cylinderWs.m_bottom : cylinderWs.m_top);
		Vector3 spineDir = (i == 0 ? (cylinderWs.m_bottom - cylinderWs.m_top) : (cylinderWs.m_top - cylinderWs.m_bottom));
		spineDir.Normalize();

		// Get the vector from the end point to the plane
		Vector3 projectedEndPoint = planeWs.GetProjectedPointOntoPlane(endPoint);
		Vector3 endPointToPlane = projectedEndPoint - endPoint;

		// Project onto the spine vector
		float dot = DotProduct(spineDir, endPointToPlane);

		// Get the projection of the endPointToPlane onto the disc of the cylinder
		Vector3 discVector = endPointToPlane - spineDir * dot;
		discVector.SafeNormalize(discVector); // In the case the cylinder is aligned with the normal of the plane, this vector will be 0 - that's fine, this will return the end point as the contact point

		// In the case that the end point is below the plane, make sure we go the other direction
		// to get the lower "edge" of the cylinder disc
		if (DotProduct(discVector, planeWs.m_normal) > 0.f)
		{
			discVector *= -1.0f;
		}

		Vector3 contactPos = endPoint + discVector * cylinderWs.m_radius;
		float pen = planeWs.GetDistanceFromPlane(contactPos);
		if (pen < 0.f)
		{
			// Contact!
			contactToFill->position = contactPos;
			contactToFill->normal = planeWs.m_normal;
			contactToFill->penetration = Abs(pen);
			FillOutColliderInfo(contactToFill, bCylinderCol, aHalfSpaceCol);

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
int CollisionDetector::GenerateContacts_SphereBox(const Collider* a, const Collider* b, Contact* out_contacts, int limit)
{
	const SphereCollider* aSphereCol = a->GetAsType<SphereCollider>();
	const BoxCollider* bBoxCol = b->GetAsType<BoxCollider>();
	ASSERT_OR_DIE(aSphereCol != nullptr && bBoxCol != nullptr, "Colliders are of wrong type!");

	if (limit <= 0)
		return 0;

	Sphere3D sphereWs = aSphereCol->GetDataInWorldSpace();
	OBB3 boxWs = bBoxCol->GetDataInWorldSpace();

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
	FillOutColliderInfo(out_contacts, bBoxCol, aSphereCol);

	out_contacts->CheckValuesAreReasonable();
	return 1;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts_SphereCylinder(const Collider* a, const Collider* b, Contact* out_contacts, int limit)
{
	if (limit <= 0)
		return 0;

	int numContacts = 0;
	const SphereCollider* aSphereCol = a->GetAsType<SphereCollider>();
	const CylinderCollider* bCylinderCol = b->GetAsType<CylinderCollider>();
	ASSERT_OR_DIE(aSphereCol != nullptr && bCylinderCol != nullptr, "Colliders are of wrong type!");

	const Sphere3D sphereWs = aSphereCol->GetDataInWorldSpace();
	const Cylinder3D cylinderWs = bCylinderCol->GetDataInWorldSpace();

	Vector3 bottomToSphere = (sphereWs.center - cylinderWs.m_bottom);

	Vector3 cylSpine = (cylinderWs.m_top - cylinderWs.m_bottom);
	float height = cylSpine.Normalize();
	float dot = DotProduct(bottomToSphere, cylSpine);
	Vector3 discVector = bottomToSphere - cylSpine * dot;

	bool sphereVertical = (discVector.GetLengthSquared() < cylinderWs.m_radius * cylinderWs.m_radius);
	float verticalPen = FLT_MAX;
	Vector3 verticalNormal = Vector3::ZERO;
	Vector3 verticalContactPos = Vector3::ZERO;

	if (sphereVertical)
	{
		Plane3 topPlane(cylSpine, cylinderWs.m_top);
		Plane3 bottomPlane(-1.0f * cylSpine, cylinderWs.m_bottom);
		float topDist = topPlane.GetDistanceFromPlane(sphereWs.center);
		float bottomDist = bottomPlane.GetDistanceFromPlane(sphereWs.center);

		if (Abs(topDist) < Abs(bottomDist))
		{
			verticalNormal = cylSpine;
			verticalPen = sphereWs.radius - topDist;
			verticalContactPos = topPlane.GetProjectedPointOntoPlane(sphereWs.center);
		}
		else
		{
			verticalNormal = -1.0f * cylSpine;
			verticalPen = sphereWs.radius - bottomDist;
			verticalContactPos = bottomPlane.GetProjectedPointOntoPlane(sphereWs.center);
		}
	}

	bool sphereHorizontal = (dot > 0.f && dot < height);
	float horizontalPen = FLT_MAX;
	Vector3 horizontalNormal = Vector3::ZERO;
	Vector3 closestSpinePt;

	if (sphereHorizontal)
	{
		float distanceToSpine = GetClosestPointOnLineSegment(cylinderWs.m_bottom, cylinderWs.m_top, sphereWs.center, closestSpinePt);
		horizontalNormal = (sphereWs.center - closestSpinePt) / distanceToSpine;
		ASSERT_OR_DIE(AreMostlyEqual(horizontalNormal.GetLength(), 1.0f), "My trick didn't work!");
		horizontalPen = (sphereWs.radius + cylinderWs.m_radius) - distanceToSpine;
	}

	bool hasHorizontalPen = (horizontalPen > 0.f && horizontalPen < FLT_MAX);
	bool hasVerticalPen = (verticalPen > 0.f && verticalPen < FLT_MAX);

	if (hasHorizontalPen && hasVerticalPen)
	{
		// Sphere center is inside the cylinder, so push either vertically or horizontally, depending on which has less of a correction
		if (verticalPen < horizontalPen)
		{
			out_contacts->position = verticalContactPos;
			out_contacts->penetration = verticalPen;
			out_contacts->normal = verticalNormal;
			FillOutColliderInfo(out_contacts, aSphereCol, bCylinderCol);
			numContacts++;
		}
		else
		{
			out_contacts->position = sphereWs.center - sphereWs.radius * horizontalNormal;
			out_contacts->penetration = horizontalPen;
			out_contacts->normal = horizontalNormal;
			FillOutColliderInfo(out_contacts, aSphereCol, bCylinderCol);
			numContacts++;
		}
	}
	else if (hasVerticalPen)
	{
		// Sphere is overlapping a disc face, so push normal to the face
		out_contacts->position = verticalContactPos;
		out_contacts->penetration = verticalPen;
		out_contacts->normal = verticalNormal;
		FillOutColliderInfo(out_contacts, aSphereCol, bCylinderCol);
		numContacts++;
	}
	else if (hasHorizontalPen)
	{
		// Sphere is overlapping the cylinder sides, so push outward
		out_contacts->position = closestSpinePt + horizontalNormal * cylinderWs.m_radius;
		out_contacts->penetration = horizontalPen;
		out_contacts->normal = horizontalNormal;
		FillOutColliderInfo(out_contacts, aSphereCol, bCylinderCol);
		numContacts++;
	}
	else
	{
		// Sphere isn't overlapping the sides or the top/bottom faces, so check if it's overlapping an edge
		Vector3 edgePoint = cylinderWs.GetFurthestEdgePointInDirection(bottomToSphere);
		Vector3 edgeToSphere = sphereWs.center - edgePoint;
		
		if (edgeToSphere.GetLengthSquared() < sphereWs.radius * sphereWs.radius)
		{
			out_contacts->position = edgePoint;
			out_contacts->penetration = sphereWs.radius - edgeToSphere.Normalize();
			out_contacts->normal = edgeToSphere;
			FillOutColliderInfo(out_contacts, aSphereCol, bCylinderCol);
			numContacts++;
		}
	}

	if (numContacts > 0)
	{
		out_contacts->CheckValuesAreReasonable();
	}

	return numContacts;
}


//-------------------------------------------------------------------------------------------------
static inline float TransformToAxis(const BoxCollider* box, const Vector3 &axis)
{
	OBB3 boxWs = box->GetDataInWorldSpace();
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
static inline float GetPenetrationOnAxis(const BoxCollider* a, const BoxCollider* b, const Vector3 &axis, const Vector3 &aToB)
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
static inline bool CheckAxis(const BoxCollider* a, const BoxCollider* b, Vector3 axis, const Vector3& toCentre, unsigned index, float& out_smallestPen, unsigned &out_smallestIndex)
{
	// Make sure we have a normalized axis, and don't check almost parallel axes
	if (AreMostlyEqual(axis.GetLengthSquared(), 0.f)) 
		return true;

	axis.Normalize();
	float penetration = GetPenetrationOnAxis(a, b, axis, toCentre);

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
static int CreateFaceVertexContact(const BoxCollider* faceCol, const BoxCollider* vertexCol, const Vector3 &aToB, Contact* out_contact, const int bestAxisIndex, int limit)
{
	// This method is called when we know that a vertex from
	// box two is in contact with box one.
	OBB3 one = faceCol->GetDataInWorldSpace();
	OBB3 two = vertexCol->GetDataInWorldSpace();
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
    if (!CheckAxis(aBoxCol, bBoxCol, (axis), aToB, (index), pen, best)) return 0;

int CollisionDetector::GenerateContacts_BoxBox(const Collider* a, const Collider* b, Contact* out_contacts, int limit)
{
	const BoxCollider* aBoxCol = a->GetAsType<BoxCollider>();
	const BoxCollider* bBoxCol = b->GetAsType<BoxCollider>();
	ASSERT_OR_DIE(aBoxCol != nullptr && bBoxCol != nullptr, "Colliders are of wrong type!");

	if (limit <= 0)
		return 0;

	OBB3 aBox = aBoxCol->GetDataInWorldSpace();
	OBB3 bBox = bBoxCol->GetDataInWorldSpace();
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
		return CreateFaceVertexContact(aBoxCol, bBoxCol, aToB, out_contacts, best, limit);
	}
	else if (best < 6)
	{
		// We've got vertices of box one on a face of box two.
		// We use the same algorithm as above, but swap around
		// one and two (and therefore also the vector between their
		// centers).
		return CreateFaceVertexContact(bBoxCol, aBoxCol, aToB*-1.0f, out_contacts, best - 3, limit);
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
		FillOutColliderInfo(out_contacts, aBoxCol, bBoxCol);

		out_contacts->CheckValuesAreReasonable();
		return 1;
	}

	return 0;
}
#undef CHECK_OVERLAP


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts_HalfSpaceCapsule(const Collider* a, const Collider* b, Contact* out_contacts, int limit)
{
	const HalfSpaceCollider* aHalfSpaceCol = a->GetAsType<HalfSpaceCollider>();
	const CapsuleCollider* bCapsuleCol = b->GetAsType<CapsuleCollider>();
	ASSERT_OR_DIE(aHalfSpaceCol != nullptr && bCapsuleCol != nullptr, "Colliders are of wrong type!");
	
	if (limit <= 0)
		return 0;

	Plane3 planeWs = aHalfSpaceCol->GetDataInWorldSpace();
	Capsule3D capsuleWs = bCapsuleCol->GetDataInWorldSpace();

	float startDistance = planeWs.GetDistanceFromPlane(capsuleWs.start) - capsuleWs.radius;
	float endDistance = planeWs.GetDistanceFromPlane(capsuleWs.end) - capsuleWs.radius;

	if (startDistance >= 0.f && endDistance >= 0.f)
		return 0;

	int numAdded = 0;
	Contact* contactToFill = &out_contacts[0];
	if (startDistance < 0.f)
	{
		contactToFill->normal = planeWs.GetNormal();
		contactToFill->penetration = -startDistance;
		contactToFill->position = capsuleWs.start - capsuleWs.radius * contactToFill->normal;
		FillOutColliderInfo(contactToFill, bCapsuleCol, aHalfSpaceCol);
		contactToFill->CheckValuesAreReasonable();
		numAdded++;
		contactToFill = &out_contacts[numAdded];

		if (limit == 1)
			return 1;
	}

	if (endDistance < 0.f)
	{
		contactToFill->normal = planeWs.GetNormal();
		contactToFill->penetration = -endDistance;
		contactToFill->position = capsuleWs.end - capsuleWs.radius * contactToFill->normal;
		FillOutColliderInfo(contactToFill, bCapsuleCol, aHalfSpaceCol);
		contactToFill->CheckValuesAreReasonable();
		numAdded++;
	}

	return numAdded;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts_SphereCapsule(const Collider* a, const Collider* b, Contact* out_contacts, int limit)
{
	const SphereCollider* aSphereCol = a->GetAsType<SphereCollider>();
	const CapsuleCollider* bCapsuleCol = b->GetAsType<CapsuleCollider>();
	ASSERT_OR_DIE(aSphereCol != nullptr && bCapsuleCol != nullptr, "Colliders are of wrong type!");

	if (limit <= 0)
		return 0;

	Sphere3D sphereWs = aSphereCol->GetDataInWorldSpace();
	Capsule3D capsuleWs = bCapsuleCol->GetDataInWorldSpace();

	Vector3 closestCapsulePointWs;
	float distance = GetClosestPointOnLineSegment(capsuleWs.start, capsuleWs.end, sphereWs.center, closestCapsulePointWs);
	float overlap = (sphereWs.radius + capsuleWs.radius) - distance;

	if (overlap > 0.f)
	{
		out_contacts->normal = (sphereWs.center - closestCapsulePointWs) / distance;
		out_contacts->penetration = overlap;
		out_contacts->position = 0.5f * (closestCapsulePointWs + sphereWs.center);
		FillOutColliderInfo(out_contacts, aSphereCol, bCapsuleCol);
		out_contacts->CheckValuesAreReasonable();

		return 1;
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts_CapsuleCapsule(const Collider* a, const Collider* b, Contact* out_contacts, int limit)
{
	const CapsuleCollider* aCapsuleCol = a->GetAsType<CapsuleCollider>();
	const CapsuleCollider* bCapsuleCol = b->GetAsType<CapsuleCollider>();
	ASSERT_OR_DIE(aCapsuleCol != nullptr && bCapsuleCol != nullptr, "Colliders are of wrong type!");

	if (limit <= 0)
		return 0;

	Capsule3D capsuleA = aCapsuleCol->GetDataInWorldSpace();
	Capsule3D capsuleB = bCapsuleCol->GetDataInWorldSpace();

	Vector3 ptOnA, ptOnB;
	float distance = FindClosestPointsOnLineSegments(capsuleA.start, capsuleA.end, capsuleB.start, capsuleB.end, ptOnA, ptOnB);
	float overlap = (capsuleA.radius + capsuleB.radius) - distance;

	if (overlap > 0.f)
	{
		out_contacts->normal = (ptOnA - ptOnB) / distance;
		out_contacts->penetration = overlap;
		out_contacts->position = 0.5f * (ptOnA + ptOnB);
		FillOutColliderInfo(out_contacts, aCapsuleCol, bCapsuleCol);
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

	// Idea
	// For each axis, see if the end point is "in the tube" of that axis (i.e. for x, make sure we're within the y and x extents - it's ok if we're to the left or right though, that's along the tube)
	// If we are, that qualifies to be a face contact, so calculate the amount of push needed to correct out either side of the box for that axis
	// At the end, take the min pen and that normal, making sure you use the right sign
	Vector3 pens[2];
	Vector3 signs[2];

	// For each end point...
	for (int i = 0; i < 2; ++i)
	{
		Vector3 endPoint = (i == 0 ? startLs : endLs);

		// For each axis
		for (int j = 0; j < 3; ++j)
		{
			// Check the other 2 components to see if we're "in the tube"
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

			// If we're in the tube
			if (inLateralBounds)
			{
				float posPen = extents.data[j] - endPoint.data[j] + radius;
				float negPen = extents.data[j] + endPoint.data[j] + radius;

				// See which direction along the tube is the shorter correction
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
				// Not in the tube - can't make a face contact on this axis (it'd be an edge contact at that point)
				pens[i].data[j] = FLT_MAX;
			}
		}
	}

	// Determine the best local normal for each end point
	Vector3 normalsLs[2];
	for (int i = 0; i < 2; ++i)
	{
		normalsLs[i] = Vector3::ZERO;

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
				normalsLs[i].data[j] = 1.f * signs[i].data[j];
				out_pens[i] = minPen;
				break;
			}
		}
	}

	// If the endpoints find 2 different best faces, the capsule must be intersecting an edge
	// Instead, just generate an edgepoint
	float normalDot = DotProduct(normalsLs[0], normalsLs[1]);
	if ((out_pens[0] < FLT_MAX && out_pens[1] < FLT_MAX) && AreMostlyEqual(normalDot, 0.f))
	{
		return false;
	}
	else if (AreMostlyEqual(normalDot, -1.f))
	{
		// The best normals are opposite sides of the cube haha
		// Might be ok to just arbitrarily pick a way to push, as it might not matter that much at this point ("You shouldn't be here!")
		ERROR_AND_DIE("Ha, capsule is stuck isn't it");
	}

	// Start and/or end could be valid contacts, but at this point they share a normal, so either once can calculate it
	if (out_pens[0] < FLT_MAX)
	{
		Matrix3 basis(box.rotation);
		out_normal = normalsLs[0].x * basis.columnVectors[0] + normalsLs[0].y * basis.columnVectors[1] + normalsLs[0].z * basis.columnVectors[2];
		out_positions[0] = capsule.start - out_normal * radius;
	}

	if (out_pens[1] < FLT_MAX)
	{
		Matrix3 basis(box.rotation);
		out_normal = normalsLs[1].x * basis.columnVectors[0] + normalsLs[1].y * basis.columnVectors[1] + normalsLs[1].z * basis.columnVectors[2];
		out_positions[1] = capsule.end - out_normal * radius;
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
int CollisionDetector::GenerateContacts_CapsuleBox(const Collider* a, const Collider* b, Contact* out_contacts, int limit)
{
	const CapsuleCollider* aCapsuleCol = a->GetAsType<CapsuleCollider>();
	const BoxCollider* bBoxCol = b->GetAsType<BoxCollider>();
	ASSERT_OR_DIE(aCapsuleCol != nullptr && bBoxCol != nullptr, "Colliders are of wrong type!");

	if (limit <= 0)
		return 0;

	Capsule3D capsuleWs = aCapsuleCol->GetDataInWorldSpace();
	OBB3 boxWs = bBoxCol->GetDataInWorldSpace();

	float facePens[2];
	Vector3 faceNormal;
	Vector3 faceContactPos[2];
	bool hasFaceOverlap = GetMinPlanePen(boxWs, capsuleWs, facePens, faceContactPos, faceNormal);

	float edgePen = FLT_MAX;
	Vector3 edgeNormal = Vector3::ZERO;
	Vector3 edgePos = Vector3::ZERO;
	bool hasEdgeOverlap = GetMinEdgePen(boxWs, capsuleWs, edgePen, edgeNormal, edgePos);

	// If we have face overlap but no edge overlap *or* the face overlap has less pen than the edge, make face contacts
	int numContactsAdded = 0;

	if (hasFaceOverlap)
	{
		if (facePens[0] < FLT_MAX)
		{
			out_contacts->normal = faceNormal;
			out_contacts->penetration = facePens[0];
			out_contacts->position = faceContactPos[0];
			FillOutColliderInfo(out_contacts, aCapsuleCol, bBoxCol);
			out_contacts->CheckValuesAreReasonable();
			out_contacts++;
			numContactsAdded++;
		}

		if (facePens[1] < FLT_MAX && numContactsAdded < limit)
		{
			out_contacts->normal = faceNormal;
			out_contacts->penetration = facePens[1];
			out_contacts->position = faceContactPos[1];
			FillOutColliderInfo(out_contacts, aCapsuleCol, bBoxCol);
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
		FillOutColliderInfo(out_contacts, aCapsuleCol, bBoxCol);
		out_contacts->CheckValuesAreReasonable();

		numContactsAdded++;
	}

	return numContactsAdded;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts_CapsuleCylinder(const Collider* a, const Collider* b, Contact* out_contacts, int limit)
{
	if (limit <= 0)
		return 0;

	const CapsuleCollider* aCapsuleCol = a->GetAsType<CapsuleCollider>();
	const CylinderCollider* bCylinderCol = b->GetAsType<CylinderCollider>();
	ASSERT_OR_DIE(aCapsuleCol != nullptr && bCylinderCol != nullptr, "Colliders are of wrong type!");

	CapsuleCylinderCollision collision(aCapsuleCol, bCylinderCol, out_contacts, limit);
	collision.Solve();

	// This is bad, but bear with me
	int numContacts = collision.GetNumContacts();
	for (int i = 0; i < numContacts; ++i)
	{
		FillOutColliderInfo(&out_contacts[i], aCapsuleCol, bCylinderCol);
	}

	return numContacts;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts_PlaneSphere(const Collider* a, const Collider* b, Contact* out_contacts, int limit)
{
	const PlaneCollider* aPlaneCol = a->GetAsType<PlaneCollider>();
	const SphereCollider* bSphereCol = b->GetAsType<SphereCollider>();
	ASSERT_OR_DIE(aPlaneCol != nullptr && bSphereCol != nullptr, "Colliders are of wrong type!");

	if (limit <= 0)
		return 0;

	Plane3 planeWs = aPlaneCol->GetDataInWorldSpace();
	Sphere3D sphereWs = bSphereCol->GetDataInWorldSpace();

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
	FillOutColliderInfo(out_contacts, bSphereCol, aPlaneCol);

	out_contacts->CheckValuesAreReasonable();
	return 1;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts_PlaneCapsule(const Collider* a, const Collider* b, Contact* out_contacts, int limit)
{
	const PlaneCollider* aPlaneCol = a->GetAsType<PlaneCollider>();
	const CapsuleCollider* bCapsuleCol = b->GetAsType<CapsuleCollider>();
	ASSERT_OR_DIE(aPlaneCol != nullptr && bCapsuleCol != nullptr, "Capsules are of wrong type!");

	if (limit <= 0)
		return 0;

	Plane3 planeWs = aPlaneCol->GetDataInWorldSpace();
	Capsule3D capsuleWs = bCapsuleCol->GetDataInWorldSpace();

	float startDistance = planeWs.GetDistanceFromPlane(capsuleWs.start);
	float endDistance = planeWs.GetDistanceFromPlane(capsuleWs.end);
	bool startCloser = Abs(startDistance) < Abs(endDistance);
	bool bisected = (startDistance > 0.f && endDistance < 0.f) || (startDistance < 0.f && endDistance > 0.f);

	Vector3 normal = Vector3::ZERO;
	if (bisected)
	{
		// Push out the direction of least correction
		if (startCloser)
		{
			normal = (startDistance < 0.f ? planeWs.m_normal : -1.0f * planeWs.m_normal);
		}
		else
		{
			normal = (endDistance < 0.f ? planeWs.m_normal : -1.0f * planeWs.m_normal);
		}
	}
	else
	{
		// Both endpoints are on the same side, so they both have the same normal direction
		normal = (startDistance > 0.f ? planeWs.m_normal : -1.0f * planeWs.m_normal);
	}

	// Just for convenience
	planeWs.m_normal = normal;

	int numAdded = 0;
	if (Abs(startDistance) < capsuleWs.radius || (bisected && startCloser))
	{
		out_contacts->normal = normal;
		out_contacts->penetration = (planeWs.IsPointInFront(capsuleWs.start) ? (capsuleWs.radius - Abs(startDistance)) : (capsuleWs.radius + Abs(startDistance)));
		out_contacts->position = capsuleWs.start - normal * capsuleWs.radius;
		FillOutColliderInfo(out_contacts, bCapsuleCol, aPlaneCol);
		out_contacts->CheckValuesAreReasonable();
		numAdded++;
		out_contacts++;

		if (limit == 1)
			return 1;
	}

	if (Abs(endDistance) < capsuleWs.radius || (bisected && !startCloser))
	{
		out_contacts->normal = normal;
		out_contacts->penetration = (planeWs.IsPointInFront(capsuleWs.end) ? (capsuleWs.radius - Abs(endDistance)) : (capsuleWs.radius + Abs(endDistance)));
		out_contacts->position = capsuleWs.end - normal * capsuleWs.radius;
		FillOutColliderInfo(out_contacts, bCapsuleCol, aPlaneCol);
		out_contacts->CheckValuesAreReasonable();
		numAdded++;
	}

	return numAdded;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts_PlaneBox(const Collider* a, const Collider* b, Contact* out_contacts, int limit)
{
	const PlaneCollider* aPlaneCol = a->GetAsType<PlaneCollider>();
	const BoxCollider* bBoxCol = b->GetAsType<BoxCollider>();
	ASSERT_OR_DIE(aPlaneCol != nullptr && bBoxCol != nullptr, "Colliders are of wrong type!");

	if (limit <= 0)
		return 0;

	Plane3 planeWs = aPlaneCol->GetDataInWorldSpace();
	OBB3 boxWs = bBoxCol->GetDataInWorldSpace();

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
		FillOutColliderInfo(contactToFill, bBoxCol, aPlaneCol);

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


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts_PlaneCylinder(const Collider* a, const Collider* b, Contact* out_contacts, int limit)
{
	const PlaneCollider* aPlaneCol = a->GetAsType<PlaneCollider>();
	const CylinderCollider* bCylinderCol = b->GetAsType<CylinderCollider>();
	ASSERT_OR_DIE(aPlaneCol != nullptr && bCylinderCol != nullptr, "Colliders are of wrong type!");

	if (limit <= 0)
		return 0;

	const Plane3 planeWs = aPlaneCol->GetDataInWorldSpace();
	const Cylinder3D cylinderWs = bCylinderCol->GetDataInWorldSpace();

	int numContactsAdded = 0;
	Contact* contactToFill = out_contacts;

	bool limitToTop = true;
	Vector3 frontTopPointWs = cylinderWs.GetFurthestEdgePointInDirection(planeWs.m_normal, &limitToTop);
	Vector3 backTopPointWs = cylinderWs.GetFurthestEdgePointInDirection(-1.0f * planeWs.m_normal, &limitToTop);

	limitToTop = false;
	Vector3 frontBottomPointWs = cylinderWs.GetFurthestEdgePointInDirection(planeWs.m_normal, &limitToTop);
	Vector3 backBottomPointWs = cylinderWs.GetFurthestEdgePointInDirection(-1.0f * planeWs.m_normal, &limitToTop);

	// If the points are on the same side of the plane, there's no intersection
	float frontTopDist = planeWs.GetDistanceFromPlane(frontTopPointWs);
	float backTopDist = planeWs.GetDistanceFromPlane(backTopPointWs);
	float frontBottomDist = planeWs.GetDistanceFromPlane(frontBottomPointWs);
	float backBottomDist = planeWs.GetDistanceFromPlane(backBottomPointWs);

	// Check top face intersecting
	bool topIntersection = (frontTopDist * backTopDist < 0.f);
	bool bottomIntersection = (frontBottomDist * backBottomDist < 0.f);

	if (topIntersection && !bottomIntersection)
	{
		if (Abs(frontTopDist) < Abs(backTopDist))
		{
			// Push against the plane normal to correct
			contactToFill->position = frontTopPointWs;
			contactToFill->normal = -1.0f * planeWs.m_normal;
			contactToFill->penetration = Abs(frontTopDist);
		}
		else
		{
			// Push towards the plane normal to correct
			contactToFill->position = backTopPointWs;
			contactToFill->normal = planeWs.m_normal;
			contactToFill->penetration = Abs(backTopDist);
		}

		FillOutColliderInfo(contactToFill, bCylinderCol, aPlaneCol);

		contactToFill->CheckValuesAreReasonable();
		numContactsAdded++;
	}
	else if (!topIntersection && bottomIntersection)
	{
		if (Abs(frontBottomDist) < Abs(backBottomDist))
		{
			// Push against the plane normal to correct
			contactToFill->position = frontBottomPointWs;
			contactToFill->normal = -1.0f * planeWs.m_normal;
			contactToFill->penetration = Abs(frontBottomDist);
		}
		else
		{
			// Push towards the plane normal to correct
			contactToFill->position = backBottomPointWs;
			contactToFill->normal = planeWs.m_normal;
			contactToFill->penetration = Abs(backBottomDist);
		}

		FillOutColliderInfo(contactToFill, bCylinderCol, aPlaneCol);

		contactToFill->CheckValuesAreReasonable();
		numContactsAdded++;
	}
	else if (topIntersection && bottomIntersection)
	{
		// Make sure we don't push the cylinder into a bad state
		// This occurs if the top want to correct the opposite direction the bottom wants

		// If both are wanting to go to the same side of the plane, no problem!
		bool topWantsFront = Abs(backTopDist) < Abs(frontTopDist);
		bool bottomWantsFront = Abs(backBottomDist) < Abs(frontBottomDist);

		if (topWantsFront == bottomWantsFront)
		{
			Vector3 normal = (topWantsFront ? planeWs.m_normal : -1.0f * planeWs.m_normal);

			if (topWantsFront)
			{
				contactToFill->position = backTopPointWs;
				contactToFill->normal = normal;
				contactToFill->penetration = Abs(backTopDist);
				FillOutColliderInfo(contactToFill, bCylinderCol, aPlaneCol);
				contactToFill->CheckValuesAreReasonable();
				numContactsAdded++;

				if (numContactsAdded < limit) { contactToFill = &out_contacts[numContactsAdded]; }
				else
				{
					return numContactsAdded;
				}

				contactToFill->position = backBottomPointWs;
				contactToFill->normal = normal;
				contactToFill->penetration = Abs(backBottomDist);
				FillOutColliderInfo(contactToFill, bCylinderCol, aPlaneCol);
				contactToFill->CheckValuesAreReasonable();
				numContactsAdded++;
			}
			else
			{
				contactToFill->position = frontTopPointWs;
				contactToFill->normal = normal;
				contactToFill->penetration = Abs(frontTopDist);
				FillOutColliderInfo(contactToFill, bCylinderCol, aPlaneCol);
				contactToFill->CheckValuesAreReasonable();
				numContactsAdded++;

				if (numContactsAdded < limit) { contactToFill = &out_contacts[numContactsAdded]; }
				else
				{
					return numContactsAdded;
				}

				contactToFill->position = frontBottomPointWs;
				contactToFill->normal = normal;
				contactToFill->penetration = Abs(frontBottomDist);
				FillOutColliderInfo(contactToFill, bCylinderCol, aPlaneCol);
				contactToFill->CheckValuesAreReasonable();
				numContactsAdded++;
			}
		}
		else
		{
			// Conflict - whoever has the min distance gets the say, and the other has to just go along with it
			Vector3 topContactPos, bottomContactPos;
			float topPen, bottomPen;
			Vector3 normal;

			float minDist = Min(frontBottomDist, backBottomDist, frontTopDist, backTopDist);

			if (minDist == frontBottomDist || minDist == frontTopDist)
			{
				// Push backwards
				normal = -1.0f * planeWs.m_normal;

				bottomContactPos = frontBottomPointWs;
				bottomPen = Abs(frontBottomDist);

				topContactPos = frontTopPointWs;
				topPen = Abs(frontTopDist);
			}
			else
			{
				// Push forward
				normal = planeWs.m_normal;

				bottomContactPos = backBottomPointWs;
				bottomPen = Abs(backBottomDist);

				topContactPos = backTopPointWs;
				topPen = Abs(backTopDist);
			}

			contactToFill->position = topContactPos;
			contactToFill->normal = normal;
			contactToFill->penetration = topPen;
			FillOutColliderInfo(contactToFill, bCylinderCol, aPlaneCol);
			contactToFill->CheckValuesAreReasonable();
			numContactsAdded++;

			if (numContactsAdded < limit) { contactToFill = &out_contacts[numContactsAdded]; }
			else
			{
				return numContactsAdded;
			}

			contactToFill->position = bottomContactPos;
			contactToFill->normal = normal;
			contactToFill->penetration = bottomPen;
			FillOutColliderInfo(contactToFill, bCylinderCol, aPlaneCol);
			contactToFill->CheckValuesAreReasonable();
			numContactsAdded++;
		}
	}
	else
	{
		float maxDist = Max(frontBottomDist, backBottomDist, frontTopDist, backTopDist);
		float minDist = Min(frontBottomDist, backBottomDist, frontTopDist, backTopDist);

		if (minDist * maxDist < 0.f)
		{
			// If the min pen is closer to the plane than max, it's shorter to push with the normal
			bool pushInFrontOfPlane = Abs(minDist) < Abs(maxDist);
			Vector3 normal = pushInFrontOfPlane ? planeWs.m_normal : -1.0f * planeWs.m_normal;
			bool isTopFace = (pushInFrontOfPlane ? (minDist == backTopDist) : (maxDist == frontTopDist));

			// Shorter to push the cylinder in front of the plane
			// Determine if the top or bottom of the cylinder is behind the plane
			if (isTopFace)
			{
				contactToFill->position = backTopPointWs;
				contactToFill->normal = normal;
				contactToFill->penetration = Abs(backTopDist);
				FillOutColliderInfo(contactToFill, bCylinderCol, aPlaneCol);
				contactToFill->CheckValuesAreReasonable();
				numContactsAdded++;

				if (numContactsAdded < limit) { contactToFill = &out_contacts[numContactsAdded]; }
				else
				{
					return numContactsAdded;
				}

				// In case the cylinder spine is parallel to the plane normal, the two edge points would be the endpoint itself
				if (!AreMostlyEqual(backTopPointWs, frontTopPointWs))
				{
					contactToFill->position = frontTopPointWs;
					contactToFill->normal = normal;
					contactToFill->penetration = Abs(frontTopDist);
					FillOutColliderInfo(contactToFill, bCylinderCol, aPlaneCol);
					contactToFill->CheckValuesAreReasonable();
					numContactsAdded++;
				}
			}
			else
			{
				contactToFill->position = backBottomPointWs;
				contactToFill->normal = normal;
				contactToFill->penetration = Abs(backBottomDist);
				FillOutColliderInfo(contactToFill, bCylinderCol, aPlaneCol);
				contactToFill->CheckValuesAreReasonable();
				numContactsAdded++;

				if (numContactsAdded < limit) { contactToFill = &out_contacts[numContactsAdded]; }
				else
				{
					return numContactsAdded;
				}

				// In case the cylinder spine is parallel to the plane normal, the two edge points would be the endpoint itself
				if (!AreMostlyEqual(backBottomPointWs, frontBottomPointWs))
				{
					contactToFill->position = frontBottomPointWs;
					contactToFill->normal = normal;
					contactToFill->penetration = Abs(frontBottomDist);
					FillOutColliderInfo(contactToFill, bCylinderCol, aPlaneCol);
					contactToFill->CheckValuesAreReasonable();
					numContactsAdded++;
				}
			}
		}
	}

	return numContactsAdded;
}


