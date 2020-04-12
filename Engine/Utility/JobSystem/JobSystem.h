#pragma once

#include <cstdint>
#include <atomic>

namespace JobSystem
{
typedef void(*Work)(const void*);

const unsigned cacheline_size = 64;

struct alignas(cacheline_size) Job
{
	Work function;
	std::atomic<int> *counter;
	uint8_t data[cacheline_size - sizeof(Work) - sizeof(std::atomic<int>*)];
};
static_assert(sizeof(Job) == cacheline_size, "Job size is invalid");

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

void run(Work func, const void *data, unsigned n, std::atomic<int> *counter = nullptr);
void wait(const std::atomic<int> *counter, const int value);

unsigned worker_count();
unsigned worker_id();


inline unsigned div_ceil(unsigned a, unsigned b)
{
	return (a + b - 1) / b;
}

template <typename D>
inline void run(Work func, const D *data, std::atomic<int> *counter = nullptr);

template <typename T, typename D>
unsigned parallel_for(Work func, ParallelFor<T, D> *data, std::atomic<int> *counter = nullptr);
}