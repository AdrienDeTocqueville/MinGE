#include "JobSystem/JobSystem.h"

namespace JobSystem
{

template <typename D>
inline void run(Work func, const D *data, std::atomic<int> *counter)
{
	run(func, data, sizeof(D), counter);
}

template <typename T, typename D>
unsigned parallel_for(Work func, ParallelFor<T, D> *data, std::atomic<int> *counter)
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

void mark_job(std::atomic<int> *counter)
{
	counter->fetch_add(1, std::memory_order_acquire);
}

unsigned div_ceil(unsigned a, unsigned b)
{
	return (a + b - 1) / b;
}

}