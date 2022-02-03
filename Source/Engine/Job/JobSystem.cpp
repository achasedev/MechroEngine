///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 25th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Job/Job.h"
#include "Engine/Job/JobSystem.h"
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
JobSystem* g_jobSystem = nullptr;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
JobSystem::JobSystem()
{
	// JobSystem is only safe against deadlock/race conditions if only the main thread
	// is allowed to queue jobs, so cache off it's ID for asserting
	m_mainThreadID = std::this_thread::get_id();
}


//-------------------------------------------------------------------------------------------------
JobSystem::~JobSystem()
{
	DestroyAllWorkerThreads();
	DestroyAllJobs();
}


//-------------------------------------------------------------------------------------------------
void JobSystem::Initialize()
{
	g_jobSystem = new JobSystem();

	// Create one thread just for general work
	g_jobSystem->CreateWorkerThread("DEFAULT", WORKER_FLAGS_ALL);
}


//-------------------------------------------------------------------------------------------------
void JobSystem::Shutdown()
{
	SAFE_DELETE(g_jobSystem);
}


//-------------------------------------------------------------------------------------------------
void JobSystem::CreateWorkerThread(const char* name, WorkerThreadFlags flags)
{
	JobWorkerThread* workerThread = new JobWorkerThread(name, flags);
	m_workerThreads.push_back(workerThread);
}


