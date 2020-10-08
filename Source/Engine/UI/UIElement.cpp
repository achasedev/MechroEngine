///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 3rd, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/UI/Canvas.h"
#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIPanel.h"
#include "Engine/UI/UIText.h"
#include "Engine/Utility/StringID.h"
#include "Engine/Utility/XMLUtils.h"

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

//-------------------------------------------------------------------------------------------------
static void SetXPositionYPositionFromXML(const XMLElem& element, RectTransform& transform)
{
	// Lots of validation!
	bool xPositionSpecified = XML::DoesAttributeExist(element, "x_position");
	bool yPositionSpecified = XML::DoesAttributeExist(element, "y_position");
	bool separatePositionsSpecified = xPositionSpecified && yPositionSpecified;
	bool combinedPositionSpecified = XML::DoesAttributeExist(element, "position");
	bool widthSpecified = XML::DoesAttributeExist(element, "width");
	bool heightSpecified = XML::DoesAttributeExist(element, "height");
	bool separateDimensionsSpecified = widthSpecified && heightSpecified;
	bool combinedDimensionsSpecified = XML::DoesAttributeExist(element, "dimensions");
	bool anyPaddingSpecified = XML::DoesAttributeExist(element, "x_padding") || XML::DoesAttributeExist(element, "y_padding");

	GUARANTEE_OR_DIE(separatePositionsSpecified || combinedPositionSpecified, "Anchors are set up to be positional but no positions specified!");
	GUARANTEE_OR_DIE((separatePositionsSpecified && !combinedPositionSpecified) || (!xPositionSpecified && !yPositionSpecified && combinedPositionSpecified), "Duplicate positions specified!");
	GUARANTEE_OR_DIE(separateDimensionsSpecified || combinedDimensionsSpecified, "Anchors are set up to be positional but no dimensions specified!");
	GUARANTEE_OR_DIE((separateDimensionsSpecified && !combinedDimensionsSpecified) || (!widthSpecified && !heightSpecified && combinedDimensionsSpecified), "Duplicate dimensions specified!");
	GUARANTEE_RECOVERABLE(!anyPaddingSpecified, "Padding specified for positional anchors, they will not be used!");

	Vector2 position;
	if (combinedPositionSpecified)
	{
		position = XML::ParseAttribute(element, "position", Vector2::ZERO);
	}
	else
	{
		position.x = XML::ParseAttribute(element, "x_position", 0.f);
		position.y = XML::ParseAttribute(element, "y_position", 0.f);
	}

	Vector2 dimensions;
	if (combinedDimensionsSpecified)
	{
		dimensions = XML::ParseAttribute(element, "dimensions", Vector2::ZERO);
	}
	else
	{
		dimensions.x = XML::ParseAttribute(element, "width", 0.f);
		dimensions.y = XML::ParseAttribute(element, "height", 0.f);
	}
	
	transform.SetPosition(position);
	transform.SetDimensions(dimensions);
}


//-------------------------------------------------------------------------------------------------
static void SetXPositionYPaddingFromXML(const XMLElem& element, RectTransform& transform)
{
	bool xPaddingSpecified			= XML::DoesAttributeExist(element, "x_padding");
	bool yPaddingSpecified			= XML::DoesAttributeExist(element, "y_padding");
	bool xPositionSpecified			= XML::DoesAttributeExist(element, "x_position");
	bool yPositionSpecified			= XML::DoesAttributeExist(element, "y_position");
	bool combinedPositionSpecified	= XML::DoesAttributeExist(element, "position");
	bool widthSpecified				= XML::DoesAttributeExist(element, "width");
	bool heightSpecified			= XML::DoesAttributeExist(element, "height");

	GUARANTEE_OR_DIE(xPositionSpecified, "Horizontal anchors are set up to be positional but no x_position specified!");
	GUARANTEE_OR_DIE(widthSpecified, "Horizontal anchors are set up to be positional but no width specified!");
	GUARANTEE_OR_DIE(yPaddingSpecified, "Vertical anchors are set up to use padding but no y_padding specified!");
	GUARANTEE_RECOVERABLE(!xPaddingSpecified, "X Padding specified for horizontal positional anchors, it won't be used!");
	GUARANTEE_RECOVERABLE(!heightSpecified, "Height specified for vertically padded anchors, it won't be used!");
	GUARANTEE_RECOVERABLE(!yPositionSpecified, "Y position specified for vertically padded anchors, it won't be used!");
	GUARANTEE_RECOVERABLE(!combinedPositionSpecified, "Position specified but vertical anchors require padding, it won't be used!");

	float xPosition = XML::ParseAttribute(element, "x_position", 0.f);
	float width = XML::ParseAttribute(element, "width", 0.f);
	Vector2 yPadding = XML::ParseAttribute(element, "y_padding", Vector2::ZERO);

	transform.SetXPosition(xPosition);
	transform.SetWidth(width);
	transform.SetVerticalPadding(yPadding.x, yPadding.y);
}


