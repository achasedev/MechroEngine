///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 25th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include <shared_mutex>
#include <vector>
#include <thread>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Job;

enum JobStatus
{
	JOB_STATUS_QUEUED,
	JOB_STATUS_RUNNING,
	JOB_STATUS_FINISHED,
	JOB_STATUS_NOT_FOUND
};

enum WorkerThreadFlags : uint32
{
	WORKER_FLAGS_ALL = 0xFFFFFFFF,
	WORKER_FLAGS_DISK = 0x1,
	WORKER_FLAGS_ALL_BUT_DISK = WORKER_FLAGS_ALL & ~WORKER_FLAGS_DISK
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class JobSystem
{
	friend class JobWorkerThread;

public:
	//-----Public Methods-----

	static void			Initialize();
	static void			Shutdown();

	void				CreateWorkerThread(const char* name, WorkerThreadFlags flags);
	void				DestroyWorkerThread(const char* name);
	void				DestroyAllWorkerThreads();

	int					QueueJob(Job* job);

	JobStatus			GetJobStatus(int jobID);
	bool				IsJobFinished(int jobID);

	void				FinalizeAllFinishedJobs();
	void				FinalizeAllFinishedJobsOfType(int jobType);
	void				BlockUntilJobIsFinalized(int jobID);
	void				BlockUntilAllJobsOfTypeAreFinalized(int jobType);


private:
	//-----Private Methods-----

	JobSystem();
	~JobSystem();
	JobSystem(const JobSystem& copy) = delete;

	void				DestroyAllJobs();
	int					GetNextJobID();


private:
	//-----Private Data-----
	
	// Threads
	std::thread::id					m_mainThreadID;
	std::vector<JobWorkerThread*>	m_workerThreads;

	// Job IDs
	int								m_nextJobID = 0;

	// Jobs waiting to be run
	std::shared_mutex				m_queuedLock;
	std::vector<Job*>				m_queuedJobs;

	// Jobs being executed
	std::shared_mutex				m_runningLock;
	std::vector<Job*>				m_runningJobs;

	// Jobs waiting to be collected
	std::shared_mutex				m_finishedLock;
	std::vector<Job*>				m_finishedJobs;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
