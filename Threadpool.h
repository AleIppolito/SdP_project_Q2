/*
 * New ThreadPool implementation, no futures
 * Also divided in .h and .cpp
 */

#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <iostream>
#include <deque>
#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <fstream>
#include <vector>
#include <queue>
#include <sys/time.h>
#include <algorithm>

class ThreadPool {
	public:
		ThreadPool(unsigned int n);
		~ThreadPool();
		void waitFinished();
		unsigned int getProcessed() const;
		template<typename F, typename...A> void addJob(F&& f, A&&... a) {	// generic push function
			std::unique_lock<std::mutex> lock(queue_mutex);
			auto wrapper = std::bind(&f, a...);
			tasks.emplace_back(wrapper);
			cv_task.notify_one();
		}
	private:
		friend class Grail;
		friend class Graph;
		std::vector<std::thread> workers;
		std::deque<std::function<void()>> tasks;
		std::mutex queue_mutex;
		std::condition_variable cv_task;
		std::condition_variable cv_finished;
		std::atomic_uint processed;
		unsigned int busy;
		bool stop;
    	void thread_proc();
};

#endif // _THREADPOOL_H
