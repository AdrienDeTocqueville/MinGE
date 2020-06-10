#include "Profiler/profiler.h"

#include <assert.h>
#include <thread>

#ifdef __linux__
#include <pthread.h>
#include <ucontext.h>
#elif _WIN32
#define NOMINMAX
#include <Windows.h>
#else
#error Unsupported OS
#endif

#include <SDL2/SDL.h>

#include "JobSystem/JobSystem.inl"
#include "Math/Random.h"
#include "Utility/Time.h"
#include "Utility/stb_sprintf.h"
#include "Core/Platform.h"

#define SINGLE_THREADED


namespace JobSystem
{
// Job system
static bool work = true; // flag to tell workers to stop working
static bool goto_sleep = false;

// Workers
static unsigned num_worker;
static struct Worker *workers = nullptr;
static thread_local unsigned this_worker;


// Job pools
static const unsigned JOB_POOL_SIZE = 512;
static thread_local Job job_pool[JOB_POOL_SIZE];
static thread_local uint32_t job_pool_index = 0;
#ifdef __linux__
static thread_local ucontext_t job_contexts[JOB_POOL_SIZE];
static thread_local uint8_t job_stacks[JOB_POOL_SIZE][0x1000];
#endif

static inline Job* allocate_job()
{
	const uint32_t index = job_pool_index++;
	return &job_pool[index & (JOB_POOL_SIZE-1u)];
}

#include "JobSystem/Fiber.h"

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

inline Job *pop()
{
	return workers[this_worker].pop();
}

inline Job *steal()
{
#ifdef SINGLE_THREADED
	return nullptr;
#else
	// Pick a random worker to steal from
	unsigned steal_worker = Random::next<int>(0, num_worker - 1);
	steal_worker += (steal_worker >= this_worker);

	return workers[steal_worker].steal();
#endif
}

static Job *pop_or_steal()
{
	if (Job *j = pop())
		return j;
	return steal();
}

static Job *steal_or_pop()
{
	if (Job *j = steal())
		return j;
	return pop();
}

// Not inlined because this_fiber is TLS and it could cause problems
void yield()
{
	Job *job = this_job;
#ifdef PROFILE
	if (job->data != job->scratch && job->token)
		MicroProfileLeave(job->token, job->tick);
#endif

	switch_fiber(job->fiber, this_fiber);

#ifdef PROFILE
	if (job->data != job->scratch && job->token)
		job->tick = MicroProfileEnter(job->token);
#endif
}

static void job_run(Job *job)
{
reset_fiber:

#ifdef __linux__
	this_job = job;
#endif

#ifdef PROFILE
	if (job->data != job->scratch && job->token)
	{
		job->tick = MicroProfileEnter(job->token);
		job->function(job->data);
		MicroProfileLeave(job->token, job->tick);
	} else
#endif

	job->function(job->data);

	job->function = NULL;
	job->counter->n.fetch_sub(1, std::memory_order_release);

	switch_fiber(job->fiber, get_this_fiber_fls());
	goto reset_fiber;
}

void run(Work func, void *data, void *scratch, size_t n, Semaphore *counter)
{
	assert(this_worker == 0 && "TODO: init fibers on other threads");

	MICROPROFILE_SCOPEI("JOB_SYSTEM", "run");

	Job* job = allocate_job();
	job->function = func;
	job->counter = counter;

	if (data && scratch)
	{
		job->data = data;
		IF_PROFILE(memcpy(job->scratch, scratch, sizeof(MicroProfileToken)));
	}
	else if (scratch)
	{
		assert(n <= sizeof(Job::scratch));
		job->data = job->scratch;
		memcpy(job->scratch, scratch, n);
	}
	else
	{
		job->data = data;
		IF_PROFILE(memset(job->scratch, 0, sizeof(MicroProfileToken)));
	}

	if (counter && counter->n == 0) counter->n = 1;

	workers[this_worker].push(job);
}

inline void do_sleep()
{
	MICROPROFILE_SCOPEI("JOB_SYSTEM", "sleep");

	uint32_t dt = Time::frame_duration();
	uint32_t freq = 1000 / 30;
	if (dt < freq) SDL_Delay(freq - dt);
}

#define EXEC_WHILE(condition) 						\
{									\
	Job *job = NULL;						\
	if (condition) while (true) {					\
		if (job || (job = pop_or_steal()))			\
		{							\
			switch_fiber(this_fiber, job->fiber);		\
			if (!(condition)) break;			\
			if (job->function == NULL)			\
				job = pop_or_steal();			\
			else /* Job yielded */				\
			{						\
				workers[this_worker].push(job);		\
				job = steal_or_pop();			\
			}						\
		}							\
									\
		else if (!(condition)) break;				\
		else if (!JobSystem::work) break;			\
		else if (JobSystem::goto_sleep)	do_sleep();		\
	}								\
}

void Semaphore::wait()
{
	EXEC_WHILE(n.load(std::memory_order_acquire) != 0);
}

static void worker_main(const int i)
{
	this_worker = i; // TLS

#ifdef PROFILE
	char worker_name[16];
	stbsp_snprintf(worker_name, sizeof(worker_name), "worker %d", i);
	MicroProfileOnThreadCreate(worker_name);
#endif

	INIT_FIBER_THREAD();

	EXEC_WHILE(true);

	DESTROY_FIBER_THREAD();

	IF_PROFILE(MicroProfileOnThreadExit());
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

#ifdef PROFILE
	MicroProfileOnThreadCreate("main thread");

	MicroProfileSetEnableAllGroups(true);
	MicroProfileSetForceMetaCounters(true);
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

	INIT_FIBER_THREAD();

	for (unsigned i(0); i < JOB_POOL_SIZE; i++)
		job_pool[i].fiber = create_fiber(job_run, i);
}

void destroy()
{
	work = false;

	for (unsigned i(1); i < num_worker; i++)
		workers[i].thread.join();
	delete[] workers;

	for (unsigned i(0); i < JOB_POOL_SIZE; i++)
		delete_fiber(job_pool[i].fiber);

	DESTROY_FIBER_THREAD();

	IF_PROFILE(MicroProfileShutdown());
}

void sleep()
{
	MICROPROFILE_SCOPEI("JOB_SYSTEM", "sleep");

	uint32_t dt = Time::frame_duration();
	uint32_t freq = 1000 / 30;
	if (dt < freq)
	{
		goto_sleep = true;
		SDL_Delay(freq - dt);
		goto_sleep = false;
	}
}

} // namespace
