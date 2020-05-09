#pragma once

#include "JobSystem/JobSystem.h"

namespace JobSystem
{

template <typename S>
inline void run(WorkT<S*> func, S &scratch, Semaphore *counter)
{
	run((Work)func, NULL, &scratch, sizeof(S), counter);
}

template <typename D>
inline void run(WorkT<D*> func, D *data, Semaphore *counter)
{
	run((Work)func, data, NULL, 0, counter);
}

#ifdef PROFILE
template <typename D>
inline void run(WorkT<D*> func, D *data, Semaphore *counter, MicroProfileToken token)
{
	run((Work)func, data, &token, 0, counter);
}
#endif


/*
template <typename T, typename D>
unsigned parallel_for(WorkT<D> func, ParallelFor<T, D> *data, Semaphore *counter)
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
*/

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

unsigned div_ceil(unsigned a, unsigned b)
{
	return (a + b - 1) / b;
}

}
