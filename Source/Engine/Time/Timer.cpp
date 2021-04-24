///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 3rd, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Time/Time.h"
#include "Engine/Time/Timer.h"

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
Timer::Timer()
{
	Reset();
}



//-------------------------------------------------------------------------------------------------
void Timer::Reset()
{
	m_startHPC = GetPerformanceCounter();
	m_endHPC = m_startHPC;
}


//-------------------------------------------------------------------------------------------------
void Timer::SetInterval(float seconds)
{
	uint64 interval = TimeSystem::SecondsToPerformanceCount(seconds);
	m_startHPC = GetPerformanceCounter();
	m_endHPC = m_startHPC + interval;
}


//-------------------------------------------------------------------------------------------------
void Timer::SetElapsedTime(float secondsElapsed)
{
	// Save off the interval length to preserve it
	uint64 intervalLength = m_endHPC - m_startHPC;

	uint64 elapsedHPC = TimeSystem::SecondsToPerformanceCount(secondsElapsed);
	uint64 currentHPC = GetPerformanceCounter();

	m_startHPC = currentHPC - elapsedHPC;
	m_endHPC = m_startHPC + intervalLength;
}


//-------------------------------------------------------------------------------------------------
bool Timer::CheckAndReset()
{
	if (HasIntervalElapsed())
	{
		Reset();
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool Timer::DecrementByIntervalOnce()
{
	if (HasIntervalElapsed())
	{
		uint64 interval = m_endHPC - m_startHPC;

		m_startHPC += interval;
		m_endHPC += interval;
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
int Timer::DecrementByIntervalAll()
{
	uint64 currentHPC = GetPerformanceCounter();
	uint64 interval = m_endHPC - m_startHPC;

	int numElapses = 0;
	while (m_endHPC < currentHPC)
	{
		m_startHPC += interval;
		m_endHPC += interval;
		numElapses++;
	}

	return numElapses;
}


//-------------------------------------------------------------------------------------------------
float Timer::GetElapsedSeconds() const
{
	uint64 currentHPC = GetPerformanceCounter();
	uint64 elapsedHPC = currentHPC - m_startHPC;

	return static_cast<float>(TimeSystem::PerformanceCountToSeconds(elapsedHPC));
}


//-------------------------------------------------------------------------------------------------
float Timer::GetElapsedTimeNormalized() const
{
	float elapsedSeconds = GetElapsedSeconds();
	float intervalSeconds = static_cast<float>(TimeSystem::PerformanceCountToSeconds(m_endHPC - m_startHPC));

	return (elapsedSeconds / intervalSeconds);
}


//-------------------------------------------------------------------------------------------------
float Timer::GetSecondsUntilIntervalEnds() const
{
	uint64 currentHPC = GetPerformanceCounter();

	float currentSeconds = (float)TimeSystem::PerformanceCountToSeconds(currentHPC);
	float endSeconds = (float)TimeSystem::PerformanceCountToSeconds(m_endHPC);

	float diff = endSeconds - currentSeconds;
	float timeUntilIntervalEnds = Clamp(diff, 0.f, diff);

	return timeUntilIntervalEnds;
}


//-------------------------------------------------------------------------------------------------
bool Timer::HasIntervalElapsed() const
{
	uint64 currentHPC = GetPerformanceCounter();

	return (currentHPC >= m_endHPC);
}
