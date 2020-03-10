///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 30th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** INCLUDES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/File.h"
#include "Engine/Framework/EngineCommon.h"
#include <stdio.h>
#include <cstdlib>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>


///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** DEFINES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                              *** TYPES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** STRUCTS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                        *** GLOBALS AND STATICS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                           *** C FUNCTIONS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
FILE* OpenFile(const char* filepath, const char* flags)
{
	FILE *fp = nullptr;
	fopen_s(&fp, filepath, flags);
	if (fp == nullptr) {
		return nullptr;
	}

	fseek(fp, 0L, SEEK_SET);
	return fp;
}


//-------------------------------------------------------------------------------------------------
bool CloseFile(FILE* fileHandle)
{
	// Don't close a nullptr
	if (fileHandle == nullptr)
	{
		return true;
	}

	int err = fclose(fileHandle);
	if (err != 0)
	{
		ERROR_RECOVERABLE("Warning: CloseFile could not close the file.");
		return false;
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
void* FileReadToNewBuffer(char const *filename, size_t& out_size)
{
	FILE* fp = OpenFile(filename, "r");
	if (fp == nullptr)
	{
		return nullptr;
	}

	out_size = 0U;

	fseek(fp, 0L, SEEK_END);
	out_size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);

	unsigned char *buffer = (unsigned char*)malloc(out_size + 1U); // space for NULL 

	size_t read = fread(buffer, 1, out_size, fp);

	CloseFile(fp);

	buffer[read] = NULL;
	return buffer;
}


//-------------------------------------------------------------------------------------------------
bool FileWriteFromBuffer(char const *filename, char const* buffer, int bufferSize)
{
	FILE *fp = nullptr;
	fopen_s(&fp, filename, "w+");
	if (fp == nullptr)
	{
		return false;
	}

	fwrite(buffer, sizeof(char), bufferSize, fp);

	return CloseFile(fp);
}


//-------------------------------------------------------------------------------------------------
std::string GetWorkingDirectory()
{
	char buffer[MAX_PATH + 1];

	GetCurrentDirectoryA(MAX_PATH, buffer);
	buffer[MAX_PATH] = '\0';

	return std::string(buffer);
}


//-------------------------------------------------------------------------------------------------
std::string GetFullFilePath(const std::string& localFilePath)
{
	return GetWorkingDirectory() + "\\" + localFilePath;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** CLASSES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
File::~File()
{
	Close();
}


//-------------------------------------------------------------------------------------------------
bool File::Open(const char* filepath, const char* flags)
{
	if (m_filePointer != nullptr)
	{
		Close();
	}

	m_filePointer = (void*)OpenFile(filepath, flags);

	if (m_filePointer != nullptr)
	{
		m_filePathOpened = filepath;
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool File::Close()
{
	bool success = CloseFile((FILE*)m_filePointer);
	m_filePointer = nullptr;

	if (m_data != nullptr)
	{
		free((void*)m_data);
		m_data = nullptr;
	}

	// Reset members
	m_size = 0;
	m_offset = 0;
	m_isAtEndOfFile = false;
	m_lineNumber = 0;

	return success;
}


//-------------------------------------------------------------------------------------------------
void File::Write(const char* buffer, size_t length)
{
	fwrite(buffer, sizeof(char), length, (FILE*)m_filePointer);
}


//-------------------------------------------------------------------------------------------------
void File::Flush()
{
	fflush((FILE*)m_filePointer);
}


//-------------------------------------------------------------------------------------------------
bool File::LoadFileToMemory()
{
	m_size = 0U;

	FILE* fp = (FILE*)m_filePointer;

	// Get the file size
	fseek(fp, 0L, SEEK_END);
	m_size = ftell(fp);

	// Set back to beginning
	fseek(fp, 0L, SEEK_SET);

	// Make the buffer
	unsigned char* data = (unsigned char*)malloc(m_size + 1U); // space for NULL 

	// Read the data in
	size_t read = fread(data, 1, m_size, fp);

	// Null terminate and return
	data[read] = NULL;
	m_data = (const char*)data;

	return true;
}


//-------------------------------------------------------------------------------------------------
unsigned int File::GetNextLine(std::string& out_string)
{
	if (m_data == nullptr || m_offset >= m_size || m_data[m_offset] == NULL)
	{
		out_string = "";
		m_isAtEndOfFile = true;
		return m_lineNumber;
	}

	unsigned int endIndex = m_offset;
	while (m_data[endIndex] != '\n' && endIndex < m_size)
	{
		endIndex++;
	}

	size_t length = (endIndex - m_offset);
	out_string = std::string(&m_data[m_offset], length);

	m_offset = endIndex + 1;
	m_lineNumber++;
	m_isAtEndOfFile = (m_offset >= m_size);

	return m_lineNumber;
}


//-------------------------------------------------------------------------------------------------
void File::ResetMemoryReadHead()
{
	m_offset = 0;
	m_isAtEndOfFile = false;
}
