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


/*thread_pool_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void thread_pool_type::thread_loop(unsigned int thread_index)
{	
	void (*assigned_jobs[23])(void*, unsigned short);

	void* assigned_args_ptrs[23];
	unsigned short assigned_jobs_indices[23];

	while (true)
	{
		token.lock();
		
		if (wakeup == true)
		{
			if (job_stack.stack_top != 0)
			{
				unsigned short jobs;

				if (job_stack.stack_top < jobs_per_iteration)
				{
					jobs = job_stack.stack_top;
				}
				else
				{
					jobs = jobs_per_iteration;
				}

				for (unsigned short a = 0; a < jobs; ++a)
				{
					assigned_jobs[a] = job_stack.jobs[job_stack.stack_top - 1];
					assigned_args_ptrs[a] = job_stack.arg_ptrs[job_stack.stack_top - 1];
					assigned_jobs_indices[a] = job_stack.job_indices[job_stack.stack_top - 1];
					job_stack.pop();
				}

				token.unlock();

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
				if (terminate == true)
				{
					token.unlock();

					return;
				}
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

			std::this_thread::sleep_for(std::chrono::nanoseconds(100000));
		}
	}
}


thread_pool_type::thread_pool_type()
{
	intern_thread_count = std::thread::hardware_concurrency() - 1;

	/*Is for debugging*/
	/*intern_thread_count = 1u;*/

	threads = new std::thread[intern_thread_count];

	for (unsigned int a = 0; a < intern_thread_count; ++a)
	{
		threads[a] = std::thread(&thread_pool_type::thread_loop, this, a);
	}
}


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


bool thread_pool_type::add_jobs(void(**jobs)(void*, unsigned short), void* arg_ptr)
{
	token.lock();

	unsigned short jobs_added = 0;

	if ((job_stack.stack_top + thread_count) <= max_job_amount)
	{
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
		token.unlock();

		return false;
	}
}


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


bool thread_pool_type::job_stack_type::push(void(*job)(void*, unsigned short), void* arg_ptr, const unsigned short& job_index)
{
	if (stack_top < 65535)
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