///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 3rd, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Time/Clock.h"
#include "Engine/Time/FrameTimer.h"
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

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
FrameTimer::FrameTimer()
{
	SetClock(nullptr);
	Reset();
}


//-------------------------------------------------------------------------------------------------
FrameTimer::FrameTimer(Clock* referenceClock)
{
	SetClock(referenceClock);
	Reset();
}


//-------------------------------------------------------------------------------------------------
void FrameTimer::Reset()
{
	m_startHPC = m_referenceClock->GetTotalHPC();
	m_endHPC = m_startHPC;
}


//-------------------------------------------------------------------------------------------------
void FrameTimer::SetClock(Clock* clock)
{
	m_referenceClock = (clock != nullptr ? clock : Clock::GetMasterClock());
}


//-------------------------------------------------------------------------------------------------
void FrameTimer::SetInterval(float seconds)
{
	uint64 interval = TimeSystem::SecondsToPerformanceCount(seconds);
	m_startHPC = m_referenceClock->GetTotalHPC();
	m_endHPC = m_startHPC + interval;
}


//-------------------------------------------------------------------------------------------------
void FrameTimer::SetElapsedTime(float secondsElapsed)
{
	// Save off the interval length to preserve it
	uint64 intervalLength = m_endHPC - m_startHPC;

	uint64 elapsedHPC = TimeSystem::SecondsToPerformanceCount(secondsElapsed);
	uint64 currentHPC = m_referenceClock->GetTotalHPC();

	m_startHPC = currentHPC - elapsedHPC;
	m_endHPC = m_startHPC + intervalLength;
}


//-------------------------------------------------------------------------------------------------
bool FrameTimer::CheckAndReset()
{
	if (HasIntervalElapsed())
	{
		Reset();
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool FrameTimer::CheckAndDecrementAll()
{
	if (DecrementByIntervalAll() > 0)
	{
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool FrameTimer::DecrementByIntervalOnce()
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
int FrameTimer::DecrementByIntervalAll()
{
	uint64 currentHPC = m_referenceClock->GetTotalHPC();
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
float FrameTimer::GetElapsedSeconds() const
{
	uint64 currentHPC = m_referenceClock->GetTotalHPC();
	uint64 elapsedHPC = currentHPC - m_startHPC;

	return static_cast<float>(TimeSystem::PerformanceCountToSeconds(elapsedHPC));
}


//-------------------------------------------------------------------------------------------------
float FrameTimer::GetElapsedTimeNormalized() const
{
	float elapsedSeconds = GetElapsedSeconds();
	float intervalSeconds = static_cast<float>(TimeSystem::PerformanceCountToSeconds(m_endHPC - m_startHPC));

	return (elapsedSeconds / intervalSeconds);
}


//-------------------------------------------------------------------------------------------------
float FrameTimer::GetSecondsUntilIntervalEnds() const
{
	uint64 currentHPC = m_referenceClock->GetTotalHPC();

	float currentSeconds = (float)TimeSystem::PerformanceCountToSeconds(currentHPC);
	float endSeconds = (float)TimeSystem::PerformanceCountToSeconds(m_endHPC);

	float diff = endSeconds - currentSeconds;
	float timeUntilIntervalEnds = Clamp(diff, 0.f, diff);

	return timeUntilIntervalEnds;
}


//-------------------------------------------------------------------------------------------------
bool FrameTimer::HasIntervalElapsed() const
{
	uint64 currentHPC = m_referenceClock->GetTotalHPC();

	return (currentHPC >= m_endHPC);
}


//-------------------------------------------------------------------------------------------------
float FrameTimer::GetDeltaSeconds() const
{
	return m_referenceClock->GetDeltaSeconds();
}
