///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 15th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/AABB3.h"
#include "Engine/Math/OBB2.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Render/Font/Font.h"
#include "Engine/Render/Font/FontAtlas.h"
#include "Engine/Render/Mesh/MeshBuilder.h"

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
float ConvertPixelOffsetToNormalizedOffset(int pixelOffset, float pixelsPerUnit, float referenceRange)
{
	float valueInRange = static_cast<float>(pixelOffset) * pixelsPerUnit;
	return valueInRange / referenceRange;
}


//-------------------------------------------------------------------------------------------------
// All units in canvas space
Vector2 CalcLineStartFromAlignment(const AABB2& textBounds, const Vector2& textDimensions, FontAtlas* atlas, const Vector2& canvasUnitsPerPixel, int lineIndex, int totalLines, HorizontalAlignment xAlign, VerticalAlignment yAlign)
{
	uint32	pixelLineSpacing = atlas->GetVerticalLineSpacingPixels();
	float	canvasLineSpacing = canvasUnitsPerPixel.y * (float)pixelLineSpacing;
	float	textBoundsWidth = textBounds.GetWidth();
	float	textBoundsHeight = textBounds.GetHeight();
	Vector2 startingPos = Vector2::ZERO;
	int		maxAscentPixels = atlas->GetMaxAscentPixels();
	float	canvasMaxAscent = canvasUnitsPerPixel.x * (float)maxAscentPixels;

	// Horizontal Alignment
	switch (xAlign)
	{
	case ALIGNMENT_LEFT: 
		startingPos.x = textBounds.left; 
		break;
	case ALIGNMENT_CENTER: 
		startingPos.x = textBounds.left + 0.5f * (textBoundsWidth - textDimensions.x);
		break;
	case ALIGNMENT_RIGHT: 
		startingPos.x = textBounds.right - textDimensions.x; 
		break;
	default:
		ERROR_AND_DIE("Bad horizontal alignment!");
		break;
	}

	// Vertical
	switch (yAlign)
	{
	case ALIGNMENT_TOP:
		startingPos.y = textBounds.top - canvasMaxAscent - ((float)lineIndex * canvasLineSpacing);
		break;
	case ALIGNMENT_MIDDLE:
		// This one sucks, but this formula seems to work
		startingPos.y = (textBounds.bottom + 0.5f * textBoundsHeight) + (0.5f * (float)totalLines - (float)lineIndex - 1.0f) * canvasLineSpacing;
		break;
	case ALIGNMENT_BOTTOM:
	{
		startingPos.y = textBounds.bottom + (float)(totalLines - lineIndex - 1) * canvasLineSpacing;

		// Another hack - if the alignment is bottom, ensure the max descent of
		// the current font stays within the box
		int maxDescentPixels = atlas->GetMaxDescentPixels();
		if (maxDescentPixels < 0)
		{
			startingPos.y -= (float)maxDescentPixels * canvasUnitsPerPixel.y;
		}
	}
	break;
	default:
		ERROR_AND_DIE("Bad vertical alignment!");
		break;
	}

	// Calculate its normalized position
	startingPos.x = (startingPos.x - textBounds.left) / textBoundsWidth;
	startingPos.y = (startingPos.y - textBounds.bottom) / textBoundsHeight;

	return startingPos;
}


//-------------------------------------------------------------------------------------------------
static uint32 PushText_Default(
	MeshBuilder& mb,
	const std::vector<ColoredText>& textLines,
	uint32 pixelHeight,
	Font* font,
	const AABB2& textBounds,
	const Vector2& canvasUnitsPerPixel,
	HorizontalAlignment xAlign,
	VerticalAlignment yAlign,
	std::vector<std::vector<AABB2>>* out_glyphBounds)
{
	mb.AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST);

	ASSERT_OR_DIE(font != nullptr, "Null font!");
	FontAtlas* atlas = font->CreateOrGetAtlasForPixelHeight(pixelHeight);

	// Make sure we create all lines
	// If there's an empty line, we need to reserve space for it
	if (out_glyphBounds != nullptr)
	{
		out_glyphBounds->clear();
		out_glyphBounds->resize(textLines.size());
	}

	for (int lineIndex = 0; lineIndex < static_cast<int>(textLines.size()); ++lineIndex)
	{
		const std::string& text = textLines[lineIndex].m_text;
		int numChars = static_cast<int>(text.size());

		IntVector2 strPixelDimensions = atlas->GetTextDimensionsPixels(text);
		Vector2 textCanvasDimensions = Vector2(canvasUnitsPerPixel.x * (float)strPixelDimensions.x, canvasUnitsPerPixel.y * (float)strPixelDimensions.y);
		float textBoundsWidth = textBounds.GetWidth();
		float textBoundsHeight = textBounds.GetHeight();

		// Calculate the line start
		Vector2 runningPos = CalcLineStartFromAlignment(textBounds, textCanvasDimensions, atlas, canvasUnitsPerPixel, lineIndex, static_cast<int>(textLines.size()), xAlign, yAlign);

		for (int charIndex = 0; charIndex < numChars; ++charIndex)
		{
			char currChar = text[charIndex];

			GlyphInfo info = atlas->CreateOrGetGlyphInfo(currChar);
			int pixelKerning = (charIndex > 0 ? font->GetKerningInPixels(pixelHeight, text[charIndex - 1], currChar) : 0);

			// Calculate the starting position for the glyph
			// Small hack - if the glyph has a negative leftSideBearing and it's the start of a line, shift it right to prevent it going out of bounds/off screen
			// Do this outside CalcLineStartFromAlignment since we also use this value below
			uint32 pixelLeftSideAdjustment = 0;
			if (charIndex == 0 && info.m_pixelLeftSideBearing < 0 && xAlign == ALIGNMENT_LEFT)
			{
				pixelLeftSideAdjustment = -info.m_pixelLeftSideBearing;
			}

			Vector2 startOffset;
			startOffset.x = ConvertPixelOffsetToNormalizedOffset(info.m_pixelLeftSideBearing + pixelLeftSideAdjustment + pixelKerning, canvasUnitsPerPixel.x, textBoundsWidth);
			startOffset.y = -1.0f * ConvertPixelOffsetToNormalizedOffset(info.m_pixelBottomSideBearing, canvasUnitsPerPixel.y, textBoundsHeight);

			AABB2 glyphBounds;
			glyphBounds.mins = runningPos + startOffset;

			// The vertex shader creates the bounds shape from a ZERO_TO_ONE AABB2, so these bounds need to be normalized within this space
			glyphBounds.right = glyphBounds.left + ConvertPixelOffsetToNormalizedOffset(info.m_pixelWidth, canvasUnitsPerPixel.x, textBoundsWidth);
			glyphBounds.top = glyphBounds.bottom + ConvertPixelOffsetToNormalizedOffset(info.m_pixelHeight, canvasUnitsPerPixel.y, textBoundsHeight);

			mb.PushQuad2D(glyphBounds, info.m_glyphUVs, textLines[lineIndex].m_color);

			if (out_glyphBounds != nullptr)
			{
				// Calculate the local space bounds for the glyph
				AABB2 localBounds;

				localBounds.mins.x = glyphBounds.mins.x * textBoundsWidth;
				localBounds.mins.y = glyphBounds.mins.y * textBoundsHeight;
				localBounds.maxs.x = glyphBounds.maxs.x * textBoundsWidth;
				localBounds.maxs.y = glyphBounds.maxs.y * textBoundsHeight;

				(*out_glyphBounds)[lineIndex].push_back(localBounds);
			}

			// Update running position
			runningPos.x += ConvertPixelOffsetToNormalizedOffset(info.m_pixelHorizontalAdvance + pixelLeftSideAdjustment, canvasUnitsPerPixel.x, textBoundsWidth);
			runningPos.y += ConvertPixelOffsetToNormalizedOffset(info.m_pixelVerticalAdvance, canvasUnitsPerPixel.y, textBoundsHeight);
		}
	}

	return pixelHeight;
}


