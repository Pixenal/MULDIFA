/*
 -------------------------------------------------------------------------------------------------------------
	Copyright(C) 2021 Pixenal


	This file is part of blend_df_tool.

	blend_df_tool is free software : you can redistribute it and /or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	blend_df_tool is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with blend_df_tool.If not, see < https://www.gnu.org/licenses/>.
	-------------------------------------------------------------------------------------------------------------
*/


#include "../df_common/thread_pool.h"
#include <chrono>
#include <limits>


/*thread_pool_type*/
/*-------------------------------------------------------------------------------------------------------------*/


/*	This function contains the 'main' loop of run by each thread	*/
void thread_pool_type::thread_loop(unsigned int thread_index)
{	
	void (*assigned_jobs[23])(void*, unsigned short);

	void* assigned_args_ptrs[23];
	unsigned short assigned_jobs_indices[23];

	/*	'main' loop	*/
	while (true)
	{
		token.lock();
		
		/*	"thread_pool_type" contains a constant flag used to notifying threads when
			to start checking for jobs	*/
		if (wakeup == true)
		{
			if (job_stack.stack_top != 0)
			{
				/*	"jobs" referes to the number of jobs to take from the job stack	*/
				unsigned short jobs;
				if (job_stack.stack_top < jobs_per_iteration)
				{
					/* 	If the number of jobs in the job stack is less than jobs per iteraton,
						set amount of jobs to take equal the amount of jobs in the stack	*/
					jobs = job_stack.stack_top;
				}
				else
				{

					/*	Else set equal to the full amount defined in "jobs_per_iteration"	*/
					jobs = jobs_per_iteration;
				}

				/*	Take jobs from job stack	*/
				for (unsigned short a = 0; a < jobs; ++a)
				{
					assigned_jobs[a] = job_stack.jobs[job_stack.stack_top - 1];
					assigned_args_ptrs[a] = job_stack.arg_ptrs[job_stack.stack_top - 1];
					assigned_jobs_indices[a] = job_stack.job_indices[job_stack.stack_top - 1];
					job_stack.pop();
				}

				token.unlock();

				/*	Execute jobs */
				for (unsigned short a = 0; a < jobs; ++a)
				{
					if (assigned_jobs[a] != nullptr)
					{
						assigned_jobs[a](assigned_args_ptrs[a], assigned_jobs_indices[a]);
					}
				}
			}
			else
			{
				/*	If no jobs in job stack, check if terminal flag is true	*/
				if (terminate == true)
				{
					token.unlock();

					return;
				}
				/*	If not, set wakup to false (last one out closes the door)	*/
				else
				{
					wakeup = false;

					token.unlock();
				}
			}
		}
		else
		{
			token.unlock();

			/* Prevents the thread from hogging cpu while sleeping */
			std::this_thread::sleep_for(std::chrono::nanoseconds(100000));
		}
	}
}

/*	Constructor */
thread_pool_type::thread_pool_type()
{
	intern_thread_count = std::thread::hardware_concurrency() - 1;

	/*	Is for debugging, uncomment to run single threaded	*/
	/*	intern_thread_count = 1u;	*/

	/* Dynamically create array in which to store threads */
	threads = new std::thread[intern_thread_count];

	/*	Create individual threads	*/
	for (unsigned int a = 0; a < intern_thread_count; ++a)
	{
		threads[a] = std::thread(&thread_pool_type::thread_loop, this, a);
	}
}


/*	Destructor	*/
thread_pool_type::~thread_pool_type()
{
	wakeup = true;
	terminate = true;

	for (unsigned int a = 0; a < intern_thread_count; ++a)
	{
		threads[a].join();
	}

	delete[] threads;
}


/*	Adds jobs to the job stack */
bool thread_pool_type::add_jobs(void(**jobs)(void*, unsigned short), void* arg_ptr)
{
	token.lock();

	unsigned short jobs_added = 0;

	/*	Checks if the amount of jobs to be added can fit within the job stack
		(the amount added is equal to the number of threads (legacy reasons))	*/
	if ((job_stack.stack_top + thread_count) <= max_job_amount)
	{
		/*	If so, add jobs, wakeup threads, and return true */
		for (unsigned short a = 0; a < thread_count; ++a)
		{
			job_stack.push(jobs[a], arg_ptr, a);
		}

		wakeup = true;

		token.unlock();

		return true;
	}
	else
	{
		/*	If not, don't add any and return false */
		token.unlock();

		return false;
	}
}


/*	Sets the number of jobs each thread takes from the job stack at a time	*/
int thread_pool_type::set_jobs_per_iteration(const unsigned short amount)
{
	if (amount <= max_jobs_per_itereation)
	{
		jobs_per_iteration = amount;
		return 0;
	}
	else
	{
		return 1;
	}
}


/*thread_pool_type::job_stack_type*/
/*-------------------------------------------------------------------------------------------------------------*/

/* Pushes the specified job onto the job stack */
bool thread_pool_type::job_stack_type::push(void(*job)(void*, unsigned short), void* arg_ptr, const unsigned short& job_index)
{
	if (stack_top < (std::numeric_limits<unsigned short>::max)())
	{
		jobs[stack_top] = job;
		arg_ptrs[stack_top] = arg_ptr;
		job_indices[stack_top] = job_index;
		stack_top += 1;

		return true;
	}
	else
	{
		return false;
	}

}


/*	Pops the last job from the job stack */
void thread_pool_type::job_stack_type::pop()
{
	if (stack_top != 0)
	{
		stack_top -= 1;
		jobs[stack_top] = nullptr;
		arg_ptrs[stack_top] = nullptr;
		job_indices[stack_top] = 0;

	}
}