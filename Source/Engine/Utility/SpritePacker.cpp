///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 18th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/IO/Image.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/Utility/SpritePacker.h"

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
void SpritePacker::Initialize(uint32 texelWidth, uint32 texelHeigth)
{
	m_image = new Image();
	m_image->Initialize(IntVector2(texelWidth, texelHeigth), Rgba(0, 0, 0, 0));

	m_texture = new Texture2D();
	m_texture->CreateFromImage(*m_image);
}


//-------------------------------------------------------------------------------------------------
void SpritePacker::Initialize(Texture2D* texture)
{
	m_image = new Image();
	m_image->Initialize(IntVector2(texture->GetWidth(), texture->GetHeight()), Rgba(0, 0, 0, 0));

	m_texture = texture;
}


//-------------------------------------------------------------------------------------------------
bool SpritePacker::PackSprite(const uint8* src, int spriteWidth, int spriteHeight, int srcComponentCount, AABB2& out_uvs)
{
	// Fixed size for now
	int verticalSpace = m_image->GetTexelHeight() - m_writePosition.y;
	ASSERT_RECOVERABLE(verticalSpace > 0, "SpritePacker ran out of vertical space!");
	if (verticalSpace <= 0)
	{
		return false;
	}

	// See if the sprite will fit this line
	int horizontalSpace = m_image->GetTexelWidth() - m_writePosition.x;

	if (horizontalSpace < spriteWidth)
	{
		MoveHeadToNextLine();
	}

	BlitSpriteToImage(src, spriteWidth, spriteHeight, srcComponentCount);

	// Save the UVs for the sprite
	out_uvs = CalculateUVsForSprite(spriteWidth, spriteHeight);

	// Move write head across for the next sprite
	m_writePosition.x += spriteWidth;

	// Save off tallest sprite this line so we know where to move when starting the next line
	m_maxHeightThisLine = Max(m_maxHeightThisLine, spriteHeight);

	m_imageDirty = true;
	return true;
}


//-------------------------------------------------------------------------------------------------
void SpritePacker::UpdateTexture()
{
	if (m_imageDirty)
	{
		m_texture->CreateFromImage(*m_image);
		m_imageDirty = false;
	}
}


//-------------------------------------------------------------------------------------------------
Texture2D* SpritePacker::GetTexture()
{
	UpdateTexture();
	return m_texture;
}


//-------------------------------------------------------------------------------------------------
void SpritePacker::MoveHeadToNextLine()
{
	m_writePosition.x = 0;
	m_writePosition.y += m_maxHeightThisLine;
	m_maxHeightThisLine = 0;
}


//-------------------------------------------------------------------------------------------------
void SpritePacker::BlitSpriteToImage(const uint8* src, int spriteWidth, int spriteHeight, int srcComponentCount)
{
	for (int yTexelIndex = 0; yTexelIndex < spriteHeight; ++yTexelIndex)
	{
		for (int xTexelIndex = 0; xTexelIndex < spriteWidth; ++xTexelIndex)
		{
			const uint8* currSpriteTexel = src + srcComponentCount * (spriteWidth * yTexelIndex + xTexelIndex);

			Rgba texelColor = Rgba::BLACK;

			for (int componentIndex = 0; componentIndex < srcComponentCount; ++componentIndex)
			{
				texelColor.data[componentIndex] = currSpriteTexel[componentIndex];
			}

			m_image->SetTexelColor(m_writePosition.x + xTexelIndex, m_writePosition.y + yTexelIndex, texelColor);
		}
	}
}


//-------------------------------------------------------------------------------------------------
AABB2 SpritePacker::CalculateUVsForSprite(int spriteWidth, int spriteHeight)
{
	float imageWidth = static_cast<float>(m_image->GetTexelWidth());
	float imageHeight = static_cast<float>(m_image->GetTexelHeight());

	float startU = static_cast<float>(m_writePosition.x) / imageWidth;
	float startV = static_cast<float>(m_writePosition.y) / imageHeight;

	float endU = static_cast<float>(m_writePosition.x + spriteWidth) / imageWidth;
	float endV = static_cast<float>(m_writePosition.y + spriteHeight) / imageWidth;

	return AABB2(startU, startV, endU, endV);
}
