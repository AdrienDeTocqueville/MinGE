#include "Profiler/profiler.h"

#include "JobSystem.inl"
#include "Math/Random.h"

#include <cassert>
#include <cstring>
#include <thread>

#ifdef __linux__
#include <pthread.h>
#elif _WIN32
#include <Windows.h>
#else
#error Unsupported OS
#endif


namespace JobSystem
{
// Job system
bool work = true; // flag to tell workers to stop working

// Workers
unsigned num_worker;
struct Worker *workers = nullptr;
thread_local unsigned this_worker;

// Job pools
const unsigned JOB_POOL_SIZE = 512;
thread_local Job job_pool[JOB_POOL_SIZE];
thread_local uint32_t job_pool_index = 0;

static inline Job* allocate_job()
{
	const uint32_t index = job_pool_index++;
	return &job_pool[index & (JOB_POOL_SIZE-1u)];
}

#ifdef __linux__
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

#define ATOMIC_EXCHANGE  __sync_lock_test_and_set
#define COMPARE_EXCHANGE __sync_bool_compare_and_swap

#define THIS_THREAD pthread_self()

#elif _WIN32
#define COMPILER_BARRIER() std::atomic_thread_fence(std::memory_order_release);

#define ATOMIC_EXCHANGE InterlockedExchange
#define COMPARE_EXCHANGE(dest, cmp, val) (InterlockedCompareExchange(dest, val, cmp) == cmp)

#define THIS_THREAD GetCurrentThread()

#endif

struct Worker
{
	static const unsigned NUMBER_OF_JOBS = 512u;
	static const unsigned MASK = NUMBER_OF_JOBS - 1u;

	std::thread thread;
	long bottom = 0, top = 0;
	Job *jobs[NUMBER_OF_JOBS];

	void push(Job* job)
	{
		long b = bottom;
		jobs[b & MASK] = job;

		// ensure the job is written before b+1 is published to other threads.
		// on x86/64, a compiler barrier is enough.
		COMPILER_BARRIER();

		bottom = b + 1;
	}

	Job* pop()
	{
		long b = bottom - 1;
		ATOMIC_EXCHANGE(&bottom, b);

		long t = top;
		if (t <= b)
		{
			// non-empty queue
			Job* job = jobs[b & MASK];
			if (t != b)
			{
				// there's still more than one item left in the queue
				return job;
			}

			// this is the last item in the queue
			if (!COMPARE_EXCHANGE(&top, t, t+1))
			{
				// failed race against steal operation
				job = nullptr;
			}

			bottom = t+1;
			return job;
		}
		else
		{
			// deque was already empty
			bottom = t;
			return nullptr;
		}
	}