//-------------------------------------------------------------------------------------------------
static uint32 PushText_ShrinkToFit(
	MeshBuilder& mb,
	const std::vector<ColoredText>& textLines,
	uint32 pixelHeight,
	Font* font,
	const AABB2& textBounds,
	const Vector2& canvasUnitsPerPixel,
	HorizontalAlignment xAlign,
	VerticalAlignment yAlign,
	std::vector<std::vector<AABB2>>* out_glyphBounds)
{
	mb.AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST);

	FontAtlas* atlas = font->CreateOrGetAtlasForPixelHeight(pixelHeight);
	uint32 finalHeight = pixelHeight;

	for (size_t lineIndex = 0; lineIndex < textLines.size(); ++lineIndex)
	{
		IntVector2 strPixelDimensions = atlas->GetTextDimensionsPixels(textLines[lineIndex].m_text);
		Vector2 textCanvasDimensions = Vector2(canvasUnitsPerPixel.x * (float)strPixelDimensions.x, canvasUnitsPerPixel.y * (float)strPixelDimensions.y);

		float boundsWidth = textBounds.GetWidth();
		float boundsHeight = textBounds.GetHeight();

		// Shrink to ensure it fits in the x
		uint32 xDesiredHeight = pixelHeight;
		if (textCanvasDimensions.x > boundsWidth)
		{
			float xFactor = boundsWidth / textCanvasDimensions.x;
			xDesiredHeight = static_cast<uint32>(static_cast<float>(xDesiredHeight) * xFactor);
		}

		// Shrink to ensure it fits in the y
		uint32 yDesiredHeight = pixelHeight;
		if (textCanvasDimensions.y > boundsHeight)
		{
			float yFactor = boundsHeight / textCanvasDimensions.y;
			yDesiredHeight = static_cast<uint32>(static_cast<float>(yDesiredHeight) * yFactor);
		}

		// Take the min to ensure the text fits in both dimensions
		finalHeight = Min(finalHeight, xDesiredHeight, yDesiredHeight);
	}

	return PushText_Default(mb, textLines, finalHeight, font, textBounds, canvasUnitsPerPixel, xAlign, yAlign, out_glyphBounds);
}


//-------------------------------------------------------------------------------------------------
static uint32 PushText_ExpandToFill(
	MeshBuilder& mb,
	const std::vector<ColoredText>& textLines,
	uint32 pixelHeight,
	Font* font,
	const AABB2& textBounds,
	const Vector2& canvasUnitsPerPixel,
	HorizontalAlignment xAlign,
	VerticalAlignment yAlign,
	std::vector<std::vector<AABB2>>* out_glyphBounds)
{
	mb.AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST);

	FontAtlas* atlas = font->CreateOrGetAtlasForPixelHeight(pixelHeight);
	uint32 finalHeight = pixelHeight;

	for (size_t lineIndex = 0; lineIndex < textLines.size(); ++lineIndex)
	{
		IntVector2 strPixelDimensions = atlas->GetTextDimensionsPixels(textLines[lineIndex].m_text);
		Vector2 textCanvasDimensions = Vector2(canvasUnitsPerPixel.x * (float)strPixelDimensions.x, canvasUnitsPerPixel.y * (float)strPixelDimensions.y);

		float boundsWidth = textBounds.GetWidth();
		float boundsHeight = textBounds.GetHeight();

		// Scale up x so it fills up the space
		uint32 xDesiredHeight = pixelHeight;
		if (textCanvasDimensions.x < boundsWidth)
		{
			float xFactor = boundsWidth / textCanvasDimensions.x;
			xDesiredHeight = static_cast<uint32>(static_cast<float>(xDesiredHeight) * xFactor);
		}

		// Scale up y so it fills up the space
		uint32 yDesiredHeight = pixelHeight;
		if (textCanvasDimensions.y < boundsHeight)
		{
			float yFactor = boundsHeight / textCanvasDimensions.y;
			yDesiredHeight = static_cast<uint32>(static_cast<float>(yDesiredHeight) * yFactor);
		}

		// Max to ensure both dimensions are filled
		finalHeight = Max(finalHeight, xDesiredHeight, yDesiredHeight);
	}
	

	return PushText_Default(mb, textLines, finalHeight, font, textBounds, canvasUnitsPerPixel, xAlign, yAlign, out_glyphBounds);
}


