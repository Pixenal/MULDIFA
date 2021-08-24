#pragma once

#include <mutex>
#include <thread>
#include <iostream>

/*Thread pool class definition*/
/*-------------------------------------------------------------------------------------------------------------*/

class thread_pool_type
{
private:

	/*Types*/

	struct job_stack_type
	{
		/*Data Members*/

		void(*jobs[65535])(void*, unsigned short) = {};
		void* arg_ptrs[65535] = {};
		unsigned short job_indices[65535] = {};
		unsigned short stack_top = 0;

		/*Member Functions*/

		bool push(void(*job)(void*, unsigned short), void* arg_ptr, const unsigned short& job_index);
		void pop();

	};

	/*Data Members*/

	unsigned int intern_thread_count;
	std::thread* threads;
	bool wakeup = false;
	std::mutex token;
	job_stack_type job_stack;
	bool terminate = false;
	unsigned short jobs_per_iteration = 1u;

	/*Member Functions*/

	void thread_loop(unsigned int thread_num);

public:

	/*Data Members*/

	const unsigned short max_jobs_per_itereation = 23u;;
	const unsigned int& thread_count = intern_thread_count;
	const unsigned short max_job_amount = 65535;
	const unsigned short& job_amount = job_stack.stack_top;
	

	/*Member Functions*/

	thread_pool_type();
	~thread_pool_type();
	bool add_jobs(void(**jobs)(void*, unsigned short), void* tri_local_state);
	int set_jobs_per_iteration(const unsigned short amount);
};