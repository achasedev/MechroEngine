///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: September 9th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Physics/Collision2D.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

enum EvolveSimplexResult
{
	NO_INTERSECTION,
	INTERSECTION_FOUND,
	SIMPLEX_STILL_EVOLVING
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Vector2 GetFarthestVertexInDirectionOfMinkowskiDifference(const Polygon2D& first, const Polygon2D& second, Vector2& direction)
{
	return (first.GetFarthestVertexInDirection(direction) - second.GetFarthestVertexInDirection(-1.0f * direction));
}


//-------------------------------------------------------------------------------------------------
void SetupSimplex(const Polygon2D& first, const Polygon2D& second, std::vector <Vector2>& simplex)
{
	// A vertex
	Vector2 direction = second.GetCenter() - first.GetCenter();
	simplex.push_back(GetFarthestVertexInDirectionOfMinkowskiDifference(first, second, direction));

	// B vertex
	direction = -1.0f * direction;
	simplex.push_back(GetFarthestVertexInDirectionOfMinkowskiDifference(first, second, direction));

	// C vertex
	Vector2 a = simplex[0];
	Vector2 b = simplex[1];
	Vector2 ab = b - a;
	Vector2 abPerp = Vector2(-ab.y, ab.x);

	// Make sure the new direction is pointing towards the origin
	Vector2 aToOrigin = -1.0f * a;

	if (DotProduct(abPerp, aToOrigin) < 0.f)
	{
		abPerp *= -1.0f;

		// So....we're going to do "left-side" perps - flip components and negate the X component, 
		// so the perp of AB will pivot +90 degrees on A
		// Therefore, I want to make it so that the left-side perp is pointing towards the origin for AB. 
		// If it isn't, just switch A and B around, so now it is. Then we can safely just use the left-side 
		// perp on BC and CA below to test if the origin is inside.
		Vector2 temp = a;
		simplex[0] = b;
		simplex[1] = temp;
	}

	simplex.push_back(GetFarthestVertexInDirectionOfMinkowskiDifference(first, second, abPerp));
}


//-------------------------------------------------------------------------------------------------
static EvolveSimplexResult EvolveSimplex(const Polygon2D& first, const Polygon2D& second, std::vector<Vector2>& evolvingSimplex)
{
	ASSERT_OR_DIE(evolvingSimplex.size() == 3, "Wrong number of verts for simplex!");

	Vector2 a = evolvingSimplex[0];
	Vector2 b = evolvingSimplex[1];
	Vector2 c = evolvingSimplex[2];

	Vector2 cToOrigin = -1.0f * c;
	Vector2 bc = c - b;
	Vector2 ca = a - c;

	Vector2 bcPerp = Vector2(-bc.y, bc.x);
	Vector2 caPerp = Vector2(-ca.y, ca.x);

	if (DotProduct(bcPerp, cToOrigin) < 0) // Check if origin is outside bc side
	{
		evolvingSimplex.clear();
		evolvingSimplex.push_back(c);
		evolvingSimplex.push_back(b);

		// Swapping order of b and c means left-hand perp of 0 and 1 is now pointing towards origin
		Vector2 newAB = evolvingSimplex[1] - evolvingSimplex[0];
		Vector2 newABPerp = Vector2(-newAB.y, newAB.x);

		evolvingSimplex.push_back(GetFarthestVertexInDirectionOfMinkowskiDifference(first, second, newABPerp));

		// Check if we even went past the origin
		bool wentPastOrigin = (DotProduct(newABPerp, evolvingSimplex[2]) >= 0);
		return (wentPastOrigin ? SIMPLEX_STILL_EVOLVING : NO_INTERSECTION);
	}
	else if (DotProduct(caPerp, cToOrigin) < 0) // Check if origin is outside ca side
	{
		evolvingSimplex.clear();
		evolvingSimplex.push_back(a);
		evolvingSimplex.push_back(c);

		// Keeping this order of a and c means left-hand perp of 0 and 1 is now pointing towards origin
		Vector2 newAB = evolvingSimplex[1] - evolvingSimplex[0];
		Vector2 newABPerp = Vector2(-newAB.y, newAB.x);

		evolvingSimplex.push_back(GetFarthestVertexInDirectionOfMinkowskiDifference(first, second, newABPerp));

		// Check if we even went past the origin
		bool wentPastOrigin = (DotProduct(newABPerp, evolvingSimplex[2]) >= 0);
		return (wentPastOrigin ? SIMPLEX_STILL_EVOLVING : NO_INTERSECTION);
	}

	// Otherwise both sides contain t	he origin, so intersection!
	return INTERSECTION_FOUND;
}


//-------------------------------------------------------------------------------------------------
bool Physics2D::ArePolygonsColliding(const Polygon2D& first, const Polygon2D& second)
{
	EvolveSimplexResult result = SIMPLEX_STILL_EVOLVING;
	std::vector<Vector2> simplex;

	// Set up initial vertices
	SetupSimplex(first, second, simplex);

	while (result == SIMPLEX_STILL_EVOLVING)
	{
		result = EvolveSimplex(first, second, simplex);
	}

	return (result == INTERSECTION_FOUND);
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

