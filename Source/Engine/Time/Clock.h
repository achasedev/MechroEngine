///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 26th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include <vector>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
struct TimeData
{
	TimeData()
		: m_hpc(0) , m_seconds(0) {}

	uint64	m_hpc;
	double	m_seconds;
};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Clock
{
public:
	//-----Public Methods-----

	// Constructors
	Clock(Clock* parent);

	static void		ResetMaster();
	static void		BeginMasterFrame();
	void			FrameStep(uint64 frameDeltaHPC);

	// Mutators
	void			ResetTimeData();
	void			AddChild(Clock* child);
	void			SetMaxDeltaTimeSeconds(float maxSeconds);
	void			SetScale(float newScale) { m_scale = newScale; }
	void			SetPaused(bool isPaused) { m_isPaused = isPaused; }

	// Accessors
	float			GetDeltaSeconds() const	{ return static_cast<float>(m_frameData.m_seconds); }
	float			GetTotalSeconds() const	{ return static_cast<float>(m_totalData.m_seconds); }
	uint64			GetDeltaTimeHPC() const	{ return m_frameData.m_hpc; }
	uint64			GetTotalHPC() const		{ return m_totalData.m_hpc; }
	static Clock*	GetMasterClock()		{ return &s_masterClock; }


private:
	//-----Private Methods

	Clock() {}							// Default only allowed for master clock
	Clock(const Clock& copy) = delete;	// No copying allowed


private:
	//-----Private Data-----

	Clock*				m_parent = nullptr;		// This clock's hierarchical parent
	std::vector<Clock*> m_childClocks;			// and children

	uint32				m_frameCount = 0;
	double				m_scale = 1.0f;
	bool				m_isPaused = false;
	double				m_deltaLimitSeconds = 999999999999.f;

	TimeData			m_frameData;
	TimeData			m_totalData;

	uint64				m_lastFrameHPC = 0;

	// static by value so it's constructed before main
	static Clock		s_masterClock;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
