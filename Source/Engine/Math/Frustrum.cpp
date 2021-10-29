///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Oct 29th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/Frustrum.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Matrix4.h"

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
// Default Constructor - Make it invalid
Frustrum::Frustrum()
{
	for (int i = 0; i < 8; ++i)
	{
		m_points[i] = Vector3::ZERO;
	}

	for (int i = 0; i < 6; ++i)
	{
		m_planes[i] = Plane3(Vector3::ZERO, 0.f);
	}
}


//-------------------------------------------------------------------------------------------------
// Constructor
Frustrum::Frustrum(const Matrix4& view, const Matrix4& projection)
{
	Set(view, projection);
}


//-------------------------------------------------------------------------------------------------
// Sets the points to represent the frustrum created by the given view and projection
void Frustrum::Set(const Matrix4& view, const Matrix4& projection)
{
	const Vector4 ndcPositions[8] =
	{
		Vector4(-1.f, -1.f, 0.f, 1.0f),
		Vector4(-1.f, 1.f, 0.f, 1.0f),
		Vector4(1.f, 1.f, 0.f, 1.0f),
		Vector4(1.f, -1.f, 0.f, 1.0f),
		Vector4(1.f, -1.f, 1.f, 1.0f),
		Vector4(1.f, 1.f, 1.f, 1.0f),
		Vector4(-1.f, 1.f, 1.f, 1.0f),
		Vector4(-1.f, -1.f, 1.f, 1.0f)
	};

	Matrix4 inverseViewProjection = Matrix4::GetInverse(projection * view);

	for (int i = 0; i < 8; ++i)
	{
		Vector4 homogenousCoord = inverseViewProjection * ndcPositions[i];
		homogenousCoord.w = 1.f / homogenousCoord.w;

		m_points[i].x = homogenousCoord.x * homogenousCoord.w;
		m_points[i].y = homogenousCoord.y * homogenousCoord.w;
		m_points[i].z = homogenousCoord.z * homogenousCoord.w;
	}

	// Make the planes
	// Right
	{
		Vector3 a = m_points[4] - m_points[3];
		Vector3 b = m_points[2] - m_points[3];
		Vector3 normal = CrossProduct(b, a);

		m_planes[0] = Plane3(normal, m_points[3]);
	}

	// Left
	{
		Vector3 a = m_points[0] - m_points[7];
		Vector3 b = m_points[6] - m_points[7];
		Vector3 normal = CrossProduct(b, a);

		m_planes[1] = Plane3(normal, m_points[7]);
	}

	// Top
	{
		Vector3 a = m_points[2] - m_points[1];
		Vector3 b = m_points[6] - m_points[1];
		Vector3 normal = CrossProduct(b, a);

		m_planes[2] = Plane3(normal, m_points[1]);
	}

	// Bottom
	{
		Vector3 a = m_points[4] - m_points[7];
		Vector3 b = m_points[0] - m_points[7];
		Vector3 normal = CrossProduct(b, a);

		m_planes[3] = Plane3(normal, m_points[7]);
	}

	// Front
	{
		Vector3 a = m_points[7] - m_points[4];
		Vector3 b = m_points[5] - m_points[4];
		Vector3 normal = CrossProduct(b, a);

		m_planes[4] = Plane3(normal, m_points[4]);
	}

	// Back
	{
		Vector3 a = m_points[3] - m_points[0];
		Vector3 b = m_points[1] - m_points[0];
		Vector3 normal = CrossProduct(b, a);

		m_planes[5] = Plane3(normal, m_points[0]);
	}
}


//-------------------------------------------------------------------------------------------------
// Returns true if the point is within the bounds of the frustrum
bool Frustrum::IsPointInside(const Vector3& point) const
{
	for (int i = 0; i < 6; ++i)
	{
		if (m_planes[i].IsPointBehind(point))
		{
			return true;
		}
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
// Returns the planes of the frustrum
void Frustrum::GetPlanes(std::vector<Plane3> out_planes) const
{
	out_planes.clear();

	for (int i = 0; i < 6; ++i)
	{
		out_planes.push_back(m_planes[i]);	
	}
}
