
#pragma once

#include <condition_variable>
#include <functional>
#include <thread>
#include <mutex>
#include <queue>

namespace example
{
    class thread_pool 
    {
        using mutex_t = std::mutex;
        using vec_t   = std::vector<std::thread>;
        using task_t  = std::function<void()>;
        using tasks_t = std::queue<task_t>;
        using cond_t  = std::condition_variable;
    public:
        thread_pool(const size_t threads = std::thread::hardware_concurrency()) 
            : m_mutex()
            , m_condition()
            , m_workers()
            , m_tasks()
            , m_stop()
        {
            auto worker = [this] 
            {
                const auto check = [this] 
                { 
                    return this->m_stop || !this->m_tasks.empty(); 
                };

                for(;;) 
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> 
                            lock(this->m_mutex);
                        this->m_condition.wait(lock, check);
                        if(this->m_stop && this->m_tasks.empty())
                            return;
                        task = std::move(this->m_tasks.front());
                        this->m_tasks.pop();
                    }
                    task();
                }
            };

            for(size_t i = 0; i < threads; ++i)
                this->m_workers.emplace_back(std::move(worker));
        }
        
        template<class F> void add(F&& f) 
        {
            {
                const std::unique_lock<std::mutex> 
                    lock(this->m_mutex);
                this->m_tasks.emplace(std::forward<F>(f));
            }
            this->m_condition.notify_one();
        }
    
        ~thread_pool() 
        {
            {
                const std::unique_lock<std::mutex> 
                    lock(this->m_mutex);
                this->m_stop = true;
            }
            this->m_condition.notify_all();
            for(std::thread& worker: this->m_workers)
                worker.join();
        }
    private:
        mutex_t m_mutex;
        cond_t m_condition;
        vec_t m_workers;
        tasks_t m_tasks;
        bool m_stop;
    };

} // namespace example
