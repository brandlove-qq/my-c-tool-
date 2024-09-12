#include "task_worker.h"

zhb::zhb_threadpool::task_worker::task_worker(size_t max_queue_size)
    : m_normal_task_queque(max_queue_size)
    , m_high_task_queque(max_queue_size)
    , m_is_thread_running(true)
    , m_worker_id(ULLONG_MAX)
    , m_has_task_running(false)
{

}

zhb::zhb_threadpool::task_worker::~task_worker()
{
    stop();
}

void zhb::zhb_threadpool::task_worker::run(size_t id)
{
    m_worker_id = id;
    m_thread = std::thread(&task_worker::run_thread, this);
}

void zhb::zhb_threadpool::task_worker::stop()
{
    if(m_is_thread_running.load(std::memory_order_relaxed))
    {
        m_is_thread_running.store(false, std::memory_order_relaxed);
        m_signal.notify_all();
        if(m_thread.joinable())
        {
            m_thread.join(); //等待线程执行完毕
        }

        m_worker_id = ULLONG_MAX;
        m_has_task_running.store(false, std::memory_order_relaxed);
    }

}

void zhb::zhb_threadpool::task_worker::run_thread()
{
    while(is_thread_running())
    {
        //线程挂起，等待信号
        std::unique_lock<std::mutex>    locker(m_mutex_signal);
        m_signal.wait(locker, [this]()
        {
            return (waited_task_count() > 0) || !is_thread_running();
        });

        task_function task;
        //先检测高级别任务,再检测普通级别任务
        bool b = m_high_task_queque.try_dequeue(task) ? true : m_normal_task_queque.try_dequeue(task);

        if(b)
        {
            m_has_task_running.store(true, std::memory_order_relaxed);
            task();
            m_has_task_running.store(false, std::memory_order_relaxed);
        }
    }
}

#if 0
void zhb::zhb_threadpool::task_worker::run_thread()
{
    while(m_is_thread_running.load(std::memory_order_relaxed))
    {
        task_function task;
        //先检测高级别任务,再检测普通级别任务
        bool b = m_high_task_queque.try_dequeue(task) ? true : m_normal_task_queque.try_dequeue(task);

        if(b)
        {
            m_has_task_running.store(true, std::memory_order_relaxed);
            task();
            m_has_task_running.store(false, std::memory_order_relaxed);

            m_waited_task_count.fetch_sub(1, std::memory_order_acq_rel);
//            if(0 == m_waited_task_count.load(std::memory_order_relaxed))
//            {
//                //任务执行清空，释放占位标识
//                m_key.store(invalidKey, std::memory_order_relaxed);
//            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::microseconds(500));
        }

    }
}
#endif
