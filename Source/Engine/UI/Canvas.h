///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 4th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/AABB2.h"
#include "Engine/UI/UIElement.h"
#include <vector>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Camera;
class Texture2D;

enum ScreenMatchMode
{
	SCREEN_MATCH_WIDTH_OR_HEIGHT,
	SCREEN_MATCH_EXPAND_TO_FILL,
	SCREEN_MATCH_SHRINK_TO_FIT
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Canvas : public UIElement
{
public:
	//-----Public Methods-----

	Canvas();
	~Canvas();

	void				Initialize(Texture2D* outputTexture, const Vector2& resolution, ScreenMatchMode mode, float widthHeightBlend = 1.0f);
	void				InitializeFromXML(const char* xmlFilePath);
	virtual void		InitializeFromXML(const XMLElem& element) override;
	AABB2				GenerateOrthoBounds() const;
	Matrix44			GenerateOrthoMatrix() const;
	virtual void		Render() override;

	void				SetScreenMatchMode(ScreenMatchMode mode, float widthHeightBlend = 1.0f);
	void				SetResolution(float height, float width);

	Texture2D*			GetOutputTexture() const;
	Vector2				GetResolution() const { return m_resolution; }

	float				GetAspect() const;
	Vector2				GetPixelsPerUnit() const;
	Vector2				GetCanvasUnitsPerPixel() const;
	uint32				ToPixelWidth(float canvasWidth) const;
	uint32				ToPixelHeight(float canvasHeight) const;
	float				ToCanvasWidth(uint32 pixelWidth) const;
	float				ToCanvasHeight(uint32 pixelHeight) const;


private:
	//-----Private Data-----

	Vector2				m_resolution = Vector2(100.f);
	ScreenMatchMode		m_matchMode = SCREEN_MATCH_WIDTH_OR_HEIGHT;
	Texture2D*			m_outputTexture = nullptr;
	float				m_widthOrHeightBlend = 1.0f; // 1.0 is match to height

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