//-------------------------------------------------------------------------------------------------
static void SetXPaddingYPositionFromXML(const XMLElem& element, RectTransform& transform)
{
	bool xPaddingSpecified			= XML::DoesAttributeExist(element, "x_padding");
	bool yPaddingSpecified			= XML::DoesAttributeExist(element, "y_padding");
	bool xPositionSpecified			= XML::DoesAttributeExist(element, "x_position");
	bool yPositionSpecified			= XML::DoesAttributeExist(element, "y_position");
	bool combinedPositionSpecified	= XML::DoesAttributeExist(element, "position");
	bool widthSpecified				= XML::DoesAttributeExist(element, "width");
	bool heightSpecified			= XML::DoesAttributeExist(element, "height");

	GUARANTEE_OR_DIE(xPaddingSpecified, "Horizontal anchors are set up to use padding but no x_padding specified!");
	GUARANTEE_OR_DIE(yPositionSpecified, "Anchors are set up to  specified!");
	GUARANTEE_OR_DIE(heightSpecified, "Horizontal anchors are set up to be positional but no width specified!");
	GUARANTEE_RECOVERABLE(!yPaddingSpecified, "Y Padding specified for vertical positional anchors, it won't be used!");
	GUARANTEE_RECOVERABLE(!widthSpecified, "Width specified for horizontally padded anchors, it won't be used!");
	GUARANTEE_RECOVERABLE(!xPositionSpecified, "X position specified for horizontally padded anchors, it won't be used!");
	GUARANTEE_RECOVERABLE(!combinedPositionSpecified, "Position specified but horizontal anchors require padding, it won't be used!");

	Vector2 xPadding = XML::ParseAttribute(element, "x_padding", Vector2::ZERO);
	float yPosition = XML::ParseAttribute(element, "y_position", 0.f);
	float height = XML::ParseAttribute(element, "height", 0.f);

	transform.SetHorizontalPadding(xPadding.x, xPadding.y);
	transform.SetYPosition(yPosition);
	transform.SetHeight(height);
}


