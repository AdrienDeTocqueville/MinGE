#pragma once

#include <cstdint>
#include <atomic>

namespace JobSystem
{

using Work = void (*)(void*);
template <typename D>
using WorkT = void (*)(D*);

const unsigned cacheline_size = 64;

struct alignas(cacheline_size) Job
{
	Work function;
	std::atomic<int> *counter;
	union { // determine value of union if depen_end is NULL
		std::atomic<int> *dependency;
		struct {
			std::atomic<int> **depen_begin, **depen_end;
		};
	};
	uint8_t data[cacheline_size - sizeof(Work) - 3*sizeof(std::atomic<int>*)];
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

void run(Work func, void *data, unsigned n, std::atomic<int> *counter = nullptr);
void run_child(Work func, void *data, unsigned n, std::atomic<int> *dependency, std::atomic<int> *counter = nullptr);
void run_child(Work func, void *data, unsigned n, std::atomic<int> **dependencies, uint64_t dependency_count, std::atomic<int> *counter = nullptr);
void wait(const std::atomic<int> *counter, int value = 0);

template <typename D>
inline void run(WorkT<D> func, D *data, std::atomic<int> *counter = nullptr);
template <typename D>
inline void run_child(WorkT<D> func, D *data, std::atomic<int> *dependency, std::atomic<int> *counter = nullptr);
template <typename D>
inline void run_child(WorkT<D> func, D *data, std::atomic<int> **dependencies, uint64_t dependency_count, std::atomic<int> *counter = nullptr);

template <typename T, typename D>
unsigned parallel_for(WorkT<D> func, ParallelFor<T, D> *data, std::atomic<int> *counter = nullptr);

inline unsigned worker_count();
inline unsigned worker_id();

inline void add_dependency(std::atomic<int> *counter);
inline void remove_dependency(std::atomic<int> *counter);
inline void reset_dependencies(std::atomic<int> *counter);
inline bool dependencies_done(std::atomic<int> *counter);

inline unsigned div_ceil(unsigned a, unsigned b);
}
