#pragma once

#include <cstdint>
#include <atomic>

// https://blog.molecular-matters.com/tag/job-system/

namespace JobSystem
{
typedef void (*Work)(const void*);

class Job
{
	Work function;
	const void *data;
	std::atomic<int> *counter;

	friend void run(Work func, const void *data, std::atomic<int> *counter);

public:
	void run();
};

void init();
void destroy();

void run(Work func, const void *data, std::atomic<int> *counter = nullptr);
void wait(const std::atomic<int> *counter, const int value);

unsigned worker_count();
}
