/*
 * modified from https://github.com/progschj/ThreadPool
 */

#ifndef IVTB_THREAD_THREAD_POOL_H
#define IVTB_THREAD_THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

#include "thread_util.h"

namespace ivtb{

class ThreadPool {
  public:
    inline explicit ThreadPool(std::string name = "");

    inline explicit ThreadPool(size_t threads, std::string name = "");

    inline ~ThreadPool();

    inline void start(size_t);

    /**
     * Enqueue a task. this is thread safe
     * @tparam F Function type
     * @tparam Args Arg types
     * @param f function
     * @param args arguments
     * @return
     */
    template<class F, class... Args>
    std::future<typename std::result_of<F(Args...)>::type> enqueue(F&& f, Args&&... args);

    inline int threads();

    inline void stop();

    inline void stopAndClear();

  protected:
    static inline unsigned int GenId(){
        static std::atomic<unsigned int> id_pool{0};
        return id_pool.fetch_add(1, std::memory_order_relaxed);
    }

    static inline std::string GenName(const std::string &input);

  public:
    const std::string name_;

  private:
    // const unsigned int id_;

    // need to keep track of threads so we can join them
    std::vector< std::thread > workers_;
    // the task queue
    std::queue< std::function<void()> > tasks_;

    // synchronization
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_ = true;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(std::string name): name_(std::move(GenName(name))) {}

ThreadPool::ThreadPool(size_t threads, std::string name): name_(std::move(GenName(name))) {
    start(threads);
}

std::string ThreadPool::GenName(const std::string &input){
    static std::atomic<unsigned int> id_pool{0};
    if (input.empty()){
        return "TP" + std::to_string(id_pool.fetch_add(1, std::memory_order_relaxed));
    }
    else{
        return input;
    }
}

void ThreadPool::start(size_t threads){
    std::lock_guard<std::mutex> lock(queue_mutex_);

    if(!stop_) throw std::runtime_error("ThreadPool already started");
    stop_ = false;

    workers_.reserve(threads);
    for(size_t i = 0;i<threads;++i) {
        workers_.emplace_back(
            [this,i] {
                NameThread(name_ + "_t"+std::to_string(i));
                for (;;) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        this->condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                        if (stop_ && tasks_.empty())
                            return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }

                    task();
                }
            }
        );
    }
}

// add new work item to the pool
template<class F, class... Args>
std::future<typename std::result_of<F(Args...)>::type> ThreadPool::enqueue(F&& f, Args&&... args){
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

    std::future<return_type> res = task->get_future();
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);

        // don't allow enqueueing after stopping the pool
        if(stop_)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks_.emplace([task](){ (*task)(); });
    }
    // this is thread safe
    condition_.notify_one();
    return res;
}

int ThreadPool::threads() {
    return workers_.size();
}

void ThreadPool::stop() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if(stop_)
            return;
        stop_ = true;
    }
    condition_.notify_all();
    for(std::thread &worker: workers_)
        worker.join();
}

void ThreadPool::stopAndClear() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if(stop_)
            return;
        stop_ = true;
        tasks_ = {};
    }
    condition_.notify_all();
    for(std::thread &worker: workers_)
        worker.join();
}

// the destructor joins all threads
ThreadPool::~ThreadPool(){
    stop();
}

}
#endif