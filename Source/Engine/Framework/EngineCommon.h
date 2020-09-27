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
#define SAFE_DELETE_POINTER(p)  if (p != nullptr) { delete p; p = nullptr; }
#define SAFE_FREE_POINTER(p)  if (p != nullptr) { free(p); p = nullptr; }
#define BIT_FLAG(x) (1 << x)
#define _QUOTE(x) # x
#define QUOTE(x) _QUOTE(x)
#define UNIMPLEMENTED()  QUOTE(__FILE__) " (" QUOTE(__LINE__) ")" ; ERROR_AND_DIE("Function unimplemented!") 
#define _CAT(A,B) A##B
#define CAT(A,B) _CAT(A,B)
#define NO_RETURN_VAL

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

class DebugSIDSystem;
class EventSystem;
class FontLoader;
class InputSystem;
class JobSystem;
class RenderContext;
class Window;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
extern DebugSIDSystem*	g_debugSIDSystem;
extern EventSystem*		g_eventSystem;
extern FontLoader*	g_FontLoader;
extern InputSystem*		g_inputSystem;
extern JobSystem*		g_jobSystem;
extern RenderContext*	g_renderContext;
extern Window*			g_window;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
