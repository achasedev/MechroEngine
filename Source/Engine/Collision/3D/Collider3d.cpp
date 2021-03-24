///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 18, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Collision/3D/Collider3d.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/Transform.h"
#include "Engine/Render/Core/RenderContext.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
RTTI_TYPE_DEFINE(Collider3d);
RTTI_TYPE_DEFINE(SphereCollider3d);
RTTI_TYPE_DEFINE(BoxCollider3d);
RTTI_TYPE_DEFINE(CapsuleCollider3d);
RTTI_TYPE_DEFINE(PolytopeCollider3d);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Material;

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
Sphere3d SphereCollider3d::GetWorldShape()
{
	Vector3 positionWs = m_bounds.m_center;
	positionWs = m_transform.TransformPositionLocalToWorld(positionWs);
	
	return Sphere3d(positionWs, m_bounds.m_radius);
}


//-------------------------------------------------------------------------------------------------
BoxCollider3d::BoxCollider3d(const OBB3& localBounds)
{
	SetShapeWs(localBounds);
}

#include "Engine/Render/Core/Renderable.h"
#include "Engine/Render/Mesh/MeshBuilder.h"
//-------------------------------------------------------------------------------------------------
void BoxCollider3d::DebugRender(Material* material)
{
	MeshBuilder mb;
	mb.BeginBuilding(true);

	mb.PushCube(Vector3::ZERO, 2.f * Vector3::ONES);
	mb.FinishBuilding();

	Mesh mesh;
	mb.UpdateMesh<Vertex3D_PCU>(mesh);

	Matrix44 toWorld = m_transform.GetLocalToWorldMatrix();
	Matrix44 shapeAsMatrix = m_shape.GetModelMatrix();
	Matrix44 model = toWorld * shapeAsMatrix;

	Renderable rend;
	rend.SetRenderableMatrix(model);
	rend.AddDraw(&mesh, material);

	g_renderContext->DrawRenderable(rend);	

	OBB3 bounds = GetWorldShape();

	Vector3 points[8];
	bounds.GetPoints(points);

	for (int i = 0; i < 8; ++i)
	{
		g_renderContext->DrawPoint3D(points[i], 0.25f, material, Rgba::MAGENTA);
	}
}


//-------------------------------------------------------------------------------------------------
void BoxCollider3d::SetShapeWs(const OBB3& localBounds)
{
	m_shape = localBounds;
}


//-------------------------------------------------------------------------------------------------
OBB3 BoxCollider3d::GetWorldShape()
{
	Matrix44 toWorld = m_transform.GetLocalToWorldMatrix();
	Vector3 centerWs = toWorld.TransformPoint(m_shape.center).xyz();
	Quaternion rotation = m_transform.GetWorldRotation() * m_shape.rotation;

	Vector3 scale = m_transform.GetWorldScale();
	scale.x *= m_shape.extents.x;
	scale.y *= m_shape.extents.y;
	scale.z *= m_shape.extents.z;

	return OBB3(centerWs, scale, rotation);
}
