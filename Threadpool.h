/*
 * @file ThreadPool.h
 * @authors Alessando Ippolito, Federico Maresca
 * @brief
 * @version 1
 * @date 2021-02-07
 *
 * 
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <iostream>
#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <chrono>
#include <climits>
#include <random>
#include <regex>
/**
 * @brief Choose whether to use Bidirectional Reach or Basic Reach
 * 
 */
#define DEBUG false

/**
 * @brief Do the files contain a ground truth? 
 */
#define GROUND_TRUTH false

/**
 * @brief This constant is used in the program to chunkify various operations
 * so as to maximise usage of each thread 
 */
#define CHUNK_N std::thread::hardware_concurrency()

using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;

class ThreadPool {
	public:
		ThreadPool();
		ThreadPool(const unsigned);
		~ThreadPool();
		void waitFinished();
		template<typename F, typename ...A> void addJob(F&& f, A&&... a) {	// generic push function
			std::unique_lock<std::mutex> lock(queue_mutex);
			tasks.emplace([=]{f(a...);} );
			cv_task.notify_one();
		}
	private:
		std::vector<std::thread> workers;
		std::queue<std::function<void()>> tasks;
		std::mutex queue_mutex;
		std::condition_variable cv_task, cv_finished;
		unsigned busy;
		bool stop;
    	void thread_proc();
};

#endif /* _THREADPOOL_H */
