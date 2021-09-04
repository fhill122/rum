//
// A scheduler that supports :
//    - one shot task
//    - repeat task
//    - frontend / threadpool execution
//    - monotonic clock
//
// Created by Ivan B on 2021/8/27.
//

#ifndef IVTB_THREAD_SCHEDULER_H_
#define IVTB_THREAD_SCHEDULER_H_

#include <string>
#include <thread>
#include <chrono>
#include <utility>
#include <set>
#include <queue>

#include "thread_pool.h"

namespace ivtb{

// std::string timeToStr(std::chrono::steady_clock::time_point &t){
//     using namespace std;
//     // time_t t_t = chrono::steady_clock::to_time_t()
// }

double DurationToSeconds(const std::chrono::steady_clock::duration &dur){
    using namespace std::chrono;
    return duration_cast<duration<double>>(dur).count();
}

class Scheduler{
  public:
    struct Task{
        std::string name;  // not used for now
        bool repeat = false;
        double period = 0;  // seconds
        double delay = 0; // seconds
        std::function<void()> action;

        // create a one shot task
        template<class F>
        explicit Task(F&& f, double delay = 0, std::string name="") :
                action(f), delay(delay), name(std::move(name)){}

        // create a repeated task
        template<class F>
        Task(F&& f, double period, double delay, std::string name = "") :
                action(f), period(period), repeat(true), delay(delay), name(std::move(name)){}
    };

    struct Param{
        double tolerance = 0.1;  //seconds
    };

  private:
    struct ScheduledTask{
        std::shared_ptr<Task> task;
        // double last_run_t = 0;
        std::chrono::steady_clock::time_point next_run_t = std::chrono::steady_clock::time_point::min();
        std::chrono::steady_clock::time_point last_run_t = std::chrono::steady_clock::time_point::min();

        ScheduledTask() = default;

        explicit ScheduledTask(std::unique_ptr<Task> task) : task(std::move(task)) {
            using namespace std::chrono;
            next_run_t = steady_clock::now() +
                    duration_cast<steady_clock::duration>(duration<double>{this->task->delay});
        }

        bool operator<( const ScheduledTask& rhs ) const {
            return next_run_t < rhs.next_run_t;
        }

        bool operator>( const ScheduledTask& rhs ) const {
            return next_run_t > rhs.next_run_t;
        }
    };

  private:
    Param param;
    // std::set<ScheduledTask> tasks_;
    std::priority_queue<ScheduledTask, std::vector<ScheduledTask>, std::greater<ScheduledTask>> tasks_;
    std::mutex tasks_mu_;
    std::unique_ptr<std::thread> schedule_t_;
    std::unique_ptr<ThreadPool> task_tp_;
    std::condition_variable cv_;
    bool stop_ = false;
  public:

  private:
    inline void loop();

    // task will be moved in this function
    inline void schedule(ScheduledTask &&task);

    inline void executeTask(ScheduledTask &&task);

  public:
    inline explicit Scheduler(int task_threads = 0);
    inline ~Scheduler();

    inline void schedule(std::unique_ptr<Task> task);
    inline void schedule(Task &&task);
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

void Scheduler::schedule(ScheduledTask &&task) {
    bool need_notify = false;
    {
        std::lock_guard<std::mutex> lock(tasks_mu_);
        if (stop_) return;
        if (tasks_.empty()){
            need_notify = true;
            tasks_.push(std::move(task));
        }
        else{
            auto next_run = tasks_.top().next_run_t;
            tasks_.push(std::move(task));
            if (tasks_.top().next_run_t < next_run)
                need_notify = true;
        }

        // todo ivan. delete
        // auto t = tasks_.top().next_run_t - std::chrono::steady_clock::now();
        // printf("scheduled a task after %f s, need to notify: %d\n",
        //        DurationToSeconds(t), need_notify);
    }
    if (need_notify) cv_.notify_one();
}

void Scheduler::schedule(std::unique_ptr<Task> task) {
    ScheduledTask scheduled_task{std::move(task)};
    schedule(std::move(scheduled_task));
}

void Scheduler::schedule(Task &&task){
    ScheduledTask scheduled_task{std::make_unique<Task>(std::move(task))};
    schedule(std::move(scheduled_task));
}

void Scheduler::executeTask(ScheduledTask &&task) {
    using namespace std::chrono;
    task.last_run_t = steady_clock::now();
    task.task->action();
    if (task.task->repeat){
        task.next_run_t = task.last_run_t +
                duration_cast<steady_clock::duration>(duration<double>{task.task->period});
        schedule(std::move(task));
    }
}

void Scheduler::loop() {
    using namespace std::chrono;

    auto timeForTask = [this]{
        if(tasks_.empty()) return false;
        return steady_clock::now() + duration<double>(param.tolerance) >= tasks_.top().next_run_t;
    };

    NameThread("Scheduler");

    while(true){
        // sleep until next task
        {
            std::unique_lock<std::mutex> lock(tasks_mu_);

            while(!timeForTask() && !stop_){
                if(tasks_.empty()){
                    cv_.wait(lock);
                }
                else{
                    cv_.wait_until(lock, tasks_.top().next_run_t);
                }
            }
            if(stop_) break;

            // todo ivan. delete
            // printf("wait precision: %f s\n",
            //        DurationToSeconds(tasks_.top().next_run_t - std::chrono::steady_clock::now()));
        }

        // make action
        while(true){
            ScheduledTask task;
            {
                std::lock_guard<std::mutex> lock(tasks_mu_);
                if (!timeForTask() || stop_) break;

                task = tasks_.top();
                tasks_.pop();
            }

            // todo ivan. delete
            // printf("execute precision: %f s\n",
            //        DurationToSeconds(task.next_run_t - std::chrono::steady_clock::now()));

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
