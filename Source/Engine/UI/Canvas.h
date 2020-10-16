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
#include "Engine/IO/Mouse.h"
#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIPanel.h"
#include "Engine/UI/UIText.h"
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

	void				ProcessInput();
	virtual void		Update() override;
	virtual void		Render() override;

	AABB2				GenerateOrthoBounds() const;
	Matrix44			GenerateOrthoMatrix() const;

	void				SetScreenMatchMode(ScreenMatchMode mode, float widthHeightBlend = 1.0f);
	void				SetResolution(float height, float width);
	void				AddElementToGlobalMap(UIElement* element);
	void				RemoveElementFromGlobalMap(UIElement* element);

	virtual	void*		GetType() const override { return &s_type; }
	Vector2				GetResolution() const { return m_resolution; }
	Texture2D*			GetOutputTexture() const;
	UIElement*			FindElementByID(StringID id);
	float				GetAspect() const;
	Vector2				GetPixelsPerUnit() const;
	Vector2				GetCanvasUnitsPerPixel() const;
	uint32				ToPixelWidth(float canvasWidth) const;
	uint32				ToPixelHeight(float canvasHeight) const;
	float				ToCanvasWidth(uint32 pixelWidth) const;
	float				ToCanvasHeight(uint32 pixelHeight) const;
	StringID			GetNextUnspecifiedID();
	uint32				GetTopLayerIndex() const;
	Vector2				GetMousePosition() const;
	Vector2				GetMousePositionLastFrame() const;
	bool				WasHoveredLastFrame(UIElement* element) const;

	static void*		GetTypeStatic() { return &s_type; }

	template <typename T>
	T*					FindElementAsType(StringID id);

	bool				Event_WindowResize(NamedProperties& args);


private:
	//-----Private Methods-----

	void SetupUIMouseInfo(UIMouseInfo& out_input);
	void FindMouseHoveredElements(const Vector2& cursorCanvasPos, std::vector<UIElement*>& out_hoverStack) const;
	void HandleMouseJustHovers(const std::vector<UIElement*>& hoverStack, const UIMouseInfo& mouseInfo);
	void HandleMouseHovers(const std::vector<UIElement*>& hoverStack, const UIMouseInfo& mouseInfo);
	void HandleMouseUnhovers(const std::vector<UIElement*>& hoverStack, const UIMouseInfo& mouseInfo);
	void HandleMouseClicks(const std::vector<UIElement*>& hoverStack, const UIMouseInfo& mouseInfo);


private:
	//-----Private Data-----

	Vector2							m_resolution = Vector2(100.f);
	ScreenMatchMode					m_matchMode = SCREEN_MATCH_WIDTH_OR_HEIGHT;
	Texture2D*						m_outputTexture = nullptr;
	int								m_outputTextureHeight = 0; // For detecting changes that will require fonts to be re-rendered
	int								m_nextUnspecifiedIDSuffix = 0;

	float							m_widthOrHeightBlend = 1.0f; // 1.0 is match to height
	std::map<StringID, UIElement*>	m_globalElementMap; // For speed and tracking

	std::vector<UIElement*>			m_elementsHoveredLastFrame;
	UIMouseInfo						m_lastFrameUIMouseInfo;
	UIElement*						m_currentClickedElement = nullptr;

	static int s_type;

};


//-------------------------------------------------------------------------------------------------
template <typename T>
T* Canvas::FindElementAsType(StringID id)
{
	UIElement* element = FindElementByID(id);
	if (element->GetType() == T::GetTypeStatic())
	{
		return (T*)element;
	}

	return nullptr;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
