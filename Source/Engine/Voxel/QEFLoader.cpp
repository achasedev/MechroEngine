///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: September 4th, 2020
/// Description: Implementation of the QEFLoader class
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/Rgba.h"
#include "Engine/IO/File.h"
#include "Engine/Math/IntVector2.h"
#include "Engine/Math/IntVector3.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Render/Mesh/MeshBuilder.h"
#include "Engine/Voxel/QEFLoader.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#define VOXEL_SIZE 1.0f

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

enum LayerDirection
{
	LAYER_RIGHT,
	LAYER_LEFT,
	LAYER_TOP,
	LAYER_BOTTOM,
	LAYER_FRONT,
	LAYER_BACK
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
IntVector3 GetVoxelCoordsFromCoverMapCoords(const IntVector2& coverMapCoords, const IntVector3& startVoxelCoords, LayerDirection direction)
{
	IntVector3 voxelCoords = startVoxelCoords;

	switch (direction)
	{
	case LAYER_RIGHT:	voxelCoords += IntVector3(0, coverMapCoords.y, coverMapCoords.x); break;
	case LAYER_LEFT:	voxelCoords += IntVector3(0, coverMapCoords.y, -coverMapCoords.x); break;
	case LAYER_TOP:		voxelCoords += IntVector3(coverMapCoords.x, 0, coverMapCoords.y); break;
	case LAYER_BOTTOM:	voxelCoords += IntVector3(coverMapCoords.x, 0, -coverMapCoords.y); break;
	case LAYER_FRONT:	voxelCoords += IntVector3(-coverMapCoords.x, coverMapCoords.y, 0); break;
	case LAYER_BACK:	voxelCoords += IntVector3(coverMapCoords.x, coverMapCoords.y, 0); break;
	default:
		ERROR_RECOVERABLE("Bad direction?");
		break;
	}

	return voxelCoords;
}


//-------------------------------------------------------------------------------------------------
int GetVoxelIndexFromVoxelCoords(const IntVector3& coords, const IntVector3& dimensions)
{
	int voxelsPerY = dimensions.x * dimensions.z;
	return (voxelsPerY * coords.y) + (dimensions.x * coords.z) + coords.x;
}


//-------------------------------------------------------------------------------------------------
bool IsFaceVisibleForDirection(uint8 visibility, LayerDirection direction)
{
	switch (direction)
	{
	case LAYER_RIGHT:	return (visibility & 2) == 2; break;
	case LAYER_LEFT:	return (visibility & 4) == 4; break;
	case LAYER_TOP:		return (visibility & 8) == 8; break;
	case LAYER_BOTTOM:	return (visibility & 16) == 16; break;
	case LAYER_FRONT:	return (visibility & 32) == 32; break;
	case LAYER_BACK:	return (visibility & 64) == 64; break;
	default:
		ERROR_RETURN(false, "Bad direction?");
		break;
	}
}


//-------------------------------------------------------------------------------------------------
uint8* Create2DCoverMap(const IntVector3& startVoxelCoords, LayerDirection direction, const uint8* visibilityMasks, const IntVector3& dimensions, IntVector2& out_coverMapDimensions)
{
	switch (direction)
	{
	case LAYER_RIGHT:
	case LAYER_LEFT:
		out_coverMapDimensions = IntVector2(dimensions.z, dimensions.y);
		break;
	case LAYER_TOP:
	case LAYER_BOTTOM:
		out_coverMapDimensions = IntVector2(dimensions.x, dimensions.z);
		break;
	case LAYER_FRONT:
	case LAYER_BACK:
		out_coverMapDimensions = IntVector2(dimensions.x, dimensions.y);
		break;
	default:
		ERROR_AND_DIE("Bad direction?");
		break;
	}

	int numVoxels = out_coverMapDimensions.x * out_coverMapDimensions.y;
	uint8* coverMap = (uint8*)malloc(numVoxels);
	memset(coverMap, 0xFFFFFFFF, numVoxels);

	bool visibleFound = false;
	for (int coverY = 0; coverY < out_coverMapDimensions.y; ++coverY)
	{
		for (int coverX = 0; coverX < out_coverMapDimensions.x; ++coverX)
		{
			IntVector2 coverCoords = IntVector2(coverX, coverY);
			IntVector3 voxelCoords = GetVoxelCoordsFromCoverMapCoords(coverCoords, startVoxelCoords, direction);
			int voxelIndex = GetVoxelIndexFromVoxelCoords(voxelCoords, dimensions);

			if (IsFaceVisibleForDirection(visibilityMasks[voxelIndex], direction))
			{
				int coverIndex = out_coverMapDimensions.x * coverY + coverX;
				coverMap[coverIndex] = 0U;
				visibleFound = true;
			}
		}
	}

	// Optimization - if there's literally nothing visible here, then just move on
	if (!visibleFound)
	{
		SAFE_FREE(coverMap);
		return nullptr;
	}

	return coverMap;
}


//-------------------------------------------------------------------------------------------------
void PushQuad(const IntVector3& quadMinCoords, const IntVector3& quadMaxCoords, LayerDirection direction, const Rgba& color, MeshBuilder& mb)
{
	Vector3 bottomLeft(quadMinCoords);
	Vector3 topRight(quadMaxCoords);

	Vector3 topLeft, bottomRight;
	switch (direction)
	{
	case LAYER_RIGHT:
		bottomLeft += Vector3(VOXEL_SIZE, 0.f, 0.f);
		topRight += Vector3(VOXEL_SIZE);

		topLeft = Vector3(bottomLeft.x, topRight.y, bottomLeft.z);
		bottomRight = Vector3(topRight.x, bottomLeft.y, topRight.z);
		break;
	case LAYER_LEFT:
		bottomLeft += Vector3(0.f, 0.f, VOXEL_SIZE);
		topRight += Vector3(0.f, VOXEL_SIZE, 0.f);

		topLeft = Vector3(bottomLeft.x, topRight.y, bottomLeft.z);
		bottomRight = Vector3(topRight.x, bottomLeft.y, topRight.z);
		break;
	case LAYER_TOP:
		bottomLeft += Vector3(0.f, VOXEL_SIZE, 0.f);
		topRight += Vector3(VOXEL_SIZE);

		topLeft = Vector3(bottomLeft.x, bottomLeft.y, topRight.z);
		bottomRight = Vector3(topRight.x, topRight.y, bottomLeft.z);
		break;
	case LAYER_BOTTOM:
		bottomLeft += Vector3(0.f, 0.f, VOXEL_SIZE);
		topRight += Vector3(VOXEL_SIZE, 0.f, 0.f);

		topLeft = Vector3(bottomLeft.x, topRight.y, topRight.z);
		bottomRight = Vector3(topRight.x, bottomLeft.y, bottomLeft.z);
		break;
	case LAYER_FRONT:
		bottomLeft += Vector3(VOXEL_SIZE, 0.f, VOXEL_SIZE);
		topRight += Vector3(0.f, VOXEL_SIZE, VOXEL_SIZE);

		topLeft = Vector3(bottomLeft.x, topRight.y, bottomLeft.z);
		bottomRight = Vector3(topRight.x, bottomLeft.y, topRight.z);
		break;
	case LAYER_BACK:
		topRight += Vector3(VOXEL_SIZE, VOXEL_SIZE, 0.f);

		topLeft = Vector3(bottomLeft.x, topRight.y, bottomLeft.z);
		bottomRight = Vector3(topRight.x, bottomLeft.y, topRight.z);
		break;
	default:
		ERROR_RETURN(, "Bad direction?");
		break;
	}

	mb.PushQuad3D(bottomLeft, topLeft, topRight, bottomRight, AABB2::ZERO_TO_ONE, color);
}


//-------------------------------------------------------------------------------------------------
void PushQuadsForLayer(const IntVector3& startVoxelCoords, LayerDirection direction, const uint8* visibilityMasks, const Rgba* colors, const uint8* colorIndices, const IntVector3& dimensions, MeshBuilder& mb)
{
	// Get our 2D array to track voxel faces covered
	IntVector2 coverMapDimensions;
	uint8* coverMap = Create2DCoverMap(startVoxelCoords, direction, visibilityMasks, dimensions, coverMapDimensions);

	// If the cover map is nullptr, there's nothing visible in this layer for this direction
	if (coverMap == nullptr)
	{
		return;
	}

	// Iterate across these faces
	for (int coverY = 0; coverY < coverMapDimensions.y; ++coverY)
	{
		for (int coverX = 0; coverX < coverMapDimensions.x; ++coverX)
		{
			// If the current face is already covered, don't bother
			// If a face isn't even visible, it's already considered covered
			if (coverMap[coverY * coverMapDimensions.x + coverX] > 0)
			{
				continue;
			}

			IntVector3 voxelCoords = GetVoxelCoordsFromCoverMapCoords(IntVector2(coverX, coverY), startVoxelCoords, direction);
			uint32 voxelIndex = GetVoxelIndexFromVoxelCoords(voxelCoords, dimensions);
			uint8 colorIndex = colorIndices[voxelIndex];
			
			// We can start a run!
			// Starting from this X, find the right quad width that maximizes area
			// Need to check for what the max height would be for each chosen width

			IntVector2 bestMaxCoordsInclusive(-1, -1);
			int lowestYThisRun = 9999999;
			int bestArea = -1;
			for (int runX = coverX; runX < coverMapDimensions.x; ++runX)
			{
				// Don't check the first voxel, we know it's already good
				if (runX != coverX)
				{
					// If it is already covered, we're done
					// (We should never meet an already covered visible quad?? So we
					// only hit this if the face isn't visible)
					if (coverMap[coverY * coverMapDimensions.x + runX] > 0)
					{
						break;
					}

					// The adjacent face also needs to be the same color
					IntVector3 runVoxelCoords = GetVoxelCoordsFromCoverMapCoords(IntVector2(runX, coverY), startVoxelCoords, direction);
					uint32 runVoxelIndex = GetVoxelIndexFromVoxelCoords(runVoxelCoords, dimensions);
					uint8 runColorIndex = colorIndices[runVoxelIndex];

					if (runColorIndex != colorIndex)
					{
						break;
					}
				}

				// Find height now
				int maxYForThisX = coverY;
				for (int runY = coverY; runY < coverMapDimensions.y; ++runY)
				{
					// If it is already covered, we're done
					// (We should never meet an already covered visible quad?? So we
					// only hit this if the face isn't visible)
					if (coverMap[runY * coverMapDimensions.x + runX] > 0)
					{
						break;
					}

					// Check color again
					IntVector3 yVoxelCoords = GetVoxelCoordsFromCoverMapCoords(IntVector2(runX, runY), startVoxelCoords, direction);
					uint32 runYVoxelIndex = GetVoxelIndexFromVoxelCoords(yVoxelCoords, dimensions);
					uint8 runYColorIndex = colorIndices[runYVoxelIndex];

					if (runYColorIndex != colorIndex)
					{
						break;
					}

					maxYForThisX = runY;
				}

				// This Y can't be greater than the lowerst Y so far
				maxYForThisX = Min(maxYForThisX, lowestYThisRun);			
				lowestYThisRun = maxYForThisX;

				// Now update our best Y/area if this is better
				int currentArea = (runX - coverX + 1) * (maxYForThisX - coverY + 1);
				if (currentArea > bestArea)
				{
					bestArea = currentArea;
					bestMaxCoordsInclusive = IntVector2(runX, maxYForThisX);
				}
			}

			// Push a quad for this
			PushQuad(voxelCoords, GetVoxelCoordsFromCoverMapCoords(bestMaxCoordsInclusive, startVoxelCoords, direction), direction, colors[colorIndex], mb);

			// Mark these faces as covered
			for (int runX = coverX; runX <= bestMaxCoordsInclusive.x; ++runX)
			{
				for (int runY = coverY; runY <= bestMaxCoordsInclusive.y; ++runY)
				{
					coverMap[runY * coverMapDimensions.x + runX] = 1U;
				}
			}

			// Update our moving cover coords to the next starting point 
			coverX = bestMaxCoordsInclusive.x;
		}
	}

	SAFE_FREE(coverMap);
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
bool QEFLoader::LoadFile(const char* filepath)
{
	if (m_file != nullptr)
	{
		Clear();
	}

	ASSERT_RECOVERABLE(GetFileExtension(filepath) == ".qef", "Path is not to a .qef file?");

	m_file = new File();
	bool success = m_file->Open(filepath, "r");

	if (!success)
	{
		Clear();
		return false;
	}

	m_file->LoadFileToMemory();
	return true;
}


//-------------------------------------------------------------------------------------------------
Mesh* QEFLoader::CreateMesh()
{
	ASSERT_RETURN(m_file != nullptr, nullptr, "QEFLoader can't make a mesh without loading a file first!");

	std::string currLine;

	// Qubicle Exchange Format
	m_file->GetNextLine(currLine);
	ASSERT_RECOVERABLE(currLine == "Qubicle Exchange Format", "Bad QEF header?");

	// Version <VERSION NUMBER>
	m_file->GetNextLine(currLine);

	// www.minddesk.com
	m_file->GetNextLine(currLine);
	
	// Next line is the XYZ dimensions of the model (size of its bounding box)
	std::string dimensionsText;
	m_file->GetNextLine(dimensionsText);
	IntVector3 dimensions = StringToIntVector3(dimensionsText);

	// Next line is the number of unique colors used in the model
	std::string colorCountText;
	m_file->GetNextLine(colorCountText);
	int numColors = StringToInt(colorCountText);

	// Next numColor lines are the colors
	Rgba* colors = (Rgba*)malloc(sizeof(Rgba) * numColors);

	std::string colorText;
	for (int i = 0; i < numColors; ++i)
	{
		m_file->GetNextLine(colorText);
		StringToRgba(colorText, colors[i]);
	}

	uint32 maxVoxels = dimensions.x * dimensions.y * dimensions.z;
	uint8* visibilityMasks = (uint8*)malloc(maxVoxels);
	uint8* colorIndices = (uint8*)malloc(maxVoxels);
	memset(visibilityMasks, 0, maxVoxels);
	memset(colorIndices, 0, maxVoxels);

	// Now get all the voxels and build a mesh
	MeshBuilder mb;
	mb.BeginBuilding(true);

	m_file->GetNextLine(currLine);
	while (!m_file->IsAtEndOfFile())
	{
		ASSERT_RECOVERABLE(currLine.size() > 0, "Empty string?");
		
		std::vector<std::string> voxelTokens;
		Tokenize(currLine, ' ', voxelTokens);

		// Should always have x, y, z, color index, visibility mask
		ASSERT_OR_DIE(voxelTokens.size() == 5, "Invalid voxel line!");

		IntVector3 position;

		position.x = StringToInt(voxelTokens[0]);
		position.y = StringToInt(voxelTokens[1]);
		position.z = StringToInt(voxelTokens[2]);
		
		uint8 colorIndex = (uint8)StringToInt(voxelTokens[3]);
		colorIndices[dimensions.x * dimensions.z * position.y + dimensions.x * position.z + position.x] = colorIndex;

		// Check the visibility mask to determine which faces to push
		uint8 visibility = (uint8) StringToInt(voxelTokens[4]);
		visibilityMasks[dimensions.x * dimensions.z * position.y + dimensions.x * position.z + position.x] = visibility;

		// Right
		//if ((visibility & 2) == 2)
		//{
		//	mb.PushQuad3D(position + Vector3(1.f, 0.f, 0.f), Vector2::ONES, AABB2::ZERO_TO_ONE, colors[colorIndex], Vector3::Z_AXIS, Vector3::Y_AXIS, Vector2::ZERO);
		//}

		//// Left
		//if ((visibility & 4) == 4)
		//{
		//	mb.PushQuad3D(position + Vector3(0.f, 0.f, 1.f), Vector2::ONES, AABB2::ZERO_TO_ONE, colors[colorIndex], Vector3::MINUS_Z_AXIS, Vector3::Y_AXIS, Vector2::ZERO);
		//}

		//// Top
		//if ((visibility & 8) == 8)
		//{
		//	mb.PushQuad3D(position + Vector3(0.f, 1.f, 0.f), Vector2::ONES, AABB2::ZERO_TO_ONE, colors[colorIndex], Vector3::X_AXIS, Vector3::Z_AXIS, Vector2::ZERO);
		//}

		//// Bottom
		//if ((visibility & 16) == 16)
		//{
		//	mb.PushQuad3D(position + Vector3(0.f, 0.f, 1.f), Vector2::ONES, AABB2::ZERO_TO_ONE, colors[colorIndex], Vector3::X_AXIS, Vector3::MINUS_Z_AXIS, Vector2::ZERO);
		//}

		//// Front
		//if ((visibility & 32) == 32)
		//{
		//	mb.PushQuad3D(position + Vector3(1.f, 0.f, 1.f), Vector2::ONES, AABB2::ZERO_TO_ONE, colors[colorIndex], Vector3::MINUS_X_AXIS, Vector3::Y_AXIS, Vector2::ZERO);
		//}

		//// Back
		//if ((visibility & 64) == 64)
		//{
		//	mb.PushQuad3D(position, Vector2::ONES, AABB2::ZERO_TO_ONE, colors[colorIndex], Vector3::X_AXIS, Vector3::Y_AXIS, Vector2::ZERO);
		//}

		m_file->GetNextLine(currLine);
	}

	// Build the mesh for each layer of each axis, in both directions along the axis
	
	// Right faces
	for (int i = 0; i < dimensions.x; ++i)
	{
		PushQuadsForLayer(IntVector3(i, 0, 0), LAYER_RIGHT, visibilityMasks, colors, colorIndices, dimensions, mb);
	}

	// Left faces
	for (int i = 0; i < dimensions.x; ++i)
	{
		PushQuadsForLayer(IntVector3(i, 0, dimensions.z - 1), LAYER_LEFT, visibilityMasks, colors, colorIndices, dimensions, mb);
	}

	// Top faces
	for (int i = 0; i < dimensions.y; ++i)
	{
		PushQuadsForLayer(IntVector3(0, i, 0), LAYER_TOP, visibilityMasks, colors, colorIndices, dimensions, mb);
	}

	// Bottom faces
	for (int i = 0; i < dimensions.y; ++i)
	{
		PushQuadsForLayer(IntVector3(0, i, dimensions.z - 1), LAYER_BOTTOM, visibilityMasks, colors, colorIndices, dimensions, mb);
	}

	// Front faces
	for (int i = 0; i < dimensions.z; ++i)
	{
		PushQuadsForLayer(IntVector3(dimensions.x - 1, 0, i), LAYER_FRONT, visibilityMasks, colors, colorIndices, dimensions, mb);
	}

	// Back faces
	for (int i = 0; i < dimensions.z; ++i)
	{
		PushQuadsForLayer(IntVector3(0, 0, i), LAYER_BACK, visibilityMasks, colors, colorIndices, dimensions, mb);
	}

	SAFE_FREE(colors);
	SAFE_FREE(colorIndices);
	SAFE_FREE(visibilityMasks);

	mb.FinishBuilding();
	Mesh* mesh = mb.CreateMesh<Vertex3D_PCU>();
	mb.Clear();

	return mesh;
}


//-------------------------------------------------------------------------------------------------
void QEFLoader::Clear()
{
	SAFE_DELETE(m_file);
}
