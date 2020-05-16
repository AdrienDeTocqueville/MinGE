#pragma once

#include <stdint.h>
#include <atomic>

#include "Profiler/profiler.h"

namespace JobSystem
{

using Work = void (*)(void*);
template <typename T>
using WorkT = void (*)(T);

const unsigned job_alignment = 64;

// Not really a semaphore I think
struct Semaphore
{
	Semaphore(): n(0) {}
	void wait();
	std::atomic<int> n;
};

struct alignas(job_alignment) Job
{
	void *fiber;
	Work function;
	Semaphore *counter;
	void *data;

	union {
	uint8_t scratch[job_alignment - 2*sizeof(void*) - sizeof(Work) - sizeof(Semaphore*)];
	IF_PROFILE(struct { MicroProfileToken token; uint64_t tick; });
	};
};
static_assert(sizeof(Job) == job_alignment, "Job size is invalid");

template <typename T, typename D>
struct ParallelFor
{
	template <typename... Args>
	ParallelFor(T *_start, T *_end, Args&&... data) :
		start(_start), end(_end), user_data{ data... } {}

	template <typename... Args>
	ParallelFor(T *_start, unsigned _count, Args&&... data) :
		ParallelFor(_start, _start + _count, data...) {}

	T *start, *end;
	D user_data;
};

void init();
void destroy();
void sleep();

void yield();

void run(Work func, void *data, void *scratch, size_t n, Semaphore *counter = nullptr);

template <typename S>
inline void run(WorkT<S*> func, S &scratch, Semaphore *counter = nullptr);
template <typename D>
inline void run(WorkT<D*> func, D *data, Semaphore *counter = nullptr);

#ifdef PROFILE
template <typename D>
inline void run(WorkT<D*> func, D *data, Semaphore *counter, MicroProfileToken token);
#endif

//template <typename T, typename D>
//unsigned parallel_for(WorkT<D> func, ParallelFor<T, D> *data, Semaphore *counter = nullptr);

inline unsigned worker_count();
inline unsigned worker_id();

inline unsigned div_ceil(unsigned a, unsigned b);
}
