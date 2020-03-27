///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 26th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/MathUtils.h"
#include "Engine/Time/Clock.h"
#include "Engine/Time/Time.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
Clock Clock::s_masterClock;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Clock::Clock(Clock* parent)
{
	m_parent = (parent != nullptr ? parent : &s_masterClock);
	m_parent->AddChild(this);
	ResetTimeData();
}


//-------------------------------------------------------------------------------------------------
void Clock::ResetMaster()
{
	s_masterClock.ResetTimeData();
	s_masterClock.m_deltaLimitSeconds = 0.2f; // To prevent breakpoints from stepping too large
}


//-------------------------------------------------------------------------------------------------
void Clock::BeginMasterFrame()
{
	uint64 currentHPC = GetPerformanceCounter();
	uint64 frameHPCDelta = currentHPC - s_masterClock.m_lastFrameHPC;
	s_masterClock.m_lastFrameHPC = currentHPC;

	// Update based on elapsed
	s_masterClock.FrameStep(frameHPCDelta);
}


//-------------------------------------------------------------------------------------------------
void Clock::FrameStep(uint64 frameDeltaHPC)
{
	if (m_isPaused)
	{
		frameDeltaHPC = 0;
	}
	else
	{
		// Apply scaling
		frameDeltaHPC = (uint64)((double)frameDeltaHPC * m_scale);
		m_frameCount++;
	}

	double frameDeltaSeconds = TimeSystem::PerformanceCountToSeconds(frameDeltaHPC);

	// Clamp the elapsed to fit the delta max (useful for breakpoint debugging)
	if (frameDeltaSeconds > m_deltaLimitSeconds)
	{
		frameDeltaSeconds = m_deltaLimitSeconds;
		frameDeltaHPC = TimeSystem::SecondsToPerformanceCount(frameDeltaSeconds);
	}

	m_frameData.m_seconds = frameDeltaSeconds;
	m_frameData.m_hpc = frameDeltaHPC;

	m_totalData.m_seconds += frameDeltaSeconds;
	m_totalData.m_hpc += frameDeltaHPC;

	// Step all children
	for (uint32 clockIndex = 0; clockIndex < (uint32)m_childClocks.size(); ++clockIndex)
	{
		m_childClocks[clockIndex]->FrameStep(frameDeltaHPC);
	}
}


//-------------------------------------------------------------------------------------------------
void Clock::ResetTimeData()
{
	m_lastFrameHPC = GetPerformanceCounter();
	memset(&m_frameData, 0, sizeof(TimeData));
	memset(&m_totalData, 0, sizeof(TimeData));
	m_frameCount = 0;
}


//-------------------------------------------------------------------------------------------------
void Clock::AddChild(Clock* child)
{
	ASSERT_OR_DIE(child != nullptr, "Adding nullptr child clock!");
	m_childClocks.push_back(child);
}


//-------------------------------------------------------------------------------------------------
void Clock::SetMaxDeltaTimeSeconds(float maxSeconds)
{
	m_deltaLimitSeconds = Clamp(maxSeconds, 0.f, maxSeconds);
}
