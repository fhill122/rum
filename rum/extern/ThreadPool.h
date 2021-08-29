/*
 * modified from https://github.com/progschj/ThreadPool
 */

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

#include "ThreadUtil.h"

class ThreadPool {
  public:
    inline ThreadPool(): id(GenId()){};

    inline explicit ThreadPool(size_t threads);

    ~ThreadPool();

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

  protected:
    inline unsigned int GenId(){
        static std::atomic<unsigned int> id_pool{0};
        return id_pool.fetch_add(1, std::memory_order_relaxed);
    }

    const unsigned int id;

    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;
    // the task queue
    std::queue< std::function<void()> > tasks;

    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = true;
};

inline ThreadPool::ThreadPool(size_t threads): id(GenId()){
    start(threads);
}

void ThreadPool::start(size_t threads){
    std::lock_guard<std::mutex> lock(queue_mutex);

    if(!stop) throw std::runtime_error("ThreadPool already started");
    stop = false;

    workers.reserve(threads);
    for(size_t i = 0;i<threads;++i) {
        workers.emplace_back(
                [this,i] {
                    tu::NameThread("tp"+std::to_string(id) + "_t"+std::to_string(i));
                    for (;;) {
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock(queue_mutex);
                            this->condition.wait(lock, [this] { return stop || !tasks.empty(); });
                            if (stop && tasks.empty())
                                return;
                            task = std::move(tasks.front());
                            tasks.pop();
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
        std::lock_guard<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task](){ (*task)(); });
    }
    // this is thread safe
    condition.notify_one();
    return res;
}

int ThreadPool::threads() {
    return workers.size();
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool(){
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    // gil release here?
    for(std::thread &worker: workers)
        worker.join();
}

#endif