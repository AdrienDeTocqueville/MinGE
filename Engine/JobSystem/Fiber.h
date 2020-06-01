#ifdef __linux__ /// LINUX

#define INIT_FIBER_THREAD()
#define DESTROY_FIBER_THREAD()

static thread_local ucontext_t main_context;
static thread_local ucontext_t *const this_fiber = &main_context;
#define switch_fiber(from, to) swapcontext((ucontext_t*)(from), (ucontext_t*)(to))

inline void *create_fiber(void (*func)(Job*), unsigned idx)
{
	getcontext(job_contexts + idx);
	job_contexts[idx].uc_stack.ss_sp   = job_stacks[idx];
	job_contexts[idx].uc_stack.ss_size = sizeof(job_stacks[idx]);
	makecontext(job_contexts + idx, (void (*)())func, 1, job_pool + idx);
	return job_contexts + idx;
}

#define delete_fiber(fiber)

static Job *this_job;

#elif _WIN32 /// WINDOWS

static thread_local void *this_fiber;
#define INIT_FIBER_THREAD() this_fiber = ConvertThreadToFiber(0);
#define DESTROY_FIBER_THREAD() ConvertFiberToThread();
#define switch_fiber(from, to) SwitchToFiber(to)

#define create_fiber(func, idx) CreateFiber(0, (Work)func, job_pool + idx);
#define delete_fiber(fiber)	DeleteFiber(fiber);

#define this_job ((Job*)GetFiberData())

#endif // END PLATFORM

NO_INLINE const void *get_this_fiber_fls()
{
	return this_fiber;
}
