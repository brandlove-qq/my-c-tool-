#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include "task_worker.h"
#include "threadpool_errorcode.h"

namespace zhb
{
namespace zhb_threadpool {

//设定无效的占位符
static const size_t invalidKey = 0xffff;

class threadpool_cpp11
{
public:
    /*!
     * \brief threadpool_cpp11 构造函数
     * \param threadNums 线程数量
     */
    explicit threadpool_cpp11(size_t thread_nums = std::thread::hardware_concurrency());

    /*!
     * \brief threadpool_cpp11 析构函数
     */
    ~threadpool_cpp11();

    /*!
     * \brief stop 停止线程池
     */
    void stop();

    /*!
     * \brief set_key 设置占位符
     * \param key 占位符
     * \return 详见threadpool_errorcode
     */
    TP_CODE set_key(size_t key);

    /*!
     * \brief delete_key 删除占位符
     * \param key 占位符
     * \return 详见threadpool_errorcode
     */
    TP_CODE delete_key(size_t key);

    /*!
     * \brief 占位插入, 普通级别任务
     */
    template<class F, class ...Args>
    TP_CODE post_normal_task_with_key(size_t key, F &&f, Args && ...args)
    {
        std::lock_guard<std::mutex> locker(m_mutex_worker);
        auto b = m_key2index.find(key);
        if(m_key2index.end() != b)
        {
            return m_workers[b->second]->post_normal_task_with_key(std::move(f), std::move(args)...) ? \
                    Threadpool_Success : Threadpool_PostTaskFailed;
        }

        return Threadpool_KeyNotExist;
    }

    /*!
     * \brief 占位插入, 高级别任务
     */
    template<class F, class ...Args>
    TP_CODE post_high_task_with_key(size_t key, F &&f, Args &&...args)
    {
        std::lock_guard<std::mutex> locker(m_mutex_worker);
        auto b = m_key2index.find(key);
        if(m_key2index.end() != b)
        {
            return m_workers[b->second]->post_high_task_with_key(std::move(f), std::move(args)...) ? \
                    Threadpool_Success : Threadpool_PostTaskFailed;
        }

        return Threadpool_KeyNotExist;
    }

    /*!
     * \brief post_task 增加任务，优先查找空闲线程
     */
    template<class F, class ...Args>
    TP_CODE post_task(F&& f, Args&& ...args)
    {
        std::lock_guard<std::mutex> locker(m_mutex_worker);

        size_t index = m_worker_size;    ////合适的工作线程索引id
        size_t min_nums = 0xffff;            ////最少的工作线程等待任务数
        //优先检查有没有空闲任务线程
        for(auto &m : m_index2key)
        {
            if(m.second != invalidKey)
            {
                continue;
            }

            //增加筛选条件，防止在极短时间内插入大量任务，导致任务堆积到一个线程中去
            auto idx = m.first;
            auto task_count = m_workers[idx]->waited_task_count();
            if(!m_workers[idx]->has_task_running() && 0 == task_count)
            {
                index = idx;
                break;
            }
            else
            {
                if(min_nums == 0xffff || min_nums > task_count)
                {
                   min_nums = task_count;
                   index = idx;
                }
            }

        }

//        if(m_workers.size() == index)
//        {
//            //扩展线程池
//            m_workers.emplace_back(new task_worker());
//            m_workers[index]->run(index);

//            //更新映射关系
//            m_index2key.emplace(std::make_pair(index, invalidKey));
//        }

        if(m_worker_size == index)
        {
            return Threadpool_PostTaskFailed;
        }

        return m_workers[index]->post_task(std::forward<F>(f), std::forward<Args>(args)...) ? \
                    Threadpool_Success : Threadpool_PostTaskFailed;
    }

private:
    explicit threadpool_cpp11(const threadpool_cpp11&) = delete ;
    threadpool_cpp11& operator=(const threadpool_cpp11&) = delete ;

private:
    std::mutex  m_mutex_worker;
    //线程队列
    std::vector<std::unique_ptr<task_worker>> m_workers;
    size_t m_worker_size{0};
    //key到index的映射
    std::unordered_map<size_t, size_t>    m_key2index;
    //所有index到key的映射
    std::unordered_map<size_t, size_t>    m_index2key;

};

}

using threadpool = zhb_threadpool::threadpool_cpp11;
}

#endif // THREADPOOL_HPP
