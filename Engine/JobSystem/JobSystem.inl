#pragma once

#include "JobSystem/JobSystem.h"

namespace JobSystem
{

template <typename D>
inline void run(WorkT<D> func, D *data, std::atomic<int> *counter)
{
	run((Work)func, data, sizeof(D), counter);
}

template <typename D>
inline void run_child(WorkT<D> func, D *data, std::atomic<int> *dependency, std::atomic<int> *counter)
{
	run_child((Work)func, data, sizeof(D), dependency, counter);
}

template <typename D>
inline void run_child(WorkT<D> func, D *data, std::atomic<int> **dependencies, uint64_t dependency_count, std::atomic<int> *counter)
{
	run_child((Work)func, data, sizeof(D), dependencies, dependency_count, counter);
}

template <typename T, typename D>
unsigned parallel_for(WorkT<D> func, ParallelFor<T, D> *data, std::atomic<int> *counter)
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

unsigned worker_count()
{
	extern unsigned num_worker;
	return num_worker;
}

unsigned worker_id()
{
	extern thread_local unsigned this_worker;
	return this_worker;
}

void add_dependency(std::atomic<int> *counter)
{
	counter->fetch_add(1, std::memory_order_acquire);
}

void remove_dependency(std::atomic<int> *counter)
{
	counter->fetch_sub(1, std::memory_order_release);
}

void reset_dependencies(std::atomic<int> *counter)
{
	counter->store(1, std::memory_order_release);
}

bool dependencies_done(std::atomic<int> *counter)
{
	return counter->load(std::memory_order_acquire) == 0;
}

unsigned div_ceil(unsigned a, unsigned b)
{
	return (a + b - 1) / b;
}

}
