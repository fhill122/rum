//
// A scheduler that supports :
//    - one shot task
//    - repeat task
//    - frontend / threadpool execution
//    - monotonic clock
//
// Note: this implementation does not aim to provide high time precision.
//
// todo ivan.
//    - support repeat n times
// Created by Ivan B on 2021/8/27.
//

#ifndef IVTB_THREAD_SCHEDULER_H_
#define IVTB_THREAD_SCHEDULER_H_

#include <string>
#include <thread>
#include <chrono>
#include <queue>
#include <unordered_set>

#include "thread_pool.h"

namespace ivtb{

double DurationToSeconds(const std::chrono::steady_clock::duration &dur){
    using namespace std::chrono;
    return duration_cast<duration<double>>(dur).count();
}

class Scheduler{
  public:
    struct Task{
        const std::string name;  // not used for now
        const bool repeat = false;
        const double period = 0;  // seconds
        const double delay = 0; // seconds
        const std::function<void()> action;

        // create a one shot task
        template<class F>
        explicit Task(F&& f, double delay = 0, std::string name="") :
                action(f), delay(delay), name(std::move(name)){}

        // create a repeated task
        template<class F>
        Task(F&& f, double period, double delay, std::string name = "") :
                action(f), period(period), repeat(true), delay(delay), name(std::move(name)){}

      private:
        friend Scheduler;
        bool cancelled = false;
    };

    struct Param{
        double tolerance = 0.1;  //seconds
    };

  private:
    struct ScheduledTask{
        unsigned long id = 0;

        std::shared_ptr<Task> task;
        // double last_run_t = 0;
        std::chrono::steady_clock::time_point next_run_t = std::chrono::steady_clock::time_point::min();
        std::chrono::steady_clock::time_point last_run_t = std::chrono::steady_clock::time_point::min();

        ScheduledTask() = default;

        explicit ScheduledTask(std::shared_ptr<Task> task) : task(std::move(task)), id(genId()) {
            using namespace std::chrono;
            next_run_t = steady_clock::now() +
                    duration_cast<steady_clock::duration>(duration<double>{this->task->delay});
        }

        ScheduledTask& operator=(const ScheduledTask&) = delete;
        ScheduledTask(const ScheduledTask&) = delete;
        ScheduledTask& operator=(ScheduledTask&&) = default;
        ScheduledTask(ScheduledTask&&) = default;
        ~ScheduledTask() = default;

        bool operator<( const ScheduledTask& rhs ) const {
            return next_run_t < rhs.next_run_t;
        }

        bool operator>( const ScheduledTask& rhs ) const {
            return next_run_t > rhs.next_run_t;
        }

        static unsigned long genId(){
            static std::atomic<unsigned long> id_pool{1};
            return id_pool.fetch_add(1, std::__1::memory_order_relaxed);
        }
    };

  private:
    Param param;
    std::priority_queue<ScheduledTask, std::vector<ScheduledTask>, std::greater<ScheduledTask>> task_q_;
    // all scheduled future tasks that will be executed
    std::unordered_set<std::shared_ptr<Task>> tasks_;
    std::mutex tasks_mu_;
    std::unique_ptr<std::thread> schedule_t_;
    std::unique_ptr<ThreadPool> task_tp_;
    std::condition_variable cv_;
    bool stop_ = false;
  public:

  private:
    inline void loop();

    inline void internalSchedule(ScheduledTask &&task);

    inline void executeTask(ScheduledTask &&task);

  public:
    inline explicit Scheduler(int task_threads = 0);
    inline ~Scheduler();

    /**
     * Schedule a task
     * @param task Task to be scheduled
     */
    inline void schedule(std::shared_ptr<Task> task);

    /**
     * Schedule a task.
     * Note: this version of schedule does not support task cancelling
     * @param task
     */
    inline void schedule(Task &&task);

    /**
     * Cancel a task if it is possible
     * @param task Task to be cancelled
     * @return true if cancelled, false if already executed or cancelled before
     */
    inline bool cancel(const std::shared_ptr<Task> &task);

