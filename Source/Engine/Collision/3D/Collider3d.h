///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 18th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/Capsule3d.h"
#include "Engine/Math/OBB3.h"
#include "Engine/Math/Sphere3d.h"
#include "Engine/Utility/StringId.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Entity;
class Material;
class Polygon3D;
class Transform;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Collider3d
{
	friend class CollisionSystem3d;

public:
	//-----Public Methods-----
	RTTI_BASE_CLASS(Collider3d);

	Collider3d() {}

	virtual void	DebugRender(Material* material) = 0;
	Entity*			GetOwningEntity() const { return m_owner; }


protected:
	//-----Protected Data-----

	Entity*		m_owner = nullptr;
	Transform	m_transform;

};


//-------------------------------------------------------------------------------------------------
class BoxCollider3d : public Collider3d
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(BoxCollider3d);

	BoxCollider3d() {}
	BoxCollider3d(const OBB3& localBounds);

	virtual void DebugRender(Material* material) override;

	void SetShapeWs(const OBB3& localBounds);
	OBB3 GetWorldShape();


private:
	//-----Private Data-----

	OBB3 m_shape;

};


//-------------------------------------------------------------------------------------------------
class SphereCollider3d : public Collider3d
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(SphereCollider3d);

	virtual void DebugRender(Material* material) override;

	Sphere3d GetWorldShape();


private:
	//-----Private Data-----

	Sphere3d m_bounds;

};


//-------------------------------------------------------------------------------------------------
class CapsuleCollider3d : public Collider3d
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(CapsuleCollider3d);

	virtual void DebugRender(Material* material) override;

	Capsule3d GetWorldBounds();


private:
	//-----Private Data-----

	Capsule3d m_bounds;

};


//-------------------------------------------------------------------------------------------------
class PolytopeCollider3d : public Collider3d 
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(PolytopeCollider3d);

	virtual void DebugRender(Material* material) override;

	Polygon3D* GetWorldBounds();


private:
	//-----Private Data-----

	Polygon3D* m_bounds = nullptr;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