//-------------------------------------------------------------------------------------------------
// Will finish the current worker's job if it has one
void JobSystem::DestroyWorkerThread(const char* name)
{
	int numThreads = (int)m_workerThreads.size();

	for (int threadIndex = 0; threadIndex < numThreads; ++threadIndex)
	{
		JobWorkerThread* workerThread = m_workerThreads[threadIndex];

		if (workerThread->GetName() == name)
		{
			m_workerThreads.erase(m_workerThreads.begin() + threadIndex);

			workerThread->StopRunning();
			workerThread->Join();

			delete workerThread;
			return;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void JobSystem::DestroyAllWorkerThreads()
{
	// Tell all to stop running first, for less potential waiting when we join each
	int numThreads = (int)m_workerThreads.size();

	for (int threadIndex = 0; threadIndex < numThreads; ++threadIndex)
	{
		m_workerThreads[threadIndex]->StopRunning();
	}

	for (int threadIndex = 0; threadIndex < numThreads; ++threadIndex)
	{
		m_workerThreads[threadIndex]->Join();
	}

	m_workerThreads.clear();
}


//-------------------------------------------------------------------------------------------------
int JobSystem::QueueJob(Job* job)
{
	// Only main thread can queue jobs!
	ASSERT_OR_DIE(std::this_thread::get_id() == m_mainThreadID, "Job queued from a non-main thread!");

	m_queuedLock.lock();
	{
		job->m_jobID = GetNextJobID();
		m_queuedJobs.push_back(job);
	}
	m_queuedLock.unlock();

	return job->m_jobID;
}



//-------------------------------------------------------------------------------------------------
// This is probably slow and will interfere heavily with workerthreads if called repeatedly
JobStatus JobSystem::GetJobStatus(int jobID)
{
	bool jobFound = false;

	// Check queued jobs
	m_queuedLock.lock_shared();
	{
		int numQueued = (int)m_queuedJobs.size();

		for (int queuedIndex = 0; queuedIndex < numQueued; ++queuedIndex)
		{
			if (m_queuedJobs[queuedIndex]->m_jobID == jobID)
			{
				jobFound = true;
				break;
			}
		}
	}
	m_queuedLock.unlock_shared();

	if (jobFound)
	{
		return JOB_STATUS_QUEUED;
	}

	// Check finished jobs
	m_finishedLock.lock_shared();
	{
		int numFinished = (int)m_finishedJobs.size();

		for (int finishedIndex = 0; finishedIndex < numFinished; ++finishedIndex)
		{
			if (m_finishedJobs[finishedIndex]->m_jobID == jobID)
			{
				jobFound = true;
				break;
			}
		}
	}
	m_finishedLock.unlock_shared();

	if (jobFound)
	{
		return JOB_STATUS_FINISHED;
	}

	// Check running jobs
	m_runningLock.lock_shared();
	{
		int numRunning = (int)m_runningJobs.size();

		for (int runningIndex = 0; runningIndex < numRunning; ++runningIndex)
		{
			if (m_runningJobs[runningIndex]->m_jobID == jobID)
			{
				jobFound = true;
				break;
			}
		}
	}
	m_runningLock.unlock_shared();

	if (jobFound)
	{
		return JOB_STATUS_RUNNING;
	}

	return JOB_STATUS_NOT_FOUND;
}


//-------------------------------------------------------------------------------------------------
bool JobSystem::IsJobFinished(int jobID)
{
	bool jobFinished = false;

	// Check finished jobs
	m_finishedLock.lock_shared();
	{
		int numFinished = (int)m_finishedJobs.size();

		for (int finishedIndex = 0; finishedIndex < numFinished; ++finishedIndex)
		{
			if (m_finishedJobs[finishedIndex]->m_jobID == jobID)
			{
				jobFinished = true;
			}
		}
	}
	m_finishedLock.unlock_shared();

	return jobFinished;
}


//-------------------------------------------------------------------------------------------------
void JobSystem::FinalizeAllFinishedJobs()
{
	m_finishedLock.lock();
	{
		int numFinished = (int)m_finishedJobs.size();

		for (int finishedIndex = 0; finishedIndex < numFinished; ++finishedIndex)
		{
			m_finishedJobs[finishedIndex]->Finalize();
			delete m_finishedJobs[finishedIndex];
		}

		m_finishedJobs.clear();
	}
	m_finishedLock.unlock();
}


//-------------------------------------------------------------------------------------------------
void JobSystem::FinalizeAllFinishedJobsOfType(int jobType)
{
	m_finishedLock.lock();
	{
		int numFinished = (int)m_finishedJobs.size();

		for (int finishedIndex = numFinished - 1; finishedIndex >= 0; --finishedIndex)
		{
			if (m_finishedJobs[finishedIndex]->m_jobType == jobType)
			{
				m_finishedJobs[finishedIndex]->Finalize();

				delete m_finishedJobs[finishedIndex];
				m_finishedJobs.erase(m_finishedJobs.begin() + finishedIndex);
			}
		}
	}
	m_finishedLock.unlock();
}


//-------------------------------------------------------------------------------------------------
void JobSystem::BlockUntilJobIsFinalized(int jobID)
{
	while (!IsJobFinished(jobID))
	{
		// Spinwait/sleep
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// Job is done - find it and finalize it, then delete it
	m_finishedLock.lock();
	{
		int numFinished = (int)m_finishedJobs.size();

		for (int finishedIndex = 0; finishedIndex < numFinished; ++finishedIndex)
		{
			if (m_finishedJobs[finishedIndex]->m_jobID == jobID)
			{
				m_finishedJobs[finishedIndex]->Finalize();

				delete m_finishedJobs[finishedIndex];
				m_finishedJobs.erase(m_finishedJobs.begin() + finishedIndex);
				break;
			}
		}
	}
	m_finishedLock.unlock();
}


//-------------------------------------------------------------------------------------------------
// *NOTE* Only guarentees that it will wait for all job types that are pushed BEFORE this function
// is called - Don't push any more after calling this!
//
void JobSystem::BlockUntilAllJobsOfTypeAreFinalized(int jobType)
{
	bool jobOfTypeStillQueuedOrRunning = true;

	while (jobOfTypeStillQueuedOrRunning)
	{
		jobOfTypeStillQueuedOrRunning = false;

		// Check queued jobs
		m_queuedLock.lock_shared();
		m_runningLock.lock_shared();
		{
			bool foundQueuedJobOfType = false;
			bool foundRunningJobOfType = false;
			int numQueued = (int)m_queuedJobs.size();

			for (int queuedIndex = 0; queuedIndex < numQueued; ++queuedIndex)
			{
				if (m_queuedJobs[queuedIndex]->m_jobType == jobType)
				{
					foundQueuedJobOfType = true;
					break;
				}
			}

			if (!foundQueuedJobOfType)
			{
				int numRunning = (int)m_runningJobs.size();
				for (int runningIndex = 0; runningIndex < numRunning; ++runningIndex)
				{
					if (m_runningJobs[runningIndex]->m_jobType == jobType)
					{
						foundRunningJobOfType = true;
						break;
					}
				}
			}

			jobOfTypeStillQueuedOrRunning = foundQueuedJobOfType || foundRunningJobOfType;
		}
		m_runningLock.unlock_shared();
		m_queuedLock.unlock_shared();

		// Reduce contention?
		if (jobOfTypeStillQueuedOrRunning)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	// No jobs of the given type are queued or running...
	// *Technically* someone could push a new job of the given type RIGHT NOW, but they shouldn't
	// be pushing more after this function is called.....
	// i.e. only the thread calling this function should be the one pushing jobs of this type, at least
	// at the same time

	// Finalize all the finished jobs of the type
	FinalizeAllFinishedJobsOfType(jobType);
}


//-------------------------------------------------------------------------------------------------
void JobSystem::AbortAllQueuedJobsOfType(int jobType)
{
	m_queuedLock.lock_shared();
	{
		int numQueued = (int)m_queuedJobs.size();

		for (int queuedIndex = numQueued - 1; queuedIndex >= 0; --queuedIndex)
		{
			if (m_queuedJobs[queuedIndex]->m_jobType == jobType)
			{
				delete m_queuedJobs[queuedIndex];
				m_queuedJobs.erase(m_queuedJobs.begin() + queuedIndex);
			}
		}
	}
	m_queuedLock.unlock_shared();
}


//-------------------------------------------------------------------------------------------------
void JobSystem::DestroyAllJobs()
{
	// Queued - just delete them
	m_queuedLock.lock();
	{
		int numQueued = (int)m_queuedJobs.size();

		for (int queuedIndex = 0; queuedIndex < numQueued; ++queuedIndex)
		{
			delete m_queuedJobs[queuedIndex];
		}

		m_queuedJobs.clear();
	}
	m_queuedLock.unlock();

	// This list *SHOULD* be empty, since this function is only called from ~JobSystem()
	ASSERT_OR_DIE(m_runningJobs.size() == 0, "JobSystem destructor still had running jobs");

	// Finished jobs - Don't finalize, since we cannot guarantee anything still exists
	m_finishedLock.lock();
	{
		int numFinished = (int)m_finishedJobs.size();

		for (int finishedIndex = 0; finishedIndex < numFinished; ++finishedIndex)
		{
			delete m_finishedJobs[finishedIndex];
		}

		m_finishedJobs.clear();
	}
	m_finishedLock.unlock();
}


//-------------------------------------------------------------------------------------------------
int JobSystem::GetNextJobID()
{
	return m_nextJobID++;
}
