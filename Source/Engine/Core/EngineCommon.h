///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 29th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Utility/Assert.h"
#include "Engine/Utility/StringUtils.h"
#include <float.h>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x);
#define INLINE
#define STATIC
#define SAFE_DELETE(p)  if (p != nullptr) { delete p; p = nullptr; }
#define SAFE_FREE(p)  if (p != nullptr) { free(p); p = nullptr; }

#define SAFE_DELETE_VECTOR(v)															\
for (size_t safe_delete_index = 0; safe_delete_index < v.size(); ++safe_delete_index)	\
{																						\
	SAFE_DELETE(v[safe_delete_index]);													\
}																						\
v.clear();																				\

#define SAFE_FREE_VECTOR(v)														\
for (size_t safe_free_index = 0; safe_free_index < v.size(); ++safe_free_index)	\
{																				\
	SAFE_FREE(v[safe_free_index]);												\
}																				\
v.clear();																		\


#define BIT_FLAG(x) (1 << x)
#define _QUOTE(x) # x
#define QUOTE(x) _QUOTE(x)
#define UNIMPLEMENTED()  QUOTE(__FILE__) " (" QUOTE(__LINE__) ")" ; ERROR_AND_DIE("Function unimplemented!") 
#define _CAT(A,B) A##B
#define CAT(A,B) _CAT(A,B)
#define NO_RETURN_VAL

#define RTTI_BASE_CLASS(CLASS)													\
static const int s_type;														\
virtual const void* GetType() const { return &s_type; }							\
virtual const char* GetTypeAsString() const { return QUOTE(CLASS); }			\
																				\
template <typename TYPE>														\
bool IsOfType() const															\
{																				\
	return (GetType() == TYPE::GetTypeStatic());								\
}																				\
																				\
template <typename TYPE>														\
TYPE* GetAsType()																\
{																				\
	if (IsOfType<TYPE>())														\
	{																			\
		return reinterpret_cast<TYPE*>(this);									\
	}																			\
																				\
	ASSERT_RECOVERABLE(false, "GetAsType() failed!");							\
	return nullptr;																\
}																				\
																				\
template <typename TYPE>														\
const TYPE* GetAsType() const													\
{																				\
	if (IsOfType<TYPE>())														\
	{																			\
		return reinterpret_cast<const TYPE*>(this);								\
	}																			\
																				\
	ASSERT_RECOVERABLE(false, "GetAsType() failed!");							\
	return nullptr;																\
} 																				\
																				\

#define RTTI_DERIVED_CLASS(CLASS)												\
static const int s_type;														\
static const void* GetTypeStatic() { return &s_type;}							\
virtual const void* GetType() const override { return &s_type; }				\
virtual const char* GetTypeAsString() const override { return QUOTE(CLASS); }	\

#define RTTI_TYPE_DEFINE(CLASS) const int CLASS::s_type = 0; 

//-------------------------------------------------------------------------------------------------
// COMPILE-TIME OPTIONS

// To remove asserts, add #define DISABLE_ASSERTS to Assert.h
// #define DISABLE_ASSERTS

// For interning string IDs and checking for hash collisions
#define DEBUG_STRINGID

// For setting shader debug compile flags
//#define DEBUG_SHADERS

// For setting DX device debug flags
#define DEBUG_DX_DEVICE

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef char int8;
typedef short int16;
typedef long long int64;

class StringIdSystem;
class DebugRenderSystem;
class DevConsole;
class EventSystem;
class FontLoader;
class InputSystem;
class JobSystem;
class RenderContext;
class ResourceSystem;
class Window;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
extern StringIdSystem*		g_sidSystem;
extern DebugRenderSystem*	g_debugRenderSystem;
extern DevConsole*			g_devConsole;
extern EventSystem*			g_eventSystem;
extern FontLoader*			g_fontLoader;
extern InputSystem*			g_inputSystem;
extern JobSystem*			g_jobSystem;
extern RenderContext*		g_renderContext;
extern Window*				g_window;
extern ResourceSystem*		g_resourceSystem;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
