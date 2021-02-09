/*
 * New ThreadPool implementation, no futures
 * Also divided in .h and .cpp
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

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
#include <string>
#include <algorithm>
#include <chrono>

#define THREADS true
#define DEBUG true
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
class ThreadPool {
	public:
		ThreadPool();
		ThreadPool(unsigned int n);
		~ThreadPool();
		void waitFinished();
		unsigned int getProcessed() const;
		template<typename F, typename...A> void addJob(F&& f, A&&... a) {	// generic push function
			std::unique_lock<std::mutex> lock(queue_mutex);
			tasks.emplace_back(std::bind(&f, a...));
			cv_task.notify_one();
		}
	private:
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

#endif /* _THREADPOOL_H */
