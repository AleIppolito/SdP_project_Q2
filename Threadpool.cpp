/*
 * New ThreadPool implementation, no futures
 * Also divided in .h and .cpp
 */

#include "Threadpool.h"

ThreadPool::ThreadPool() : busy(), processed(), stop() {
	for(unsigned int i=0; i<std::thread::hardware_concurrency(); ++i)
			workers.emplace_back(std::bind(&ThreadPool::thread_proc, this));
}

ThreadPool::ThreadPool(unsigned int n) : busy(), processed(), stop() {
	for(unsigned int i=0; i<n; ++i)
		workers.emplace_back(std::bind(&ThreadPool::thread_proc, this));
}

ThreadPool::~ThreadPool() {
	std::unique_lock<std::mutex> latch(queue_mutex);
	stop = true;
	cv_task.notify_all();
    latch.unlock();
    for (auto &t : workers)
        t.join();
}

// waits until the queue is empty.
void ThreadPool::waitFinished() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    cv_finished.wait(lock, [this](){return tasks.empty() && (busy == 0);});
}

void ThreadPool::thread_proc() {
    while (true) {
    	std::unique_lock<std::mutex> latch(queue_mutex);
    	cv_task.wait(latch, [this](){return stop || !tasks.empty();});
    	if (!tasks.empty()) {
    		++busy;
            auto fn = tasks.front();
            tasks.pop_front();
            latch.unlock();
            fn();
            ++processed;
            latch.lock();
            --busy;
            cv_finished.notify_one();
        } else if (stop) {
            break;
        }
    }
}

unsigned int ThreadPool::getProcessed() const {return processed;}
