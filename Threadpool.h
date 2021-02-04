#include <iostream>
#include <deque>
#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <random>

//thread pool
class ThreadPool
{
public:
    ThreadPool(unsigned int n);
    ~ThreadPool();
    void waitFinished();
    unsigned int getProcessed() const;
    // generic function push
    template<typename F, typename...A> void addJob(F&& f, A&&... a) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        auto wrapper = std::bind(&f, a...);
        tasks.emplace_back(wrapper);
        cv_task.notify_one();
    }
private:
    std::vector< std::thread > workers;
    std::deque< std::function<void()> > tasks;
    std::mutex queue_mutex;
    std::condition_variable cv_task;
    std::condition_variable cv_finished;
    std::atomic_uint processed;
    unsigned int busy;
    bool stop;

    void thread_proc();
};
