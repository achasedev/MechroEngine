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
#include "Engine/Render/Mesh/MeshBuilder.h"
#include "Engine/Voxel/QEFLoader.h"

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
		colors[i] = StringToRgba(colorText);
	}

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

		Vector3 position;

		position.x = StringToFloat(voxelTokens[0]);
		position.y = StringToFloat(voxelTokens[1]);
		position.z = StringToFloat(voxelTokens[2]);
		
		int colorIndex = StringToInt(voxelTokens[3]);

		// Check the visibility mask to determine which faces to push
		int visibility = StringToInt(voxelTokens[4]);

		// Right
		if ((visibility & 2) == 2)
		{
			mb.PushQuad3D(position + Vector3(1.f, 0.f, 0.f), Vector2::ONES, AABB2::ZERO_TO_ONE, colors[colorIndex], Vector3::Z_AXIS, Vector3::Y_AXIS, Vector2::ZERO);
		}

		// Left
		if ((visibility & 4) == 4)
		{
			mb.PushQuad3D(position + Vector3(0.f, 0.f, 1.f), Vector2::ONES, AABB2::ZERO_TO_ONE, colors[colorIndex], Vector3::MINUS_Z_AXIS, Vector3::Y_AXIS, Vector2::ZERO);
		}

		// Top
		if ((visibility & 8) == 8)
		{
			mb.PushQuad3D(position + Vector3(0.f, 1.f, 0.f), Vector2::ONES, AABB2::ZERO_TO_ONE, colors[colorIndex], Vector3::X_AXIS, Vector3::Z_AXIS, Vector2::ZERO);
		}

		// Bottom
		if ((visibility & 16) == 16)
		{
			mb.PushQuad3D(position + Vector3(0.f, 0.f, 1.f), Vector2::ONES, AABB2::ZERO_TO_ONE, colors[colorIndex], Vector3::X_AXIS, Vector3::MINUS_Z_AXIS, Vector2::ZERO);
		}

		// Front
		if ((visibility & 32) == 32)
		{
			mb.PushQuad3D(position + Vector3(1.f, 0.f, 1.f), Vector2::ONES, AABB2::ZERO_TO_ONE, colors[colorIndex], Vector3::MINUS_X_AXIS, Vector3::Y_AXIS, Vector2::ZERO);
		}

		// Back
		if ((visibility & 64) == 64)
		{
			mb.PushQuad3D(position, Vector2::ONES, AABB2::ZERO_TO_ONE, colors[colorIndex], Vector3::X_AXIS, Vector3::Y_AXIS, Vector2::ZERO);
		}

		m_file->GetNextLine(currLine);
	}

	mb.FinishBuilding();
	Mesh* mesh = mb.CreateMesh<Vertex3D_PCU>();
	mb.Clear();

	return mesh;
}


//-------------------------------------------------------------------------------------------------
void QEFLoader::Clear()
{
	SAFE_DELETE_POINTER(m_file);
}
