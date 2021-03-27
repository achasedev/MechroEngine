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
#include "Engine/Math/Polygon3d.h"
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
class Polygon3d;
class Rgba;
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

	virtual void	DebugRender(Material* material, const Rgba& color) = 0;
	Entity*			GetOwningEntity() const { return m_owner; }


protected:
	//-----Protected Data-----

	Entity*		m_owner = nullptr;
	Transform	m_transform;

};


//-------------------------------------------------------------------------------------------------
class SphereCollider3d : public Collider3d
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(SphereCollider3d);

	virtual void DebugRender(Material* material, const Rgba& color) override;

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

	virtual void DebugRender(Material* material, const Rgba& color) override;

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

	PolytopeCollider3d(const OBB3& boxShape);
	virtual void		DebugRender(Material* material, const Rgba& color) override;

	void				SetShape(Polygon3d* shape);
	const Polygon3d*	GetShape() const;


protected:
	//-----Protected Data-----

	Polygon3d* m_shape = nullptr;;

};


//-------------------------------------------------------------------------------------------------
//class BoxCollider3d : public Collider3d
//{
//public:
//	//-----Public Methods-----
//	RTTI_DERIVED_CLASS(BoxCollider3d);
//
//	BoxCollider3d() {}
//	BoxCollider3d(const OBB3& localBounds);
//
//	virtual void DebugRender(Material* material) override;
//
//	void SetLocalShape(const OBB3& localShape);
//	OBB3 GetWorldShape();
//
//
//private:
//	//-----Private Methods-----
//
//
//private:
//	//-----Private Data-----
//
//	OBB3		m_shapeLs;
//	Polygon3d	m_polygonLs;
//
//};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