	Job* steal()
	{
		long t = top;

		// ensure that top is always read before bottom.
		// loads will not be reordered with other loads on x86, so a compiler barrier is enough.
		COMPILER_BARRIER();

		long b = bottom;
		if (t < b)
		{
			// non-empty queue
			Job* job = jobs[t & MASK];

			// the interlocked function serves as a compiler barrier, and guarantees that the read happens before the CAS.
			if (!COMPARE_EXCHANGE(&top, t, t+1))
			{
				// a concurrent steal or pop operation removed an element from the deque in the meantime.
				return nullptr;
			}

			return job;
		}
		else
		{
			// empty queue
			return nullptr;
		}
	}
};

// Returns a job whose dependencies may not be satisfied
static Job *pop_or_steal()
{
	if (Job *j = workers[this_worker].pop())
		return j;

#ifndef SINGLE_THREADED
	// Pick a random worker to steal from
	unsigned steal_worker = Random::next<int>(0, num_worker - 1);
	steal_worker += (steal_worker >= this_worker);

	if (Job *j = workers[steal_worker].steal())
		return j;
#endif

	return nullptr;
}

static Job *get_job();
static inline bool is_job_ready(Job *j, Job **alternative)
{
	if (j->depen_begin == j->depen_end)
		return true;
	if (j->depen_end == NULL) // Only one dependency
	{
		if (JobSystem::dependencies_done(j->dependency))
			return true;
		*alternative = get_job();
		return false;
	}
	do {
		if (!JobSystem::dependencies_done(*j->depen_begin))
		{
			*alternative = get_job();
			return false;
		}
		j->depen_begin++;
	}
	while (j->depen_begin != j->depen_end);
	return true;
}

// Returns a job ready to be executed
static Job *get_job()
{
	if (Job *j = pop_or_steal())
	{
		Job *alternative;
		if (is_job_ready(j, &alternative))
			return j;
		workers[this_worker].push(j);
		return alternative;
	}
	return nullptr;
}

static inline void job_run(Job *job)
{
	job->function(job->data);
	if (job->counter)
		remove_dependency(job->counter);
}

static void worker_main(const int i)
{
	this_worker = i; // TLS

#ifdef PROFILE
	char worker_name[16];
	snprintf(worker_name, sizeof(worker_name), "worker %d", i);
	MicroProfileOnThreadCreate(worker_name);
#endif

	while (true)
	{
		if (Job* job = get_job())
			job_run(job);

		else if (!JobSystem::work) break;
		else std::this_thread::yield();
	}

#ifdef PROFILE
	MicroProfileOnThreadExit();
#endif
}

static void set_cpu_affinity(const std::thread::native_handle_type handle, const int cpu)
{
	bool failed;

#ifdef __linux__
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	failed = pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset);
#elif _WIN32
	failed = SetThreadAffinityMask(handle, uint64_t(1) << cpu) == 0;
#endif

#ifdef DEBUG
	if (failed) printf("[WARNING] Failed to set cpu affinity for cpu %d\n", cpu);
#endif
}

void init()
{
	if (workers)
		return;

#ifdef SINGLE_THREADED
	num_worker = 1;
#else
	num_worker = std::thread::hardware_concurrency();
#endif

	workers = new Worker[num_worker];
	work = true;

	// Launch threads
	set_cpu_affinity(THIS_THREAD, 0); // main thread
	for (unsigned i(1); i < num_worker; i++)
	{
		workers[i].thread = std::thread(worker_main, i);
		set_cpu_affinity(workers[i].thread.native_handle(), i);
	}
}

void destroy()
{
	work = false;

	for (unsigned i(1); i < num_worker; i++)
		workers[i].thread.join();
	delete[] workers;
}

void run(Work func, void *data, unsigned n, std::atomic<int> *counter)
{
	assert(n <= sizeof(Job::data));

	Job* job = allocate_job();
	job->function = func;
	job->counter = counter;
	job->depen_begin = NULL;
	job->depen_end = NULL;
	memcpy(job->data, data, n);

	workers[this_worker].push(job);
}

void run_child(Work func, void *data, unsigned n, std::atomic<int> *dependency, std::atomic<int> *counter)
{
	assert(n <= sizeof(Job::data));

	Job* job = allocate_job();
	job->function = func;
	job->counter = counter;
	job->dependency = dependency;
	job->depen_end = NULL;
	memcpy(job->data, data, n);

	workers[this_worker].push(job);
}

void run_child(Work func, void *data, unsigned n, std::atomic<int> **dependencies, uint64_t dependency_count, std::atomic<int> *counter)
{
	assert(n <= sizeof(Job::data));

	Job* job = allocate_job();
	job->function = func;
	job->counter = counter;
	job->depen_begin = dependencies;
	job->depen_end = dependencies + dependency_count;
	memcpy(job->data, data, n);

	workers[this_worker].push(job);
}

void wait(const std::atomic<int> *counter, int value)
{
	while (counter->load(std::memory_order_acquire) != value)
	{
		if (Job* job = get_job())
			job_run(job);

		else std::this_thread::yield();
	}
}

} // namespace