//-------------------------------------------------------------------------------------------------
static void SetXPaddingYPaddingFromXML(const XMLElem& element, RectTransform& transform)
{
	bool xPaddingSpecified			= XML::DoesAttributeExist(element, "x_padding");
	bool yPaddingSpecified			= XML::DoesAttributeExist(element, "y_padding");
	bool xPositionSpecified			= XML::DoesAttributeExist(element, "x_position");
	bool yPositionSpecified			= XML::DoesAttributeExist(element, "y_position");
	bool combinedPositionSpecified	= XML::DoesAttributeExist(element, "position");
	bool widthSpecified				= XML::DoesAttributeExist(element, "width");
	bool heightSpecified			= XML::DoesAttributeExist(element, "height");
	bool dimensionsSpecified		= XML::DoesAttributeExist(element, "dimensions");
	bool anyPositionSpecified		= xPositionSpecified || yPositionSpecified || combinedPositionSpecified;
	bool anyDimensionSpecified		= widthSpecified || heightSpecified || dimensionsSpecified;

	GUARANTEE_OR_DIE(xPaddingSpecified, "Anchors are set up to use padding but no x_padding specified!");
	GUARANTEE_OR_DIE(yPaddingSpecified, "Anchors are set up to use padding but no y_padding specified!");
	GUARANTEE_RECOVERABLE(!anyPositionSpecified, "Anchors are set up for padding only but position was specified, it won't be used!");
	GUARANTEE_RECOVERABLE(!anyDimensionSpecified, "Anchors are set up for padding only but dimensions were specified, they won't be used!");

	Vector2 xPadding = XML::ParseAttribute(element, "x_padding", Vector2::ZERO);
	Vector2 yPadding = XML::ParseAttribute(element, "y_padding", Vector2::ZERO);

	transform.SetHorizontalPadding(xPadding.x, xPadding.y);
	transform.SetVerticalPadding(yPadding.x, yPadding.y);
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
UIElement::UIElement(Canvas* canvas)
	: m_transform(RectTransform(canvas))
	, m_canvas(canvas)
{
}


//-------------------------------------------------------------------------------------------------
UIElement::~UIElement()
{
	uint32 numChildren = (uint32)m_children.size();
	for (uint32 childIndex = 0; childIndex < numChildren; ++childIndex)
	{
		SAFE_DELETE(m_children[childIndex]);
	}

	m_children.clear();
}


//-------------------------------------------------------------------------------------------------
void UIElement::Render()
{
	// Parent should already have rendered themselves
	// Now render the children on top
	uint32 numChildren = (uint32)m_children.size();
	for (uint32 childIndex = 0; childIndex < numChildren; ++childIndex)
	{
		m_children[childIndex]->Render();
	}
}


//-------------------------------------------------------------------------------------------------
void UIElement::AddChild(UIElement* child)
{
	m_children.push_back(child);
	child->m_parent = this;
	child->m_transform.SetParentTransform(&m_transform);
}


//-------------------------------------------------------------------------------------------------
void UIElement::SetCanvas(Canvas* canvas)
{
	m_canvas = canvas;
}


//-------------------------------------------------------------------------------------------------
void UIElement::InitializeFromXML(const XMLElem& element)
{
	// name
	m_id = SID(element.Name());

	// pivot
	Vector2 pivot = XML::ParseAttribute(element, "pivot", Vector2::ZERO);
	m_transform.SetPivot(pivot);

	// anchors or anchor_preset
	// anchors takes precedence
	if (XML::DoesAttributeExist(element, "anchors"))
	{
		AABB2 anchors = XML::ParseAttribute(element, "anchors", AABB2::ZEROS);
		m_transform.SetAnchors(anchors);
	}
	else
	{
		std::string presetText = XML::ParseAttribute(element, "anchor_preset", "bottom_right");
		AnchorPreset preset = StringToAnchorPreset(presetText);
		m_transform.SetAnchors(preset);
	}

	// Search for the right positional/padding data based on the anchors
	AnchorMode anchorMode = m_transform.GetAnchorMode();

	switch (anchorMode)
	{
	case AnchorMode::X_POSITIONAL_Y_POSITIONAL: SetXPositionYPositionFromXML(element, m_transform); break;
	case AnchorMode::X_POSITIONAL_Y_PADDING:	SetXPositionYPaddingFromXML(element, m_transform); break;
	case AnchorMode::X_PADDING_Y_POSITIONAL:	SetXPaddingYPositionFromXML(element, m_transform); break;
	case AnchorMode::X_PADDING_Y_PADDING:		SetXPaddingYPaddingFromXML(element, m_transform); break;
	default:
		ERROR_RECOVERABLE("Bad AnchorMode!");
		break;
	}

	// Recursively create and add children
	const XMLElem* child = element.FirstChildElement();
	while (child != nullptr)
	{
		UIElement* uiElement = CreateUIElementFromXML(*child, m_canvas);

		if (uiElement != nullptr)
		{
			m_children.push_back(uiElement);
		}

		child = child->NextSiblingElement();
	}
}


//-------------------------------------------------------------------------------------------------
OBB2 UIElement::CalculateFinalBounds() const
{
	return m_transform.GetBounds();
}


//-------------------------------------------------------------------------------------------------
Matrix44 UIElement::CalculateModelMatrix() const
{
	OBB2 finalBounds = CalculateFinalBounds();
	return CalculateModelMatrix(finalBounds);
}


//-------------------------------------------------------------------------------------------------
Matrix44 UIElement::CalculateModelMatrix(const OBB2& finalBounds) const
{
	// Account for pivot:
	// - Translate in normalized space
	// - Apply translation/rotation/scale (now rotating about correct point)
	// - Translate back in new scaled space to "undo" the translation to line up the rotation pivot

	Vector3 translation = Vector3(finalBounds.alignedBounds.GetBottomLeft(), 0.f);
	Vector3 rotation = Vector3(0.f, 0.f, finalBounds.orientationDegrees);
	Vector3 scale = Vector3(finalBounds.alignedBounds.GetWidth(), finalBounds.alignedBounds.GetHeight(), 1.0f);
	Vector3 pivotTranslation = Vector3(m_transform.GetPivot(), 0.f);
	Vector3 postRotateCorrection = Vector3(pivotTranslation.x * scale.x, pivotTranslation.y * scale.y, 0.f);

	return Matrix44::MakeTranslation(postRotateCorrection) * Matrix44::MakeModelMatrix(translation, rotation, scale) * Matrix44::MakeTranslation(-1.0f * pivotTranslation);
}


//-------------------------------------------------------------------------------------------------
UIElement* UIElement::FindChildByID(StringID id)
{
	if (m_id == id)
	{
		return this;
	}

	size_t numChildren = m_children.size();

	for (size_t childIndex = 0; childIndex < numChildren; ++childIndex)
	{
		UIElement* childWithID = m_children[childIndex]->FindChildByID(id);

		if (childWithID != nullptr)
		{
			return childWithID;
		}
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
STATIC UIElement* UIElement::CreateUIElementFromXML(const XMLElem& element, Canvas* canvas)
{
	std::string name = element.Name();
	size_t underscoreIndex = name.find_last_of('_');
	ASSERT_OR_DIE(underscoreIndex != std::string::npos && underscoreIndex != name.size() - 1, "UIElement element name %s needs to have \"_<UIElement type>\" at the end of it!", name.c_str());

	std::string elementType = name.substr(underscoreIndex + 1);

	UIElement* uiElement = nullptr;
	if (elementType == "panel") { uiElement = new UIPanel(canvas); }
	else if (elementType == "text") { uiElement = new UIText(canvas); }
	else
	{
		ERROR_RECOVERABLE("Cannot create UIElement of type %s!", elementType.c_str());
		return nullptr;
	}

	uiElement->InitializeFromXML(element);
	return uiElement;
}
