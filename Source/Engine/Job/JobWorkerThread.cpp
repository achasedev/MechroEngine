///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 25th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Job/Job.h"
#include "Engine/Job/JobWorkerThread.h"

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
JobWorkerThread::JobWorkerThread(const char* name, WorkerThreadFlags flags)
	: m_name(name)
	, m_workerFlags(flags)
{
	m_threadHandle = std::thread(&JobWorkerThread::JobWorkerThreadEntry, this);
}


//-------------------------------------------------------------------------------------------------
JobWorkerThread::~JobWorkerThread()
{
	StopRunning();
	Join();
}


//-------------------------------------------------------------------------------------------------
void JobWorkerThread::Join()
{
	if (m_isRunning)
	{
		StopRunning();
	}

	m_threadHandle.join();
}


//-------------------------------------------------------------------------------------------------
void JobWorkerThread::JobWorkerThreadEntry()
{
	while (m_isRunning)
	{
		Job* nextJob = DequeueJobForExecution();

		if (nextJob != nullptr)
		{
			nextJob->Execute();
			MarkJobAsFinished(nextJob);
		}
		else
		{
			// Sleep to alleviate contention over JobSystem arrays
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}
}


//-------------------------------------------------------------------------------------------------
Job* JobWorkerThread::DequeueJobForExecution()
{
	Job* jobToExecute = nullptr;

	std::vector<Job*>& queuedJobs = g_jobSystem->m_queuedJobs;
	std::vector<Job*>& runningJobs = g_jobSystem->m_runningJobs;

	// Need to get both locks simultaneously to avoid job status falsely being
	// reported as "not found" - it needs to always exist in some array at some point in time
	g_jobSystem->m_queuedLock.lock();
	g_jobSystem->m_runningLock.lock();
	{
		int numQueuedJobs = (int)queuedJobs.size();

		for (int queueIndex = 0; queueIndex < numQueuedJobs; ++queueIndex)
		{
			uint32 jobFlags = queuedJobs[queueIndex]->GetFlags();

			if ((jobFlags & m_workerFlags) == jobFlags)
			{
				jobToExecute = queuedJobs[queueIndex];
				queuedJobs.erase(queuedJobs.begin() + queueIndex);
				break;
			}
		}

		if (jobToExecute != nullptr)
		{
			runningJobs.push_back(jobToExecute);
		}
	}
	g_jobSystem->m_runningLock.unlock();
	g_jobSystem->m_queuedLock.unlock();

	return jobToExecute;
}


//-------------------------------------------------------------------------------------------------
void JobWorkerThread::MarkJobAsFinished(Job* finishedJob)
{
	std::vector<Job*>& runningJobs = g_jobSystem->m_runningJobs;
	std::vector<Job*>& finishedJobs = g_jobSystem->m_finishedJobs;

	// Need to get both locks simultaneously to avoid job status falsely being
	// reported as "not found" - it needs to always exist in some array at some point in time
	g_jobSystem->m_runningLock.lock();
	g_jobSystem->m_finishedLock.lock();
	{
		int numRunningJobs = (int)runningJobs.size();
		bool foundRunning = false;
		for (int runningIndex = 0; runningIndex < numRunningJobs; ++runningIndex)
		{
			if (runningJobs[runningIndex] == finishedJob)
			{
				runningJobs.erase(runningJobs.begin() + runningIndex);
				foundRunning = true;
				break;
			}
		}

		ASSERT_OR_DIE(foundRunning, "Job finished but wasn't in running array!");
		finishedJobs.push_back(finishedJob);
	}
	g_jobSystem->m_finishedLock.unlock();
	g_jobSystem->m_runningLock.unlock();
}
