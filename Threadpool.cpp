/*
 * @file ThreadPool.cpp
 * @authors Alessando Ippolito, Federico Maresca
 * @brief
 * @version 1
 * @date 2021-02-07
 *
 * @copyright Copyright (c) 2021
 */

#include "Threadpool.h"

ThreadPool::ThreadPool() : busy(), processed(), stop() {
	for(unsigned int i=0; i<std::thread::hardware_concurrency(); ++i)
		workers.emplace_back([&, this] {thread_proc();});
}

ThreadPool::ThreadPool(const unsigned n) : busy(), processed(), stop() {
	for(unsigned int i=0; i<n; ++i)
		workers.emplace_back([&, this] {thread_proc();});
}

ThreadPool::~ThreadPool() {
	std::unique_lock<std::mutex> lock(queue_mutex);
	stop = true;
	cv_task.notify_all();
    lock.unlock();
    for (auto &t : workers)
        t.join();
}

// waits until the queue is empty.
void ThreadPool::waitFinished() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    cv_finished.wait(lock, [this](){return tasks.empty() && (busy == 0);});
}

void ThreadPool::thread_proc() {
    while(true) {
    	std::unique_lock<std::mutex> lock(queue_mutex);
    	cv_task.wait(lock, [this](){return stop || !tasks.empty();});
    	if (!tasks.empty()) {
    		++busy;
            auto fn = tasks.front();
            tasks.pop();
            lock.unlock();
            fn();
            ++processed;
            lock.lock();
            --busy;
            cv_finished.notify_one();
        } else if (stop) {
            break;
        }
    }
}

unsigned int ThreadPool::getProcessed() const {return processed;}
