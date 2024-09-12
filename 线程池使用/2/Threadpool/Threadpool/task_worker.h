#ifndef TASKWORK_HPP
#define TASKWORK_HPP
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <utility>
#include <climits>
#include <future>
#include <string>
#include <condition_variable>
#include "concurrentqueue_with_safecount.h"
#include "threadpool_errorcode.h"

namespace zhb {

namespace zhb_threadpool
{
using  task_function = std::function<void()>;

#define DEFAULT_QUEEUE_CAPACITY 512

class task_worker
{
public:
    /*!
     * \brief task_worker task_worker
     * \param max_queue_size 默认任务列表大小
     */
    explicit task_worker(size_t max_queue_size = DEFAULT_QUEEUE_CAPACITY);

    ~task_worker();

public:
    /*!
     * \brief run 启动当前任务
     * \param id    任务ID
     */
    void run(size_t id);

    /*!
     * \brief stop
     */
    void stop();

    //任务的优先级别划分
    enum task_priority
    {
        PRIORITY_HIGH,  //高级别
        PRIORITY_NORMAL, //普通级别
    };

    /*!
     * \brief 占位插入,普通级别任务
     */
    template<class F, class ...Args>
    bool post_normal_task_with_key(F &&f, Args &&...args)
    {
        return inner_post(task_priority::PRIORITY_NORMAL, std::move(f), std::move(args)...);
    }

    /*!
     * \brief 占位插入,高级别任务
     */
    template<class F, class ...Args>
    bool post_high_task_with_key(F &&f, Args &&...args)
    {
        return inner_post(task_priority::PRIORITY_HIGH, std::move(f), std::move(args)...);
    }

    /*!
     * \brief post_task 插入普通级别任务
     * \param task (右值引用的方式)
     */
    template<class F, class ... Args>
    bool post_task(F &&f, Args&&... args)
    {
        return inner_post(task_priority::PRIORITY_NORMAL, std::move(f), std::move(args)...);
    }

#if 0
    /*!
     * \brief post_normal_task 插入普通级别任务
     * \param task (右值引用的方式)
     */
    template<class F, class ... Args>
    bool post_normal_task(F &&f, Args&&... args)
    {
        return inner_post(task_priority::PRIORITY_NORMAL, std::move(f), std::move(args)...);
    }


    /*!
     * \brief post_high_task 插入高级别任务
     * \param task (右值引用的方式)
     */
    template<class F, class ... Args>
    bool post_high_task(F &&f, Args&&... args)
    {
        return inner_post(task_priority::PRIORITY_HIGH, std::move(f), std::move(args)...);
    }
#endif

    /*!
     * \brief waited_task_count 任务队列中的任务数量
     * \return 待执行任务数量
     */
    size_t waited_task_count()
    {
        auto count = m_normal_task_queque.size_approx() + m_high_task_queque.size_approx();
        return count;
    }

    /*!
     * \brief has_task_running 当前线程是否有任务正在运行
     * \return
     */
    bool has_task_running()
    {
        return (m_has_task_running.load(std::memory_order_relaxed)) && \
                (m_is_thread_running.load(std::memory_order_relaxed));
    }

    /*!
     * \brief is_thread_running 当前线程是否正常运行
     * \return
     */
    bool is_thread_running()
    {
        return m_is_thread_running.load(std::memory_order_relaxed);
    }

public:
    explicit task_worker(const task_worker&) = delete;
    task_worker& operator=(const task_worker &) = delete;

private:
    /*!
     * \brief run_thread 线程函数
     */
    void run_thread();

    /*!
     * \brief tryPost 插入任务,不能插入有占位符的
     * \param task (右值引用的方式)
     */
    template<class F, class ... Args>
    bool inner_post(task_priority tp, F &&f, Args&&... args)
    {
        using ttype = decltype (f(args...));
        auto task = std::make_shared<std::packaged_task<ttype()>>(std::bind(std::forward<F>(f), \
                                                                            std::forward<Args>(args)...));
        //添加任务
        auto post_task = [task]()->void
        {
            (*task)();
        };

        bool b{true};
        if(task_priority::PRIORITY_HIGH == tp)
        {
            //b = m_high_task_queque.try_enqueue(std::forward<task_function>(post_task));
            b = m_high_task_queque.enqueue(std::forward<task_function>(post_task));
        }
        else if(task_priority::PRIORITY_NORMAL == tp)
        {
            //b = m_normal_task_queque.try_enqueue(std::forward<task_function>(post_task));
            b = m_normal_task_queque.enqueue(std::forward<task_function>(post_task));
        }
        else
        {
            b = false;
        }

        if(b)
        {           
            m_signal.notify_one();
        }

        return b;
    }

private:
    using task_queque = zhb::ConcurrentQueueSafeCount<task_function>;
    //任务列表，存放普通优先级别的任务
    task_queque             m_normal_task_queque{DEFAULT_QUEEUE_CAPACITY};

    //任务列表，存放高级别优先级别的任务
    task_queque             m_high_task_queque{DEFAULT_QUEEUE_CAPACITY};

    //线程运行标识
    std::atomic<bool>       m_is_thread_running{true};

    //线程
    std::thread             m_thread;

    //当前任务线程索引id
    size_t                  m_worker_id{ULLONG_MAX};

    //是否有任务正在运行的标识
    std::atomic<bool>       m_has_task_running{false};

    //信号量，阻塞/唤醒线程
    std::condition_variable m_signal;
    std::mutex              m_mutex_signal;
};

}
}

#endif // TASKWORK_HPP
