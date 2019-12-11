#pragma once

#include <cstdint>
#include <atomic>

namespace JobSystem
{
typedef void (*Work)(const void*);

const unsigned cacheline_size = 64;

struct alignas(cacheline_size) Job
{
	void run();

private:
	Work function;
	std::atomic<int> *counter;
	uint8_t data[cacheline_size - sizeof(Work) - sizeof(std::atomic<int>*)];

	friend void run(Work func, const void *data, unsigned n, std::atomic<int> *counter);
};
static_assert(sizeof(Job) == cacheline_size, "Job size is invalid");

template <typename T, typename D>
struct ParallelFor
{
	template <typename... Args>
	ParallelFor(T *_start, T *_end, Args&&... data):
		start(_start), end(_end), user_data{data...} {}

	template <typename... Args>
	ParallelFor(T *_start, unsigned _count, Args&&... data):
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

template <typename T, typename D>
unsigned parallel_for(Work func, ParallelFor<T, D> *data, std::atomic<int> *counter = nullptr)
{
	T *first = data->start;
	T *last = data->end;

	unsigned num_worker = worker_count();
	unsigned count = last - first;
	unsigned i = 0, load = div_ceil(count, num_worker);
	if (load < 32u) load = 32u;

	while (i < num_worker)
	{
		data->start = first + (i++) * load;
		if (i * load > count) data->end = last;
		else data->end = first + i * load;

		run(func, data, sizeof(*data), counter);

		if (data->end == last) break;
	}

	return i;
}
}
