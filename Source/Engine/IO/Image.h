///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 14th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/Rgba.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/IntVector2.h"
#include "Engine/Resource/Resource.h"

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
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// (0,0) is BOTTOM LEFT
// To use an image as a texture, it must be flipped vertically (DirectX uses (0,0) top left)
class Image : public Resource
{
public:
	//------Public Methods-----

	Image() {}
	Image(const IntVector2& dimensions, const Rgba& color = Rgba::WHITE);
	~Image();

	void			Initialize(const IntVector2& dimensions, const Rgba& color = Rgba::WHITE);
	bool			Load(const char* filepath, bool flipVertically = true);
	void			Clear();

	void			SetTexelColor(uint32 x, uint32 y, Rgba color);

	Rgba			GetTexelColor(uint32 x, uint32 y) const;
	uint32			GetTexelCount() const { return m_dimensions.x * m_dimensions.y; }
	int				GetTexelWidth() const { return m_dimensions.x; }
	int				GetTexelHeight() const { return m_dimensions.y; }
	IntVector2		GetDimensions() const { return m_dimensions; }
	uint32			GetNumComponentsPerTexel() const { return m_numComponentsPerTexel; }
	uint8*			GetData() const { return m_data; }
	uint32			GetSize() const { return m_size; }


private:
	//-----Private Methods-----

	uint32 GetDataIndexForTexel(uint32 x, uint32 y) const;


private:
	//-----Private Data-----

	uint32		m_size = 0;
	IntVector2	m_dimensions = IntVector2(0,0);
	int			m_numComponentsPerTexel = 0;
	uint8*		m_data = nullptr;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
