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
			try{
				std::unique_lock<std::mutex> lock(queue_mutex);
				auto wrapper = std::bind(&f, a...);
				tasks.emplace_back(wrapper);
				cv_task.notify_one();
			}
			catch(...) {
				std::cout << "Errore during function submit" << std::endl;
			}
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

  ThreadPool & operator=(const ThreadPool &) = delete;
  ThreadPool & operator=(ThreadPool &&) = delete;

  // Inits thread pool
  void init() {
    for (int i = 0; i < m_threads.size(); ++i) {
      m_threads[i] = std::thread(ThreadWorker(this, i));
    }
  }

  // Waits until threads finish their current task and shutdowns the pool
  void shutdown() {
    m_shutdown = true;
    m_conditional_lock.notify_all();

    for (int i = 0; i < m_threads.size(); ++i) {
      if(m_threads[i].joinable()) {
        m_threads[i].join();
      }
    }
  }

  // Submit a function to be executed asynchronously by the pool
  template<typename F, typename...Args>
  auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
    // Create a function with bounded parameters ready to execute
    std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    // Encapsulate it into a shared ptr in order to be able to copy construct / assign
    auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

    // Wrap packaged task into void function
    std::function<void()> wrapper_func = [task_ptr]() {
      (*task_ptr)();
    };

    // Enqueue generic wrapper function
    m_queue.enqueue(wrapper_func);

    // Wake up one thread if its waiting
    m_conditional_lock.notify_one();

    // Return future from promise
    return task_ptr->get_future();
  }
};

#endif // _THREADPOOL_H