    /**
     * Manually stop the scheduler, after which remained tasks will never be executed.
     * Note: Destructor do this automatically.
     */
    inline void stop();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

Scheduler::Scheduler(int task_threads) {
    if (task_threads>0) task_tp_ = std::make_unique<ThreadPool>(task_threads, "SchedulerTp");
    schedule_t_ = std::make_unique<std::thread>([this]{loop();});
}

Scheduler::~Scheduler() {
    stop();
}

void Scheduler::internalSchedule(ScheduledTask &&task) {
    auto addTask = [this](ScheduledTask &&task){
        tasks_.emplace(task.task);
        task_q_.push(std::move(task));
    };

    bool need_notify = false;
    {
        std::lock_guard<std::mutex> lock(tasks_mu_);
        if (stop_) return;
        if (task_q_.empty()){
            need_notify = true;
            addTask(std::move(task));
        }
        else{
            auto next_run = task_q_.top().next_run_t;
            addTask(std::move(task));
            if (task_q_.top().next_run_t < next_run)
                need_notify = true;
        }
    }
    if (need_notify) cv_.notify_one();
}

void Scheduler::schedule(std::shared_ptr<Task> task) {
    ScheduledTask scheduled_task{std::move(task)};
    internalSchedule(std::move(scheduled_task));
}

void Scheduler::schedule(Task &&task){
    ScheduledTask scheduled_task{std::make_unique<Task>(std::move(task))};
    internalSchedule(std::move(scheduled_task));
}

bool Scheduler::cancel(const std::shared_ptr<Task> &task){
    std::lock_guard<std::mutex> lock(tasks_mu_);
    auto itr = tasks_.find(task);
    if (itr == tasks_.end()){
        return false;
    }

    task->cancelled = true;
    tasks_.erase(itr);
    return true;
}

void Scheduler::executeTask(ScheduledTask &&task) {
    using namespace std::chrono;
    {
        std::lock_guard<std::mutex> lock(tasks_mu_);
        if (!task.task->repeat) tasks_.erase(task.task);
        if (task.task->cancelled) return;
    }

    task.last_run_t = steady_clock::now();
    task.task->action();
    if (task.task->repeat){
        task.next_run_t = task.last_run_t +
                duration_cast<steady_clock::duration>(duration<double>{task.task->period});
        internalSchedule(std::move(task));
    }
}

void Scheduler::loop() {
    using namespace std::chrono;

    auto timeForTask = [this]{
        if(task_q_.empty()) return false;
        return steady_clock::now() + duration<double>(param.tolerance) >= task_q_.top().next_run_t;
    };

    NameThread("Scheduler");

    while(true){
        // sleep until next task
        {
            std::unique_lock<std::mutex> lock(tasks_mu_);

            while(!timeForTask() && !stop_){
                if(task_q_.empty()){
                    cv_.wait(lock);
                }
                else{
                    cv_.wait_until(lock, task_q_.top().next_run_t);
                }
            }
            if(stop_) break;
        }

        // make action
        while(true){
            ScheduledTask task;
            {
                std::lock_guard<std::mutex> lock(tasks_mu_);
                if (!timeForTask() || stop_) break;

                // a hacky way to move, as top() returns a const ref
                task = std::move(const_cast<ScheduledTask&>(task_q_.top()));
                task_q_.pop();
                if (task.task->cancelled) continue;
            }

            if (task_tp_){
                task_tp_->enqueue([this, task = std::move(task)]() mutable {executeTask(std::move(task));});
            }
            else{
                executeTask(std::move(task));
            }
        }
    }
}

void Scheduler::stop() {
    {
        std::lock_guard<std::mutex> lock(tasks_mu_);
        if (stop_) return;
        stop_=true;
        cv_.notify_one();
    }
    schedule_t_->join();
    if(task_tp_) task_tp_->stopAndClear();
}

}

#endif //IVTB_THREAD_SCHEDULER_H_
