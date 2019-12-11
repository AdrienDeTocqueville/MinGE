#include "JobSystem.h"
#include "Utility/Random.h"

#include <cassert>
#include <cstring>
#include <thread>

#ifdef __linux__
#include <pthread.h>
#elif _WIN32
#include <Windows.h>
#include <intrin.h>
#else
#error Unsupported OS
#endif


namespace JobSystem
{
// Job system
bool work = true;

// Workers
unsigned num_worker;
struct Worker *workers = nullptr;
thread_local unsigned this_worker;

// Job pools
const unsigned JOB_POOL_SIZE = 512;
thread_local Job job_pool[JOB_POOL_SIZE];
thread_local uint32_t job_pool_index = 0;

Job* allocate_job()
{
	const uint32_t index = job_pool_index++;
	return &job_pool[index & (JOB_POOL_SIZE-1u)];
}

#ifdef __linux__
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

#define ATOMIC_EXCHANGE  __sync_lock_test_and_set
#define COMPARE_EXCHANGE __sync_bool_compare_and_swap

#define MAIN_THREAD pthread_self()

#elif _WIN32
#define COMPILER_BARRIER() _ReadWriteBarrier()

#define ATOMIC_EXCHANGE _InterlockedExchange
#define COMPARE_EXCHANGE(dest, cmp, val) (_InterlockedCompareExchange(dest, val, cmp) == cmp)

#define MAIN_THREAD GetCurrentProcess()

#endif

struct Worker
{
	static const unsigned NUMBER_OF_JOBS = 512u;
	static const unsigned MASK = NUMBER_OF_JOBS - 1u;

	std::thread thread;
	long bottom, top;
	Job *jobs[NUMBER_OF_JOBS];

	Job *get_job()
	{
		if (Job *j = pop())
			return j;

		// Pick a random worker to steal from
		int steal_worker = Random::next<int>(0, num_worker - 1);
		steal_worker += (steal_worker >= this_worker);

		if (Job *j = workers[steal_worker].steal())
			return j;

		return nullptr;
	}

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

void Job::run()
{
	function(data);
	counter->fetch_add(1, std::memory_order_relaxed);
}

void worker_main(const int i)
{
	this_worker = i; // TLS

	while (true)
	{
		if (Job* job = workers[i].get_job())
			job->run();

		else if (!JobSystem::work) break;
		else std::this_thread::yield();
	}
}

void set_cpu_affinity(const std::thread::native_handle_type handle, const int cpu)
{
#ifdef __linux__
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	bool failed = pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset);
#elif _WIN32
	bool failed = SetProcessAffinityMask(handle, 1 << cpu) == 0;
#endif

	if (failed)
		printf("[WARNING] Failed to set cpu affinity\n");
}

void init()
{
	if (workers)
		return;

	num_worker = std::thread::hardware_concurrency();
	workers = new Worker[num_worker];

	work = true;
	for (unsigned i(0); i < num_worker; i++)
		workers[i].bottom = workers[i].top = 0;

	// Launch threads
	set_cpu_affinity(MAIN_THREAD, 0);
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

void run(Work func, const void *data, unsigned n, std::atomic<int> *counter)
{
	assert(n <= sizeof(Job::data));

	Job* job = allocate_job();
	job->function = func;
	job->counter = counter;
	memcpy(job->data, data, n);

	workers[this_worker].push(job);
}

void wait(const std::atomic<int> *counter, const int value)
{
	while (counter->load(std::memory_order_relaxed) != value)
	{
		if (Job* job = workers[this_worker].get_job())
			job->run();

		else std::this_thread::yield();
	}
}

unsigned worker_count()
{
	return num_worker;
}

unsigned worker_id()
{
	return this_worker;
}

} // namespace