//-------------------------------------------------------------------------------------------------
static uint32 PushText_WordWrap(
	MeshBuilder& mb,
	const std::vector<ColoredText>& textLines,
	uint32 pixelHeight,
	Font* font,
	const AABB2& textBounds,
	const Vector2& canvasUnitsPerPixel,
	HorizontalAlignment xAlign,
	VerticalAlignment yAlign,
	std::vector<std::vector<AABB2>>* out_glyphBounds)
{
	mb.AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST);

	FontAtlas* atlas = font->CreateOrGetAtlasForPixelHeight(pixelHeight);
	float textBoundsWidth = textBounds.GetWidth();
	std::vector<ColoredText> finalLines;

	for (size_t textLineIndex = 0; textLineIndex < textLines.size(); ++textLineIndex)
	{
		std::string text = textLines[textLineIndex].m_text;

		std::vector<std::string> words;
		Tokenize(text, ' ', words);
		size_t numWords = words.size();
		std::string currLine;

		for (size_t wordIndex = 0; wordIndex < numWords; ++wordIndex)
		{
			IntVector2 currLinePixelDimensions = atlas->GetTextDimensionsPixels(currLine);
			Vector2 currLineCanvasDimensions = Vector2(canvasUnitsPerPixel.x * (float)currLinePixelDimensions.x, canvasUnitsPerPixel.y * (float)currLinePixelDimensions.y);

			std::string testLine = (currLine.size() > 0 ? currLine + ' ' + words[wordIndex] : words[wordIndex]);
			IntVector2 testLinePixelDimensions = atlas->GetTextDimensionsPixels(testLine);
			Vector2 testLineCanvasDimensions = Vector2(canvasUnitsPerPixel.x * (float)testLinePixelDimensions.x, canvasUnitsPerPixel.y * (float)testLinePixelDimensions.y);

			if (testLineCanvasDimensions.x <= textBoundsWidth)
			{
				currLine = testLine;
			}
			else
			{
				// Save off the line
				finalLines.push_back(ColoredText(currLine, textLines[textLineIndex].m_color));

				// Update for where the next line will be
				currLine = words[wordIndex];
			}
		}

		// Be sure to draw any leftovers
		if (currLine.size() > 0)
		{
			finalLines.push_back(ColoredText(currLine, textLines[textLineIndex].m_color));
		}
	}

	return PushText_Default(mb, finalLines, pixelHeight, font, textBounds, canvasUnitsPerPixel, xAlign, yAlign, out_glyphBounds);
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
MeshBuilder::~MeshBuilder()
{
	Clear();
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::BeginBuilding(MeshTopology topology, bool useIndices)
{
	ASSERT_RECOVERABLE(!m_isBuilding, "You're already building!");

	Clear();

	m_instruction.m_useIndices = useIndices;
	m_instruction.m_topology = topology;
	m_isBuilding = true;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::FinishBuilding()
{
	if (m_instruction.m_useIndices) { m_instruction.m_elementCount = (uint32)m_indices.size(); }
	else							{ m_instruction.m_elementCount = (uint32)m_vertices.size(); }

	m_isBuilding = false;	
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::Clear()
{
	m_stamp = VertexMaster();

	m_vertices.clear();
	m_indices.clear();
	m_isBuilding = false;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetColor(const Rgba& color)
{
	m_stamp.m_color = color;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetUV(const Vector2& uv)
{
	m_stamp.m_uvs = uv;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetNormal(const Vector3& normal)
{
	m_stamp.m_normal = normal;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetTangent(const Vector4& tangent)
{
	m_stamp.m_tangent = tangent;
}


//-----------------------------------------------------------------------------------------------
void MeshBuilder::GenerateFlatNormals()
{
	ASSERT_OR_DIE(m_instruction.m_topology == TOPOLOGY_TRIANGLE_LIST, "Error: MeshBuilder::GenerateFlatNormals() called on builder that isn't using triangles");

	if (m_instruction.m_useIndices)
	{
		ASSERT_OR_DIE((int)m_indices.size() % 3 == 0, "Error: MeshBuilder::GenerateFlatNormals() called with weird number of indices: %i", (int)m_vertices.size());
	}
	else
	{
		ASSERT_OR_DIE((int)m_vertices.size() % 3 == 0, "Error: MeshBuilder::GenerateFlatNormals() called with weird number of vertices: %i", (int)m_vertices.size());
	}

	if (m_instruction.m_useIndices)
	{
		for (int iIndex = 0; iIndex < (int)m_indices.size(); iIndex += 3)
		{
			int iA = m_indices[iIndex];
			int iB = m_indices[iIndex + 1];
			int iC = m_indices[iIndex + 2];

			Vector3 a = m_vertices[iA].m_position;
			Vector3 b = m_vertices[iB].m_position;
			Vector3 c = m_vertices[iC].m_position;

			Vector3 leftSide = b - a;
			Vector3 rightSide = c - a;

			Vector3 normal = CrossProduct(leftSide, rightSide).GetNormalized();

			// Set all to use the same normals
			m_vertices[iA].m_normal = normal;
			m_vertices[iB].m_normal = normal;
			m_vertices[iC].m_normal = normal;
		}
	}
	else
	{
		for (int iVertex = 0; iVertex < (int)m_vertices.size(); iVertex += 3)
		{
			Vector3 a = m_vertices[iVertex].m_position;
			Vector3 b = m_vertices[iVertex + 1].m_position;
			Vector3 c = m_vertices[iVertex + 2].m_position;

			Vector3 leftSide = b - a;
			Vector3 rightSide = c - a;

			Vector3 normal = CrossProduct(leftSide, rightSide).GetNormalized();

			// Set all to use the same normals
			m_vertices[iVertex].m_normal = normal;
			m_vertices[iVertex + 1].m_normal = normal;
			m_vertices[iVertex + 2].m_normal = normal;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetDrawInstruction(const DrawInstruction& instruction)
{
	m_instruction = instruction;
}


//-------------------------------------------------------------------------------------------------
uint32 MeshBuilder::PushVertex(const Vector3& position)
{
	m_stamp.m_position = position;

	m_vertices.push_back(m_stamp);
	return (uint32)m_vertices.size() - 1;
}


//-------------------------------------------------------------------------------------------------
uint32 MeshBuilder::PushVertex(const VertexMaster& master)
{
	m_stamp = master;
	m_vertices.push_back(m_stamp);
	return (uint32)m_vertices.size() - 1;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushIndex(uint32 index)
{
	ASSERT_RECOVERABLE(m_instruction.m_useIndices, "Pushing indices with a non-indexed builder");
	m_indices.push_back(index);
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushTriangle2D(const Vector2& first, const Vector2& second, const Vector2& third, const Rgba& tint /*= Rgba::WHITE*/)
{
	AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST);

	Vector3 first3D		= Vector3(first, 0.f);
	Vector3 second3D	= Vector3(second, 0.f);
	Vector3 third3D		= Vector3(third, 0.f);

	PushTriangle3D(first3D, second3D, third3D, tint);
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushQuad2D(const AABB2& quad, const AABB2& uvs /*= AABB2::ZERO_TO_ONE*/, const Rgba& tint /*= Rgba::WHITE*/)
{
	AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST);

	Vector3 bottomLeft		= Vector3(quad.GetBottomLeft(), 0.f);
	Vector3 topLeft			= Vector3(quad.GetTopLeft(), 0.f);
	Vector3 topRight		= Vector3(quad.GetTopRight(), 0.f);
	Vector3 bottomRight		= Vector3(quad.GetBottomRight(), 0.f);

	PushQuad3D(bottomLeft, topLeft, topRight, bottomRight, uvs, tint);
}


//-------------------------------------------------------------------------------------------------
// Assumes position is in canvas space!
// It's the responsibility of the caller to determine what the final pixel height will be and provide it
uint32 MeshBuilder::PushText(
	const char* text, 
	uint32 pixelHeight, 
	Font* font, 
	const AABB2& textBounds, 
	const Vector2& canvasUnitsPerPixel, 
	const Rgba& color /*= Rgba::WHITE*/, 
	HorizontalAlignment xAlign /*= ALIGNMENT_LEFT*/, 
	VerticalAlignment yAlign /*= ALIGNMENT_TOP*/, 
	TextDrawMode drawMode /*= TEXT_DRAW_OVERRUN*/,
	std::vector<std::vector<AABB2>>* out_glyphBounds /*= nullptr*/)
{
	AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST);

	std::vector<std::string> textLines;
	BreakStringIntoLines(text, textLines);

	return PushText(textLines, pixelHeight, font, textBounds, canvasUnitsPerPixel, color, xAlign, yAlign, drawMode, out_glyphBounds);
}


//-------------------------------------------------------------------------------------------------
uint32 MeshBuilder::PushText(
	const std::vector<std::string>& textLines, 
	uint32 pixelHeight, Font* font, 
	const AABB2& textBounds, 
	const Vector2& canvasUnitsPerPixel, 
	const Rgba& color /*= Rgba::WHITE*/, 
	HorizontalAlignment xAlign /*= ALIGNMENT_LEFT*/, 
	VerticalAlignment yAlign /*= ALIGNMENT_TOP*/, 
	TextDrawMode drawMode /*= TEXT_DRAW_DEFAULT*/,
	std::vector<std::vector<AABB2>>* out_glyphBounds /*= nullptr*/)
{
	AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST);

	std::vector<ColoredText> coloredLines;

	for (size_t lineIndex = 0; lineIndex < textLines.size(); ++lineIndex)
	{
		coloredLines.push_back(ColoredText(textLines[lineIndex], color));
	}

	return PushText(coloredLines, pixelHeight, font, textBounds, canvasUnitsPerPixel, xAlign, yAlign, drawMode, out_glyphBounds);
}


//-------------------------------------------------------------------------------------------------
uint32 MeshBuilder::PushText(
	const std::vector<ColoredText>& textLines, 
	uint32 pixelHeight, Font* font, 
	const AABB2& textBounds, 
	const Vector2& canvasUnitsPerPixel, 
	HorizontalAlignment xAlign /*= ALIGNMENT_LEFT*/, 
	VerticalAlignment yAlign /*= ALIGNMENT_TOP*/, 
	TextDrawMode drawMode /*= TEXT_DRAW_DEFAULT*/,
	std::vector<std::vector<AABB2>>* out_glyphBounds /*= nullptr*/)
{
	AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST);

	switch (drawMode)
	{
	case TEXT_DRAW_DEFAULT:
		return PushText_Default(*this, textLines, pixelHeight, font, textBounds, canvasUnitsPerPixel, xAlign, yAlign, out_glyphBounds);
		break;
	case TEXT_DRAW_SHRINK_TO_FIT:
		return PushText_ShrinkToFit(*this, textLines, pixelHeight, font, textBounds, canvasUnitsPerPixel, xAlign, yAlign, out_glyphBounds);
		break;
	case TEXT_DRAW_EXPAND_TO_FILL:
		return PushText_ExpandToFill(*this, textLines, pixelHeight, font, textBounds, canvasUnitsPerPixel, xAlign, yAlign, out_glyphBounds);
		break;
	case TEXT_DRAW_WORD_WRAP:
		return PushText_WordWrap(*this, textLines, pixelHeight, font, textBounds, canvasUnitsPerPixel, xAlign, yAlign, out_glyphBounds);
		break;
	default:
		ERROR_AND_DIE("Invalid TextDrawMode!");
		break;
	}
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushLine3D(const Vector3& start, const Vector3& end, const Rgba& color /*= Rgba::WHITE*/)
{
	AssertBuildState(true, TOPOLOGY_LINE_LIST);

	SetColor(color);

	uint32 index = PushVertex(start);
	PushVertex(end);

	if (m_instruction.m_useIndices)
	{
		PushIndex(index);
		PushIndex(index + 1);
	}
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushTriangle3D(const Vector3& first, const Vector3& second, const Vector3& third, const Rgba& tint /*= Rgba::WHITE*/)
{
	AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST);

	SetColor(tint);

	uint32 index = PushVertex(first);
	PushVertex(second);
	PushVertex(third);

	if (m_instruction.m_useIndices)
	{
		PushIndex(index + 0);
		PushIndex(index + 1);
		PushIndex(index + 2);
	}
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushQuad3D(const Vector3& bottomLeft, const Vector3& topLeft, const Vector3& topRight, const Vector3& bottomRight, const AABB2& uvs /*= AABB2::ZERO_TO_ONE*/, const Rgba& tint /*= Rgba::WHITE*/)
{
	AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST);

	// Calculate normal
	Vector3 quadRight = (bottomRight - bottomLeft).GetNormalizeSafe(Vector3::X_AXIS);
	Vector3 quadUp = (topRight - bottomRight).GetNormalizeSafe(Vector3::Y_AXIS);
	Vector3 normal = CrossProduct(quadUp, quadRight);

	// Begin adding to the mesh
	SetColor(tint);
	SetNormal(normal);
	SetTangent(Vector4(quadRight, 1.0f));

	if (m_instruction.m_useIndices)
	{
		//-----Push the Vertices-----
		// Bottom Left
		SetUV(uvs.GetBottomLeft());
		uint32 index = PushVertex(bottomLeft);

		// Top Left
		SetUV(uvs.GetTopLeft());
		PushVertex(topLeft);

		// Top Right
		SetUV(uvs.GetTopRight());
		PushVertex(topRight);

		// Bottom Right
		SetUV(uvs.GetBottomRight());
		PushVertex(bottomRight);

		//-----Push the indices-----
		PushIndex(index + 0);
		PushIndex(index + 1);
		PushIndex(index + 2);
		PushIndex(index + 0);
		PushIndex(index + 2);
		PushIndex(index + 3);
	}
	else
	{
		//-----Push the Vertices-----
		// Bottom Left
		SetUV(uvs.GetBottomLeft());
		PushVertex(bottomLeft);

		// Top Left
		SetUV(uvs.GetTopLeft());
		PushVertex(topLeft);

		// Top Right
		SetUV(uvs.GetTopRight());
		PushVertex(topRight);

		// Bottom Left
		SetUV(uvs.GetBottomLeft());
		PushVertex(bottomLeft);

		// Top Right
		SetUV(uvs.GetTopRight());
		PushVertex(topRight);

		// Bottom Right
		SetUV(uvs.GetBottomRight());
		PushVertex(bottomRight);
	}
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushQuad3D(
	const Vector3& position, 
	const Vector2& dimensions, 
	const AABB2& uvs /*= AABB2::ZERO_TO_ONE*/,
	const Rgba& tint /*= Rgba::WHITE*/, 
	const Vector3& rightVector /*= Vector3::X_AXIS*/, 
	const Vector3& upVector /*= Vector3::Y_AXIS*/, 
	const Vector2& pivot /*= Vector2(0.5f, 0.5f)*/)
{
	AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST);

	//-----Set up the vertices-----
	// Find the min and max values for the AABB2 draw bounds
	float minX = -1.0f * (pivot.x * dimensions.x);
	float maxX = minX + dimensions.x;
	float minY = -1.0f * (pivot.y * dimensions.y);
	float maxY = minY + dimensions.y;

	// Determine corner locations based on the alignment vectors
	Vector3 bottomLeft	= position + minX * rightVector + minY * upVector;
	Vector3 bottomRight = position + maxX * rightVector + minY * upVector;
	Vector3 topLeft		= position + minX * rightVector + maxY * upVector;
	Vector3 topRight	= position + maxX * rightVector + maxY * upVector;

	PushQuad3D(bottomLeft, topLeft, topRight, bottomRight, uvs, tint);
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushCube(
	const Vector3& center, 
	const Vector3& dimensions, 
	const AABB2& topUVs /*= AABB2::ZERO_TO_ONE*/, 
	const AABB2& sideUVs /*= AABB2::ZERO_TO_ONE*/, 
	const AABB2& bottomUVs /*= AABB2::ZERO_TO_ONE*/, 
	const Rgba& tint /*= Rgba::WHITE*/)
{
	AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST);

	// Set up the corner vertices
	AABB3 cubeBounds = AABB3(center - dimensions * 0.5f, center + dimensions * 0.5f);

	//-------------------------------------Back face--------------------------------------------------------
	{
		Vector3 position = Vector3(center.x, center.y, cubeBounds.mins.z);
		PushQuad3D(position, Vector2(dimensions.x, dimensions.y), sideUVs, tint, Vector3::X_AXIS, Vector3::Y_AXIS, Vector2(0.5f, 0.5f));
	}

	//-------------------------------------Front face--------------------------------------------------------
	{
		Vector3 position = Vector3(center.x, center.y, cubeBounds.maxs.z);
		PushQuad3D(position, Vector2(dimensions.x, dimensions.y), sideUVs, tint, Vector3::MINUS_X_AXIS, Vector3::Y_AXIS, Vector2(0.5f, 0.5f));
	}

	//-------------------------------------Left face--------------------------------------------------------
	{
		Vector3 position = Vector3(cubeBounds.mins.x, center.y, center.z);
		PushQuad3D(position, Vector2(dimensions.z, dimensions.y), sideUVs, tint, Vector3::MINUS_Z_AXIS, Vector3::Y_AXIS, Vector2(0.5f, 0.5f));
	}

	//-------------------------------------Right face--------------------------------------------------------
	{
		Vector3 position = Vector3(cubeBounds.maxs.x, center.y, center.z);
		PushQuad3D(position, Vector2(dimensions.z, dimensions.y), sideUVs, tint, Vector3::Z_AXIS, Vector3::Y_AXIS, Vector2(0.5f, 0.5f));
	}

	//-------------------------------------Top face--------------------------------------------------------
	{
		Vector3 position = Vector3(center.x, cubeBounds.maxs.y, center.z);
		PushQuad3D(position, Vector2(dimensions.x, dimensions.z), topUVs, tint, Vector3::X_AXIS, Vector3::Z_AXIS, Vector2(0.5f, 0.5f));
	}

	//-------------------------------------Bottom face--------------------------------------------------------
	{
		Vector3 position = Vector3(center.x, cubeBounds.mins.y, center.z);
		PushQuad3D(position, Vector2(dimensions.x, dimensions.z), bottomUVs, tint, Vector3::X_AXIS, Vector3::MINUS_Z_AXIS, Vector2(0.5f, 0.5f));
	}
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushSphere(const Vector3& center, float radius, const Rgba& color /*= Rgba::WHITE*/, int numUSteps /*= 10*/, int numVSteps /*= 10*/)
{
	PushTopHemiSphere(center, radius, color, numUSteps, numVSteps / 2);
	PushBottomHemiSphere(center, radius, color, numUSteps, numVSteps / 2);
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushTopHemiSphere(const Vector3& center, float radius, const Rgba& color /*= Rgba::WHITE*/, int numUSteps /*= 10*/, int numVSteps /*= 10*/, float startV /*= 0.5f*/, float endV /*= 1.0f*/)
{
	bool useIndices = true;
	AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST, &useIndices);

	SetColor(color);

	// Get the number of vertices before we add more
	int initialVertexOffset = (int)m_vertices.size();

	for (int vStep = 0; vStep <= numVSteps; ++vStep) // <= since if we want n gaps, we need n+1 dividers
	{
		for (int uStep = 0; uStep <= numUSteps; ++uStep) // <= since if we want n gaps, we need n+1 dividers
		{
			float u = RangeMapFloat((float)uStep, 0.f, (float)numUSteps, 0.f, 1.f);
			float v = RangeMapFloat((float)vStep, 0.f, (float)numVSteps, startV, endV);

			float rotationAngle = u * 360.f;
			float azimuthAngle = RangeMapFloat(v, startV, endV, 90.f, 0.f);
			Vector3 pos = SphericalToCartesian(radius, rotationAngle, azimuthAngle) + center;

			Vector3 normal = (pos - center).GetNormalized();

			// Tangent - set the tangents at the poles to local RIGHT
			Vector3 tangent = Vector3(-SinDegrees(rotationAngle) * SinDegrees(azimuthAngle), 0.f, CosDegrees(rotationAngle) * SinDegrees(azimuthAngle));
			if (AreMostlyEqual(tangent, Vector3::ZERO))
			{
				tangent = Vector3::X_AXIS;
			}
			tangent.Normalize();

			SetUV(Vector2(u, v));
			SetNormal(normal);
			SetTangent(Vector4(tangent, 1.0f));
			PushVertex(pos);
		}
	}

	// Pushing the indices
	unsigned int numVertsPerV = numUSteps + 1;	// We push a vertex at the end of a slice that coincides with the first vertex of that slice, hence + 1

	for (int vStep = 0; vStep < (int)numVSteps; ++vStep)
	{
		for (int uStep = 0; uStep < (int)numUSteps; ++uStep)
		{
			int bottomLeft = initialVertexOffset + numVertsPerV * vStep + uStep;
			int bottomRight = bottomLeft + 1;
			int topRight = bottomRight + numVertsPerV;
			int topLeft = bottomLeft + numVertsPerV;

			PushIndex(bottomLeft);
			PushIndex(topLeft);
			PushIndex(topRight);

			PushIndex(bottomLeft);
			PushIndex(topRight);
			PushIndex(bottomRight);
		}
	}
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushBottomHemiSphere(const Vector3& center, float radius, const Rgba& color /*= Rgba::WHITE*/, int numUSteps /*= 10*/, int numVSteps /*= 10*/, float startV /*= 0.f*/, float endV /*= 0.5f*/)
{
	bool useIndices = true;
	AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST, &useIndices);

	SetColor(color);

	// Get the number of vertices before we add more
	int initialVertexOffset = (int)m_vertices.size();

	for (int vStep = 0; vStep <= numVSteps; ++vStep) // <= since if we want n gaps, we need n+1 dividers
	{
		for (int uStep = 0; uStep <= numUSteps; ++uStep) // <= since if we want n gaps, we need n+1 dividers
		{
			float u = RangeMapFloat((float)uStep, 0.f, (float)numUSteps, 0.f, 1.0f);
			float v = RangeMapFloat((float)vStep, 0.f, (float)numVSteps, startV, endV);

			float rotationAngle = u * 360.f;
			float azimuthAngle = RangeMapFloat(v, startV, endV, 180.f, 90.f);
			Vector3 pos = SphericalToCartesian(radius, rotationAngle, azimuthAngle) + center;

			Vector3 normal = (pos - center).GetNormalized();

			// Tangent - set the tangents at the poles to local RIGHT
			Vector3 tangent = Vector3(-SinDegrees(rotationAngle) * SinDegrees(azimuthAngle), 0.f, CosDegrees(rotationAngle) * SinDegrees(azimuthAngle));
			if (AreMostlyEqual(tangent, Vector3::ZERO))
			{
				tangent = Vector3::X_AXIS;
			}
			tangent.Normalize();

			SetUV(Vector2(u, v));
			SetNormal(normal);
			SetTangent(Vector4(tangent, 1.0f));
			PushVertex(pos);
		}
	}

	// Pushing the indices
	unsigned int numVertsPerV = numUSteps + 1;	// We push a vertex at the end of a slice that coincides with the first vertex of that slice, hence + 1

	for (int vStep = 0; vStep < (int)numVSteps; ++vStep)
	{
		for (int uStep = 0; uStep < (int)numUSteps; ++uStep)
		{
			int bottomLeft = initialVertexOffset + numVertsPerV * vStep + uStep;
			int bottomRight = bottomLeft + 1;
			int topRight = bottomRight + numVertsPerV;
			int topLeft = bottomLeft + numVertsPerV;

			PushIndex(bottomLeft);
			PushIndex(topLeft);
			PushIndex(topRight);

			PushIndex(bottomLeft);
			PushIndex(topRight);
			PushIndex(bottomRight);
		}
	}
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushTube(const Vector3& bottom, const Vector3& top, float radius, const Rgba& color /*= Rgba::WHITE*/, int numUSteps /*= 10*/, float startV /*= 0.f*/, float endV /*= 1.f*/)
{
	bool useIndices = true;
	AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST, &useIndices);

	Vector3 jVector = (top - bottom).GetNormalized();
	Vector3 reference = AreMostlyEqual(Abs(DotProduct(jVector, Vector3::Z_AXIS)), 1.0f) ? Vector3::MINUS_Y_AXIS : Vector3::Z_AXIS;
	Vector3 iVector = CrossProduct(jVector, reference).GetNormalized();
	Vector3 kVector = CrossProduct(iVector, jVector);

	SetColor(color);

	int vertexOffset = (int)m_vertices.size();

	// Push the vertices and indices for the cylinder
	for (int vStep = 0; vStep <= 1; ++vStep)
	{
		for (int uStep = 0; uStep <= numUSteps; ++uStep)
		{
			float u = RangeMapFloat((float)uStep, 0.f, (float)numUSteps, 0.f, 1.0f);
			float v = RangeMapFloat((float)vStep, 0.f, (float)1.f, startV, endV);

			float rotationAngle = u * 360.f;
			float cosAngle = CosDegrees(rotationAngle);
			float sinAngle = SinDegrees(rotationAngle);

			Vector3 center = (vStep == 0 ? bottom : top);
			Vector3 pos = radius * cosAngle * iVector + radius * sinAngle * kVector + center;
			Vector3 normal = (pos - center).GetNormalized();
			Vector3 tangent = CrossProduct(normal, jVector);

			SetUV(Vector2(u, v));
			SetNormal(normal);
			SetTangent(Vector4(tangent, 1.0f));
			PushVertex(pos);
		}
	}

	// Pushing the indices
	unsigned int numVertsPerV = numUSteps + 1;	// We push a vertex at the end of a slice that coincides with the first vertex of that slice, hence + 1

	for (int uStep = 0; uStep < (int)numUSteps; ++uStep)
	{
		int bottomLeft = vertexOffset + uStep;
		int bottomRight = bottomLeft + 1;
		int topRight = bottomRight + numVertsPerV;
		int topLeft = bottomLeft + numVertsPerV;

		PushIndex(bottomLeft);
		PushIndex(topLeft);
		PushIndex(topRight);

		PushIndex(bottomLeft);
		PushIndex(topRight);
		PushIndex(bottomRight);
	}
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushCapsule(const Vector3& start, const Vector3& end, float radius, const Rgba& color /*= Rgba::WHITE*/)
{
	bool useIndices = true;
	AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST, &useIndices);

	int numUSteps = 10;
	int numVSteps = 10;
	float startV = (1.f / 3.f);
	float endV = (2.f / 3.f);

	PushBottomHemiSphere(start, radius, color, numUSteps, numVSteps / 2, endV, 1.0f);
	PushTube(start, end, radius, color, numUSteps, startV, endV);
	PushTopHemiSphere(end, radius, color, numUSteps, numVSteps / 2, 0.f, startV);
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushPolygon(const Polyhedron& poly, const Rgba& color /*= Rgba::WHITE*/)
{
	bool useIndices = true;
	AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST, &useIndices);
	
	SetColor(color);
	SetUV(Vector2::ZERO);

	// Need to push one face at a time, duplicating verts in order to preserve normals per-face
	int numFaces = poly.GetNumFaces();

	for (int iFace = 0; iFace < numFaces; ++iFace)
	{
		const PolyhedronFace* face = poly.GetFace(iFace);
		int numVertsInFace = (int)face->m_indices.size();
		int vertOffset = (int)m_vertices.size();

		for (int iVertex = 0; iVertex < numVertsInFace; ++iVertex)
		{
			PushVertex(poly.GetVertexPosition(face->m_indices[iVertex]));
		}

		// Push indices to triangulate - make triangles using the first vertex and a set of edge vertices
		for (int iVertex = 1; iVertex < numVertsInFace - 1; ++iVertex)
		{
			PushIndex(vertOffset + 0);
			PushIndex(vertOffset + iVertex);
			PushIndex(vertOffset + iVertex + 1);
		}
	}
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushDisc(const Vector3& center, float radius, const Vector3& normal, const Vector3& tangent, const Rgba& color /*= Rgba::WHITE*/, int numUSteps /*= 10*/, float centerV /*= 0.f*/, float discEdgeV /*= (1.f / 3.f)*/)
{
	bool useIndices = true;
	AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST, &useIndices);

	SetColor(color);

	// Get the number of vertices before we add more
	int initialVertexOffset = (int)m_vertices.size();
	Vector3 bitangent = CrossProduct(tangent, normal);

	for (int uStep = 0; uStep <= numUSteps; ++uStep) // <= since if we want n gaps, we need n+1 dividers
	{
		// UVs
		float uInterval = 1.f / (float)numUSteps;
		float startU = (float)uStep * uInterval;
		float endU = startU + uInterval;
		float centerU = 0.5f * (startU + endU);

		// Position
		float startAngle = startU * 2.f * PI;
		float endAngle = endU * 2.f * PI;

		Vector3 startPos = radius * cosf(startAngle) * tangent + radius * sinf(startAngle) * bitangent + center;
		Vector3 endPos = radius * cosf(endAngle) * tangent + radius * sinf(endAngle) * bitangent + center;

		// Tangent
		Vector3 startTangent = Vector3(-sinf(startAngle), 0.f, cosf(startAngle));
		Vector3 endTangent = Vector3(-sinf(endAngle), 0.f, cosf(endAngle));
		Vector3 centerTangent = (startPos - endPos).GetNormalized();

		// Only push the start at the beginning, as the subsequent steps will use the previous end as their start
		if (uStep == 0)
		{
			SetUV(Vector2(startU, discEdgeV));
			SetNormal(normal);
			SetTangent(Vector4(startTangent, 1.0f));
			PushVertex(startPos);
		}

		// Push center next (clockwise winding)
		// Need to push center each time to have correct UVs for this triangle
		SetUV(Vector2(centerU, centerV));
		SetNormal(normal);
		SetTangent(Vector4(centerTangent, 1.0f));
		PushVertex(center);
	
		// Push end
		SetUV(Vector2(endU, discEdgeV));
		SetNormal(normal);
		SetTangent(Vector4(endTangent, 1.0f));
		PushVertex(endPos);
	}

	// Pushing the indices
	for (int uStep = 0; uStep < (int)numUSteps; ++uStep)
	{
		int offsetToTriangle = initialVertexOffset + uStep * 2;

		PushIndex(offsetToTriangle);
		PushIndex(offsetToTriangle + 1);
		PushIndex(offsetToTriangle + 2);
	}
}

//-------------------------------------------------------------------------------------------------
// Upright cylinder
void MeshBuilder::PushCylinder(const Vector3& bottom, const Vector3& top, float radius, const Rgba& color /*= Rgba::WHITE*/, int numUSteps /*= 10*/)
{
	bool useIndices = true;
	AssertBuildState(true, TOPOLOGY_TRIANGLE_LIST, &useIndices);

	float tubeBottomV = (1.f / 3.f);
	float tubeTopV = (2.f / 3.f);
	Vector3 jVector = (top - bottom).GetNormalized();
	Vector3 reference = AreMostlyEqual(Abs(DotProduct(jVector, Vector3::Z_AXIS)), 1.0f) ? Vector3::MINUS_Y_AXIS : Vector3::Z_AXIS;
	Vector3 iVector = CrossProduct(jVector, reference).GetNormalized();

	PushDisc(bottom, radius, -1.f * jVector, iVector, color, numUSteps, 0.f, tubeBottomV);
	PushTube(bottom, top, radius, color, numUSteps, tubeBottomV, tubeTopV);
	PushDisc(top, radius, jVector, iVector, color, numUSteps, tubeTopV, 1.0f);
}


//-------------------------------------------------------------------------------------------------
// Asserts that the MeshBuilder is in the current state to avoid misuse/bad cases
void MeshBuilder::AssertBuildState(bool shouldBeBuilding, MeshTopology topology, bool* usingIndices /*= false*/)
{
	ASSERT_OR_DIE(m_isBuilding == shouldBeBuilding, "MeshBuilder fails building check!");
	ASSERT_OR_DIE(m_instruction.m_topology == topology, "MeshBuilder fails topology check!");
	if (usingIndices != nullptr)
	{
		ASSERT_OR_DIE(m_instruction.m_useIndices == *usingIndices, "MeshBuilder fails index check!");
	}
}
