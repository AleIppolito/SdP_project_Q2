#include "Threadpool.h"

ThreadPool::ThreadPool(unsigned int n) : busy(), processed(), stop() {
	for (unsigned int i=0; i<n; ++i)
		workers.emplace_back(std::bind(&ThreadPool::thread_proc, this));
}

ThreadPool::~ThreadPool() {
	// set stop-condition
	std::unique_lock<std::mutex> latch(queue_mutex);
	stop = true;
	cv_task.notify_all();
    latch.unlock();

    // all threads terminate, then we're done.
    for (auto &t : workers)
        t.join();
}

// waits until the queue is empty.
void ThreadPool::waitFinished() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    cv_finished.wait(lock, [this](){return tasks.empty() && (busy == 0);});
}

unsigned int ThreadPool::getProcessed() const {
	return processed;
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